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

#ضمن <stdio.h>
#ضمن <libcss/libcss.h>

#ضمن "netsurf/inttypes.h"

#ضمن "css/dump.h"
#ضمن "css/utils.h"

/**
 * Dump a fixed point value to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param f       Value to write
 */
ساكن فراغ dump_css_fixed(FILE *دھارا, css_fixed f)
{
#تعریف_کرو NSCSS_ABS(x) (uint32_t)((x) < 0 ? -(x) : (x))
	uint32_t uintpart = FIXTOINT(NSCSS_ABS(f));
	/* + 500 to ensure round to nearest (division will truncate) */
	uint32_t fracpart = ((NSCSS_ABS(f) & 0x3ff) * 1000 + 500) / (1 << 10);
#تعریف_رد NSCSS_ABS

	fprintf(دھارا, "%s%"PRIu32".%03"PRIu32, f < 0 ? "-" : "", uintpart, fracpart);
}

/**
 * Dump a numeric value to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 */
ساكن فراغ dump_css_نمبر(FILE *دھارا, css_fixed val)
{
	إذا (INTTOFIX(FIXTOINT(val)) == val)
		fprintf(دھارا, "%"PRId32, FIXTOINT(val));
	وإلا
		dump_css_fixed(دھارا, val);
}

/**
 * Dump a dimension to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 * \param unit    Unit to write
 */
ساكن فراغ dump_css_unit(FILE *دھارا, css_fixed val, css_unit unit)
{
	dump_css_نمبر(دھارا, val);

	بدل (unit) {
	حالة CSS_UNIT_PX:
		fprintf(دھارا, "px");
		اكسر;
	حالة CSS_UNIT_EX:
		fprintf(دھارا, "ex");
		اكسر;
	حالة CSS_UNIT_EM:
		fprintf(دھارا, "em");
		اكسر;
	حالة CSS_UNIT_وچ:
		fprintf(دھارا, "in");
		اكسر;
	حالة CSS_UNIT_CM:
		fprintf(دھارا, "cm");
		اكسر;
	حالة CSS_UNIT_MM:
		fprintf(دھارا, "mm");
		اكسر;
	حالة CSS_UNIT_PT:
		fprintf(دھارا, "pt");
		اكسر;
	حالة CSS_UNIT_PC:
		fprintf(دھارا, "pc");
		اكسر;
	حالة CSS_UNIT_PCT:
		fprintf(دھارا, "%%");
		اكسر;
	حالة CSS_UNIT_DEG:
		fprintf(دھارا, "deg");
		اكسر;
	حالة CSS_UNIT_GRAD:
		fprintf(دھارا, "grad");
		اكسر;
	حالة CSS_UNIT_RAD:
		fprintf(دھارا, "rad");
		اكسر;
	حالة CSS_UNIT_MS:
		fprintf(دھارا, "ms");
		اكسر;
	حالة CSS_UNIT_S:
		fprintf(دھارا, "s");
		اكسر;
	حالة CSS_UNIT_HZ:
		fprintf(دھارا, "Hz");
		اكسر;
	حالة CSS_UNIT_KHZ:
		fprintf(دھارا, "kHz");
		اكسر;
	حالة CSS_UNIT_CH:
		fprintf(دھارا, "ch");
		اكسر;
	حالة CSS_UNIT_REM:
		fprintf(دھارا, "rem");
		اكسر;
	حالة CSS_UNIT_LH:
		fprintf(دھارا, "lh");
		اكسر;
	حالة CSS_UNIT_VH:
		fprintf(دھارا, "vh");
		اكسر;
	حالة CSS_UNIT_VW:
		fprintf(دھارا, "vw");
		اكسر;
	حالة CSS_UNIT_VI:
		fprintf(دھارا, "vi");
		اكسر;
	حالة CSS_UNIT_VB:
		fprintf(دھارا, "vb");
		اكسر;
	حالة CSS_UNIT_VMIN:
		fprintf(دھارا, "vmin");
		اكسر;
	حالة CSS_UNIT_VMAX:
		fprintf(دھارا, "vmax");
		اكسر;
	حالة CSS_UNIT_Q:
		fprintf(دھارا, "q");
		اكسر;
	حالة CSS_UNIT_CALC:
		fprintf(دھارا, "calc()");
		اكسر;
	}
}

/* exported interface documented in content/handlers/css/dump.h */
فراغ nscss_dump_computed_style(FILE *دھارا, ثابت css_computed_style *style)
{
	uint8_t val;
	css_color color = 0;
	lwc_string *url = عدم;
	css_fixed len1 = 0, len2 = 0;
	css_unit unit1 = CSS_UNIT_PX, unit2 = CSS_UNIT_PX;
	css_computed_clip_rect rect = { 0, 0, 0, 0, CSS_UNIT_PX, CSS_UNIT_PX,
					CSS_UNIT_PX, CSS_UNIT_PX, true, true,
					true, true };
	ثابت css_computed_content_شے *content = عدم;
	ثابت css_computed_counter *counter = عدم;
	lwc_string **string_list = عدم;
	int32_t zindex = 0;

	fprintf(دھارا, "{ ");

	/* background-attachment */
	val = css_computed_background_attachment(style);
	بدل (val) {
	حالة CSS_BACKGROUND_ATTACHMENT_FIXED:
		fprintf(دھارا, "background-attachment: fixed ");
		اكسر;
	حالة CSS_BACKGROUND_ATTACHMENT_SCROLL:
		fprintf(دھارا, "background-attachment: scroll ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* background-color */
	val = css_computed_background_color(style, &color);
	بدل (val) {
	حالة CSS_BACKGROUND_COLOR_COLOR:
		fprintf(دھارا, "background-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* background-image */
	val = css_computed_background_image(style, &url);
	إذا (val == CSS_BACKGROUND_IMAGE_IMAGE && url != عدم) {
		fprintf(دھارا, "background-image: url('%.*s') ",
				(صحيح) lwc_string_لمائی(url),
				lwc_string_انکڑے(url));
	} وإلا إذا (val == CSS_BACKGROUND_IMAGE_NONE) {
		fprintf(دھارا, "background-image: none ");
	}

	/* background-position */
	val = css_computed_background_position(style, &len1, &unit1,
			&len2, &unit2);
	إذا (val == CSS_BACKGROUND_POSITION_سیٹ) {
		fprintf(دھارا, "background-position: ");
		dump_css_unit(دھارا, len1, unit1);
		fprintf(دھارا, " ");
		dump_css_unit(دھارا, len2, unit2);
		fprintf(دھارا, " ");
	}

	/* background-repeat */
	val = css_computed_background_repeat(style);
	بدل (val) {
	حالة CSS_BACKGROUND_REPEAT_REPEAT_X:
		fprintf(دھارا, "background-repeat: repeat-x ");
		اكسر;
	حالة CSS_BACKGROUND_REPEAT_REPEAT_Y:
		fprintf(دھارا, "background-repeat: repeat-y ");
		اكسر;
	حالة CSS_BACKGROUND_REPEAT_REPEAT:
		fprintf(دھارا, "background-repeat: repeat ");
		اكسر;
	حالة CSS_BACKGROUND_REPEAT_NO_REPEAT:
		fprintf(دھارا, "background-repeat: no-repeat ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-collapse */
	val = css_computed_border_collapse(style);
	بدل (val) {
	حالة CSS_BORDER_COLLAPSE_SEPARATE:
		fprintf(دھارا, "border-collapse: separate ");
		اكسر;
	حالة CSS_BORDER_COLLAPSE_COLLAPSE:
		fprintf(دھارا, "border-collapse: collapse ");
		اكسر;
	افتراضي:

		اكسر;
	}

	/* border-spacing */
	val = css_computed_border_spacing(style, &len1, &unit1, &len2, &unit2);
	إذا (val == CSS_BORDER_SPACING_سیٹ) {
		fprintf(دھارا, "border-spacing: ");
		dump_css_unit(دھارا, len1, unit1);
		fprintf(دھارا, " ");
		dump_css_unit(دھارا, len2, unit2);
		fprintf(دھارا, " ");
	}

	/* border-top-color */
	val = css_computed_border_چوٹی_color(style, &color);
	بدل (val) {
	حالة CSS_BORDER_COLOR_COLOR:
		fprintf(دھارا, "border-top-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-right-color */
	val = css_computed_border_دایاں_color(style, &color);
	بدل (val) {
	حالة CSS_BORDER_COLOR_COLOR:
		fprintf(دھارا, "border-right-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-bottom-color */
	val = css_computed_border_bottom_color(style, &color);
	بدل (val) {
	حالة CSS_BORDER_COLOR_COLOR:
		fprintf(دھارا, "border-bottom-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-left-color */
	val = css_computed_border_بایاں_color(style, &color);
	بدل (val) {
	حالة CSS_BORDER_COLOR_COLOR:
		fprintf(دھارا, "border-left-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-top-style */
	val = css_computed_border_چوٹی_style(style);
	بدل (val) {
	حالة CSS_BORDER_STYLE_NONE:
		fprintf(دھارا, "border-top-style: none ");
		اكسر;
	حالة CSS_BORDER_STYLE_HIDDEN:
		fprintf(دھارا, "border-top-style: hidden ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOTTED:
		fprintf(دھارا, "border-top-style: dotted ");
		اكسر;
	حالة CSS_BORDER_STYLE_DASHED:
		fprintf(دھارا, "border-top-style: dashed ");
		اكسر;
	حالة CSS_BORDER_STYLE_SOLID:
		fprintf(دھارا, "border-top-style: solid ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOUBLE:
		fprintf(دھارا, "border-top-style: double ");
		اكسر;
	حالة CSS_BORDER_STYLE_GROOVE:
		fprintf(دھارا, "border-top-style: groove ");
		اكسر;
	حالة CSS_BORDER_STYLE_RIDGE:
		fprintf(دھارا, "border-top-style: ridge ");
		اكسر;
	حالة CSS_BORDER_STYLE_INSET:
		fprintf(دھارا, "border-top-style: inset ");
		اكسر;
	حالة CSS_BORDER_STYLE_OUTSET:
		fprintf(دھارا, "border-top-style: outset ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-right-style */
	val = css_computed_border_دایاں_style(style);
	بدل (val) {
	حالة CSS_BORDER_STYLE_NONE:
		fprintf(دھارا, "border-right-style: none ");
		اكسر;
	حالة CSS_BORDER_STYLE_HIDDEN:
		fprintf(دھارا, "border-right-style: hidden ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOTTED:
		fprintf(دھارا, "border-right-style: dotted ");
		اكسر;
	حالة CSS_BORDER_STYLE_DASHED:
		fprintf(دھارا, "border-right-style: dashed ");
		اكسر;
	حالة CSS_BORDER_STYLE_SOLID:
		fprintf(دھارا, "border-right-style: solid ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOUBLE:
		fprintf(دھارا, "border-right-style: double ");
		اكسر;
	حالة CSS_BORDER_STYLE_GROOVE:
		fprintf(دھارا, "border-right-style: groove ");
		اكسر;
	حالة CSS_BORDER_STYLE_RIDGE:
		fprintf(دھارا, "border-right-style: ridge ");
		اكسر;
	حالة CSS_BORDER_STYLE_INSET:
		fprintf(دھارا, "border-right-style: inset ");
		اكسر;
	حالة CSS_BORDER_STYLE_OUTSET:
		fprintf(دھارا, "border-right-style: outset ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-bottom-style */
	val = css_computed_border_bottom_style(style);
	بدل (val) {
	حالة CSS_BORDER_STYLE_NONE:
		fprintf(دھارا, "border-bottom-style: none ");
		اكسر;
	حالة CSS_BORDER_STYLE_HIDDEN:
		fprintf(دھارا, "border-bottom-style: hidden ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOTTED:
		fprintf(دھارا, "border-bottom-style: dotted ");
		اكسر;
	حالة CSS_BORDER_STYLE_DASHED:
		fprintf(دھارا, "border-bottom-style: dashed ");
		اكسر;
	حالة CSS_BORDER_STYLE_SOLID:
		fprintf(دھارا, "border-bottom-style: solid ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOUBLE:
		fprintf(دھارا, "border-bottom-style: double ");
		اكسر;
	حالة CSS_BORDER_STYLE_GROOVE:
		fprintf(دھارا, "border-bottom-style: groove ");
		اكسر;
	حالة CSS_BORDER_STYLE_RIDGE:
		fprintf(دھارا, "border-bottom-style: ridge ");
		اكسر;
	حالة CSS_BORDER_STYLE_INSET:
		fprintf(دھارا, "border-bottom-style: inset ");
		اكسر;
	حالة CSS_BORDER_STYLE_OUTSET:
		fprintf(دھارا, "border-bottom-style: outset ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-left-style */
	val = css_computed_border_بایاں_style(style);
	بدل (val) {
	حالة CSS_BORDER_STYLE_NONE:
		fprintf(دھارا, "border-left-style: none ");
		اكسر;
	حالة CSS_BORDER_STYLE_HIDDEN:
		fprintf(دھارا, "border-left-style: hidden ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOTTED:
		fprintf(دھارا, "border-left-style: dotted ");
		اكسر;
	حالة CSS_BORDER_STYLE_DASHED:
		fprintf(دھارا, "border-left-style: dashed ");
		اكسر;
	حالة CSS_BORDER_STYLE_SOLID:
		fprintf(دھارا, "border-left-style: solid ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOUBLE:
		fprintf(دھارا, "border-left-style: double ");
		اكسر;
	حالة CSS_BORDER_STYLE_GROOVE:
		fprintf(دھارا, "border-left-style: groove ");
		اكسر;
	حالة CSS_BORDER_STYLE_RIDGE:
		fprintf(دھارا, "border-left-style: ridge ");
		اكسر;
	حالة CSS_BORDER_STYLE_INSET:
		fprintf(دھارا, "border-left-style: inset ");
		اكسر;
	حالة CSS_BORDER_STYLE_OUTSET:
		fprintf(دھارا, "border-left-style: outset ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-top-width */
	val = css_computed_border_چوٹی_چوڑائی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_BORDER_چوڑائی_THIN:
		fprintf(دھارا, "border-top-width: thin ");
		اكسر;
	حالة CSS_BORDER_چوڑائی_MEDIUM:
		fprintf(دھارا, "border-top-width: medium ");
		اكسر;
	حالة CSS_BORDER_چوڑائی_THICK:
		fprintf(دھارا, "border-top-width: thick ");
		اكسر;
	حالة CSS_BORDER_چوڑائی_چوڑائی:
		fprintf(دھارا, "border-top-width: ");
		dump_css_unit(دھارا, len1, unit1);
		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-right-width */
	val = css_computed_border_دایاں_چوڑائی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_BORDER_چوڑائی_THIN:
		fprintf(دھارا, "border-right-width: thin ");
		اكسر;
	حالة CSS_BORDER_چوڑائی_MEDIUM:
		fprintf(دھارا, "border-right-width: medium ");
		اكسر;
	حالة CSS_BORDER_چوڑائی_THICK:
		fprintf(دھارا, "border-right-width: thick ");
		اكسر;
	حالة CSS_BORDER_چوڑائی_چوڑائی:
		fprintf(دھارا, "border-right-width: ");
		dump_css_unit(دھارا, len1, unit1);
		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-bottom-width */
	val = css_computed_border_bottom_چوڑائی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_BORDER_چوڑائی_THIN:
		fprintf(دھارا, "border-bottom-width: thin ");
		اكسر;
	حالة CSS_BORDER_چوڑائی_MEDIUM:
		fprintf(دھارا, "border-bottom-width: medium ");
		اكسر;
	حالة CSS_BORDER_چوڑائی_THICK:
		fprintf(دھارا, "border-bottom-width: thick ");
		اكسر;
	حالة CSS_BORDER_چوڑائی_چوڑائی:
		fprintf(دھارا, "border-bottom-width: ");
		dump_css_unit(دھارا, len1, unit1);
		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-left-width */
	val = css_computed_border_بایاں_چوڑائی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_BORDER_چوڑائی_THIN:
		fprintf(دھارا, "border-left-width: thin ");
		اكسر;
	حالة CSS_BORDER_چوڑائی_MEDIUM:
		fprintf(دھارا, "border-left-width: medium ");
		اكسر;
	حالة CSS_BORDER_چوڑائی_THICK:
		fprintf(دھارا, "border-left-width: thick ");
		اكسر;
	حالة CSS_BORDER_چوڑائی_چوڑائی:
		fprintf(دھارا, "border-left-width: ");
		dump_css_unit(دھارا, len1, unit1);
		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* bottom */
	val = css_computed_bottom(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_BOTTOM_AUTO:
		fprintf(دھارا, "bottom: auto ");
		اكسر;
	حالة CSS_BOTTOM_سیٹ:
		fprintf(دھارا, "bottom: ");
		dump_css_unit(دھارا, len1, unit1);
		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* caption-side */
	val = css_computed_caption_side(style);
	بدل (val) {
	حالة CSS_CAPTION_SIDE_چوٹی:
		fprintf(دھارا, "caption_side: top ");
		اكسر;
	حالة CSS_CAPTION_SIDE_BOTTOM:
		fprintf(دھارا, "caption_side: bottom ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* clear */
	val = css_computed_clear(style);
	بدل (val) {
	حالة CSS_CLEAR_NONE:
		fprintf(دھارا, "clear: none ");
		اكسر;
	حالة CSS_CLEAR_بایاں:
		fprintf(دھارا, "clear: left ");
		اكسر;
	حالة CSS_CLEAR_دایاں:
		fprintf(دھارا, "clear: right ");
		اكسر;
	حالة CSS_CLEAR_BOTH:
		fprintf(دھارا, "clear: both ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* clip */
	val = css_computed_clip(style, &rect);
	بدل (val) {
	حالة CSS_CLIP_AUTO:
		fprintf(دھارا, "clip: auto ");
		اكسر;
	حالة CSS_CLIP_RECT:
		fprintf(دھارا, "clip: rect( ");

		إذا (rect.چوٹی_auto)
			fprintf(دھارا, "auto");
		وإلا
			dump_css_unit(دھارا, rect.چوٹی, rect.tunit);
		fprintf(دھارا, ", ");

		إذا (rect.دایاں_auto)
			fprintf(دھارا, "auto");
		وإلا
			dump_css_unit(دھارا, rect.دایاں, rect.runit);
		fprintf(دھارا, ", ");

		إذا (rect.bottom_auto)
			fprintf(دھارا, "auto");
		وإلا
			dump_css_unit(دھارا, rect.bottom, rect.bunit);
		fprintf(دھارا, ", ");

		إذا (rect.بایاں_auto)
			fprintf(دھارا, "auto");
		وإلا
			dump_css_unit(دھارا, rect.بایاں, rect.lunit);
		fprintf(دھارا, ") ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* color */
	val = css_computed_color(style, &color);
	إذا (val == CSS_COLOR_COLOR) {
		fprintf(دھارا, "color: #%08"PRIx32" ", color);
	}

	/* content */
	val = css_computed_content(style, &content);
	بدل (val) {
	حالة CSS_CONTENT_NONE:
		fprintf(دھارا, "content: none ");
		اكسر;
	حالة CSS_CONTENT_عام:
		fprintf(دھارا, "content: normal ");
		اكسر;
	حالة CSS_CONTENT_سیٹ:
		fprintf(دھارا, "content:");

		طالما (content->type != CSS_COMPUTED_CONTENT_NONE) {
			fprintf(دھارا, " ");

			بدل (content->type) {
			حالة CSS_COMPUTED_CONTENT_STRING:
				fprintf(دھارا,	"\"%.*s\"",
						(صحيح) lwc_string_لمائی(
						content->انکڑے.string),
						lwc_string_انکڑے(
						content->انکڑے.string));
				اكسر;
			حالة CSS_COMPUTED_CONTENT_URI:
				fprintf(دھارا,	"uri(\"%.*s\")",
						(صحيح) lwc_string_لمائی(
						content->انکڑے.uri),
						lwc_string_انکڑے(
						content->انکڑے.uri));
				اكسر;
			حالة CSS_COMPUTED_CONTENT_COUNTER:
				fprintf(دھارا, "counter(%.*s)",
						(صحيح) lwc_string_لمائی(
						content->انکڑے.counter.name),
						lwc_string_انکڑے(
						content->انکڑے.counter.name));
				اكسر;
			حالة CSS_COMPUTED_CONTENT_COUNTERS:
				fprintf(دھارا, "counters(%.*s, \"%.*s\")",
						(صحيح) lwc_string_لمائی(
						content->انکڑے.counters.name),
						lwc_string_انکڑے(
						content->انکڑے.counters.name),
						(صحيح) lwc_string_لمائی(
						content->انکڑے.counters.sep),
						lwc_string_انکڑے(
						content->انکڑے.counters.sep));
				اكسر;
			حالة CSS_COMPUTED_CONTENT_ATTR:
				fprintf(دھارا, "attr(%.*s)",
						(صحيح) lwc_string_لمائی(
						content->انکڑے.attr),
						lwc_string_انکڑے(
						content->انکڑے.attr));
				اكسر;
			حالة CSS_COMPUTED_CONTENT_OPEN_QUOTE:
				fprintf(دھارا, "open-quote");
				اكسر;
			حالة CSS_COMPUTED_CONTENT_CLOSE_QUOTE:
				fprintf(دھارا, "close-quote");
				اكسر;
			حالة CSS_COMPUTED_CONTENT_NO_OPEN_QUOTE:
				fprintf(دھارا, "no-open-quote");
				اكسر;
			حالة CSS_COMPUTED_CONTENT_NO_CLOSE_QUOTE:
				fprintf(دھارا, "no-close-quote");
				اكسر;
			}

			content++;
		}

		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* counter-increment */
	val = css_computed_counter_increment(style, &counter);
	إذا ((val == CSS_COUNTER_INCREMENT_NONE) || (counter == عدم)) {
		fprintf(دھارا, "counter-increment: none ");
	} وإلا {
		fprintf(دھارا, "counter-increment:");

		طالما (counter->name != عدم) {
			fprintf(دھارا, " %.*s ",
					(صحيح) lwc_string_لمائی(counter->name),
					lwc_string_انکڑے(counter->name));

			dump_css_fixed(دھارا, counter->قدر);

			counter++;
		}

		fprintf(دھارا, " ");
	}

	/* counter-reset */
	val = css_computed_counter_reset(style, &counter);
	إذا ((val == CSS_COUNTER_RESET_NONE) || (counter == عدم)) {
		fprintf(دھارا, "counter-reset: none ");
	} وإلا {
		fprintf(دھارا, "counter-reset:");

		طالما (counter->name != عدم) {
			fprintf(دھارا, " %.*s ",
					(صحيح) lwc_string_لمائی(counter->name),
					lwc_string_انکڑے(counter->name));

			dump_css_fixed(دھارا, counter->قدر);

			counter++;
		}

		fprintf(دھارا, " ");
	}

	/* cursor */
	val = css_computed_cursor(style, &string_list);
	fprintf(دھارا, "cursor:");

	إذا (string_list != عدم) {
		طالما (*string_list != عدم) {
			fprintf(دھارا, " url\"%.*s\")",
					(صحيح) lwc_string_لمائی(*string_list),
					lwc_string_انکڑے(*string_list));

			string_list++;
		}
	}
	بدل (val) {
	حالة CSS_CURSOR_AUTO:
		fprintf(دھارا, " auto ");
		اكسر;
	حالة CSS_CURSOR_CROSSHAIR:
		fprintf(دھارا, " crosshair ");
		اكسر;
	حالة CSS_CURSOR_DEFAULT:
		fprintf(دھارا, " default ");
		اكسر;
	حالة CSS_CURSOR_اشارہ_گر:
		fprintf(دھارا, " pointer ");
		اكسر;
	حالة CSS_CURSOR_MOVE:
		fprintf(دھارا, " move ");
		اكسر;
	حالة CSS_CURSOR_E_RESIZE:
		fprintf(دھارا, " e-resize ");
		اكسر;
	حالة CSS_CURSOR_NE_RESIZE:
		fprintf(دھارا, " ne-resize ");
		اكسر;
	حالة CSS_CURSOR_NW_RESIZE:
		fprintf(دھارا, " nw-resize ");
		اكسر;
	حالة CSS_CURSOR_N_RESIZE:
		fprintf(دھارا, " n-resize ");
		اكسر;
	حالة CSS_CURSOR_SE_RESIZE:
		fprintf(دھارا, " se-resize ");
		اكسر;
	حالة CSS_CURSOR_SW_RESIZE:
		fprintf(دھارا, " sw-resize ");
		اكسر;
	حالة CSS_CURSOR_S_RESIZE:
		fprintf(دھارا, " s-resize ");
		اكسر;
	حالة CSS_CURSOR_W_RESIZE:
		fprintf(دھارا, " w-resize ");
		اكسر;
	حالة CSS_CURSOR_متن:
		fprintf(دھارا, " text ");
		اكسر;
	حالة CSS_CURSOR_WAIT:
		fprintf(دھارا, " wait ");
		اكسر;
	حالة CSS_CURSOR_HELP:
		fprintf(دھارا, " help ");
		اكسر;
	حالة CSS_CURSOR_ترقی:
		fprintf(دھارا, " progress ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* direction */
	val = css_computed_direction(style);
	بدل (val) {
	حالة CSS_DIRECTION_LTR:
		fprintf(دھارا, "direction: ltr ");
		اكسر;
	حالة CSS_DIRECTION_RTL:
		fprintf(دھارا, "direction: rtl ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* display */
	val = ns_computed_display_static(style);
	بدل (val) {
	حالة CSS_DISPLAY_INLINE:
		fprintf(دھارا, "display: inline ");
		اكسر;
	حالة CSS_DISPLAY_بلاک:
		fprintf(دھارا, "display: block ");
		اكسر;
	حالة CSS_DISPLAY_LIST_شے:
		fprintf(دھارا, "display: list-item ");
		اكسر;
	حالة CSS_DISPLAY_لگاتار_وچ:
		fprintf(دھارا, "display: run-in ");
		اكسر;
	حالة CSS_DISPLAY_INLINE_بلاک:
		fprintf(دھارا, "display: inline-block ");
		اكسر;
	حالة CSS_DISPLAY_جدول:
		fprintf(دھارا, "display: table ");
		اكسر;
	حالة CSS_DISPLAY_INLINE_جدول:
		fprintf(دھارا, "display: inline-table ");
		اكسر;
	حالة CSS_DISPLAY_جدول_صف_گروہ:
		fprintf(دھارا, "display: table-row-group ");
		اكسر;
	حالة CSS_DISPLAY_جدول_سرخی_گروہ:
		fprintf(دھارا, "display: table-header-group ");
		اكسر;
	حالة CSS_DISPLAY_جدول_FOOTER_گروہ:
		fprintf(دھارا, "display: table-footer-group ");
		اكسر;
	حالة CSS_DISPLAY_جدول_صف:
		fprintf(دھارا, "display: table-row ");
		اكسر;
	حالة CSS_DISPLAY_جدول_کالم_گروہ:
		fprintf(دھارا, "display: table-column-group ");
		اكسر;
	حالة CSS_DISPLAY_جدول_کالم:
		fprintf(دھارا, "display: table-column ");
		اكسر;
	حالة CSS_DISPLAY_جدول_CELL:
		fprintf(دھارا, "display: table-cell ");
		اكسر;
	حالة CSS_DISPLAY_جدول_CAPTION:
		fprintf(دھارا, "display: table-caption ");
		اكسر;
	حالة CSS_DISPLAY_NONE:
		fprintf(دھارا, "display: none ");
		اكسر;
	حالة CSS_DISPLAY_FLEX:
		fprintf(دھارا, "display: flex ");
		اكسر;
	حالة CSS_DISPLAY_INLINE_FLEX:
		fprintf(دھارا, "display: inline-flex ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* empty-cells */
	val = css_computed_خالی_تھاواں_cells(style);
	بدل (val) {
	حالة CSS_خالی_تھاواں_CELLS_SHOW:
		fprintf(دھارا, "empty-cells: show ");
		اكسر;
	حالة CSS_خالی_تھاواں_CELLS_HIDE:
		fprintf(دھارا, "empty-cells: hide ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* float */
	val = css_computed_float(style);
	بدل (val) {
	حالة CSS_FLOAT_بایاں:
		fprintf(دھارا, "float: left ");
		اكسر;
	حالة CSS_FLOAT_دایاں:
		fprintf(دھارا, "float: right ");
		اكسر;
	حالة CSS_FLOAT_NONE:
		fprintf(دھارا, "float: none ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* font-family */
	val = css_computed_font_family(style, &string_list);
	إذا (val != CSS_FONT_FAMILY_INHERIT) {
		fprintf(دھارا, "font-family:");

		إذا (string_list != عدم) {
			طالما (*string_list != عدم) {
				fprintf(دھارا, " \"%.*s\"",
					(صحيح) lwc_string_لمائی(*string_list),
					lwc_string_انکڑے(*string_list));

				string_list++;
			}
		}
		بدل (val) {
		حالة CSS_FONT_FAMILY_SERIF:
			fprintf(دھارا, " serif ");
			اكسر;
		حالة CSS_FONT_FAMILY_SANS_SERIF:
			fprintf(دھارا, " sans-serif ");
			اكسر;
		حالة CSS_FONT_FAMILY_CURSIVE:
			fprintf(دھارا, " cursive ");
			اكسر;
		حالة CSS_FONT_FAMILY_FANTASY:
			fprintf(دھارا, " fantasy ");
			اكسر;
		حالة CSS_FONT_FAMILY_MONOSPACE:
			fprintf(دھارا, " monospace ");
			اكسر;
		}
	}

	/* font-size */
	val = css_computed_font_ناپ(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_FONT_ناپ_XX_SMALL:
		fprintf(دھارا, "font-size: xx-small ");
		اكسر;
	حالة CSS_FONT_ناپ_X_SMALL:
		fprintf(دھارا, "font-size: x-small ");
		اكسر;
	حالة CSS_FONT_ناپ_SMALL:
		fprintf(دھارا, "font-size: small ");
		اكسر;
	حالة CSS_FONT_ناپ_MEDIUM:
		fprintf(دھارا, "font-size: medium ");
		اكسر;
	حالة CSS_FONT_ناپ_LARGE:
		fprintf(دھارا, "font-size: large ");
		اكسر;
	حالة CSS_FONT_ناپ_X_LARGE:
		fprintf(دھارا, "font-size: x-large ");
		اكسر;
	حالة CSS_FONT_ناپ_XX_LARGE:
		fprintf(دھارا, "font-size: xx-large ");
		اكسر;
	حالة CSS_FONT_ناپ_LARGER:
		fprintf(دھارا, "font-size: larger ");
		اكسر;
	حالة CSS_FONT_ناپ_SMALLER:
		fprintf(دھارا, "font-size: smaller ");
		اكسر;
	حالة CSS_FONT_ناپ_DIMENSION:
		fprintf(دھارا, "font-size: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* font-style */
	val = css_computed_font_style(style);
	بدل (val) {
	حالة CSS_FONT_STYLE_عام:
		fprintf(دھارا, "font-style: normal ");
		اكسر;
	حالة CSS_FONT_STYLE_ITALIC:
		fprintf(دھارا, "font-style: italic ");
		اكسر;
	حالة CSS_FONT_STYLE_OBLIQUE:
		fprintf(دھارا, "font-style: oblique ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* font-variant */
	val = css_computed_font_variant(style);
	بدل (val) {
	حالة CSS_FONT_VARIANT_عام:
		fprintf(دھارا, "font-variant: normal ");
		اكسر;
	حالة CSS_FONT_VARIANT_SMALL_CAPS:
		fprintf(دھارا, "font-variant: small-caps ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* font-weight */
	val = css_computed_font_بھار(style);
	بدل (val) {
	حالة CSS_FONT_بھار_عام:
		fprintf(دھارا, "font-weight: normal ");
		اكسر;
	حالة CSS_FONT_بھار_BOLD:
		fprintf(دھارا, "font-weight: bold ");
		اكسر;
	حالة CSS_FONT_بھار_BOLDER:
		fprintf(دھارا, "font-weight: bolder ");
		اكسر;
	حالة CSS_FONT_بھار_LIGHTER:
		fprintf(دھارا, "font-weight: lighter ");
		اكسر;
	حالة CSS_FONT_بھار_100:
		fprintf(دھارا, "font-weight: 100 ");
		اكسر;
	حالة CSS_FONT_بھار_200:
		fprintf(دھارا, "font-weight: 200 ");
		اكسر;
	حالة CSS_FONT_بھار_300:
		fprintf(دھارا, "font-weight: 300 ");
		اكسر;
	حالة CSS_FONT_بھار_400:
		fprintf(دھارا, "font-weight: 400 ");
		اكسر;
	حالة CSS_FONT_بھار_500:
		fprintf(دھارا, "font-weight: 500 ");
		اكسر;
	حالة CSS_FONT_بھار_600:
		fprintf(دھارا, "font-weight: 600 ");
		اكسر;
	حالة CSS_FONT_بھار_700:
		fprintf(دھارا, "font-weight: 700 ");
		اكسر;
	حالة CSS_FONT_بھار_800:
		fprintf(دھارا, "font-weight: 800 ");
		اكسر;
	حالة CSS_FONT_بھار_900:
		fprintf(دھارا, "font-weight: 900 ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/*اچائی*/
	val = css_computed_اچائی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_اچائی_AUTO:
		fprintf(دھارا, "height: auto ");
		اكسر;
	حالة CSS_اچائی_سیٹ:
		fprintf(دھارا, "height: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/*بایاں*/
	val = css_computed_بایاں(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_بایاں_AUTO:
		fprintf(دھارا, "left: auto ");
		اكسر;
	حالة CSS_بایاں_سیٹ:
		fprintf(دھارا, "left: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* letter-spacing */
	val = css_computed_letter_spacing(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_LETTER_SPACING_عام:
		fprintf(دھارا, "letter-spacing: normal ");
		اكسر;
	حالة CSS_LETTER_SPACING_سیٹ:
		fprintf(دھارا, "letter-spacing: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* line-height */
	val = css_computed_سطر_اچائی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_سطر_اچائی_عام:
		fprintf(دھارا, "line-height: normal ");
		اكسر;
	حالة CSS_سطر_اچائی_نمبر:
		fprintf(دھارا, "line-height: ");

		dump_css_fixed(دھارا, len1);

		fprintf(دھارا, " ");
		اكسر;
	حالة CSS_سطر_اچائی_DIMENSION:
		fprintf(دھارا, "line-height: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* list-style-image */
	val = css_computed_list_style_image(style, &url);
	إذا (url != عدم) {
		fprintf(دھارا, "list-style-image: url('%.*s') ",
				(صحيح) lwc_string_لمائی(url),
				lwc_string_انکڑے(url));
	} وإلا إذا (val == CSS_LIST_STYLE_IMAGE_NONE) {
		fprintf(دھارا, "list-style-image: none ");
	}

	/* list-style-position */
	val = css_computed_list_style_position(style);
	بدل (val) {
	حالة CSS_LIST_STYLE_POSITION_INSIDE:
		fprintf(دھارا, "list-style-position: inside ");
		اكسر;
	حالة CSS_LIST_STYLE_POSITION_OUTSIDE:
		fprintf(دھارا, "list-style-position: outside ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* list-style-type */
	val = css_computed_list_style_type(style);
	بدل (val) {
	حالة CSS_LIST_STYLE_TYPE_DISC:
		fprintf(دھارا, "list-style-type: disc ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_CIRCLE:
		fprintf(دھارا, "list-style-type: circle ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_مربع:
		fprintf(دھارا, "list-style-type: square ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_DECIMAL:
		fprintf(دھارا, "list-style-type: decimal ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_DECIMAL_LEADING_ZERO:
		fprintf(دھارا, "list-style-type: decimal-leading-zero ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_ہیٹھلا_ROMAN:
		fprintf(دھارا, "list-style-type: lower-roman ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_اتلا_ROMAN:
		fprintf(دھارا, "list-style-type: upper-roman ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_ہیٹھلا_GREEK:
		fprintf(دھارا, "list-style-type: lower-greek ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_ہیٹھلا_LATIN:
		fprintf(دھارا, "list-style-type: lower-latin ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_اتلا_LATIN:
		fprintf(دھارا, "list-style-type: upper-latin ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_ARMENIAN:
		fprintf(دھارا, "list-style-type: armenian ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_GEORGIAN:
		fprintf(دھارا, "list-style-type: georgian ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_ہیٹھلا_ALPHA:
		fprintf(دھارا, "list-style-type: lower-alpha ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_اتلا_ALPHA:
		fprintf(دھارا, "list-style-type: upper-alpha ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_NONE:
		fprintf(دھارا, "list-style-type: none ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* margin-top */
	val = css_computed_margin_چوٹی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MARGIN_AUTO:
		fprintf(دھارا, "margin-top: auto ");
		اكسر;
	حالة CSS_MARGIN_سیٹ:
		fprintf(دھارا, "margin-top: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* margin-right */
	val = css_computed_margin_دایاں(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MARGIN_AUTO:
		fprintf(دھارا, "margin-right: auto ");
		اكسر;
	حالة CSS_MARGIN_سیٹ:
		fprintf(دھارا, "margin-right: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* margin-bottom */
	val = css_computed_margin_bottom(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MARGIN_AUTO:
		fprintf(دھارا, "margin-bottom: auto ");
		اكسر;
	حالة CSS_MARGIN_سیٹ:
		fprintf(دھارا, "margin-bottom: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* margin-left */
	val = css_computed_margin_بایاں(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MARGIN_AUTO:
		fprintf(دھارا, "margin-left: auto ");
		اكسر;
	حالة CSS_MARGIN_سیٹ:
		fprintf(دھارا, "margin-left: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* max-height */
	val = css_computed_max_اچائی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MAX_اچائی_NONE:
		fprintf(دھارا, "max-height: none ");
		اكسر;
	حالة CSS_MAX_اچائی_سیٹ:
		fprintf(دھارا, "max-height: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* max-width */
	val = css_computed_max_چوڑائی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MAX_چوڑائی_NONE:
		fprintf(دھارا, "max-width: none ");
		اكسر;
	حالة CSS_MAX_چوڑائی_سیٹ:
		fprintf(دھارا, "max-width: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* min-height */
	val = ns_computed_min_اچائی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MIN_اچائی_سیٹ:
		fprintf(دھارا, "min-height: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* min-width */
	val = ns_computed_min_چوڑائی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MIN_چوڑائی_سیٹ:
		fprintf(دھارا, "min-width: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* opacity */
	val = css_computed_opacity(style, &len1);
	بدل (val) {
	حالة CSS_OPACITY_سیٹ:
		fprintf(دھارا, "opacity: ");

		dump_css_fixed(دھارا, len1);

		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* outline-color */
	val = css_computed_outline_color(style, &color);
	بدل (val) {
	حالة CSS_OUTLINE_COLOR_INVERT:
		fprintf(دھارا, "outline-color: invert ");
		اكسر;
	حالة CSS_OUTLINE_COLOR_COLOR:
		fprintf(دھارا, "outline-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* outline-style */
	val = css_computed_outline_style(style);
	بدل (val) {
	حالة CSS_OUTLINE_STYLE_NONE:
		fprintf(دھارا, "outline-style: none ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_DOTTED:
		fprintf(دھارا, "outline-style: dotted ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_DASHED:
		fprintf(دھارا, "outline-style: dashed ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_SOLID:
		fprintf(دھارا, "outline-style: solid ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_DOUBLE:
		fprintf(دھارا, "outline-style: double ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_GROOVE:
		fprintf(دھارا, "outline-style: groove ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_RIDGE:
		fprintf(دھارا, "outline-style: ridge ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_INSET:
		fprintf(دھارا, "outline-style: inset ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_OUTSET:
		fprintf(دھارا, "outline-style: outset ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* outline-width */
	val = css_computed_outline_چوڑائی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_OUTLINE_چوڑائی_THIN:
		fprintf(دھارا, "outline-width: thin ");
		اكسر;
	حالة CSS_OUTLINE_چوڑائی_MEDIUM:
		fprintf(دھارا, "outline-width: medium ");
		اكسر;
	حالة CSS_OUTLINE_چوڑائی_THICK:
		fprintf(دھارا, "outline-width: thick ");
		اكسر;
	حالة CSS_OUTLINE_چوڑائی_چوڑائی:
		fprintf(دھارا, "outline-width: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/*حد_توں_ودھ*/
	val = css_computed_حد_توں_ودھ_x(style);
	بدل (val) {
	حالة CSS_حد_توں_ودھ_VISIBLE:
		fprintf(دھارا, "overflow-x: visible ");
		اكسر;
	حالة CSS_حد_توں_ودھ_HIDDEN:
		fprintf(دھارا, "overflow-x: hidden ");
		اكسر;
	حالة CSS_حد_توں_ودھ_SCROLL:
		fprintf(دھارا, "overflow-x: scroll ");
		اكسر;
	حالة CSS_حد_توں_ودھ_AUTO:
		fprintf(دھارا, "overflow-x auto ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/*حد_توں_ودھ*/
	val = css_computed_حد_توں_ودھ_y(style);
	بدل (val) {
	حالة CSS_حد_توں_ودھ_VISIBLE:
		fprintf(دھارا, "overflow-y: visible ");
		اكسر;
	حالة CSS_حد_توں_ودھ_HIDDEN:
		fprintf(دھارا, "overflow-y: hidden ");
		اكسر;
	حالة CSS_حد_توں_ودھ_SCROLL:
		fprintf(دھارا, "overflow-y: scroll ");
		اكسر;
	حالة CSS_حد_توں_ودھ_AUTO:
		fprintf(دھارا, "overflow-y: auto ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* padding-top */
	val = css_computed_padding_چوٹی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_PADDING_سیٹ:
		fprintf(دھارا, "padding-top: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* padding-right */
	val = css_computed_padding_دایاں(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_PADDING_سیٹ:
		fprintf(دھارا, "padding-right: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* padding-bottom */
	val = css_computed_padding_bottom(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_PADDING_سیٹ:
		fprintf(دھارا, "padding-bottom: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* padding-left */
	val = css_computed_padding_بایاں(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_PADDING_سیٹ:
		fprintf(دھارا, "padding-left: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* position */
	val = css_computed_position(style);
	بدل (val) {
	حالة CSS_POSITION_STATIC:
		fprintf(دھارا, "position: static ");
		اكسر;
	حالة CSS_POSITION_RELATIVE:
		fprintf(دھارا, "position: relative ");
		اكسر;
	حالة CSS_POSITION_ABSOLUTE:
		fprintf(دھارا, "position: absolute ");
		اكسر;
	حالة CSS_POSITION_FIXED:
		fprintf(دھارا, "position: fixed ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* quotes */
	val = css_computed_quotes(style, &string_list);
	إذا (val == CSS_QUOTES_STRING && string_list != عدم) {
		fprintf(دھارا, "quotes:");

		طالما (*string_list != عدم) {
			fprintf(دھارا, " \"%.*s\"",
				(صحيح) lwc_string_لمائی(*string_list),
				lwc_string_انکڑے(*string_list));

			string_list++;
		}

		fprintf(دھارا, " ");
	} وإلا {
		بدل (val) {
		حالة CSS_QUOTES_NONE:
			fprintf(دھارا, "quotes: none ");
			اكسر;
		افتراضي:
			اكسر;
		}
	}

	/*دایاں*/
	val = css_computed_دایاں(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_دایاں_AUTO:
		fprintf(دھارا, "right: auto ");
		اكسر;
	حالة CSS_دایاں_سیٹ:
		fprintf(دھارا, "right: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* table-layout */
	val = css_computed_جدول_layout(style);
	بدل (val) {
	حالة CSS_جدول_LAYOUT_AUTO:
		fprintf(دھارا, "table-layout: auto ");
		اكسر;
	حالة CSS_جدول_LAYOUT_FIXED:
		fprintf(دھارا, "table-layout: fixed ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* text-align */
	val = css_computed_متن_align(style);
	بدل (val) {
	حالة CSS_متن_ALIGN_بایاں:
		fprintf(دھارا, "text-align: left ");
		اكسر;
	حالة CSS_متن_ALIGN_دایاں:
		fprintf(دھارا, "text-align: right ");
		اكسر;
	حالة CSS_متن_ALIGN_CENTER:
		fprintf(دھارا, "text-align: center ");
		اكسر;
	حالة CSS_متن_ALIGN_JUSTIFY:
		fprintf(دھارا, "text-align: justify ");
		اكسر;
	حالة CSS_متن_ALIGN_DEFAULT:
		fprintf(دھارا, "text-align: default ");
		اكسر;
	حالة CSS_متن_ALIGN_LIBCSS_بایاں:
		fprintf(دھارا, "text-align: -libcss-left ");
		اكسر;
	حالة CSS_متن_ALIGN_LIBCSS_CENTER:
		fprintf(دھارا, "text-align: -libcss-center ");
		اكسر;
	حالة CSS_متن_ALIGN_LIBCSS_دایاں:
		fprintf(دھارا, "text-align: -libcss-right ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* text-decoration */
	val = css_computed_متن_decoration(style);
	إذا (val == CSS_متن_DECORATION_NONE) {
		fprintf(دھارا, "text-decoration: none ");
	} وإلا {
		fprintf(دھارا, "text-decoration:");

		إذا (val & CSS_متن_DECORATION_BLINK) {
			fprintf(دھارا, " blink");
		}
		إذا (val & CSS_متن_DECORATION_سطر_THROUGH) {
			fprintf(دھارا, " line-through");
		}
		إذا (val & CSS_متن_DECORATION_OVERLINE) {
			fprintf(دھارا, " overline");
		}
		إذا (val & CSS_متن_DECORATION_UNDERLINE) {
			fprintf(دھارا, " underline");
		}

		fprintf(دھارا, " ");
	}

	/* text-indent */
	val = css_computed_متن_indent(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_متن_INDENT_سیٹ:
		fprintf(دھارا, "text-indent: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* text-transform */
	val = css_computed_متن_transform(style);
	بدل (val) {
	حالة CSS_متن_TRANSFORM_CAPITALIZE:
		fprintf(دھارا, "text-transform: capitalize ");
		اكسر;
	حالة CSS_متن_TRANSFORM_UPPERCASE:
		fprintf(دھارا, "text-transform: uppercase ");
		اكسر;
	حالة CSS_متن_TRANSFORM_LOWERCASE:
		fprintf(دھارا, "text-transform: lowercase ");
		اكسر;
	حالة CSS_متن_TRANSFORM_NONE:
		fprintf(دھارا, "text-transform: none ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/*چوٹی*/
	val = css_computed_چوٹی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_چوٹی_AUTO:
		fprintf(دھارا, "top: auto ");
		اكسر;
	حالة CSS_چوٹی_سیٹ:
		fprintf(دھارا, "top: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* unicode-bidi */
	val = css_computed_unicode_bidi(style);
	بدل (val) {
	حالة CSS_UNICODE_BIDI_عام:
		fprintf(دھارا, "unicode-bidi: normal ");
		اكسر;
	حالة CSS_UNICODE_BIDI_EMBED:
		fprintf(دھارا, "unicode-bidi: embed ");
		اكسر;
	حالة CSS_UNICODE_BIDI_BIDI_OVERRIDE:
		fprintf(دھارا, "unicode-bidi: bidi-override ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* vertical-align */
	val = css_computed_vertical_align(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_VERTICAL_ALIGN_BASELINE:
		fprintf(دھارا, "vertical-align: baseline ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_SUB:
		fprintf(دھارا, "vertical-align: sub ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_SUPER:
		fprintf(دھارا, "vertical-align: super ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_چوٹی:
		fprintf(دھارا, "vertical-align: top ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_متن_چوٹی:
		fprintf(دھارا, "vertical-align: text-top ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_درمیان:
		fprintf(دھارا, "vertical-align: middle ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_BOTTOM:
		fprintf(دھارا, "vertical-align: bottom ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_متن_BOTTOM:
		fprintf(دھارا, "vertical-align: text-bottom ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_سیٹ:
		fprintf(دھارا, "vertical-align: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* visibility */
	val = css_computed_visibility(style);
	بدل (val) {
	حالة CSS_VISIBILITY_VISIBLE:
		fprintf(دھارا, "visibility: visible ");
		اكسر;
	حالة CSS_VISIBILITY_HIDDEN:
		fprintf(دھارا, "visibility: hidden ");
		اكسر;
	حالة CSS_VISIBILITY_COLLAPSE:
		fprintf(دھارا, "visibility: collapse ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* white-space */
	val = css_computed_white_space(style);
	بدل (val) {
	حالة CSS_WHITE_SPACE_عام:
		fprintf(دھارا, "white-space: normal ");
		اكسر;
	حالة CSS_WHITE_SPACE_PRE:
		fprintf(دھارا, "white-space: pre ");
		اكسر;
	حالة CSS_WHITE_SPACE_NOWRAP:
		fprintf(دھارا, "white-space: nowrap ");
		اكسر;
	حالة CSS_WHITE_SPACE_PRE_WRAP:
		fprintf(دھارا, "white-space: pre-wrap ");
		اكسر;
	حالة CSS_WHITE_SPACE_PRE_سطر:
		fprintf(دھارا, "white-space: pre-line ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/*چوڑائی*/
	val = css_computed_چوڑائی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_چوڑائی_AUTO:
		fprintf(دھارا, "width: auto ");
		اكسر;
	حالة CSS_چوڑائی_سیٹ:
		fprintf(دھارا, "width: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* word-spacing */
	val = css_computed_لفظ_spacing(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_لفظ_SPACING_عام:
		fprintf(دھارا, "word-spacing: normal ");
		اكسر;
	حالة CSS_لفظ_SPACING_سیٹ:
		fprintf(دھارا, "word-spacing: ");

		dump_css_unit(دھارا, len1, unit1);

		fprintf(دھارا, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* z-index */
	val = css_computed_z_اشاریہ(style, &zindex);
	بدل (val) {
	حالة CSS_Z_اشاریہ_AUTO:
		fprintf(دھارا, "z-index: auto ");
		اكسر;
	حالة CSS_Z_اشاریہ_سیٹ:
		fprintf(دھارا, "z-index: %"PRId32" ", zindex);
		اكسر;
	افتراضي:
		اكسر;
	}

	fprintf(دھارا, "}");
}
