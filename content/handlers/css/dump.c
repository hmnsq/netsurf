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

	নির্বাচন (unit) {
	ক্ষেত্র CSS_UNIT_PX:
		fprintf(stream, "px");
		বিরতি;
	ক্ষেত্র CSS_UNIT_EX:
		fprintf(stream, "ex");
		বিরতি;
	ক্ষেত্র CSS_UNIT_EM:
		fprintf(stream, "em");
		বিরতি;
	ক্ষেত্র CSS_UNIT_IN:
		fprintf(stream, "in");
		বিরতি;
	ক্ষেত্র CSS_UNIT_CM:
		fprintf(stream, "cm");
		বিরতি;
	ক্ষেত্র CSS_UNIT_MM:
		fprintf(stream, "mm");
		বিরতি;
	ক্ষেত্র CSS_UNIT_PT:
		fprintf(stream, "pt");
		বিরতি;
	ক্ষেত্র CSS_UNIT_PC:
		fprintf(stream, "pc");
		বিরতি;
	ক্ষেত্র CSS_UNIT_PCT:
		fprintf(stream, "%%");
		বিরতি;
	ক্ষেত্র CSS_UNIT_DEG:
		fprintf(stream, "deg");
		বিরতি;
	ক্ষেত্র CSS_UNIT_GRAD:
		fprintf(stream, "grad");
		বিরতি;
	ক্ষেত্র CSS_UNIT_RAD:
		fprintf(stream, "rad");
		বিরতি;
	ক্ষেত্র CSS_UNIT_MS:
		fprintf(stream, "ms");
		বিরতি;
	ক্ষেত্র CSS_UNIT_S:
		fprintf(stream, "s");
		বিরতি;
	ক্ষেত্র CSS_UNIT_HZ:
		fprintf(stream, "Hz");
		বিরতি;
	ক্ষেত্র CSS_UNIT_KHZ:
		fprintf(stream, "kHz");
		বিরতি;
	ক্ষেত্র CSS_UNIT_CH:
		fprintf(stream, "ch");
		বিরতি;
	ক্ষেত্র CSS_UNIT_REM:
		fprintf(stream, "rem");
		বিরতি;
	ক্ষেত্র CSS_UNIT_LH:
		fprintf(stream, "lh");
		বিরতি;
	ক্ষেত্র CSS_UNIT_VH:
		fprintf(stream, "vh");
		বিরতি;
	ক্ষেত্র CSS_UNIT_VW:
		fprintf(stream, "vw");
		বিরতি;
	ক্ষেত্র CSS_UNIT_VI:
		fprintf(stream, "vi");
		বিরতি;
	ক্ষেত্র CSS_UNIT_VB:
		fprintf(stream, "vb");
		বিরতি;
	ক্ষেত্র CSS_UNIT_VMIN:
		fprintf(stream, "vmin");
		বিরতি;
	ক্ষেত্র CSS_UNIT_VMAX:
		fprintf(stream, "vmax");
		বিরতি;
	ক্ষেত্র CSS_UNIT_Q:
		fprintf(stream, "q");
		বিরতি;
	ক্ষেত্র CSS_UNIT_CALC:
		fprintf(stream, "calc()");
		বিরতি;
	}
}

/* exported interface documented in content/handlers/css/dump.h */
শূন্য nscss_dump_computed_style(FILE *stream, ধ্রুবক css_computed_style *style)
{
	uint8_t val;
	css_color color = 0;
	lwc_string *url = ফাঁকা;
	css_fixed len1 = 0, len2 = 0;
	css_unit unit1 = CSS_UNIT_PX, unit2 = CSS_UNIT_PX;
	css_computed_clip_rect rect = { 0, 0, 0, 0, CSS_UNIT_PX, CSS_UNIT_PX,
					CSS_UNIT_PX, CSS_UNIT_PX, true, true,
					true, true };
	ধ্রুবক css_computed_content_item *content = ফাঁকা;
	ধ্রুবক css_computed_গণক *গণক = ফাঁকা;
	lwc_string **string_list = ফাঁকা;
	int32_t zindex = 0;

	fprintf(stream, "{ ");

	/* background-attachment */
	val = css_computed_background_attachment(style);
	নির্বাচন (val) {
	ক্ষেত্র CSS_BACKGROUND_ATTACHMENT_FIXED:
		fprintf(stream, "background-attachment: fixed ");
		বিরতি;
	ক্ষেত্র CSS_BACKGROUND_ATTACHMENT_SCROLL:
		fprintf(stream, "background-attachment: scroll ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* background-color */
	val = css_computed_background_color(style, &color);
	নির্বাচন (val) {
	ক্ষেত্র CSS_BACKGROUND_COLOR_COLOR:
		fprintf(stream, "background-color: #%08"PRIx32" ", color);
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* background-image */
	val = css_computed_background_image(style, &url);
	যদি (val == CSS_BACKGROUND_IMAGE_IMAGE && url != ফাঁকা) {
		fprintf(stream, "background-image: url('%.*s') ",
				(পূর্ণসংখ্যা) lwc_string_দৈর্ঘ্য(url),
				lwc_string_তথ্য(url));
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
	নির্বাচন (val) {
	ক্ষেত্র CSS_BACKGROUND_REPEAT_REPEAT_X:
		fprintf(stream, "background-repeat: repeat-x ");
		বিরতি;
	ক্ষেত্র CSS_BACKGROUND_REPEAT_REPEAT_Y:
		fprintf(stream, "background-repeat: repeat-y ");
		বিরতি;
	ক্ষেত্র CSS_BACKGROUND_REPEAT_REPEAT:
		fprintf(stream, "background-repeat: repeat ");
		বিরতি;
	ক্ষেত্র CSS_BACKGROUND_REPEAT_NO_REPEAT:
		fprintf(stream, "background-repeat: no-repeat ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* border-collapse */
	val = css_computed_border_collapse(style);
	নির্বাচন (val) {
	ক্ষেত্র CSS_BORDER_COLLAPSE_SEPARATE:
		fprintf(stream, "border-collapse: separate ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_COLLAPSE_COLLAPSE:
		fprintf(stream, "border-collapse: collapse ");
		বিরতি;
	পূর্বনির্ধারিত:

		বিরতি;
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
	নির্বাচন (val) {
	ক্ষেত্র CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-top-color: #%08"PRIx32" ", color);
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* border-right-color */
	val = css_computed_border_ডান_color(style, &color);
	নির্বাচন (val) {
	ক্ষেত্র CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-right-color: #%08"PRIx32" ", color);
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* border-bottom-color */
	val = css_computed_border_bottom_color(style, &color);
	নির্বাচন (val) {
	ক্ষেত্র CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-bottom-color: #%08"PRIx32" ", color);
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* border-left-color */
	val = css_computed_border_বাম_color(style, &color);
	নির্বাচন (val) {
	ক্ষেত্র CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-left-color: #%08"PRIx32" ", color);
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* border-top-style */
	val = css_computed_border_শীর্ষ_style(style);
	নির্বাচন (val) {
	ক্ষেত্র CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-top-style: none ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-top-style: hidden ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-top-style: dotted ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-top-style: dashed ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-top-style: solid ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-top-style: double ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-top-style: groove ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-top-style: ridge ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-top-style: inset ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-top-style: outset ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* border-right-style */
	val = css_computed_border_ডান_style(style);
	নির্বাচন (val) {
	ক্ষেত্র CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-right-style: none ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-right-style: hidden ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-right-style: dotted ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-right-style: dashed ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-right-style: solid ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-right-style: double ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-right-style: groove ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-right-style: ridge ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-right-style: inset ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-right-style: outset ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* border-bottom-style */
	val = css_computed_border_bottom_style(style);
	নির্বাচন (val) {
	ক্ষেত্র CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-bottom-style: none ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-bottom-style: hidden ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-bottom-style: dotted ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-bottom-style: dashed ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-bottom-style: solid ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-bottom-style: double ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-bottom-style: groove ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-bottom-style: ridge ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-bottom-style: inset ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-bottom-style: outset ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* border-left-style */
	val = css_computed_border_বাম_style(style);
	নির্বাচন (val) {
	ক্ষেত্র CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-left-style: none ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-left-style: hidden ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-left-style: dotted ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-left-style: dashed ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-left-style: solid ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-left-style: double ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-left-style: groove ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-left-style: ridge ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-left-style: inset ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-left-style: outset ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* border-top-width */
	val = css_computed_border_শীর্ষ_width(style, &len1, &unit1);
	নির্বাচন (val) {
	ক্ষেত্র CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-top-width: thin ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-top-width: medium ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-top-width: thick ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-top-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* border-right-width */
	val = css_computed_border_ডান_width(style, &len1, &unit1);
	নির্বাচন (val) {
	ক্ষেত্র CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-right-width: thin ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-right-width: medium ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-right-width: thick ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-right-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* border-bottom-width */
	val = css_computed_border_bottom_width(style, &len1, &unit1);
	নির্বাচন (val) {
	ক্ষেত্র CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-bottom-width: thin ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-bottom-width: medium ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-bottom-width: thick ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-bottom-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* border-left-width */
	val = css_computed_border_বাম_width(style, &len1, &unit1);
	নির্বাচন (val) {
	ক্ষেত্র CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-left-width: thin ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-left-width: medium ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-left-width: thick ");
		বিরতি;
	ক্ষেত্র CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-left-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* bottom */
	val = css_computed_bottom(style, &len1, &unit1);
	নির্বাচন (val) {
	ক্ষেত্র CSS_BOTTOM_AUTO:
		fprintf(stream, "bottom: auto ");
		বিরতি;
	ক্ষেত্র CSS_BOTTOM_SET:
		fprintf(stream, "bottom: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* caption-side */
	val = css_computed_caption_side(style);
	নির্বাচন (val) {
	ক্ষেত্র CSS_CAPTION_SIDE_শীর্ষ:
		fprintf(stream, "caption_side: top ");
		বিরতি;
	ক্ষেত্র CSS_CAPTION_SIDE_BOTTOM:
		fprintf(stream, "caption_side: bottom ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* clear */
	val = css_computed_clear(style);
	নির্বাচন (val) {
	ক্ষেত্র CSS_CLEAR_NONE:
		fprintf(stream, "clear: none ");
		বিরতি;
	ক্ষেত্র CSS_CLEAR_বাম:
		fprintf(stream, "clear: left ");
		বিরতি;
	ক্ষেত্র CSS_CLEAR_ডান:
		fprintf(stream, "clear: right ");
		বিরতি;
	ক্ষেত্র CSS_CLEAR_BOTH:
		fprintf(stream, "clear: both ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* clip */
	val = css_computed_clip(style, &rect);
	নির্বাচন (val) {
	ক্ষেত্র CSS_CLIP_AUTO:
		fprintf(stream, "clip: auto ");
		বিরতি;
	ক্ষেত্র CSS_CLIP_RECT:
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
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* color */
	val = css_computed_color(style, &color);
	যদি (val == CSS_COLOR_COLOR) {
		fprintf(stream, "color: #%08"PRIx32" ", color);
	}

	/* content */
	val = css_computed_content(style, &content);
	নির্বাচন (val) {
	ক্ষেত্র CSS_CONTENT_NONE:
		fprintf(stream, "content: none ");
		বিরতি;
	ক্ষেত্র CSS_CONTENT_NORMAL:
		fprintf(stream, "content: normal ");
		বিরতি;
	ক্ষেত্র CSS_CONTENT_SET:
		fprintf(stream, "content:");

		যখন (content->type != CSS_COMPUTED_CONTENT_NONE) {
			fprintf(stream, " ");

			নির্বাচন (content->type) {
			ক্ষেত্র CSS_COMPUTED_CONTENT_STRING:
				fprintf(stream,	"\"%.*s\"",
						(পূর্ণসংখ্যা) lwc_string_দৈর্ঘ্য(
						content->তথ্য.string),
						lwc_string_তথ্য(
						content->তথ্য.string));
				বিরতি;
			ক্ষেত্র CSS_COMPUTED_CONTENT_URI:
				fprintf(stream,	"uri(\"%.*s\")",
						(পূর্ণসংখ্যা) lwc_string_দৈর্ঘ্য(
						content->তথ্য.uri),
						lwc_string_তথ্য(
						content->তথ্য.uri));
				বিরতি;
			ক্ষেত্র CSS_COMPUTED_CONTENT_গণক:
				fprintf(stream, "counter(%.*s)",
						(পূর্ণসংখ্যা) lwc_string_দৈর্ঘ্য(
						content->তথ্য.গণক.name),
						lwc_string_তথ্য(
						content->তথ্য.গণক.name));
				বিরতি;
			ক্ষেত্র CSS_COMPUTED_CONTENT_COUNTERS:
				fprintf(stream, "counters(%.*s, \"%.*s\")",
						(পূর্ণসংখ্যা) lwc_string_দৈর্ঘ্য(
						content->তথ্য.counters.name),
						lwc_string_তথ্য(
						content->তথ্য.counters.name),
						(পূর্ণসংখ্যা) lwc_string_দৈর্ঘ্য(
						content->তথ্য.counters.sep),
						lwc_string_তথ্য(
						content->তথ্য.counters.sep));
				বিরতি;
			ক্ষেত্র CSS_COMPUTED_CONTENT_ATTR:
				fprintf(stream, "attr(%.*s)",
						(পূর্ণসংখ্যা) lwc_string_দৈর্ঘ্য(
						content->তথ্য.attr),
						lwc_string_তথ্য(
						content->তথ্য.attr));
				বিরতি;
			ক্ষেত্র CSS_COMPUTED_CONTENT_OPEN_QUOTE:
				fprintf(stream, "open-quote");
				বিরতি;
			ক্ষেত্র CSS_COMPUTED_CONTENT_CLOSE_QUOTE:
				fprintf(stream, "close-quote");
				বিরতি;
			ক্ষেত্র CSS_COMPUTED_CONTENT_NO_OPEN_QUOTE:
				fprintf(stream, "no-open-quote");
				বিরতি;
			ক্ষেত্র CSS_COMPUTED_CONTENT_NO_CLOSE_QUOTE:
				fprintf(stream, "no-close-quote");
				বিরতি;
			}

			content++;
		}

		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* counter-increment */
	val = css_computed_গণক_increment(style, &গণক);
	যদি ((val == CSS_গণক_INCREMENT_NONE) || (গণক == ফাঁকা)) {
		fprintf(stream, "counter-increment: none ");
	} অন্যথা {
		fprintf(stream, "counter-increment:");

		যখন (গণক->name != ফাঁকা) {
			fprintf(stream, " %.*s ",
					(পূর্ণসংখ্যা) lwc_string_দৈর্ঘ্য(গণক->name),
					lwc_string_তথ্য(গণক->name));

			dump_css_fixed(stream, গণক->মান);

			গণক++;
		}

		fprintf(stream, " ");
	}

	/* counter-reset */
	val = css_computed_গণক_reset(style, &গণক);
	যদি ((val == CSS_গণক_RESET_NONE) || (গণক == ফাঁকা)) {
		fprintf(stream, "counter-reset: none ");
	} অন্যথা {
		fprintf(stream, "counter-reset:");

		যখন (গণক->name != ফাঁকা) {
			fprintf(stream, " %.*s ",
					(পূর্ণসংখ্যা) lwc_string_দৈর্ঘ্য(গণক->name),
					lwc_string_তথ্য(গণক->name));

			dump_css_fixed(stream, গণক->মান);

			গণক++;
		}

		fprintf(stream, " ");
	}

	/* cursor */
	val = css_computed_cursor(style, &string_list);
	fprintf(stream, "cursor:");

	যদি (string_list != ফাঁকা) {
		যখন (*string_list != ফাঁকা) {
			fprintf(stream, " url\"%.*s\")",
					(পূর্ণসংখ্যা) lwc_string_দৈর্ঘ্য(*string_list),
					lwc_string_তথ্য(*string_list));

			string_list++;
		}
	}
	নির্বাচন (val) {
	ক্ষেত্র CSS_CURSOR_AUTO:
		fprintf(stream, " auto ");
		বিরতি;
	ক্ষেত্র CSS_CURSOR_CROSSHAIR:
		fprintf(stream, " crosshair ");
		বিরতি;
	ক্ষেত্র CSS_CURSOR_DEFAULT:
		fprintf(stream, " default ");
		বিরতি;
	ক্ষেত্র CSS_CURSOR_POINTER:
		fprintf(stream, " pointer ");
		বিরতি;
	ক্ষেত্র CSS_CURSOR_MOVE:
		fprintf(stream, " move ");
		বিরতি;
	ক্ষেত্র CSS_CURSOR_E_RESIZE:
		fprintf(stream, " e-resize ");
		বিরতি;
	ক্ষেত্র CSS_CURSOR_NE_RESIZE:
		fprintf(stream, " ne-resize ");
		বিরতি;
	ক্ষেত্র CSS_CURSOR_NW_RESIZE:
		fprintf(stream, " nw-resize ");
		বিরতি;
	ক্ষেত্র CSS_CURSOR_N_RESIZE:
		fprintf(stream, " n-resize ");
		বিরতি;
	ক্ষেত্র CSS_CURSOR_SE_RESIZE:
		fprintf(stream, " se-resize ");
		বিরতি;
	ক্ষেত্র CSS_CURSOR_SW_RESIZE:
		fprintf(stream, " sw-resize ");
		বিরতি;
	ক্ষেত্র CSS_CURSOR_S_RESIZE:
		fprintf(stream, " s-resize ");
		বিরতি;
	ক্ষেত্র CSS_CURSOR_W_RESIZE:
		fprintf(stream, " w-resize ");
		বিরতি;
	ক্ষেত্র CSS_CURSOR_TEXT:
		fprintf(stream, " text ");
		বিরতি;
	ক্ষেত্র CSS_CURSOR_WAIT:
		fprintf(stream, " wait ");
		বিরতি;
	ক্ষেত্র CSS_CURSOR_HELP:
		fprintf(stream, " help ");
		বিরতি;
	ক্ষেত্র CSS_CURSOR_PROGRESS:
		fprintf(stream, " progress ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* direction */
	val = css_computed_direction(style);
	নির্বাচন (val) {
	ক্ষেত্র CSS_DIRECTION_LTR:
		fprintf(stream, "direction: ltr ");
		বিরতি;
	ক্ষেত্র CSS_DIRECTION_RTL:
		fprintf(stream, "direction: rtl ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* display */
	val = ns_computed_display_static(style);
	নির্বাচন (val) {
	ক্ষেত্র CSS_DISPLAY_INLINE:
		fprintf(stream, "display: inline ");
		বিরতি;
	ক্ষেত্র CSS_DISPLAY_BLOCK:
		fprintf(stream, "display: block ");
		বিরতি;
	ক্ষেত্র CSS_DISPLAY_LIST_ITEM:
		fprintf(stream, "display: list-item ");
		বিরতি;
	ক্ষেত্র CSS_DISPLAY_RUN_IN:
		fprintf(stream, "display: run-in ");
		বিরতি;
	ক্ষেত্র CSS_DISPLAY_INLINE_BLOCK:
		fprintf(stream, "display: inline-block ");
		বিরতি;
	ক্ষেত্র CSS_DISPLAY_TABLE:
		fprintf(stream, "display: table ");
		বিরতি;
	ক্ষেত্র CSS_DISPLAY_INLINE_TABLE:
		fprintf(stream, "display: inline-table ");
		বিরতি;
	ক্ষেত্র CSS_DISPLAY_TABLE_ROW_GROUP:
		fprintf(stream, "display: table-row-group ");
		বিরতি;
	ক্ষেত্র CSS_DISPLAY_TABLE_HEADER_GROUP:
		fprintf(stream, "display: table-header-group ");
		বিরতি;
	ক্ষেত্র CSS_DISPLAY_TABLE_FOOTER_GROUP:
		fprintf(stream, "display: table-footer-group ");
		বিরতি;
	ক্ষেত্র CSS_DISPLAY_TABLE_ROW:
		fprintf(stream, "display: table-row ");
		বিরতি;
	ক্ষেত্র CSS_DISPLAY_TABLE_COLUMN_GROUP:
		fprintf(stream, "display: table-column-group ");
		বিরতি;
	ক্ষেত্র CSS_DISPLAY_TABLE_COLUMN:
		fprintf(stream, "display: table-column ");
		বিরতি;
	ক্ষেত্র CSS_DISPLAY_TABLE_CELL:
		fprintf(stream, "display: table-cell ");
		বিরতি;
	ক্ষেত্র CSS_DISPLAY_TABLE_CAPTION:
		fprintf(stream, "display: table-caption ");
		বিরতি;
	ক্ষেত্র CSS_DISPLAY_NONE:
		fprintf(stream, "display: none ");
		বিরতি;
	ক্ষেত্র CSS_DISPLAY_FLEX:
		fprintf(stream, "display: flex ");
		বিরতি;
	ক্ষেত্র CSS_DISPLAY_INLINE_FLEX:
		fprintf(stream, "display: inline-flex ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* empty-cells */
	val = css_computed_empty_cells(style);
	নির্বাচন (val) {
	ক্ষেত্র CSS_EMPTY_CELLS_SHOW:
		fprintf(stream, "empty-cells: show ");
		বিরতি;
	ক্ষেত্র CSS_EMPTY_CELLS_HIDE:
		fprintf(stream, "empty-cells: hide ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* float */
	val = css_computed_float(style);
	নির্বাচন (val) {
	ক্ষেত্র CSS_FLOAT_বাম:
		fprintf(stream, "float: left ");
		বিরতি;
	ক্ষেত্র CSS_FLOAT_ডান:
		fprintf(stream, "float: right ");
		বিরতি;
	ক্ষেত্র CSS_FLOAT_NONE:
		fprintf(stream, "float: none ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* font-family */
	val = css_computed_font_family(style, &string_list);
	যদি (val != CSS_FONT_FAMILY_INHERIT) {
		fprintf(stream, "font-family:");

		যদি (string_list != ফাঁকা) {
			যখন (*string_list != ফাঁকা) {
				fprintf(stream, " \"%.*s\"",
					(পূর্ণসংখ্যা) lwc_string_দৈর্ঘ্য(*string_list),
					lwc_string_তথ্য(*string_list));

				string_list++;
			}
		}
		নির্বাচন (val) {
		ক্ষেত্র CSS_FONT_FAMILY_SERIF:
			fprintf(stream, " serif ");
			বিরতি;
		ক্ষেত্র CSS_FONT_FAMILY_SANS_SERIF:
			fprintf(stream, " sans-serif ");
			বিরতি;
		ক্ষেত্র CSS_FONT_FAMILY_CURSIVE:
			fprintf(stream, " cursive ");
			বিরতি;
		ক্ষেত্র CSS_FONT_FAMILY_FANTASY:
			fprintf(stream, " fantasy ");
			বিরতি;
		ক্ষেত্র CSS_FONT_FAMILY_MONOSPACE:
			fprintf(stream, " monospace ");
			বিরতি;
		}
	}

	/* font-size */
	val = css_computed_font_আকার(style, &len1, &unit1);
	নির্বাচন (val) {
	ক্ষেত্র CSS_FONT_আকার_XX_SMALL:
		fprintf(stream, "font-size: xx-small ");
		বিরতি;
	ক্ষেত্র CSS_FONT_আকার_X_SMALL:
		fprintf(stream, "font-size: x-small ");
		বিরতি;
	ক্ষেত্র CSS_FONT_আকার_SMALL:
		fprintf(stream, "font-size: small ");
		বিরতি;
	ক্ষেত্র CSS_FONT_আকার_MEDIUM:
		fprintf(stream, "font-size: medium ");
		বিরতি;
	ক্ষেত্র CSS_FONT_আকার_LARGE:
		fprintf(stream, "font-size: large ");
		বিরতি;
	ক্ষেত্র CSS_FONT_আকার_X_LARGE:
		fprintf(stream, "font-size: x-large ");
		বিরতি;
	ক্ষেত্র CSS_FONT_আকার_XX_LARGE:
		fprintf(stream, "font-size: xx-large ");
		বিরতি;
	ক্ষেত্র CSS_FONT_আকার_LARGER:
		fprintf(stream, "font-size: larger ");
		বিরতি;
	ক্ষেত্র CSS_FONT_আকার_SMALLER:
		fprintf(stream, "font-size: smaller ");
		বিরতি;
	ক্ষেত্র CSS_FONT_আকার_DIMENSION:
		fprintf(stream, "font-size: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* font-style */
	val = css_computed_font_style(style);
	নির্বাচন (val) {
	ক্ষেত্র CSS_FONT_STYLE_NORMAL:
		fprintf(stream, "font-style: normal ");
		বিরতি;
	ক্ষেত্র CSS_FONT_STYLE_ITALIC:
		fprintf(stream, "font-style: italic ");
		বিরতি;
	ক্ষেত্র CSS_FONT_STYLE_OBLIQUE:
		fprintf(stream, "font-style: oblique ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* font-variant */
	val = css_computed_font_variant(style);
	নির্বাচন (val) {
	ক্ষেত্র CSS_FONT_VARIANT_NORMAL:
		fprintf(stream, "font-variant: normal ");
		বিরতি;
	ক্ষেত্র CSS_FONT_VARIANT_SMALL_CAPS:
		fprintf(stream, "font-variant: small-caps ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* font-weight */
	val = css_computed_font_weight(style);
	নির্বাচন (val) {
	ক্ষেত্র CSS_FONT_WEIGHT_NORMAL:
		fprintf(stream, "font-weight: normal ");
		বিরতি;
	ক্ষেত্র CSS_FONT_WEIGHT_BOLD:
		fprintf(stream, "font-weight: bold ");
		বিরতি;
	ক্ষেত্র CSS_FONT_WEIGHT_BOLDER:
		fprintf(stream, "font-weight: bolder ");
		বিরতি;
	ক্ষেত্র CSS_FONT_WEIGHT_LIGHTER:
		fprintf(stream, "font-weight: lighter ");
		বিরতি;
	ক্ষেত্র CSS_FONT_WEIGHT_100:
		fprintf(stream, "font-weight: 100 ");
		বিরতি;
	ক্ষেত্র CSS_FONT_WEIGHT_200:
		fprintf(stream, "font-weight: 200 ");
		বিরতি;
	ক্ষেত্র CSS_FONT_WEIGHT_300:
		fprintf(stream, "font-weight: 300 ");
		বিরতি;
	ক্ষেত্র CSS_FONT_WEIGHT_400:
		fprintf(stream, "font-weight: 400 ");
		বিরতি;
	ক্ষেত্র CSS_FONT_WEIGHT_500:
		fprintf(stream, "font-weight: 500 ");
		বিরতি;
	ক্ষেত্র CSS_FONT_WEIGHT_600:
		fprintf(stream, "font-weight: 600 ");
		বিরতি;
	ক্ষেত্র CSS_FONT_WEIGHT_700:
		fprintf(stream, "font-weight: 700 ");
		বিরতি;
	ক্ষেত্র CSS_FONT_WEIGHT_800:
		fprintf(stream, "font-weight: 800 ");
		বিরতি;
	ক্ষেত্র CSS_FONT_WEIGHT_900:
		fprintf(stream, "font-weight: 900 ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* height */
	val = css_computed_height(style, &len1, &unit1);
	নির্বাচন (val) {
	ক্ষেত্র CSS_HEIGHT_AUTO:
		fprintf(stream, "height: auto ");
		বিরতি;
	ক্ষেত্র CSS_HEIGHT_SET:
		fprintf(stream, "height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/*বাম*/
	val = css_computed_বাম(style, &len1, &unit1);
	নির্বাচন (val) {
	ক্ষেত্র CSS_বাম_AUTO:
		fprintf(stream, "left: auto ");
		বিরতি;
	ক্ষেত্র CSS_বাম_SET:
		fprintf(stream, "left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* letter-spacing */
	val = css_computed_letter_spacing(style, &len1, &unit1);
	নির্বাচন (val) {
	ক্ষেত্র CSS_LETTER_SPACING_NORMAL:
		fprintf(stream, "letter-spacing: normal ");
		বিরতি;
	ক্ষেত্র CSS_LETTER_SPACING_SET:
		fprintf(stream, "letter-spacing: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* line-height */
	val = css_computed_সারি_height(style, &len1, &unit1);
	নির্বাচন (val) {
	ক্ষেত্র CSS_সারি_HEIGHT_NORMAL:
		fprintf(stream, "line-height: normal ");
		বিরতি;
	ক্ষেত্র CSS_সারি_HEIGHT_সংখ্যা:
		fprintf(stream, "line-height: ");

		dump_css_fixed(stream, len1);

		fprintf(stream, " ");
		বিরতি;
	ক্ষেত্র CSS_সারি_HEIGHT_DIMENSION:
		fprintf(stream, "line-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* list-style-image */
	val = css_computed_list_style_image(style, &url);
	যদি (url != ফাঁকা) {
		fprintf(stream, "list-style-image: url('%.*s') ",
				(পূর্ণসংখ্যা) lwc_string_দৈর্ঘ্য(url),
				lwc_string_তথ্য(url));
	} অন্যথা যদি (val == CSS_LIST_STYLE_IMAGE_NONE) {
		fprintf(stream, "list-style-image: none ");
	}

	/* list-style-position */
	val = css_computed_list_style_position(style);
	নির্বাচন (val) {
	ক্ষেত্র CSS_LIST_STYLE_POSITION_INSIDE:
		fprintf(stream, "list-style-position: inside ");
		বিরতি;
	ক্ষেত্র CSS_LIST_STYLE_POSITION_OUTSIDE:
		fprintf(stream, "list-style-position: outside ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* list-style-type */
	val = css_computed_list_style_type(style);
	নির্বাচন (val) {
	ক্ষেত্র CSS_LIST_STYLE_TYPE_DISC:
		fprintf(stream, "list-style-type: disc ");
		বিরতি;
	ক্ষেত্র CSS_LIST_STYLE_TYPE_CIRCLE:
		fprintf(stream, "list-style-type: circle ");
		বিরতি;
	ক্ষেত্র CSS_LIST_STYLE_TYPE_SQUARE:
		fprintf(stream, "list-style-type: square ");
		বিরতি;
	ক্ষেত্র CSS_LIST_STYLE_TYPE_DECIMAL:
		fprintf(stream, "list-style-type: decimal ");
		বিরতি;
	ক্ষেত্র CSS_LIST_STYLE_TYPE_DECIMAL_LEADING_ZERO:
		fprintf(stream, "list-style-type: decimal-leading-zero ");
		বিরতি;
	ক্ষেত্র CSS_LIST_STYLE_TYPE_LOWER_ROMAN:
		fprintf(stream, "list-style-type: lower-roman ");
		বিরতি;
	ক্ষেত্র CSS_LIST_STYLE_TYPE_UPPER_ROMAN:
		fprintf(stream, "list-style-type: upper-roman ");
		বিরতি;
	ক্ষেত্র CSS_LIST_STYLE_TYPE_LOWER_GREEK:
		fprintf(stream, "list-style-type: lower-greek ");
		বিরতি;
	ক্ষেত্র CSS_LIST_STYLE_TYPE_LOWER_LATIN:
		fprintf(stream, "list-style-type: lower-latin ");
		বিরতি;
	ক্ষেত্র CSS_LIST_STYLE_TYPE_UPPER_LATIN:
		fprintf(stream, "list-style-type: upper-latin ");
		বিরতি;
	ক্ষেত্র CSS_LIST_STYLE_TYPE_ARMENIAN:
		fprintf(stream, "list-style-type: armenian ");
		বিরতি;
	ক্ষেত্র CSS_LIST_STYLE_TYPE_GEORGIAN:
		fprintf(stream, "list-style-type: georgian ");
		বিরতি;
	ক্ষেত্র CSS_LIST_STYLE_TYPE_LOWER_ALPHA:
		fprintf(stream, "list-style-type: lower-alpha ");
		বিরতি;
	ক্ষেত্র CSS_LIST_STYLE_TYPE_UPPER_ALPHA:
		fprintf(stream, "list-style-type: upper-alpha ");
		বিরতি;
	ক্ষেত্র CSS_LIST_STYLE_TYPE_NONE:
		fprintf(stream, "list-style-type: none ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* margin-top */
	val = css_computed_margin_শীর্ষ(style, &len1, &unit1);
	নির্বাচন (val) {
	ক্ষেত্র CSS_MARGIN_AUTO:
		fprintf(stream, "margin-top: auto ");
		বিরতি;
	ক্ষেত্র CSS_MARGIN_SET:
		fprintf(stream, "margin-top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* margin-right */
	val = css_computed_margin_ডান(style, &len1, &unit1);
	নির্বাচন (val) {
	ক্ষেত্র CSS_MARGIN_AUTO:
		fprintf(stream, "margin-right: auto ");
		বিরতি;
	ক্ষেত্র CSS_MARGIN_SET:
		fprintf(stream, "margin-right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* margin-bottom */
	val = css_computed_margin_bottom(style, &len1, &unit1);
	নির্বাচন (val) {
	ক্ষেত্র CSS_MARGIN_AUTO:
		fprintf(stream, "margin-bottom: auto ");
		বিরতি;
	ক্ষেত্র CSS_MARGIN_SET:
		fprintf(stream, "margin-bottom: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* margin-left */
	val = css_computed_margin_বাম(style, &len1, &unit1);
	নির্বাচন (val) {
	ক্ষেত্র CSS_MARGIN_AUTO:
		fprintf(stream, "margin-left: auto ");
		বিরতি;
	ক্ষেত্র CSS_MARGIN_SET:
		fprintf(stream, "margin-left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* max-height */
	val = css_computed_max_height(style, &len1, &unit1);
	নির্বাচন (val) {
	ক্ষেত্র CSS_MAX_HEIGHT_NONE:
		fprintf(stream, "max-height: none ");
		বিরতি;
	ক্ষেত্র CSS_MAX_HEIGHT_SET:
		fprintf(stream, "max-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* max-width */
	val = css_computed_max_width(style, &len1, &unit1);
	নির্বাচন (val) {
	ক্ষেত্র CSS_MAX_WIDTH_NONE:
		fprintf(stream, "max-width: none ");
		বিরতি;
	ক্ষেত্র CSS_MAX_WIDTH_SET:
		fprintf(stream, "max-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* min-height */
	val = ns_computed_min_height(style, &len1, &unit1);
	নির্বাচন (val) {
	ক্ষেত্র CSS_MIN_HEIGHT_SET:
		fprintf(stream, "min-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* min-width */
	val = ns_computed_min_width(style, &len1, &unit1);
	নির্বাচন (val) {
	ক্ষেত্র CSS_MIN_WIDTH_SET:
		fprintf(stream, "min-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* opacity */
	val = css_computed_opacity(style, &len1);
	নির্বাচন (val) {
	ক্ষেত্র CSS_OPACITY_SET:
		fprintf(stream, "opacity: ");

		dump_css_fixed(stream, len1);

		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* outline-color */
	val = css_computed_outline_color(style, &color);
	নির্বাচন (val) {
	ক্ষেত্র CSS_OUTLINE_COLOR_INVERT:
		fprintf(stream, "outline-color: invert ");
		বিরতি;
	ক্ষেত্র CSS_OUTLINE_COLOR_COLOR:
		fprintf(stream, "outline-color: #%08"PRIx32" ", color);
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* outline-style */
	val = css_computed_outline_style(style);
	নির্বাচন (val) {
	ক্ষেত্র CSS_OUTLINE_STYLE_NONE:
		fprintf(stream, "outline-style: none ");
		বিরতি;
	ক্ষেত্র CSS_OUTLINE_STYLE_DOTTED:
		fprintf(stream, "outline-style: dotted ");
		বিরতি;
	ক্ষেত্র CSS_OUTLINE_STYLE_DASHED:
		fprintf(stream, "outline-style: dashed ");
		বিরতি;
	ক্ষেত্র CSS_OUTLINE_STYLE_SOLID:
		fprintf(stream, "outline-style: solid ");
		বিরতি;
	ক্ষেত্র CSS_OUTLINE_STYLE_DOUBLE:
		fprintf(stream, "outline-style: double ");
		বিরতি;
	ক্ষেত্র CSS_OUTLINE_STYLE_GROOVE:
		fprintf(stream, "outline-style: groove ");
		বিরতি;
	ক্ষেত্র CSS_OUTLINE_STYLE_RIDGE:
		fprintf(stream, "outline-style: ridge ");
		বিরতি;
	ক্ষেত্র CSS_OUTLINE_STYLE_INSET:
		fprintf(stream, "outline-style: inset ");
		বিরতি;
	ক্ষেত্র CSS_OUTLINE_STYLE_OUTSET:
		fprintf(stream, "outline-style: outset ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* outline-width */
	val = css_computed_outline_width(style, &len1, &unit1);
	নির্বাচন (val) {
	ক্ষেত্র CSS_OUTLINE_WIDTH_THIN:
		fprintf(stream, "outline-width: thin ");
		বিরতি;
	ক্ষেত্র CSS_OUTLINE_WIDTH_MEDIUM:
		fprintf(stream, "outline-width: medium ");
		বিরতি;
	ক্ষেত্র CSS_OUTLINE_WIDTH_THICK:
		fprintf(stream, "outline-width: thick ");
		বিরতি;
	ক্ষেত্র CSS_OUTLINE_WIDTH_WIDTH:
		fprintf(stream, "outline-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* overflow */
	val = css_computed_overflow_x(style);
	নির্বাচন (val) {
	ক্ষেত্র CSS_OVERFLOW_VISIBLE:
		fprintf(stream, "overflow-x: visible ");
		বিরতি;
	ক্ষেত্র CSS_OVERFLOW_HIDDEN:
		fprintf(stream, "overflow-x: hidden ");
		বিরতি;
	ক্ষেত্র CSS_OVERFLOW_SCROLL:
		fprintf(stream, "overflow-x: scroll ");
		বিরতি;
	ক্ষেত্র CSS_OVERFLOW_AUTO:
		fprintf(stream, "overflow-x auto ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* overflow */
	val = css_computed_overflow_y(style);
	নির্বাচন (val) {
	ক্ষেত্র CSS_OVERFLOW_VISIBLE:
		fprintf(stream, "overflow-y: visible ");
		বিরতি;
	ক্ষেত্র CSS_OVERFLOW_HIDDEN:
		fprintf(stream, "overflow-y: hidden ");
		বিরতি;
	ক্ষেত্র CSS_OVERFLOW_SCROLL:
		fprintf(stream, "overflow-y: scroll ");
		বিরতি;
	ক্ষেত্র CSS_OVERFLOW_AUTO:
		fprintf(stream, "overflow-y: auto ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* padding-top */
	val = css_computed_padding_শীর্ষ(style, &len1, &unit1);
	নির্বাচন (val) {
	ক্ষেত্র CSS_PADDING_SET:
		fprintf(stream, "padding-top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* padding-right */
	val = css_computed_padding_ডান(style, &len1, &unit1);
	নির্বাচন (val) {
	ক্ষেত্র CSS_PADDING_SET:
		fprintf(stream, "padding-right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* padding-bottom */
	val = css_computed_padding_bottom(style, &len1, &unit1);
	নির্বাচন (val) {
	ক্ষেত্র CSS_PADDING_SET:
		fprintf(stream, "padding-bottom: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* padding-left */
	val = css_computed_padding_বাম(style, &len1, &unit1);
	নির্বাচন (val) {
	ক্ষেত্র CSS_PADDING_SET:
		fprintf(stream, "padding-left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* position */
	val = css_computed_position(style);
	নির্বাচন (val) {
	ক্ষেত্র CSS_POSITION_STATIC:
		fprintf(stream, "position: static ");
		বিরতি;
	ক্ষেত্র CSS_POSITION_RELATIVE:
		fprintf(stream, "position: relative ");
		বিরতি;
	ক্ষেত্র CSS_POSITION_ABSOLUTE:
		fprintf(stream, "position: absolute ");
		বিরতি;
	ক্ষেত্র CSS_POSITION_FIXED:
		fprintf(stream, "position: fixed ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* quotes */
	val = css_computed_quotes(style, &string_list);
	যদি (val == CSS_QUOTES_STRING && string_list != ফাঁকা) {
		fprintf(stream, "quotes:");

		যখন (*string_list != ফাঁকা) {
			fprintf(stream, " \"%.*s\"",
				(পূর্ণসংখ্যা) lwc_string_দৈর্ঘ্য(*string_list),
				lwc_string_তথ্য(*string_list));

			string_list++;
		}

		fprintf(stream, " ");
	} অন্যথা {
		নির্বাচন (val) {
		ক্ষেত্র CSS_QUOTES_NONE:
			fprintf(stream, "quotes: none ");
			বিরতি;
		পূর্বনির্ধারিত:
			বিরতি;
		}
	}

	/*ডান*/
	val = css_computed_ডান(style, &len1, &unit1);
	নির্বাচন (val) {
	ক্ষেত্র CSS_ডান_AUTO:
		fprintf(stream, "right: auto ");
		বিরতি;
	ক্ষেত্র CSS_ডান_SET:
		fprintf(stream, "right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* table-layout */
	val = css_computed_table_layout(style);
	নির্বাচন (val) {
	ক্ষেত্র CSS_TABLE_LAYOUT_AUTO:
		fprintf(stream, "table-layout: auto ");
		বিরতি;
	ক্ষেত্র CSS_TABLE_LAYOUT_FIXED:
		fprintf(stream, "table-layout: fixed ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* text-align */
	val = css_computed_text_align(style);
	নির্বাচন (val) {
	ক্ষেত্র CSS_TEXT_ALIGN_বাম:
		fprintf(stream, "text-align: left ");
		বিরতি;
	ক্ষেত্র CSS_TEXT_ALIGN_ডান:
		fprintf(stream, "text-align: right ");
		বিরতি;
	ক্ষেত্র CSS_TEXT_ALIGN_CENTER:
		fprintf(stream, "text-align: center ");
		বিরতি;
	ক্ষেত্র CSS_TEXT_ALIGN_JUSTIFY:
		fprintf(stream, "text-align: justify ");
		বিরতি;
	ক্ষেত্র CSS_TEXT_ALIGN_DEFAULT:
		fprintf(stream, "text-align: default ");
		বিরতি;
	ক্ষেত্র CSS_TEXT_ALIGN_LIBCSS_বাম:
		fprintf(stream, "text-align: -libcss-left ");
		বিরতি;
	ক্ষেত্র CSS_TEXT_ALIGN_LIBCSS_CENTER:
		fprintf(stream, "text-align: -libcss-center ");
		বিরতি;
	ক্ষেত্র CSS_TEXT_ALIGN_LIBCSS_ডান:
		fprintf(stream, "text-align: -libcss-right ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
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
	নির্বাচন (val) {
	ক্ষেত্র CSS_TEXT_INDENT_SET:
		fprintf(stream, "text-indent: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* text-transform */
	val = css_computed_text_transform(style);
	নির্বাচন (val) {
	ক্ষেত্র CSS_TEXT_TRANSFORM_CAPITALIZE:
		fprintf(stream, "text-transform: capitalize ");
		বিরতি;
	ক্ষেত্র CSS_TEXT_TRANSFORM_UPPERCASE:
		fprintf(stream, "text-transform: uppercase ");
		বিরতি;
	ক্ষেত্র CSS_TEXT_TRANSFORM_LOWERCASE:
		fprintf(stream, "text-transform: lowercase ");
		বিরতি;
	ক্ষেত্র CSS_TEXT_TRANSFORM_NONE:
		fprintf(stream, "text-transform: none ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/*শীর্ষ*/
	val = css_computed_শীর্ষ(style, &len1, &unit1);
	নির্বাচন (val) {
	ক্ষেত্র CSS_শীর্ষ_AUTO:
		fprintf(stream, "top: auto ");
		বিরতি;
	ক্ষেত্র CSS_শীর্ষ_SET:
		fprintf(stream, "top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* unicode-bidi */
	val = css_computed_unicode_bidi(style);
	নির্বাচন (val) {
	ক্ষেত্র CSS_UNICODE_BIDI_NORMAL:
		fprintf(stream, "unicode-bidi: normal ");
		বিরতি;
	ক্ষেত্র CSS_UNICODE_BIDI_EMBED:
		fprintf(stream, "unicode-bidi: embed ");
		বিরতি;
	ক্ষেত্র CSS_UNICODE_BIDI_BIDI_OVERRIDE:
		fprintf(stream, "unicode-bidi: bidi-override ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* vertical-align */
	val = css_computed_vertical_align(style, &len1, &unit1);
	নির্বাচন (val) {
	ক্ষেত্র CSS_VERTICAL_ALIGN_BASELINE:
		fprintf(stream, "vertical-align: baseline ");
		বিরতি;
	ক্ষেত্র CSS_VERTICAL_ALIGN_SUB:
		fprintf(stream, "vertical-align: sub ");
		বিরতি;
	ক্ষেত্র CSS_VERTICAL_ALIGN_SUPER:
		fprintf(stream, "vertical-align: super ");
		বিরতি;
	ক্ষেত্র CSS_VERTICAL_ALIGN_শীর্ষ:
		fprintf(stream, "vertical-align: top ");
		বিরতি;
	ক্ষেত্র CSS_VERTICAL_ALIGN_TEXT_শীর্ষ:
		fprintf(stream, "vertical-align: text-top ");
		বিরতি;
	ক্ষেত্র CSS_VERTICAL_ALIGN_মধ্য:
		fprintf(stream, "vertical-align: middle ");
		বিরতি;
	ক্ষেত্র CSS_VERTICAL_ALIGN_BOTTOM:
		fprintf(stream, "vertical-align: bottom ");
		বিরতি;
	ক্ষেত্র CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
		fprintf(stream, "vertical-align: text-bottom ");
		বিরতি;
	ক্ষেত্র CSS_VERTICAL_ALIGN_SET:
		fprintf(stream, "vertical-align: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* visibility */
	val = css_computed_visibility(style);
	নির্বাচন (val) {
	ক্ষেত্র CSS_VISIBILITY_VISIBLE:
		fprintf(stream, "visibility: visible ");
		বিরতি;
	ক্ষেত্র CSS_VISIBILITY_HIDDEN:
		fprintf(stream, "visibility: hidden ");
		বিরতি;
	ক্ষেত্র CSS_VISIBILITY_COLLAPSE:
		fprintf(stream, "visibility: collapse ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* white-space */
	val = css_computed_white_space(style);
	নির্বাচন (val) {
	ক্ষেত্র CSS_WHITE_SPACE_NORMAL:
		fprintf(stream, "white-space: normal ");
		বিরতি;
	ক্ষেত্র CSS_WHITE_SPACE_PRE:
		fprintf(stream, "white-space: pre ");
		বিরতি;
	ক্ষেত্র CSS_WHITE_SPACE_NOWRAP:
		fprintf(stream, "white-space: nowrap ");
		বিরতি;
	ক্ষেত্র CSS_WHITE_SPACE_PRE_WRAP:
		fprintf(stream, "white-space: pre-wrap ");
		বিরতি;
	ক্ষেত্র CSS_WHITE_SPACE_PRE_সারি:
		fprintf(stream, "white-space: pre-line ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* width */
	val = css_computed_width(style, &len1, &unit1);
	নির্বাচন (val) {
	ক্ষেত্র CSS_WIDTH_AUTO:
		fprintf(stream, "width: auto ");
		বিরতি;
	ক্ষেত্র CSS_WIDTH_SET:
		fprintf(stream, "width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* word-spacing */
	val = css_computed_word_spacing(style, &len1, &unit1);
	নির্বাচন (val) {
	ক্ষেত্র CSS_WORD_SPACING_NORMAL:
		fprintf(stream, "word-spacing: normal ");
		বিরতি;
	ক্ষেত্র CSS_WORD_SPACING_SET:
		fprintf(stream, "word-spacing: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	/* z-index */
	val = css_computed_z_সূচক(style, &zindex);
	নির্বাচন (val) {
	ক্ষেত্র CSS_Z_সূচক_AUTO:
		fprintf(stream, "z-index: auto ");
		বিরতি;
	ক্ষেত্র CSS_Z_সূচক_SET:
		fprintf(stream, "z-index: %"PRId32" ", zindex);
		বিরতি;
	পূর্বনির্ধারিত:
		বিরতি;
	}

	fprintf(stream, "}");
}
