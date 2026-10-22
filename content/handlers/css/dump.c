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

#شامل <stdio.h>
#شامل <libcss/libcss.h>

#شامل "netsurf/inttypes.h"

#شامل "css/dump.h"
#شامل "css/utils.h"

/**
 * Dump a fixed point value to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param f       Value to write
 */
ساکن خالی dump_css_fixed(FILE *دھارا, css_fixed f)
{
#تعریف_کرو NSCSS_ABS(x) (uint32_t)((x) < 0 ? -(x) : (x))
	uint32_t uintpart = FIXTOINT(NSCSS_ABS(f));
	/* + 500 to ensure round to nearest (division will truncate) */
	uint32_t fracpart = ((NSCSS_ABS(f) & 0x3ff) * 1000 + 500) / (1 << 10);
#تعریف_منسوخ NSCSS_ABS

	fprintf(دھارا, "%s%"PRIu32".%03"PRIu32, f < 0 ? "-" : "", uintpart, fracpart);
}

/**
 * Dump a numeric value to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 */
ساکن خالی dump_css_نمبر(FILE *دھارا, css_fixed val)
{
	اگر (INTTOFIX(FIXTOINT(val)) == val)
		fprintf(دھارا, "%"PRId32, FIXTOINT(val));
	ورنہ
		dump_css_fixed(دھارا, val);
}

/**
 * Dump a dimension to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 * \param unit    Unit to write
 */
ساکن خالی dump_css_unit(FILE *دھارا, css_fixed val, css_unit unit)
{
	dump_css_نمبر(دھارا, val);

	انتخاب (unit) {
	حالت CSS_UNIT_PX:
		fprintf(دھارا, "px");
		توڑو;
	حالت CSS_UNIT_EX:
		fprintf(دھارا, "ex");
		توڑو;
	حالت CSS_UNIT_EM:
		fprintf(دھارا, "em");
		توڑو;
	حالت CSS_UNIT_اندر:
		fprintf(دھارا, "in");
		توڑو;
	حالت CSS_UNIT_CM:
		fprintf(دھارا, "cm");
		توڑو;
	حالت CSS_UNIT_MM:
		fprintf(دھارا, "mm");
		توڑو;
	حالت CSS_UNIT_PT:
		fprintf(دھارا, "pt");
		توڑو;
	حالت CSS_UNIT_PC:
		fprintf(دھارا, "pc");
		توڑو;
	حالت CSS_UNIT_PCT:
		fprintf(دھارا, "%%");
		توڑو;
	حالت CSS_UNIT_DEG:
		fprintf(دھارا, "deg");
		توڑو;
	حالت CSS_UNIT_GRAD:
		fprintf(دھارا, "grad");
		توڑو;
	حالت CSS_UNIT_RAD:
		fprintf(دھارا, "rad");
		توڑو;
	حالت CSS_UNIT_MS:
		fprintf(دھارا, "ms");
		توڑو;
	حالت CSS_UNIT_S:
		fprintf(دھارا, "s");
		توڑو;
	حالت CSS_UNIT_HZ:
		fprintf(دھارا, "Hz");
		توڑو;
	حالت CSS_UNIT_KHZ:
		fprintf(دھارا, "kHz");
		توڑو;
	حالت CSS_UNIT_CH:
		fprintf(دھارا, "ch");
		توڑو;
	حالت CSS_UNIT_REM:
		fprintf(دھارا, "rem");
		توڑو;
	حالت CSS_UNIT_LH:
		fprintf(دھارا, "lh");
		توڑو;
	حالت CSS_UNIT_VH:
		fprintf(دھارا, "vh");
		توڑو;
	حالت CSS_UNIT_VW:
		fprintf(دھارا, "vw");
		توڑو;
	حالت CSS_UNIT_VI:
		fprintf(دھارا, "vi");
		توڑو;
	حالت CSS_UNIT_VB:
		fprintf(دھارا, "vb");
		توڑو;
	حالت CSS_UNIT_VMIN:
		fprintf(دھارا, "vmin");
		توڑو;
	حالت CSS_UNIT_VMAX:
		fprintf(دھارا, "vmax");
		توڑو;
	حالت CSS_UNIT_Q:
		fprintf(دھارا, "q");
		توڑو;
	حالت CSS_UNIT_CALC:
		fprintf(دھارا, "calc()");
		توڑو;
	}
}

/* exported interface documented in content/handlers/css/dump.h */
خالی nscss_dump_computed_style(FILE *دھارا, ثابت css_computed_style *style)
{
	uint8_t val;
	css_color color = 0;
	lwc_string *url = تہی;
	css_fixed len1 = 0, len2 = 0;
	css_unit unit1 = CSS_UNIT_PX, unit2 = CSS_UNIT_PX;
	css_computed_clip_rect rect = { 0, 0, 0, 0, CSS_UNIT_PX, CSS_UNIT_PX,
					CSS_UNIT_PX, CSS_UNIT_PX, true, true,
					true, true };
	ثابت css_computed_content_شے *content = تہی;
	ثابت css_computed_شمار *شمار_2 = تہی;
	lwc_string **string_list = تہی;
	int32_t zindex = 0;

	fprintf(دھارا, "{ ");

	/* background-attachment */
	val = css_computed_background_attachment(style);
	انتخاب (val) {
	حالت CSS_BACKGROUND_ATTACHMENT_FIXED:
		fprintf(دھارا, "background-attachment: fixed ");
		توڑو;
	حالت CSS_BACKGROUND_ATTACHMENT_SCROLL:
		fprintf(دھارا, "background-attachment: scroll ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* background-color */
	val = css_computed_background_color(style, &color);
	انتخاب (val) {
	حالت CSS_BACKGROUND_COLOR_COLOR:
		fprintf(دھارا, "background-color: #%08"PRIx32" ", color);
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* background-image */
	val = css_computed_background_image(style, &url);
	اگر (val == CSS_BACKGROUND_IMAGE_IMAGE && url != تہی) {
		fprintf(دھارا, "background-image: url('%.*s') ",
				(صحیح) lwc_string_لمبائی(url),
				lwc_string_ڈیٹا(url));
	} ورنہ اگر (val == CSS_BACKGROUND_IMAGE_NONE) {
		fprintf(دھارا, "background-image: none ");
	}

	/* background-position */
	val = css_computed_background_position(style, &len1, &unit1,
			&len2, &unit2);
	اگر (val == CSS_BACKGROUND_POSITION_سیٹ) {
		fprintf(دھارا, "background-position: ");
		dump_css_unit(دھارا, len1, unit1);
		fprintf(دھارا, " ");
		dump_css_unit(دھارا, len2, unit2);
		fprintf(دھارا, " ");
	}

	/* background-repeat */
	val = css_computed_background_repeat(style);
	انتخاب (val) {
	حالت CSS_BACKGROUND_REPEAT_REPEAT_X:
		fprintf(دھارا, "background-repeat: repeat-x ");
		توڑو;
	حالت CSS_BACKGROUND_REPEAT_REPEAT_Y:
		fprintf(دھارا, "background-repeat: repeat-y ");
		توڑو;
	حالت CSS_BACKGROUND_REPEAT_REPEAT:
		fprintf(دھارا, "background-repeat: repeat ");
		توڑو;
	حالت CSS_BACKGROUND_REPEAT_NO_REPEAT:
		fprintf(دھارا, "background-repeat: no-repeat ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* border-collapse */
	val = css_computed_border_collapse(style);
	انتخاب (val) {
	حالت CSS_BORDER_COLLAPSE_SEPARATE:
		fprintf(دھارا, "border-collapse: separate ");
		توڑو;
	حالت CSS_BORDER_COLLAPSE_COLLAPSE:
		fprintf(دھارا, "border-collapse: collapse ");
		توڑو;
	طےشدہ:

		توڑو;
	}

	/* border-spacing */
	val = css_computed_border_spacing(style, &len1, &unit1, &len2, &unit2);
	اگر (val == CSS_BORDER_SPACING_سیٹ) {
		fprintf(دھارا, "border-spacing: ");
		dump_css_unit(دھارا, len1, unit1);
		fprintf(دھارا, " ");
		dump_css_unit(دھارا, len2, unit2);
		fprintf(دھارا, " ");
	}

	/* border-top-color */
	val = css_computed_border_چوٹی_color(style, &color);
	انتخاب (val) {
	حالت CSS_BORDER_COLOR_COLOR:
		fprintf(دھارا, "border-top-color: #%08"PRIx32" ", color);
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* border-right-color */
	val = css_computed_border_دایاں_color(style, &color);
	انتخاب (val) {
	حالت CSS_BORDER_COLOR_COLOR:
		fprintf(دھارا, "border-right-color: #%08"PRIx32" ", color);
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* border-bottom-color */
	val = css_computed_border_bottom_color(style, &color);
	انتخاب (val) {
	حالت CSS_BORDER_COLOR_COLOR:
		fprintf(دھارا, "border-bottom-color: #%08"PRIx32" ", color);
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* border-left-color */
	val = css_computed_border_بایاں_color(style, &color);
	انتخاب (val) {
	حالت CSS_BORDER_COLOR_COLOR:
		fprintf(دھارا, "border-left-color: #%08"PRIx32" ", color);
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* border-top-style */
	val = css_computed_border_چوٹی_style(style);
	انتخاب (val) {
	حالت CSS_BORDER_STYLE_NONE:
		fprintf(دھارا, "border-top-style: none ");
		توڑو;
	حالت CSS_BORDER_STYLE_HIDDEN:
		fprintf(دھارا, "border-top-style: hidden ");
		توڑو;
	حالت CSS_BORDER_STYLE_DOTTED:
		fprintf(دھارا, "border-top-style: dotted ");
		توڑو;
	حالت CSS_BORDER_STYLE_DASHED:
		fprintf(دھارا, "border-top-style: dashed ");
		توڑو;
	حالت CSS_BORDER_STYLE_SOLID:
		fprintf(دھارا, "border-top-style: solid ");
		توڑو;
	حالت CSS_BORDER_STYLE_DOUBLE:
		fprintf(دھارا, "border-top-style: double ");
		توڑو;
	حالت CSS_BORDER_STYLE_GROOVE:
		fprintf(دھارا, "border-top-style: groove ");
		توڑو;
	حالت CSS_BORDER_STYLE_RIDGE:
		fprintf(دھارا, "border-top-style: ridge ");
		توڑو;
	حالت CSS_BORDER_STYLE_INSET:
		fprintf(دھارا, "border-top-style: inset ");
		توڑو;
	حالت CSS_BORDER_STYLE_OUTSET:
		fprintf(دھارا, "border-top-style: outset ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* border-right-style */
	val = css_computed_border_دایاں_style(style);
	انتخاب (val) {
	حالت CSS_BORDER_STYLE_NONE:
		fprintf(دھارا, "border-right-style: none ");
		توڑو;
	حالت CSS_BORDER_STYLE_HIDDEN:
		fprintf(دھارا, "border-right-style: hidden ");
		توڑو;
	حالت CSS_BORDER_STYLE_DOTTED:
		fprintf(دھارا, "border-right-style: dotted ");
		توڑو;
	حالت CSS_BORDER_STYLE_DASHED:
		fprintf(دھارا, "border-right-style: dashed ");
		توڑو;
	حالت CSS_BORDER_STYLE_SOLID:
		fprintf(دھارا, "border-right-style: solid ");
		توڑو;
	حالت CSS_BORDER_STYLE_DOUBLE:
		fprintf(دھارا, "border-right-style: double ");
		توڑو;
	حالت CSS_BORDER_STYLE_GROOVE:
		fprintf(دھارا, "border-right-style: groove ");
		توڑو;
	حالت CSS_BORDER_STYLE_RIDGE:
		fprintf(دھارا, "border-right-style: ridge ");
		توڑو;
	حالت CSS_BORDER_STYLE_INSET:
		fprintf(دھارا, "border-right-style: inset ");
		توڑو;
	حالت CSS_BORDER_STYLE_OUTSET:
		fprintf(دھارا, "border-right-style: outset ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* border-bottom-style */
	val = css_computed_border_bottom_style(style);
	انتخاب (val) {
	حالت CSS_BORDER_STYLE_NONE:
		fprintf(دھارا, "border-bottom-style: none ");
		توڑو;
	حالت CSS_BORDER_STYLE_HIDDEN:
		fprintf(دھارا, "border-bottom-style: hidden ");
		توڑو;
	حالت CSS_BORDER_STYLE_DOTTED:
		fprintf(دھارا, "border-bottom-style: dotted ");
		توڑو;
	حالت CSS_BORDER_STYLE_DASHED:
		fprintf(دھارا, "border-bottom-style: dashed ");
		توڑو;
	حالت CSS_BORDER_STYLE_SOLID:
		fprintf(دھارا, "border-bottom-style: solid ");
		توڑو;
	حالت CSS_BORDER_STYLE_DOUBLE:
		fprintf(دھارا, "border-bottom-style: double ");
		توڑو;
	حالت CSS_BORDER_STYLE_GROOVE:
		fprintf(دھارا, "border-bottom-style: groove ");
		توڑو;
	حالت CSS_BORDER_STYLE_RIDGE:
		fprintf(دھارا, "border-bottom-style: ridge ");
		توڑو;
	حالت CSS_BORDER_STYLE_INSET:
		fprintf(دھارا, "border-bottom-style: inset ");
		توڑو;
	حالت CSS_BORDER_STYLE_OUTSET:
		fprintf(دھارا, "border-bottom-style: outset ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* border-left-style */
	val = css_computed_border_بایاں_style(style);
	انتخاب (val) {
	حالت CSS_BORDER_STYLE_NONE:
		fprintf(دھارا, "border-left-style: none ");
		توڑو;
	حالت CSS_BORDER_STYLE_HIDDEN:
		fprintf(دھارا, "border-left-style: hidden ");
		توڑو;
	حالت CSS_BORDER_STYLE_DOTTED:
		fprintf(دھارا, "border-left-style: dotted ");
		توڑو;
	حالت CSS_BORDER_STYLE_DASHED:
		fprintf(دھارا, "border-left-style: dashed ");
		توڑو;
	حالت CSS_BORDER_STYLE_SOLID:
		fprintf(دھارا, "border-left-style: solid ");
		توڑو;
	حالت CSS_BORDER_STYLE_DOUBLE:
		fprintf(دھارا, "border-left-style: double ");
		توڑو;
	حالت CSS_BORDER_STYLE_GROOVE:
		fprintf(دھارا, "border-left-style: groove ");
		توڑو;
	حالت CSS_BORDER_STYLE_RIDGE:
		fprintf(دھارا, "border-left-style: ridge ");
		توڑو;
	حالت CSS_BORDER_STYLE_INSET:
		fprintf(دھارا, "border-left-style: inset ");
		توڑو;
	حالت CSS_BORDER_STYLE_OUTSET:
		fprintf(دھارا, "border-left-style: outset ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* border-top-width */
	val = css_computed_border_چوٹی_چوڑائی(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_BORDER_چوڑائی_THIN:
		fprintf(دھارا, "border-top-width: thin ");
		توڑو;
	حالت CSS_BORDER_چوڑائی_MEDIUM:
		fprintf(دھارا, "border-top-width: medium ");
		توڑو;
	حالت CSS_BORDER_چوڑائی_THICK:
		fprintf(دھارا, "border-top-width: thick ");
		توڑو;
	حالت CSS_BORDER_چوڑائی_چوڑائی:
		fprintf(دھارا, "border-top-width: ");
		dump_css_unit(دھارا, len1, unit1);
		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* border-right-width */
	val = css_computed_border_دایاں_چوڑائی(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_BORDER_چوڑائی_THIN:
		fprintf(دھارا, "border-right-width: thin ");
		توڑو;
	حالت CSS_BORDER_چوڑائی_MEDIUM:
		fprintf(دھارا, "border-right-width: medium ");
		توڑو;
	حالت CSS_BORDER_چوڑائی_THICK:
		fprintf(دھارا, "border-right-width: thick ");
		توڑو;
	حالت CSS_BORDER_چوڑائی_چوڑائی:
		fprintf(دھارا, "border-right-width: ");
		dump_css_unit(دھارا, len1, unit1);
		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* border-bottom-width */
	val = css_computed_border_bottom_چوڑائی(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_BORDER_چوڑائی_THIN:
		fprintf(دھارا, "border-bottom-width: thin ");
		توڑو;
	حالت CSS_BORDER_چوڑائی_MEDIUM:
		fprintf(دھارا, "border-bottom-width: medium ");
		توڑو;
	حالت CSS_BORDER_چوڑائی_THICK:
		fprintf(دھارا, "border-bottom-width: thick ");
		توڑو;
	حالت CSS_BORDER_چوڑائی_چوڑائی:
		fprintf(دھارا, "border-bottom-width: ");
		dump_css_unit(دھارا, len1, unit1);
		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* border-left-width */
	val = css_computed_border_بایاں_چوڑائی(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_BORDER_چوڑائی_THIN:
		fprintf(دھارا, "border-left-width: thin ");
		توڑو;
	حالت CSS_BORDER_چوڑائی_MEDIUM:
		fprintf(دھارا, "border-left-width: medium ");
		توڑو;
	حالت CSS_BORDER_چوڑائی_THICK:
		fprintf(دھارا, "border-left-width: thick ");
		توڑو;
	حالت CSS_BORDER_چوڑائی_چوڑائی:
		fprintf(دھارا, "border-left-width: ");
		dump_css_unit(دھارا, len1, unit1);
		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* bottom */
	val = css_computed_bottom(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_BOTTOM_AUTO:
		fprintf(دھارا, "bottom: auto ");
		توڑو;
	حالت CSS_BOTTOM_سیٹ:
		fprintf(دھارا, "bottom: ");
		dump_css_unit(دھارا, len1, unit1);
		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* caption-side */
	val = css_computed_caption_side(style);
	انتخاب (val) {
	حالت CSS_CAPTION_SIDE_چوٹی:
		fprintf(دھارا, "caption_side: top ");
		توڑو;
	حالت CSS_CAPTION_SIDE_BOTTOM:
		fprintf(دھارا, "caption_side: bottom ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* clear */
	val = css_computed_clear(style);
	انتخاب (val) {
	حالت CSS_CLEAR_NONE:
		fprintf(دھارا, "clear: none ");
		توڑو;
	حالت CSS_CLEAR_بایاں:
		fprintf(دھارا, "clear: left ");
		توڑو;
	حالت CSS_CLEAR_دایاں:
		fprintf(دھارا, "clear: right ");
		توڑو;
	حالت CSS_CLEAR_BOTH:
		fprintf(دھارا, "clear: both ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* clip */
	val = css_computed_clip(style, &rect);
	انتخاب (val) {
	حالت CSS_CLIP_AUTO:
		fprintf(دھارا, "clip: auto ");
		توڑو;
	حالت CSS_CLIP_RECT:
		fprintf(دھارا, "clip: rect( ");

		اگر (rect.چوٹی_auto)
			fprintf(دھارا, "auto");
		ورنہ
			dump_css_unit(دھارا, rect.چوٹی, rect.tunit);
		fprintf(دھارا, ", ");

		اگر (rect.دایاں_auto)
			fprintf(دھارا, "auto");
		ورنہ
			dump_css_unit(دھارا, rect.دایاں, rect.runit);
		fprintf(دھارا, ", ");

		اگر (rect.bottom_auto)
			fprintf(دھارا, "auto");
		ورنہ
			dump_css_unit(دھارا, rect.bottom, rect.bunit);
		fprintf(دھارا, ", ");

		اگر (rect.بایاں_auto)
			fprintf(دھارا, "auto");
		ورنہ
			dump_css_unit(دھارا, rect.بایاں, rect.lunit);
		fprintf(دھارا, ") ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* color */
	val = css_computed_color(style, &color);
	اگر (val == CSS_COLOR_COLOR) {
		fprintf(دھارا, "color: #%08"PRIx32" ", color);
	}

	/* content */
	val = css_computed_content(style, &content);
	انتخاب (val) {
	حالت CSS_CONTENT_NONE:
		fprintf(دھارا, "content: none ");
		توڑو;
	حالت CSS_CONTENT_معمول:
		fprintf(دھارا, "content: normal ");
		توڑو;
	حالت CSS_CONTENT_سیٹ:
		fprintf(دھارا, "content:");

		جب (content->type != CSS_COMPUTED_CONTENT_NONE) {
			fprintf(دھارا, " ");

			انتخاب (content->type) {
			حالت CSS_COMPUTED_CONTENT_STRING:
				fprintf(دھارا,	"\"%.*s\"",
						(صحیح) lwc_string_لمبائی(
						content->ڈیٹا.string),
						lwc_string_ڈیٹا(
						content->ڈیٹا.string));
				توڑو;
			حالت CSS_COMPUTED_CONTENT_URI:
				fprintf(دھارا,	"uri(\"%.*s\")",
						(صحیح) lwc_string_لمبائی(
						content->ڈیٹا.uri),
						lwc_string_ڈیٹا(
						content->ڈیٹا.uri));
				توڑو;
			حالت CSS_COMPUTED_CONTENT_شمار:
				fprintf(دھارا, "counter(%.*s)",
						(صحیح) lwc_string_لمبائی(
						content->ڈیٹا.شمار_2.name),
						lwc_string_ڈیٹا(
						content->ڈیٹا.شمار_2.name));
				توڑو;
			حالت CSS_COMPUTED_CONTENT_COUNTERS:
				fprintf(دھارا, "counters(%.*s, \"%.*s\")",
						(صحیح) lwc_string_لمبائی(
						content->ڈیٹا.counters.name),
						lwc_string_ڈیٹا(
						content->ڈیٹا.counters.name),
						(صحیح) lwc_string_لمبائی(
						content->ڈیٹا.counters.sep),
						lwc_string_ڈیٹا(
						content->ڈیٹا.counters.sep));
				توڑو;
			حالت CSS_COMPUTED_CONTENT_ATTR:
				fprintf(دھارا, "attr(%.*s)",
						(صحیح) lwc_string_لمبائی(
						content->ڈیٹا.attr),
						lwc_string_ڈیٹا(
						content->ڈیٹا.attr));
				توڑو;
			حالت CSS_COMPUTED_CONTENT_OPEN_QUOTE:
				fprintf(دھارا, "open-quote");
				توڑو;
			حالت CSS_COMPUTED_CONTENT_CLOSE_QUOTE:
				fprintf(دھارا, "close-quote");
				توڑو;
			حالت CSS_COMPUTED_CONTENT_NO_OPEN_QUOTE:
				fprintf(دھارا, "no-open-quote");
				توڑو;
			حالت CSS_COMPUTED_CONTENT_NO_CLOSE_QUOTE:
				fprintf(دھارا, "no-close-quote");
				توڑو;
			}

			content++;
		}

		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* counter-increment */
	val = css_computed_شمار_increment(style, &شمار_2);
	اگر ((val == CSS_شمار_INCREMENT_NONE) || (شمار_2 == تہی)) {
		fprintf(دھارا, "counter-increment: none ");
	} ورنہ {
		fprintf(دھارا, "counter-increment:");

		جب (شمار_2->name != تہی) {
			fprintf(دھارا, " %.*s ",
					(صحیح) lwc_string_لمبائی(شمار_2->name),
					lwc_string_ڈیٹا(شمار_2->name));

			dump_css_fixed(دھارا, شمار_2->قدر);

			شمار_2++;
		}

		fprintf(دھارا, " ");
	}

	/* counter-reset */
	val = css_computed_شمار_reset(style, &شمار_2);
	اگر ((val == CSS_شمار_RESET_NONE) || (شمار_2 == تہی)) {
		fprintf(دھارا, "counter-reset: none ");
	} ورنہ {
		fprintf(دھارا, "counter-reset:");

		جب (شمار_2->name != تہی) {
			fprintf(دھارا, " %.*s ",
					(صحیح) lwc_string_لمبائی(شمار_2->name),
					lwc_string_ڈیٹا(شمار_2->name));

			dump_css_fixed(دھارا, شمار_2->قدر);

			شمار_2++;
		}

		fprintf(دھارا, " ");
	}

	/* cursor */
	val = css_computed_cursor(style, &string_list);
	fprintf(دھارا, "cursor:");

	اگر (string_list != تہی) {
		جب (*string_list != تہی) {
			fprintf(دھارا, " url\"%.*s\")",
					(صحیح) lwc_string_لمبائی(*string_list),
					lwc_string_ڈیٹا(*string_list));

			string_list++;
		}
	}
	انتخاب (val) {
	حالت CSS_CURSOR_AUTO:
		fprintf(دھارا, " auto ");
		توڑو;
	حالت CSS_CURSOR_CROSSHAIR:
		fprintf(دھارا, " crosshair ");
		توڑو;
	حالت CSS_CURSOR_DEFAULT:
		fprintf(دھارا, " default ");
		توڑو;
	حالت CSS_CURSOR_اشارہ_گر:
		fprintf(دھارا, " pointer ");
		توڑو;
	حالت CSS_CURSOR_MOVE:
		fprintf(دھارا, " move ");
		توڑو;
	حالت CSS_CURSOR_E_RESIZE:
		fprintf(دھارا, " e-resize ");
		توڑو;
	حالت CSS_CURSOR_NE_RESIZE:
		fprintf(دھارا, " ne-resize ");
		توڑو;
	حالت CSS_CURSOR_NW_RESIZE:
		fprintf(دھارا, " nw-resize ");
		توڑو;
	حالت CSS_CURSOR_N_RESIZE:
		fprintf(دھارا, " n-resize ");
		توڑو;
	حالت CSS_CURSOR_SE_RESIZE:
		fprintf(دھارا, " se-resize ");
		توڑو;
	حالت CSS_CURSOR_SW_RESIZE:
		fprintf(دھارا, " sw-resize ");
		توڑو;
	حالت CSS_CURSOR_S_RESIZE:
		fprintf(دھارا, " s-resize ");
		توڑو;
	حالت CSS_CURSOR_W_RESIZE:
		fprintf(دھارا, " w-resize ");
		توڑو;
	حالت CSS_CURSOR_متن:
		fprintf(دھارا, " text ");
		توڑو;
	حالت CSS_CURSOR_WAIT:
		fprintf(دھارا, " wait ");
		توڑو;
	حالت CSS_CURSOR_HELP:
		fprintf(دھارا, " help ");
		توڑو;
	حالت CSS_CURSOR_پیش_رفت:
		fprintf(دھارا, " progress ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* direction */
	val = css_computed_direction(style);
	انتخاب (val) {
	حالت CSS_DIRECTION_LTR:
		fprintf(دھارا, "direction: ltr ");
		توڑو;
	حالت CSS_DIRECTION_RTL:
		fprintf(دھارا, "direction: rtl ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* display */
	val = ns_computed_display_static(style);
	انتخاب (val) {
	حالت CSS_DISPLAY_INLINE:
		fprintf(دھارا, "display: inline ");
		توڑو;
	حالت CSS_DISPLAY_بلاک:
		fprintf(دھارا, "display: block ");
		توڑو;
	حالت CSS_DISPLAY_LIST_شے:
		fprintf(دھارا, "display: list-item ");
		توڑو;
	حالت CSS_DISPLAY_تسلسل_اندر:
		fprintf(دھارا, "display: run-in ");
		توڑو;
	حالت CSS_DISPLAY_INLINE_بلاک:
		fprintf(دھارا, "display: inline-block ");
		توڑو;
	حالت CSS_DISPLAY_جدول:
		fprintf(دھارا, "display: table ");
		توڑو;
	حالت CSS_DISPLAY_INLINE_جدول:
		fprintf(دھارا, "display: inline-table ");
		توڑو;
	حالت CSS_DISPLAY_جدول_صف_گروہ:
		fprintf(دھارا, "display: table-row-group ");
		توڑو;
	حالت CSS_DISPLAY_جدول_سرخی_گروہ:
		fprintf(دھارا, "display: table-header-group ");
		توڑو;
	حالت CSS_DISPLAY_جدول_FOOTER_گروہ:
		fprintf(دھارا, "display: table-footer-group ");
		توڑو;
	حالت CSS_DISPLAY_جدول_صف:
		fprintf(دھارا, "display: table-row ");
		توڑو;
	حالت CSS_DISPLAY_جدول_عمود_گروہ:
		fprintf(دھارا, "display: table-column-group ");
		توڑو;
	حالت CSS_DISPLAY_جدول_عمود:
		fprintf(دھارا, "display: table-column ");
		توڑو;
	حالت CSS_DISPLAY_جدول_CELL:
		fprintf(دھارا, "display: table-cell ");
		توڑو;
	حالت CSS_DISPLAY_جدول_CAPTION:
		fprintf(دھارا, "display: table-caption ");
		توڑو;
	حالت CSS_DISPLAY_NONE:
		fprintf(دھارا, "display: none ");
		توڑو;
	حالت CSS_DISPLAY_FLEX:
		fprintf(دھارا, "display: flex ");
		توڑو;
	حالت CSS_DISPLAY_INLINE_FLEX:
		fprintf(دھارا, "display: inline-flex ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* empty-cells */
	val = css_computed_خالی_خانے_cells(style);
	انتخاب (val) {
	حالت CSS_خالی_خانے_CELLS_SHOW:
		fprintf(دھارا, "empty-cells: show ");
		توڑو;
	حالت CSS_خالی_خانے_CELLS_HIDE:
		fprintf(دھارا, "empty-cells: hide ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* float */
	val = css_computed_float(style);
	انتخاب (val) {
	حالت CSS_FLOAT_بایاں:
		fprintf(دھارا, "float: left ");
		توڑو;
	حالت CSS_FLOAT_دایاں:
		fprintf(دھارا, "float: right ");
		توڑو;
	حالت CSS_FLOAT_NONE:
		fprintf(دھارا, "float: none ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* font-family */
	val = css_computed_font_family(style, &string_list);
	اگر (val != CSS_FONT_FAMILY_INHERIT) {
		fprintf(دھارا, "font-family:");

		اگر (string_list != تہی) {
			جب (*string_list != تہی) {
				fprintf(دھارا, " \"%.*s\"",
					(صحیح) lwc_string_لمبائی(*string_list),
					lwc_string_ڈیٹا(*string_list));

				string_list++;
			}
		}
		انتخاب (val) {
		حالت CSS_FONT_FAMILY_SERIF:
			fprintf(دھارا, " serif ");
			توڑو;
		حالت CSS_FONT_FAMILY_SANS_SERIF:
			fprintf(دھارا, " sans-serif ");
			توڑو;
		حالت CSS_FONT_FAMILY_CURSIVE:
			fprintf(دھارا, " cursive ");
			توڑو;
		حالت CSS_FONT_FAMILY_FANTASY:
			fprintf(دھارا, " fantasy ");
			توڑو;
		حالت CSS_FONT_FAMILY_MONOSPACE:
			fprintf(دھارا, " monospace ");
			توڑو;
		}
	}

	/* font-size */
	val = css_computed_font_سائز(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_FONT_سائز_XX_SMALL:
		fprintf(دھارا, "font-size: xx-small ");
		توڑو;
	حالت CSS_FONT_سائز_X_SMALL:
		fprintf(دھارا, "font-size: x-small ");
		توڑو;
	حالت CSS_FONT_سائز_SMALL:
		fprintf(دھارا, "font-size: small ");
		توڑو;
	حالت CSS_FONT_سائز_MEDIUM:
		fprintf(دھارا, "font-size: medium ");
		توڑو;
	حالت CSS_FONT_سائز_LARGE:
		fprintf(دھارا, "font-size: large ");
		توڑو;
	حالت CSS_FONT_سائز_X_LARGE:
		fprintf(دھارا, "font-size: x-large ");
		توڑو;
	حالت CSS_FONT_سائز_XX_LARGE:
		fprintf(دھارا, "font-size: xx-large ");
		توڑو;
	حالت CSS_FONT_سائز_LARGER:
		fprintf(دھارا, "font-size: larger ");
		توڑو;
	حالت CSS_FONT_سائز_SMALLER:
		fprintf(دھارا, "font-size: smaller ");
		توڑو;
	حالت CSS_FONT_سائز_DIMENSION:
		fprintf(دھارا, "font-size: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* font-style */
	val = css_computed_font_style(style);
	انتخاب (val) {
	حالت CSS_FONT_STYLE_معمول:
		fprintf(دھارا, "font-style: normal ");
		توڑو;
	حالت CSS_FONT_STYLE_ITALIC:
		fprintf(دھارا, "font-style: italic ");
		توڑو;
	حالت CSS_FONT_STYLE_OBLIQUE:
		fprintf(دھارا, "font-style: oblique ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* font-variant */
	val = css_computed_font_variant(style);
	انتخاب (val) {
	حالت CSS_FONT_VARIANT_معمول:
		fprintf(دھارا, "font-variant: normal ");
		توڑو;
	حالت CSS_FONT_VARIANT_SMALL_CAPS:
		fprintf(دھارا, "font-variant: small-caps ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* font-weight */
	val = css_computed_font_وزن(style);
	انتخاب (val) {
	حالت CSS_FONT_وزن_معمول:
		fprintf(دھارا, "font-weight: normal ");
		توڑو;
	حالت CSS_FONT_وزن_BOLD:
		fprintf(دھارا, "font-weight: bold ");
		توڑو;
	حالت CSS_FONT_وزن_BOLDER:
		fprintf(دھارا, "font-weight: bolder ");
		توڑو;
	حالت CSS_FONT_وزن_LIGHTER:
		fprintf(دھارا, "font-weight: lighter ");
		توڑو;
	حالت CSS_FONT_وزن_100:
		fprintf(دھارا, "font-weight: 100 ");
		توڑو;
	حالت CSS_FONT_وزن_200:
		fprintf(دھارا, "font-weight: 200 ");
		توڑو;
	حالت CSS_FONT_وزن_300:
		fprintf(دھارا, "font-weight: 300 ");
		توڑو;
	حالت CSS_FONT_وزن_400:
		fprintf(دھارا, "font-weight: 400 ");
		توڑو;
	حالت CSS_FONT_وزن_500:
		fprintf(دھارا, "font-weight: 500 ");
		توڑو;
	حالت CSS_FONT_وزن_600:
		fprintf(دھارا, "font-weight: 600 ");
		توڑو;
	حالت CSS_FONT_وزن_700:
		fprintf(دھارا, "font-weight: 700 ");
		توڑو;
	حالت CSS_FONT_وزن_800:
		fprintf(دھارا, "font-weight: 800 ");
		توڑو;
	حالت CSS_FONT_وزن_900:
		fprintf(دھارا, "font-weight: 900 ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/*اونچائی*/
	val = css_computed_اونچائی(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_اونچائی_AUTO:
		fprintf(دھارا, "height: auto ");
		توڑو;
	حالت CSS_اونچائی_سیٹ:
		fprintf(دھارا, "height: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/*بایاں*/
	val = css_computed_بایاں(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_بایاں_AUTO:
		fprintf(دھارا, "left: auto ");
		توڑو;
	حالت CSS_بایاں_سیٹ:
		fprintf(دھارا, "left: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* letter-spacing */
	val = css_computed_letter_spacing(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_LETTER_SPACING_معمول:
		fprintf(دھارا, "letter-spacing: normal ");
		توڑو;
	حالت CSS_LETTER_SPACING_سیٹ:
		fprintf(دھارا, "letter-spacing: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* line-height */
	val = css_computed_سطر_اونچائی(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_سطر_اونچائی_معمول:
		fprintf(دھارا, "line-height: normal ");
		توڑو;
	حالت CSS_سطر_اونچائی_نمبر:
		fprintf(دھارا, "line-height: ");

		dump_css_fixed(دھارا, len1);

		fprintf(دھارا, " ");
		توڑو;
	حالت CSS_سطر_اونچائی_DIMENSION:
		fprintf(دھارا, "line-height: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* list-style-image */
	val = css_computed_list_style_image(style, &url);
	اگر (url != تہی) {
		fprintf(دھارا, "list-style-image: url('%.*s') ",
				(صحیح) lwc_string_لمبائی(url),
				lwc_string_ڈیٹا(url));
	} ورنہ اگر (val == CSS_LIST_STYLE_IMAGE_NONE) {
		fprintf(دھارا, "list-style-image: none ");
	}

	/* list-style-position */
	val = css_computed_list_style_position(style);
	انتخاب (val) {
	حالت CSS_LIST_STYLE_POSITION_INSIDE:
		fprintf(دھارا, "list-style-position: inside ");
		توڑو;
	حالت CSS_LIST_STYLE_POSITION_OUTSIDE:
		fprintf(دھارا, "list-style-position: outside ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* list-style-type */
	val = css_computed_list_style_type(style);
	انتخاب (val) {
	حالت CSS_LIST_STYLE_TYPE_DISC:
		fprintf(دھارا, "list-style-type: disc ");
		توڑو;
	حالت CSS_LIST_STYLE_TYPE_CIRCLE:
		fprintf(دھارا, "list-style-type: circle ");
		توڑو;
	حالت CSS_LIST_STYLE_TYPE_مربع:
		fprintf(دھارا, "list-style-type: square ");
		توڑو;
	حالت CSS_LIST_STYLE_TYPE_DECIMAL:
		fprintf(دھارا, "list-style-type: decimal ");
		توڑو;
	حالت CSS_LIST_STYLE_TYPE_DECIMAL_LEADING_ZERO:
		fprintf(دھارا, "list-style-type: decimal-leading-zero ");
		توڑو;
	حالت CSS_LIST_STYLE_TYPE_زیریں_ROMAN:
		fprintf(دھارا, "list-style-type: lower-roman ");
		توڑو;
	حالت CSS_LIST_STYLE_TYPE_بالائی_ROMAN:
		fprintf(دھارا, "list-style-type: upper-roman ");
		توڑو;
	حالت CSS_LIST_STYLE_TYPE_زیریں_GREEK:
		fprintf(دھارا, "list-style-type: lower-greek ");
		توڑو;
	حالت CSS_LIST_STYLE_TYPE_زیریں_LATIN:
		fprintf(دھارا, "list-style-type: lower-latin ");
		توڑو;
	حالت CSS_LIST_STYLE_TYPE_بالائی_LATIN:
		fprintf(دھارا, "list-style-type: upper-latin ");
		توڑو;
	حالت CSS_LIST_STYLE_TYPE_ARMENIAN:
		fprintf(دھارا, "list-style-type: armenian ");
		توڑو;
	حالت CSS_LIST_STYLE_TYPE_GEORGIAN:
		fprintf(دھارا, "list-style-type: georgian ");
		توڑو;
	حالت CSS_LIST_STYLE_TYPE_زیریں_ALPHA:
		fprintf(دھارا, "list-style-type: lower-alpha ");
		توڑو;
	حالت CSS_LIST_STYLE_TYPE_بالائی_ALPHA:
		fprintf(دھارا, "list-style-type: upper-alpha ");
		توڑو;
	حالت CSS_LIST_STYLE_TYPE_NONE:
		fprintf(دھارا, "list-style-type: none ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* margin-top */
	val = css_computed_margin_چوٹی(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_MARGIN_AUTO:
		fprintf(دھارا, "margin-top: auto ");
		توڑو;
	حالت CSS_MARGIN_سیٹ:
		fprintf(دھارا, "margin-top: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* margin-right */
	val = css_computed_margin_دایاں(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_MARGIN_AUTO:
		fprintf(دھارا, "margin-right: auto ");
		توڑو;
	حالت CSS_MARGIN_سیٹ:
		fprintf(دھارا, "margin-right: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* margin-bottom */
	val = css_computed_margin_bottom(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_MARGIN_AUTO:
		fprintf(دھارا, "margin-bottom: auto ");
		توڑو;
	حالت CSS_MARGIN_سیٹ:
		fprintf(دھارا, "margin-bottom: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* margin-left */
	val = css_computed_margin_بایاں(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_MARGIN_AUTO:
		fprintf(دھارا, "margin-left: auto ");
		توڑو;
	حالت CSS_MARGIN_سیٹ:
		fprintf(دھارا, "margin-left: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* max-height */
	val = css_computed_max_اونچائی(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_MAX_اونچائی_NONE:
		fprintf(دھارا, "max-height: none ");
		توڑو;
	حالت CSS_MAX_اونچائی_سیٹ:
		fprintf(دھارا, "max-height: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* max-width */
	val = css_computed_max_چوڑائی(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_MAX_چوڑائی_NONE:
		fprintf(دھارا, "max-width: none ");
		توڑو;
	حالت CSS_MAX_چوڑائی_سیٹ:
		fprintf(دھارا, "max-width: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* min-height */
	val = ns_computed_min_اونچائی(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_MIN_اونچائی_سیٹ:
		fprintf(دھارا, "min-height: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* min-width */
	val = ns_computed_min_چوڑائی(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_MIN_چوڑائی_سیٹ:
		fprintf(دھارا, "min-width: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* opacity */
	val = css_computed_opacity(style, &len1);
	انتخاب (val) {
	حالت CSS_OPACITY_سیٹ:
		fprintf(دھارا, "opacity: ");

		dump_css_fixed(دھارا, len1);

		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* outline-color */
	val = css_computed_outline_color(style, &color);
	انتخاب (val) {
	حالت CSS_OUTLINE_COLOR_INVERT:
		fprintf(دھارا, "outline-color: invert ");
		توڑو;
	حالت CSS_OUTLINE_COLOR_COLOR:
		fprintf(دھارا, "outline-color: #%08"PRIx32" ", color);
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* outline-style */
	val = css_computed_outline_style(style);
	انتخاب (val) {
	حالت CSS_OUTLINE_STYLE_NONE:
		fprintf(دھارا, "outline-style: none ");
		توڑو;
	حالت CSS_OUTLINE_STYLE_DOTTED:
		fprintf(دھارا, "outline-style: dotted ");
		توڑو;
	حالت CSS_OUTLINE_STYLE_DASHED:
		fprintf(دھارا, "outline-style: dashed ");
		توڑو;
	حالت CSS_OUTLINE_STYLE_SOLID:
		fprintf(دھارا, "outline-style: solid ");
		توڑو;
	حالت CSS_OUTLINE_STYLE_DOUBLE:
		fprintf(دھارا, "outline-style: double ");
		توڑو;
	حالت CSS_OUTLINE_STYLE_GROOVE:
		fprintf(دھارا, "outline-style: groove ");
		توڑو;
	حالت CSS_OUTLINE_STYLE_RIDGE:
		fprintf(دھارا, "outline-style: ridge ");
		توڑو;
	حالت CSS_OUTLINE_STYLE_INSET:
		fprintf(دھارا, "outline-style: inset ");
		توڑو;
	حالت CSS_OUTLINE_STYLE_OUTSET:
		fprintf(دھارا, "outline-style: outset ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* outline-width */
	val = css_computed_outline_چوڑائی(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_OUTLINE_چوڑائی_THIN:
		fprintf(دھارا, "outline-width: thin ");
		توڑو;
	حالت CSS_OUTLINE_چوڑائی_MEDIUM:
		fprintf(دھارا, "outline-width: medium ");
		توڑو;
	حالت CSS_OUTLINE_چوڑائی_THICK:
		fprintf(دھارا, "outline-width: thick ");
		توڑو;
	حالت CSS_OUTLINE_چوڑائی_چوڑائی:
		fprintf(دھارا, "outline-width: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/*حد_سے_تجاوز*/
	val = css_computed_حد_سے_تجاوز_x(style);
	انتخاب (val) {
	حالت CSS_حد_سے_تجاوز_VISIBLE:
		fprintf(دھارا, "overflow-x: visible ");
		توڑو;
	حالت CSS_حد_سے_تجاوز_HIDDEN:
		fprintf(دھارا, "overflow-x: hidden ");
		توڑو;
	حالت CSS_حد_سے_تجاوز_SCROLL:
		fprintf(دھارا, "overflow-x: scroll ");
		توڑو;
	حالت CSS_حد_سے_تجاوز_AUTO:
		fprintf(دھارا, "overflow-x auto ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/*حد_سے_تجاوز*/
	val = css_computed_حد_سے_تجاوز_y(style);
	انتخاب (val) {
	حالت CSS_حد_سے_تجاوز_VISIBLE:
		fprintf(دھارا, "overflow-y: visible ");
		توڑو;
	حالت CSS_حد_سے_تجاوز_HIDDEN:
		fprintf(دھارا, "overflow-y: hidden ");
		توڑو;
	حالت CSS_حد_سے_تجاوز_SCROLL:
		fprintf(دھارا, "overflow-y: scroll ");
		توڑو;
	حالت CSS_حد_سے_تجاوز_AUTO:
		fprintf(دھارا, "overflow-y: auto ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* padding-top */
	val = css_computed_padding_چوٹی(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_PADDING_سیٹ:
		fprintf(دھارا, "padding-top: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* padding-right */
	val = css_computed_padding_دایاں(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_PADDING_سیٹ:
		fprintf(دھارا, "padding-right: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* padding-bottom */
	val = css_computed_padding_bottom(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_PADDING_سیٹ:
		fprintf(دھارا, "padding-bottom: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* padding-left */
	val = css_computed_padding_بایاں(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_PADDING_سیٹ:
		fprintf(دھارا, "padding-left: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* position */
	val = css_computed_position(style);
	انتخاب (val) {
	حالت CSS_POSITION_STATIC:
		fprintf(دھارا, "position: static ");
		توڑو;
	حالت CSS_POSITION_RELATIVE:
		fprintf(دھارا, "position: relative ");
		توڑو;
	حالت CSS_POSITION_ABSOLUTE:
		fprintf(دھارا, "position: absolute ");
		توڑو;
	حالت CSS_POSITION_FIXED:
		fprintf(دھارا, "position: fixed ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* quotes */
	val = css_computed_quotes(style, &string_list);
	اگر (val == CSS_QUOTES_STRING && string_list != تہی) {
		fprintf(دھارا, "quotes:");

		جب (*string_list != تہی) {
			fprintf(دھارا, " \"%.*s\"",
				(صحیح) lwc_string_لمبائی(*string_list),
				lwc_string_ڈیٹا(*string_list));

			string_list++;
		}

		fprintf(دھارا, " ");
	} ورنہ {
		انتخاب (val) {
		حالت CSS_QUOTES_NONE:
			fprintf(دھارا, "quotes: none ");
			توڑو;
		طےشدہ:
			توڑو;
		}
	}

	/*دایاں*/
	val = css_computed_دایاں(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_دایاں_AUTO:
		fprintf(دھارا, "right: auto ");
		توڑو;
	حالت CSS_دایاں_سیٹ:
		fprintf(دھارا, "right: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* table-layout */
	val = css_computed_جدول_layout(style);
	انتخاب (val) {
	حالت CSS_جدول_LAYOUT_AUTO:
		fprintf(دھارا, "table-layout: auto ");
		توڑو;
	حالت CSS_جدول_LAYOUT_FIXED:
		fprintf(دھارا, "table-layout: fixed ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* text-align */
	val = css_computed_متن_align(style);
	انتخاب (val) {
	حالت CSS_متن_ALIGN_بایاں:
		fprintf(دھارا, "text-align: left ");
		توڑو;
	حالت CSS_متن_ALIGN_دایاں:
		fprintf(دھارا, "text-align: right ");
		توڑو;
	حالت CSS_متن_ALIGN_CENTER:
		fprintf(دھارا, "text-align: center ");
		توڑو;
	حالت CSS_متن_ALIGN_JUSTIFY:
		fprintf(دھارا, "text-align: justify ");
		توڑو;
	حالت CSS_متن_ALIGN_DEFAULT:
		fprintf(دھارا, "text-align: default ");
		توڑو;
	حالت CSS_متن_ALIGN_LIBCSS_بایاں:
		fprintf(دھارا, "text-align: -libcss-left ");
		توڑو;
	حالت CSS_متن_ALIGN_LIBCSS_CENTER:
		fprintf(دھارا, "text-align: -libcss-center ");
		توڑو;
	حالت CSS_متن_ALIGN_LIBCSS_دایاں:
		fprintf(دھارا, "text-align: -libcss-right ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* text-decoration */
	val = css_computed_متن_decoration(style);
	اگر (val == CSS_متن_DECORATION_NONE) {
		fprintf(دھارا, "text-decoration: none ");
	} ورنہ {
		fprintf(دھارا, "text-decoration:");

		اگر (val & CSS_متن_DECORATION_BLINK) {
			fprintf(دھارا, " blink");
		}
		اگر (val & CSS_متن_DECORATION_سطر_THROUGH) {
			fprintf(دھارا, " line-through");
		}
		اگر (val & CSS_متن_DECORATION_OVERLINE) {
			fprintf(دھارا, " overline");
		}
		اگر (val & CSS_متن_DECORATION_UNDERLINE) {
			fprintf(دھارا, " underline");
		}

		fprintf(دھارا, " ");
	}

	/* text-indent */
	val = css_computed_متن_indent(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_متن_INDENT_سیٹ:
		fprintf(دھارا, "text-indent: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* text-transform */
	val = css_computed_متن_transform(style);
	انتخاب (val) {
	حالت CSS_متن_TRANSFORM_CAPITALIZE:
		fprintf(دھارا, "text-transform: capitalize ");
		توڑو;
	حالت CSS_متن_TRANSFORM_UPPERCASE:
		fprintf(دھارا, "text-transform: uppercase ");
		توڑو;
	حالت CSS_متن_TRANSFORM_LOWERCASE:
		fprintf(دھارا, "text-transform: lowercase ");
		توڑو;
	حالت CSS_متن_TRANSFORM_NONE:
		fprintf(دھارا, "text-transform: none ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/*چوٹی*/
	val = css_computed_چوٹی(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_چوٹی_AUTO:
		fprintf(دھارا, "top: auto ");
		توڑو;
	حالت CSS_چوٹی_سیٹ:
		fprintf(دھارا, "top: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* unicode-bidi */
	val = css_computed_unicode_bidi(style);
	انتخاب (val) {
	حالت CSS_UNICODE_BIDI_معمول:
		fprintf(دھارا, "unicode-bidi: normal ");
		توڑو;
	حالت CSS_UNICODE_BIDI_EMBED:
		fprintf(دھارا, "unicode-bidi: embed ");
		توڑو;
	حالت CSS_UNICODE_BIDI_BIDI_OVERRIDE:
		fprintf(دھارا, "unicode-bidi: bidi-override ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* vertical-align */
	val = css_computed_vertical_align(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_VERTICAL_ALIGN_BASELINE:
		fprintf(دھارا, "vertical-align: baseline ");
		توڑو;
	حالت CSS_VERTICAL_ALIGN_SUB:
		fprintf(دھارا, "vertical-align: sub ");
		توڑو;
	حالت CSS_VERTICAL_ALIGN_SUPER:
		fprintf(دھارا, "vertical-align: super ");
		توڑو;
	حالت CSS_VERTICAL_ALIGN_چوٹی:
		fprintf(دھارا, "vertical-align: top ");
		توڑو;
	حالت CSS_VERTICAL_ALIGN_متن_چوٹی:
		fprintf(دھارا, "vertical-align: text-top ");
		توڑو;
	حالت CSS_VERTICAL_ALIGN_درمیان:
		fprintf(دھارا, "vertical-align: middle ");
		توڑو;
	حالت CSS_VERTICAL_ALIGN_BOTTOM:
		fprintf(دھارا, "vertical-align: bottom ");
		توڑو;
	حالت CSS_VERTICAL_ALIGN_متن_BOTTOM:
		fprintf(دھارا, "vertical-align: text-bottom ");
		توڑو;
	حالت CSS_VERTICAL_ALIGN_سیٹ:
		fprintf(دھارا, "vertical-align: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* visibility */
	val = css_computed_visibility(style);
	انتخاب (val) {
	حالت CSS_VISIBILITY_VISIBLE:
		fprintf(دھارا, "visibility: visible ");
		توڑو;
	حالت CSS_VISIBILITY_HIDDEN:
		fprintf(دھارا, "visibility: hidden ");
		توڑو;
	حالت CSS_VISIBILITY_COLLAPSE:
		fprintf(دھارا, "visibility: collapse ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* white-space */
	val = css_computed_white_space(style);
	انتخاب (val) {
	حالت CSS_WHITE_SPACE_معمول:
		fprintf(دھارا, "white-space: normal ");
		توڑو;
	حالت CSS_WHITE_SPACE_PRE:
		fprintf(دھارا, "white-space: pre ");
		توڑو;
	حالت CSS_WHITE_SPACE_NOWRAP:
		fprintf(دھارا, "white-space: nowrap ");
		توڑو;
	حالت CSS_WHITE_SPACE_PRE_WRAP:
		fprintf(دھارا, "white-space: pre-wrap ");
		توڑو;
	حالت CSS_WHITE_SPACE_PRE_سطر:
		fprintf(دھارا, "white-space: pre-line ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/*چوڑائی*/
	val = css_computed_چوڑائی(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_چوڑائی_AUTO:
		fprintf(دھارا, "width: auto ");
		توڑو;
	حالت CSS_چوڑائی_سیٹ:
		fprintf(دھارا, "width: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* word-spacing */
	val = css_computed_لفظ_spacing(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_لفظ_SPACING_معمول:
		fprintf(دھارا, "word-spacing: normal ");
		توڑو;
	حالت CSS_لفظ_SPACING_سیٹ:
		fprintf(دھارا, "word-spacing: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* z-index */
	val = css_computed_z_اشاریہ(style, &zindex);
	انتخاب (val) {
	حالت CSS_Z_اشاریہ_AUTO:
		fprintf(دھارا, "z-index: auto ");
		توڑو;
	حالت CSS_Z_اشاریہ_سیٹ:
		fprintf(دھارا, "z-index: %"PRId32" ", zindex);
		توڑو;
	طےشدہ:
		توڑو;
	}

	fprintf(دھارا, "}");
}
