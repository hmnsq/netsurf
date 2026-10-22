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
ساكن فراغ dump_css_fixed(FILE *وهڪرو, css_fixed f)
{
#وصف_ڪريو NSCSS_ABS(x) (uint32_t)((x) < 0 ? -(x) : (x))
	uint32_t uintpart = FIXTOINT(NSCSS_ABS(f));
	/* + 500 to ensure round to nearest (division will truncate) */
	uint32_t fracpart = ((NSCSS_ABS(f) & 0x3ff) * 1000 + 500) / (1 << 10);
#وصف_رد NSCSS_ABS

	fprintf(وهڪرو, "%s%"PRIu32".%03"PRIu32, f < 0 ? "-" : "", uintpart, fracpart);
}

/**
 * Dump a numeric value to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 */
ساكن فراغ dump_css_عدد(FILE *وهڪرو, css_fixed val)
{
	إذا (INTTOFIX(FIXTOINT(val)) == val)
		fprintf(وهڪرو, "%"PRId32, FIXTOINT(val));
	وإلا
		dump_css_fixed(وهڪرو, val);
}

/**
 * Dump a dimension to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 * \param unit    Unit to write
 */
ساكن فراغ dump_css_unit(FILE *وهڪرو, css_fixed val, css_unit unit)
{
	dump_css_عدد(وهڪرو, val);

	بدل (unit) {
	حالة CSS_UNIT_PX:
		fprintf(وهڪرو, "px");
		اكسر;
	حالة CSS_UNIT_EX:
		fprintf(وهڪرو, "ex");
		اكسر;
	حالة CSS_UNIT_EM:
		fprintf(وهڪرو, "em");
		اكسر;
	حالة CSS_UNIT_اندريون:
		fprintf(وهڪرو, "in");
		اكسر;
	حالة CSS_UNIT_CM:
		fprintf(وهڪرو, "cm");
		اكسر;
	حالة CSS_UNIT_MM:
		fprintf(وهڪرو, "mm");
		اكسر;
	حالة CSS_UNIT_PT:
		fprintf(وهڪرو, "pt");
		اكسر;
	حالة CSS_UNIT_PC:
		fprintf(وهڪرو, "pc");
		اكسر;
	حالة CSS_UNIT_PCT:
		fprintf(وهڪرو, "%%");
		اكسر;
	حالة CSS_UNIT_DEG:
		fprintf(وهڪرو, "deg");
		اكسر;
	حالة CSS_UNIT_GRAD:
		fprintf(وهڪرو, "grad");
		اكسر;
	حالة CSS_UNIT_RAD:
		fprintf(وهڪرو, "rad");
		اكسر;
	حالة CSS_UNIT_MS:
		fprintf(وهڪرو, "ms");
		اكسر;
	حالة CSS_UNIT_S:
		fprintf(وهڪرو, "s");
		اكسر;
	حالة CSS_UNIT_HZ:
		fprintf(وهڪرو, "Hz");
		اكسر;
	حالة CSS_UNIT_KHZ:
		fprintf(وهڪرو, "kHz");
		اكسر;
	حالة CSS_UNIT_CH:
		fprintf(وهڪرو, "ch");
		اكسر;
	حالة CSS_UNIT_REM:
		fprintf(وهڪرو, "rem");
		اكسر;
	حالة CSS_UNIT_LH:
		fprintf(وهڪرو, "lh");
		اكسر;
	حالة CSS_UNIT_VH:
		fprintf(وهڪرو, "vh");
		اكسر;
	حالة CSS_UNIT_VW:
		fprintf(وهڪرو, "vw");
		اكسر;
	حالة CSS_UNIT_VI:
		fprintf(وهڪرو, "vi");
		اكسر;
	حالة CSS_UNIT_VB:
		fprintf(وهڪرو, "vb");
		اكسر;
	حالة CSS_UNIT_VMIN:
		fprintf(وهڪرو, "vmin");
		اكسر;
	حالة CSS_UNIT_VMAX:
		fprintf(وهڪرو, "vmax");
		اكسر;
	حالة CSS_UNIT_Q:
		fprintf(وهڪرو, "q");
		اكسر;
	حالة CSS_UNIT_CALC:
		fprintf(وهڪرو, "calc()");
		اكسر;
	}
}

/* exported interface documented in content/handlers/css/dump.h */
فراغ nscss_dump_computed_style(FILE *وهڪرو, ثابت css_computed_style *style)
{
	uint8_t val;
	css_color color = 0;
	lwc_string *url = عدم;
	css_fixed len1 = 0, len2 = 0;
	css_unit unit1 = CSS_UNIT_PX, unit2 = CSS_UNIT_PX;
	css_computed_clip_rect rect = { 0, 0, 0, 0, CSS_UNIT_PX, CSS_UNIT_PX,
					CSS_UNIT_PX, CSS_UNIT_PX, true, true,
					true, true };
	ثابت css_computed_content_شي *content = عدم;
	ثابت css_computed_counter *counter = عدم;
	lwc_string **string_list = عدم;
	int32_t zindex = 0;

	fprintf(وهڪرو, "{ ");

	/* background-attachment */
	val = css_computed_background_attachment(style);
	بدل (val) {
	حالة CSS_BACKGROUND_ATTACHMENT_FIXED:
		fprintf(وهڪرو, "background-attachment: fixed ");
		اكسر;
	حالة CSS_BACKGROUND_ATTACHMENT_SCROLL:
		fprintf(وهڪرو, "background-attachment: scroll ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* background-color */
	val = css_computed_background_color(style, &color);
	بدل (val) {
	حالة CSS_BACKGROUND_COLOR_COLOR:
		fprintf(وهڪرو, "background-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* background-image */
	val = css_computed_background_image(style, &url);
	إذا (val == CSS_BACKGROUND_IMAGE_IMAGE && url != عدم) {
		fprintf(وهڪرو, "background-image: url('%.*s') ",
				(صحيح) lwc_string_ڊيگهائي(url),
				lwc_string_معلومات(url));
	} وإلا إذا (val == CSS_BACKGROUND_IMAGE_NONE) {
		fprintf(وهڪرو, "background-image: none ");
	}

	/* background-position */
	val = css_computed_background_position(style, &len1, &unit1,
			&len2, &unit2);
	إذا (val == CSS_BACKGROUND_POSITION_سيٽ) {
		fprintf(وهڪرو, "background-position: ");
		dump_css_unit(وهڪرو, len1, unit1);
		fprintf(وهڪرو, " ");
		dump_css_unit(وهڪرو, len2, unit2);
		fprintf(وهڪرو, " ");
	}

	/* background-repeat */
	val = css_computed_background_repeat(style);
	بدل (val) {
	حالة CSS_BACKGROUND_REPEAT_REPEAT_X:
		fprintf(وهڪرو, "background-repeat: repeat-x ");
		اكسر;
	حالة CSS_BACKGROUND_REPEAT_REPEAT_Y:
		fprintf(وهڪرو, "background-repeat: repeat-y ");
		اكسر;
	حالة CSS_BACKGROUND_REPEAT_REPEAT:
		fprintf(وهڪرو, "background-repeat: repeat ");
		اكسر;
	حالة CSS_BACKGROUND_REPEAT_NO_REPEAT:
		fprintf(وهڪرو, "background-repeat: no-repeat ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-collapse */
	val = css_computed_border_collapse(style);
	بدل (val) {
	حالة CSS_BORDER_COLLAPSE_SEPARATE:
		fprintf(وهڪرو, "border-collapse: separate ");
		اكسر;
	حالة CSS_BORDER_COLLAPSE_COLLAPSE:
		fprintf(وهڪرو, "border-collapse: collapse ");
		اكسر;
	افتراضي:

		اكسر;
	}

	/* border-spacing */
	val = css_computed_border_spacing(style, &len1, &unit1, &len2, &unit2);
	إذا (val == CSS_BORDER_SPACING_سيٽ) {
		fprintf(وهڪرو, "border-spacing: ");
		dump_css_unit(وهڪرو, len1, unit1);
		fprintf(وهڪرو, " ");
		dump_css_unit(وهڪرو, len2, unit2);
		fprintf(وهڪرو, " ");
	}

	/* border-top-color */
	val = css_computed_border_چوٽي_color(style, &color);
	بدل (val) {
	حالة CSS_BORDER_COLOR_COLOR:
		fprintf(وهڪرو, "border-top-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-right-color */
	val = css_computed_border_ساڄو_color(style, &color);
	بدل (val) {
	حالة CSS_BORDER_COLOR_COLOR:
		fprintf(وهڪرو, "border-right-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-bottom-color */
	val = css_computed_border_bottom_color(style, &color);
	بدل (val) {
	حالة CSS_BORDER_COLOR_COLOR:
		fprintf(وهڪرو, "border-bottom-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-left-color */
	val = css_computed_border_کاٻو_color(style, &color);
	بدل (val) {
	حالة CSS_BORDER_COLOR_COLOR:
		fprintf(وهڪرو, "border-left-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-top-style */
	val = css_computed_border_چوٽي_style(style);
	بدل (val) {
	حالة CSS_BORDER_STYLE_NONE:
		fprintf(وهڪرو, "border-top-style: none ");
		اكسر;
	حالة CSS_BORDER_STYLE_HIDDEN:
		fprintf(وهڪرو, "border-top-style: hidden ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOTTED:
		fprintf(وهڪرو, "border-top-style: dotted ");
		اكسر;
	حالة CSS_BORDER_STYLE_DASHED:
		fprintf(وهڪرو, "border-top-style: dashed ");
		اكسر;
	حالة CSS_BORDER_STYLE_SOLID:
		fprintf(وهڪرو, "border-top-style: solid ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOUBLE:
		fprintf(وهڪرو, "border-top-style: double ");
		اكسر;
	حالة CSS_BORDER_STYLE_GROOVE:
		fprintf(وهڪرو, "border-top-style: groove ");
		اكسر;
	حالة CSS_BORDER_STYLE_RIDGE:
		fprintf(وهڪرو, "border-top-style: ridge ");
		اكسر;
	حالة CSS_BORDER_STYLE_INSET:
		fprintf(وهڪرو, "border-top-style: inset ");
		اكسر;
	حالة CSS_BORDER_STYLE_OUTSET:
		fprintf(وهڪرو, "border-top-style: outset ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-right-style */
	val = css_computed_border_ساڄو_style(style);
	بدل (val) {
	حالة CSS_BORDER_STYLE_NONE:
		fprintf(وهڪرو, "border-right-style: none ");
		اكسر;
	حالة CSS_BORDER_STYLE_HIDDEN:
		fprintf(وهڪرو, "border-right-style: hidden ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOTTED:
		fprintf(وهڪرو, "border-right-style: dotted ");
		اكسر;
	حالة CSS_BORDER_STYLE_DASHED:
		fprintf(وهڪرو, "border-right-style: dashed ");
		اكسر;
	حالة CSS_BORDER_STYLE_SOLID:
		fprintf(وهڪرو, "border-right-style: solid ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOUBLE:
		fprintf(وهڪرو, "border-right-style: double ");
		اكسر;
	حالة CSS_BORDER_STYLE_GROOVE:
		fprintf(وهڪرو, "border-right-style: groove ");
		اكسر;
	حالة CSS_BORDER_STYLE_RIDGE:
		fprintf(وهڪرو, "border-right-style: ridge ");
		اكسر;
	حالة CSS_BORDER_STYLE_INSET:
		fprintf(وهڪرو, "border-right-style: inset ");
		اكسر;
	حالة CSS_BORDER_STYLE_OUTSET:
		fprintf(وهڪرو, "border-right-style: outset ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-bottom-style */
	val = css_computed_border_bottom_style(style);
	بدل (val) {
	حالة CSS_BORDER_STYLE_NONE:
		fprintf(وهڪرو, "border-bottom-style: none ");
		اكسر;
	حالة CSS_BORDER_STYLE_HIDDEN:
		fprintf(وهڪرو, "border-bottom-style: hidden ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOTTED:
		fprintf(وهڪرو, "border-bottom-style: dotted ");
		اكسر;
	حالة CSS_BORDER_STYLE_DASHED:
		fprintf(وهڪرو, "border-bottom-style: dashed ");
		اكسر;
	حالة CSS_BORDER_STYLE_SOLID:
		fprintf(وهڪرو, "border-bottom-style: solid ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOUBLE:
		fprintf(وهڪرو, "border-bottom-style: double ");
		اكسر;
	حالة CSS_BORDER_STYLE_GROOVE:
		fprintf(وهڪرو, "border-bottom-style: groove ");
		اكسر;
	حالة CSS_BORDER_STYLE_RIDGE:
		fprintf(وهڪرو, "border-bottom-style: ridge ");
		اكسر;
	حالة CSS_BORDER_STYLE_INSET:
		fprintf(وهڪرو, "border-bottom-style: inset ");
		اكسر;
	حالة CSS_BORDER_STYLE_OUTSET:
		fprintf(وهڪرو, "border-bottom-style: outset ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-left-style */
	val = css_computed_border_کاٻو_style(style);
	بدل (val) {
	حالة CSS_BORDER_STYLE_NONE:
		fprintf(وهڪرو, "border-left-style: none ");
		اكسر;
	حالة CSS_BORDER_STYLE_HIDDEN:
		fprintf(وهڪرو, "border-left-style: hidden ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOTTED:
		fprintf(وهڪرو, "border-left-style: dotted ");
		اكسر;
	حالة CSS_BORDER_STYLE_DASHED:
		fprintf(وهڪرو, "border-left-style: dashed ");
		اكسر;
	حالة CSS_BORDER_STYLE_SOLID:
		fprintf(وهڪرو, "border-left-style: solid ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOUBLE:
		fprintf(وهڪرو, "border-left-style: double ");
		اكسر;
	حالة CSS_BORDER_STYLE_GROOVE:
		fprintf(وهڪرو, "border-left-style: groove ");
		اكسر;
	حالة CSS_BORDER_STYLE_RIDGE:
		fprintf(وهڪرو, "border-left-style: ridge ");
		اكسر;
	حالة CSS_BORDER_STYLE_INSET:
		fprintf(وهڪرو, "border-left-style: inset ");
		اكسر;
	حالة CSS_BORDER_STYLE_OUTSET:
		fprintf(وهڪرو, "border-left-style: outset ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-top-width */
	val = css_computed_border_چوٽي_ويڪر(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_BORDER_ويڪر_THIN:
		fprintf(وهڪرو, "border-top-width: thin ");
		اكسر;
	حالة CSS_BORDER_ويڪر_MEDIUM:
		fprintf(وهڪرو, "border-top-width: medium ");
		اكسر;
	حالة CSS_BORDER_ويڪر_THICK:
		fprintf(وهڪرو, "border-top-width: thick ");
		اكسر;
	حالة CSS_BORDER_ويڪر_ويڪر:
		fprintf(وهڪرو, "border-top-width: ");
		dump_css_unit(وهڪرو, len1, unit1);
		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-right-width */
	val = css_computed_border_ساڄو_ويڪر(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_BORDER_ويڪر_THIN:
		fprintf(وهڪرو, "border-right-width: thin ");
		اكسر;
	حالة CSS_BORDER_ويڪر_MEDIUM:
		fprintf(وهڪرو, "border-right-width: medium ");
		اكسر;
	حالة CSS_BORDER_ويڪر_THICK:
		fprintf(وهڪرو, "border-right-width: thick ");
		اكسر;
	حالة CSS_BORDER_ويڪر_ويڪر:
		fprintf(وهڪرو, "border-right-width: ");
		dump_css_unit(وهڪرو, len1, unit1);
		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-bottom-width */
	val = css_computed_border_bottom_ويڪر(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_BORDER_ويڪر_THIN:
		fprintf(وهڪرو, "border-bottom-width: thin ");
		اكسر;
	حالة CSS_BORDER_ويڪر_MEDIUM:
		fprintf(وهڪرو, "border-bottom-width: medium ");
		اكسر;
	حالة CSS_BORDER_ويڪر_THICK:
		fprintf(وهڪرو, "border-bottom-width: thick ");
		اكسر;
	حالة CSS_BORDER_ويڪر_ويڪر:
		fprintf(وهڪرو, "border-bottom-width: ");
		dump_css_unit(وهڪرو, len1, unit1);
		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-left-width */
	val = css_computed_border_کاٻو_ويڪر(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_BORDER_ويڪر_THIN:
		fprintf(وهڪرو, "border-left-width: thin ");
		اكسر;
	حالة CSS_BORDER_ويڪر_MEDIUM:
		fprintf(وهڪرو, "border-left-width: medium ");
		اكسر;
	حالة CSS_BORDER_ويڪر_THICK:
		fprintf(وهڪرو, "border-left-width: thick ");
		اكسر;
	حالة CSS_BORDER_ويڪر_ويڪر:
		fprintf(وهڪرو, "border-left-width: ");
		dump_css_unit(وهڪرو, len1, unit1);
		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* bottom */
	val = css_computed_bottom(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_BOTTOM_AUTO:
		fprintf(وهڪرو, "bottom: auto ");
		اكسر;
	حالة CSS_BOTTOM_سيٽ:
		fprintf(وهڪرو, "bottom: ");
		dump_css_unit(وهڪرو, len1, unit1);
		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* caption-side */
	val = css_computed_caption_side(style);
	بدل (val) {
	حالة CSS_CAPTION_SIDE_چوٽي:
		fprintf(وهڪرو, "caption_side: top ");
		اكسر;
	حالة CSS_CAPTION_SIDE_BOTTOM:
		fprintf(وهڪرو, "caption_side: bottom ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* clear */
	val = css_computed_clear(style);
	بدل (val) {
	حالة CSS_CLEAR_NONE:
		fprintf(وهڪرو, "clear: none ");
		اكسر;
	حالة CSS_CLEAR_کاٻو:
		fprintf(وهڪرو, "clear: left ");
		اكسر;
	حالة CSS_CLEAR_ساڄو:
		fprintf(وهڪرو, "clear: right ");
		اكسر;
	حالة CSS_CLEAR_BOTH:
		fprintf(وهڪرو, "clear: both ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* clip */
	val = css_computed_clip(style, &rect);
	بدل (val) {
	حالة CSS_CLIP_AUTO:
		fprintf(وهڪرو, "clip: auto ");
		اكسر;
	حالة CSS_CLIP_RECT:
		fprintf(وهڪرو, "clip: rect( ");

		إذا (rect.چوٽي_auto)
			fprintf(وهڪرو, "auto");
		وإلا
			dump_css_unit(وهڪرو, rect.چوٽي, rect.tunit);
		fprintf(وهڪرو, ", ");

		إذا (rect.ساڄو_auto)
			fprintf(وهڪرو, "auto");
		وإلا
			dump_css_unit(وهڪرو, rect.ساڄو, rect.runit);
		fprintf(وهڪرو, ", ");

		إذا (rect.bottom_auto)
			fprintf(وهڪرو, "auto");
		وإلا
			dump_css_unit(وهڪرو, rect.bottom, rect.bunit);
		fprintf(وهڪرو, ", ");

		إذا (rect.کاٻو_auto)
			fprintf(وهڪرو, "auto");
		وإلا
			dump_css_unit(وهڪرو, rect.کاٻو, rect.lunit);
		fprintf(وهڪرو, ") ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* color */
	val = css_computed_color(style, &color);
	إذا (val == CSS_COLOR_COLOR) {
		fprintf(وهڪرو, "color: #%08"PRIx32" ", color);
	}

	/* content */
	val = css_computed_content(style, &content);
	بدل (val) {
	حالة CSS_CONTENT_NONE:
		fprintf(وهڪرو, "content: none ");
		اكسر;
	حالة CSS_CONTENT_عام:
		fprintf(وهڪرو, "content: normal ");
		اكسر;
	حالة CSS_CONTENT_سيٽ:
		fprintf(وهڪرو, "content:");

		طالما (content->type != CSS_COMPUTED_CONTENT_NONE) {
			fprintf(وهڪرو, " ");

			بدل (content->type) {
			حالة CSS_COMPUTED_CONTENT_STRING:
				fprintf(وهڪرو,	"\"%.*s\"",
						(صحيح) lwc_string_ڊيگهائي(
						content->معلومات.string),
						lwc_string_معلومات(
						content->معلومات.string));
				اكسر;
			حالة CSS_COMPUTED_CONTENT_URI:
				fprintf(وهڪرو,	"uri(\"%.*s\")",
						(صحيح) lwc_string_ڊيگهائي(
						content->معلومات.uri),
						lwc_string_معلومات(
						content->معلومات.uri));
				اكسر;
			حالة CSS_COMPUTED_CONTENT_COUNTER:
				fprintf(وهڪرو, "counter(%.*s)",
						(صحيح) lwc_string_ڊيگهائي(
						content->معلومات.counter.name),
						lwc_string_معلومات(
						content->معلومات.counter.name));
				اكسر;
			حالة CSS_COMPUTED_CONTENT_COUNTERS:
				fprintf(وهڪرو, "counters(%.*s, \"%.*s\")",
						(صحيح) lwc_string_ڊيگهائي(
						content->معلومات.counters.name),
						lwc_string_معلومات(
						content->معلومات.counters.name),
						(صحيح) lwc_string_ڊيگهائي(
						content->معلومات.counters.sep),
						lwc_string_معلومات(
						content->معلومات.counters.sep));
				اكسر;
			حالة CSS_COMPUTED_CONTENT_ATTR:
				fprintf(وهڪرو, "attr(%.*s)",
						(صحيح) lwc_string_ڊيگهائي(
						content->معلومات.attr),
						lwc_string_معلومات(
						content->معلومات.attr));
				اكسر;
			حالة CSS_COMPUTED_CONTENT_OPEN_QUOTE:
				fprintf(وهڪرو, "open-quote");
				اكسر;
			حالة CSS_COMPUTED_CONTENT_CLOSE_QUOTE:
				fprintf(وهڪرو, "close-quote");
				اكسر;
			حالة CSS_COMPUTED_CONTENT_NO_OPEN_QUOTE:
				fprintf(وهڪرو, "no-open-quote");
				اكسر;
			حالة CSS_COMPUTED_CONTENT_NO_CLOSE_QUOTE:
				fprintf(وهڪرو, "no-close-quote");
				اكسر;
			}

			content++;
		}

		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* counter-increment */
	val = css_computed_counter_increment(style, &counter);
	إذا ((val == CSS_COUNTER_INCREMENT_NONE) || (counter == عدم)) {
		fprintf(وهڪرو, "counter-increment: none ");
	} وإلا {
		fprintf(وهڪرو, "counter-increment:");

		طالما (counter->name != عدم) {
			fprintf(وهڪرو, " %.*s ",
					(صحيح) lwc_string_ڊيگهائي(counter->name),
					lwc_string_معلومات(counter->name));

			dump_css_fixed(وهڪرو, counter->قدر);

			counter++;
		}

		fprintf(وهڪرو, " ");
	}

	/* counter-reset */
	val = css_computed_counter_reset(style, &counter);
	إذا ((val == CSS_COUNTER_RESET_NONE) || (counter == عدم)) {
		fprintf(وهڪرو, "counter-reset: none ");
	} وإلا {
		fprintf(وهڪرو, "counter-reset:");

		طالما (counter->name != عدم) {
			fprintf(وهڪرو, " %.*s ",
					(صحيح) lwc_string_ڊيگهائي(counter->name),
					lwc_string_معلومات(counter->name));

			dump_css_fixed(وهڪرو, counter->قدر);

			counter++;
		}

		fprintf(وهڪرو, " ");
	}

	/* cursor */
	val = css_computed_cursor(style, &string_list);
	fprintf(وهڪرو, "cursor:");

	إذا (string_list != عدم) {
		طالما (*string_list != عدم) {
			fprintf(وهڪرو, " url\"%.*s\")",
					(صحيح) lwc_string_ڊيگهائي(*string_list),
					lwc_string_معلومات(*string_list));

			string_list++;
		}
	}
	بدل (val) {
	حالة CSS_CURSOR_AUTO:
		fprintf(وهڪرو, " auto ");
		اكسر;
	حالة CSS_CURSOR_CROSSHAIR:
		fprintf(وهڪرو, " crosshair ");
		اكسر;
	حالة CSS_CURSOR_DEFAULT:
		fprintf(وهڪرو, " default ");
		اكسر;
	حالة CSS_CURSOR_اشارو:
		fprintf(وهڪرو, " pointer ");
		اكسر;
	حالة CSS_CURSOR_MOVE:
		fprintf(وهڪرو, " move ");
		اكسر;
	حالة CSS_CURSOR_E_RESIZE:
		fprintf(وهڪرو, " e-resize ");
		اكسر;
	حالة CSS_CURSOR_NE_RESIZE:
		fprintf(وهڪرو, " ne-resize ");
		اكسر;
	حالة CSS_CURSOR_NW_RESIZE:
		fprintf(وهڪرو, " nw-resize ");
		اكسر;
	حالة CSS_CURSOR_N_RESIZE:
		fprintf(وهڪرو, " n-resize ");
		اكسر;
	حالة CSS_CURSOR_SE_RESIZE:
		fprintf(وهڪرو, " se-resize ");
		اكسر;
	حالة CSS_CURSOR_SW_RESIZE:
		fprintf(وهڪرو, " sw-resize ");
		اكسر;
	حالة CSS_CURSOR_S_RESIZE:
		fprintf(وهڪرو, " s-resize ");
		اكسر;
	حالة CSS_CURSOR_W_RESIZE:
		fprintf(وهڪرو, " w-resize ");
		اكسر;
	حالة CSS_CURSOR_متن:
		fprintf(وهڪرو, " text ");
		اكسر;
	حالة CSS_CURSOR_WAIT:
		fprintf(وهڪرو, " wait ");
		اكسر;
	حالة CSS_CURSOR_HELP:
		fprintf(وهڪرو, " help ");
		اكسر;
	حالة CSS_CURSOR_ترقي:
		fprintf(وهڪرو, " progress ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* direction */
	val = css_computed_direction(style);
	بدل (val) {
	حالة CSS_DIRECTION_LTR:
		fprintf(وهڪرو, "direction: ltr ");
		اكسر;
	حالة CSS_DIRECTION_RTL:
		fprintf(وهڪرو, "direction: rtl ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* display */
	val = ns_computed_display_static(style);
	بدل (val) {
	حالة CSS_DISPLAY_INLINE:
		fprintf(وهڪرو, "display: inline ");
		اكسر;
	حالة CSS_DISPLAY_بلاڪ:
		fprintf(وهڪرو, "display: block ");
		اكسر;
	حالة CSS_DISPLAY_LIST_شي:
		fprintf(وهڪرو, "display: list-item ");
		اكسر;
	حالة CSS_DISPLAY_لاڳيتو_اندريون:
		fprintf(وهڪرو, "display: run-in ");
		اكسر;
	حالة CSS_DISPLAY_INLINE_بلاڪ:
		fprintf(وهڪرو, "display: inline-block ");
		اكسر;
	حالة CSS_DISPLAY_جدول:
		fprintf(وهڪرو, "display: table ");
		اكسر;
	حالة CSS_DISPLAY_INLINE_جدول:
		fprintf(وهڪرو, "display: inline-table ");
		اكسر;
	حالة CSS_DISPLAY_جدول_صف_گروهه:
		fprintf(وهڪرو, "display: table-row-group ");
		اكسر;
	حالة CSS_DISPLAY_جدول_سرو_گروهه:
		fprintf(وهڪرو, "display: table-header-group ");
		اكسر;
	حالة CSS_DISPLAY_جدول_FOOTER_گروهه:
		fprintf(وهڪرو, "display: table-footer-group ");
		اكسر;
	حالة CSS_DISPLAY_جدول_صف:
		fprintf(وهڪرو, "display: table-row ");
		اكسر;
	حالة CSS_DISPLAY_جدول_ڪالم_گروهه:
		fprintf(وهڪرو, "display: table-column-group ");
		اكسر;
	حالة CSS_DISPLAY_جدول_ڪالم:
		fprintf(وهڪرو, "display: table-column ");
		اكسر;
	حالة CSS_DISPLAY_جدول_CELL:
		fprintf(وهڪرو, "display: table-cell ");
		اكسر;
	حالة CSS_DISPLAY_جدول_CAPTION:
		fprintf(وهڪرو, "display: table-caption ");
		اكسر;
	حالة CSS_DISPLAY_NONE:
		fprintf(وهڪرو, "display: none ");
		اكسر;
	حالة CSS_DISPLAY_FLEX:
		fprintf(وهڪرو, "display: flex ");
		اكسر;
	حالة CSS_DISPLAY_INLINE_FLEX:
		fprintf(وهڪرو, "display: inline-flex ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* empty-cells */
	val = css_computed_خالي_جايون_cells(style);
	بدل (val) {
	حالة CSS_خالي_جايون_CELLS_SHOW:
		fprintf(وهڪرو, "empty-cells: show ");
		اكسر;
	حالة CSS_خالي_جايون_CELLS_HIDE:
		fprintf(وهڪرو, "empty-cells: hide ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* float */
	val = css_computed_float(style);
	بدل (val) {
	حالة CSS_FLOAT_کاٻو:
		fprintf(وهڪرو, "float: left ");
		اكسر;
	حالة CSS_FLOAT_ساڄو:
		fprintf(وهڪرو, "float: right ");
		اكسر;
	حالة CSS_FLOAT_NONE:
		fprintf(وهڪرو, "float: none ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* font-family */
	val = css_computed_font_family(style, &string_list);
	إذا (val != CSS_FONT_FAMILY_INHERIT) {
		fprintf(وهڪرو, "font-family:");

		إذا (string_list != عدم) {
			طالما (*string_list != عدم) {
				fprintf(وهڪرو, " \"%.*s\"",
					(صحيح) lwc_string_ڊيگهائي(*string_list),
					lwc_string_معلومات(*string_list));

				string_list++;
			}
		}
		بدل (val) {
		حالة CSS_FONT_FAMILY_SERIF:
			fprintf(وهڪرو, " serif ");
			اكسر;
		حالة CSS_FONT_FAMILY_SANS_SERIF:
			fprintf(وهڪرو, " sans-serif ");
			اكسر;
		حالة CSS_FONT_FAMILY_CURSIVE:
			fprintf(وهڪرو, " cursive ");
			اكسر;
		حالة CSS_FONT_FAMILY_FANTASY:
			fprintf(وهڪرو, " fantasy ");
			اكسر;
		حالة CSS_FONT_FAMILY_MONOSPACE:
			fprintf(وهڪرو, " monospace ");
			اكسر;
		}
	}

	/* font-size */
	val = css_computed_font_سائيز(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_FONT_سائيز_XX_SMALL:
		fprintf(وهڪرو, "font-size: xx-small ");
		اكسر;
	حالة CSS_FONT_سائيز_X_SMALL:
		fprintf(وهڪرو, "font-size: x-small ");
		اكسر;
	حالة CSS_FONT_سائيز_SMALL:
		fprintf(وهڪرو, "font-size: small ");
		اكسر;
	حالة CSS_FONT_سائيز_MEDIUM:
		fprintf(وهڪرو, "font-size: medium ");
		اكسر;
	حالة CSS_FONT_سائيز_LARGE:
		fprintf(وهڪرو, "font-size: large ");
		اكسر;
	حالة CSS_FONT_سائيز_X_LARGE:
		fprintf(وهڪرو, "font-size: x-large ");
		اكسر;
	حالة CSS_FONT_سائيز_XX_LARGE:
		fprintf(وهڪرو, "font-size: xx-large ");
		اكسر;
	حالة CSS_FONT_سائيز_LARGER:
		fprintf(وهڪرو, "font-size: larger ");
		اكسر;
	حالة CSS_FONT_سائيز_SMALLER:
		fprintf(وهڪرو, "font-size: smaller ");
		اكسر;
	حالة CSS_FONT_سائيز_DIMENSION:
		fprintf(وهڪرو, "font-size: ");

		dump_css_unit(وهڪرو, len1, unit1);

		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* font-style */
	val = css_computed_font_style(style);
	بدل (val) {
	حالة CSS_FONT_STYLE_عام:
		fprintf(وهڪرو, "font-style: normal ");
		اكسر;
	حالة CSS_FONT_STYLE_ITALIC:
		fprintf(وهڪرو, "font-style: italic ");
		اكسر;
	حالة CSS_FONT_STYLE_OBLIQUE:
		fprintf(وهڪرو, "font-style: oblique ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* font-variant */
	val = css_computed_font_variant(style);
	بدل (val) {
	حالة CSS_FONT_VARIANT_عام:
		fprintf(وهڪرو, "font-variant: normal ");
		اكسر;
	حالة CSS_FONT_VARIANT_SMALL_CAPS:
		fprintf(وهڪرو, "font-variant: small-caps ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* font-weight */
	val = css_computed_font_وزن(style);
	بدل (val) {
	حالة CSS_FONT_وزن_عام:
		fprintf(وهڪرو, "font-weight: normal ");
		اكسر;
	حالة CSS_FONT_وزن_BOLD:
		fprintf(وهڪرو, "font-weight: bold ");
		اكسر;
	حالة CSS_FONT_وزن_BOLDER:
		fprintf(وهڪرو, "font-weight: bolder ");
		اكسر;
	حالة CSS_FONT_وزن_LIGHTER:
		fprintf(وهڪرو, "font-weight: lighter ");
		اكسر;
	حالة CSS_FONT_وزن_100:
		fprintf(وهڪرو, "font-weight: 100 ");
		اكسر;
	حالة CSS_FONT_وزن_200:
		fprintf(وهڪرو, "font-weight: 200 ");
		اكسر;
	حالة CSS_FONT_وزن_300:
		fprintf(وهڪرو, "font-weight: 300 ");
		اكسر;
	حالة CSS_FONT_وزن_400:
		fprintf(وهڪرو, "font-weight: 400 ");
		اكسر;
	حالة CSS_FONT_وزن_500:
		fprintf(وهڪرو, "font-weight: 500 ");
		اكسر;
	حالة CSS_FONT_وزن_600:
		fprintf(وهڪرو, "font-weight: 600 ");
		اكسر;
	حالة CSS_FONT_وزن_700:
		fprintf(وهڪرو, "font-weight: 700 ");
		اكسر;
	حالة CSS_FONT_وزن_800:
		fprintf(وهڪرو, "font-weight: 800 ");
		اكسر;
	حالة CSS_FONT_وزن_900:
		fprintf(وهڪرو, "font-weight: 900 ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/*اوچائي*/
	val = css_computed_اوچائي(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_اوچائي_AUTO:
		fprintf(وهڪرو, "height: auto ");
		اكسر;
	حالة CSS_اوچائي_سيٽ:
		fprintf(وهڪرو, "height: ");

		dump_css_unit(وهڪرو, len1, unit1);

		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/*کاٻو*/
	val = css_computed_کاٻو(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_کاٻو_AUTO:
		fprintf(وهڪرو, "left: auto ");
		اكسر;
	حالة CSS_کاٻو_سيٽ:
		fprintf(وهڪرو, "left: ");

		dump_css_unit(وهڪرو, len1, unit1);

		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* letter-spacing */
	val = css_computed_letter_spacing(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_LETTER_SPACING_عام:
		fprintf(وهڪرو, "letter-spacing: normal ");
		اكسر;
	حالة CSS_LETTER_SPACING_سيٽ:
		fprintf(وهڪرو, "letter-spacing: ");

		dump_css_unit(وهڪرو, len1, unit1);

		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* line-height */
	val = css_computed_سٽ_اوچائي(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_سٽ_اوچائي_عام:
		fprintf(وهڪرو, "line-height: normal ");
		اكسر;
	حالة CSS_سٽ_اوچائي_عدد:
		fprintf(وهڪرو, "line-height: ");

		dump_css_fixed(وهڪرو, len1);

		fprintf(وهڪرو, " ");
		اكسر;
	حالة CSS_سٽ_اوچائي_DIMENSION:
		fprintf(وهڪرو, "line-height: ");

		dump_css_unit(وهڪرو, len1, unit1);

		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* list-style-image */
	val = css_computed_list_style_image(style, &url);
	إذا (url != عدم) {
		fprintf(وهڪرو, "list-style-image: url('%.*s') ",
				(صحيح) lwc_string_ڊيگهائي(url),
				lwc_string_معلومات(url));
	} وإلا إذا (val == CSS_LIST_STYLE_IMAGE_NONE) {
		fprintf(وهڪرو, "list-style-image: none ");
	}

	/* list-style-position */
	val = css_computed_list_style_position(style);
	بدل (val) {
	حالة CSS_LIST_STYLE_POSITION_INSIDE:
		fprintf(وهڪرو, "list-style-position: inside ");
		اكسر;
	حالة CSS_LIST_STYLE_POSITION_OUTSIDE:
		fprintf(وهڪرو, "list-style-position: outside ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* list-style-type */
	val = css_computed_list_style_type(style);
	بدل (val) {
	حالة CSS_LIST_STYLE_TYPE_DISC:
		fprintf(وهڪرو, "list-style-type: disc ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_CIRCLE:
		fprintf(وهڪرو, "list-style-type: circle ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_چورس:
		fprintf(وهڪرو, "list-style-type: square ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_DECIMAL:
		fprintf(وهڪرو, "list-style-type: decimal ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_DECIMAL_LEADING_ZERO:
		fprintf(وهڪرو, "list-style-type: decimal-leading-zero ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_هيٺاهون_ROMAN:
		fprintf(وهڪرو, "list-style-type: lower-roman ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_مٿانهون_ROMAN:
		fprintf(وهڪرو, "list-style-type: upper-roman ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_هيٺاهون_GREEK:
		fprintf(وهڪرو, "list-style-type: lower-greek ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_هيٺاهون_LATIN:
		fprintf(وهڪرو, "list-style-type: lower-latin ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_مٿانهون_LATIN:
		fprintf(وهڪرو, "list-style-type: upper-latin ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_ARMENIAN:
		fprintf(وهڪرو, "list-style-type: armenian ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_GEORGIAN:
		fprintf(وهڪرو, "list-style-type: georgian ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_هيٺاهون_ALPHA:
		fprintf(وهڪرو, "list-style-type: lower-alpha ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_مٿانهون_ALPHA:
		fprintf(وهڪرو, "list-style-type: upper-alpha ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_NONE:
		fprintf(وهڪرو, "list-style-type: none ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* margin-top */
	val = css_computed_margin_چوٽي(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MARGIN_AUTO:
		fprintf(وهڪرو, "margin-top: auto ");
		اكسر;
	حالة CSS_MARGIN_سيٽ:
		fprintf(وهڪرو, "margin-top: ");

		dump_css_unit(وهڪرو, len1, unit1);

		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* margin-right */
	val = css_computed_margin_ساڄو(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MARGIN_AUTO:
		fprintf(وهڪرو, "margin-right: auto ");
		اكسر;
	حالة CSS_MARGIN_سيٽ:
		fprintf(وهڪرو, "margin-right: ");

		dump_css_unit(وهڪرو, len1, unit1);

		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* margin-bottom */
	val = css_computed_margin_bottom(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MARGIN_AUTO:
		fprintf(وهڪرو, "margin-bottom: auto ");
		اكسر;
	حالة CSS_MARGIN_سيٽ:
		fprintf(وهڪرو, "margin-bottom: ");

		dump_css_unit(وهڪرو, len1, unit1);

		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* margin-left */
	val = css_computed_margin_کاٻو(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MARGIN_AUTO:
		fprintf(وهڪرو, "margin-left: auto ");
		اكسر;
	حالة CSS_MARGIN_سيٽ:
		fprintf(وهڪرو, "margin-left: ");

		dump_css_unit(وهڪرو, len1, unit1);

		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* max-height */
	val = css_computed_max_اوچائي(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MAX_اوچائي_NONE:
		fprintf(وهڪرو, "max-height: none ");
		اكسر;
	حالة CSS_MAX_اوچائي_سيٽ:
		fprintf(وهڪرو, "max-height: ");

		dump_css_unit(وهڪرو, len1, unit1);

		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* max-width */
	val = css_computed_max_ويڪر(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MAX_ويڪر_NONE:
		fprintf(وهڪرو, "max-width: none ");
		اكسر;
	حالة CSS_MAX_ويڪر_سيٽ:
		fprintf(وهڪرو, "max-width: ");

		dump_css_unit(وهڪرو, len1, unit1);

		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* min-height */
	val = ns_computed_min_اوچائي(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MIN_اوچائي_سيٽ:
		fprintf(وهڪرو, "min-height: ");

		dump_css_unit(وهڪرو, len1, unit1);

		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* min-width */
	val = ns_computed_min_ويڪر(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MIN_ويڪر_سيٽ:
		fprintf(وهڪرو, "min-width: ");

		dump_css_unit(وهڪرو, len1, unit1);

		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* opacity */
	val = css_computed_opacity(style, &len1);
	بدل (val) {
	حالة CSS_OPACITY_سيٽ:
		fprintf(وهڪرو, "opacity: ");

		dump_css_fixed(وهڪرو, len1);

		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* outline-color */
	val = css_computed_outline_color(style, &color);
	بدل (val) {
	حالة CSS_OUTLINE_COLOR_INVERT:
		fprintf(وهڪرو, "outline-color: invert ");
		اكسر;
	حالة CSS_OUTLINE_COLOR_COLOR:
		fprintf(وهڪرو, "outline-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* outline-style */
	val = css_computed_outline_style(style);
	بدل (val) {
	حالة CSS_OUTLINE_STYLE_NONE:
		fprintf(وهڪرو, "outline-style: none ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_DOTTED:
		fprintf(وهڪرو, "outline-style: dotted ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_DASHED:
		fprintf(وهڪرو, "outline-style: dashed ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_SOLID:
		fprintf(وهڪرو, "outline-style: solid ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_DOUBLE:
		fprintf(وهڪرو, "outline-style: double ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_GROOVE:
		fprintf(وهڪرو, "outline-style: groove ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_RIDGE:
		fprintf(وهڪرو, "outline-style: ridge ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_INSET:
		fprintf(وهڪرو, "outline-style: inset ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_OUTSET:
		fprintf(وهڪرو, "outline-style: outset ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* outline-width */
	val = css_computed_outline_ويڪر(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_OUTLINE_ويڪر_THIN:
		fprintf(وهڪرو, "outline-width: thin ");
		اكسر;
	حالة CSS_OUTLINE_ويڪر_MEDIUM:
		fprintf(وهڪرو, "outline-width: medium ");
		اكسر;
	حالة CSS_OUTLINE_ويڪر_THICK:
		fprintf(وهڪرو, "outline-width: thick ");
		اكسر;
	حالة CSS_OUTLINE_ويڪر_ويڪر:
		fprintf(وهڪرو, "outline-width: ");

		dump_css_unit(وهڪرو, len1, unit1);

		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/*حد_کان_مٿي*/
	val = css_computed_حد_کان_مٿي_x(style);
	بدل (val) {
	حالة CSS_حد_کان_مٿي_VISIBLE:
		fprintf(وهڪرو, "overflow-x: visible ");
		اكسر;
	حالة CSS_حد_کان_مٿي_HIDDEN:
		fprintf(وهڪرو, "overflow-x: hidden ");
		اكسر;
	حالة CSS_حد_کان_مٿي_SCROLL:
		fprintf(وهڪرو, "overflow-x: scroll ");
		اكسر;
	حالة CSS_حد_کان_مٿي_AUTO:
		fprintf(وهڪرو, "overflow-x auto ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/*حد_کان_مٿي*/
	val = css_computed_حد_کان_مٿي_y(style);
	بدل (val) {
	حالة CSS_حد_کان_مٿي_VISIBLE:
		fprintf(وهڪرو, "overflow-y: visible ");
		اكسر;
	حالة CSS_حد_کان_مٿي_HIDDEN:
		fprintf(وهڪرو, "overflow-y: hidden ");
		اكسر;
	حالة CSS_حد_کان_مٿي_SCROLL:
		fprintf(وهڪرو, "overflow-y: scroll ");
		اكسر;
	حالة CSS_حد_کان_مٿي_AUTO:
		fprintf(وهڪرو, "overflow-y: auto ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* padding-top */
	val = css_computed_padding_چوٽي(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_PADDING_سيٽ:
		fprintf(وهڪرو, "padding-top: ");

		dump_css_unit(وهڪرو, len1, unit1);

		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* padding-right */
	val = css_computed_padding_ساڄو(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_PADDING_سيٽ:
		fprintf(وهڪرو, "padding-right: ");

		dump_css_unit(وهڪرو, len1, unit1);

		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* padding-bottom */
	val = css_computed_padding_bottom(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_PADDING_سيٽ:
		fprintf(وهڪرو, "padding-bottom: ");

		dump_css_unit(وهڪرو, len1, unit1);

		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* padding-left */
	val = css_computed_padding_کاٻو(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_PADDING_سيٽ:
		fprintf(وهڪرو, "padding-left: ");

		dump_css_unit(وهڪرو, len1, unit1);

		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* position */
	val = css_computed_position(style);
	بدل (val) {
	حالة CSS_POSITION_STATIC:
		fprintf(وهڪرو, "position: static ");
		اكسر;
	حالة CSS_POSITION_RELATIVE:
		fprintf(وهڪرو, "position: relative ");
		اكسر;
	حالة CSS_POSITION_ABSOLUTE:
		fprintf(وهڪرو, "position: absolute ");
		اكسر;
	حالة CSS_POSITION_FIXED:
		fprintf(وهڪرو, "position: fixed ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* quotes */
	val = css_computed_quotes(style, &string_list);
	إذا (val == CSS_QUOTES_STRING && string_list != عدم) {
		fprintf(وهڪرو, "quotes:");

		طالما (*string_list != عدم) {
			fprintf(وهڪرو, " \"%.*s\"",
				(صحيح) lwc_string_ڊيگهائي(*string_list),
				lwc_string_معلومات(*string_list));

			string_list++;
		}

		fprintf(وهڪرو, " ");
	} وإلا {
		بدل (val) {
		حالة CSS_QUOTES_NONE:
			fprintf(وهڪرو, "quotes: none ");
			اكسر;
		افتراضي:
			اكسر;
		}
	}

	/*ساڄو*/
	val = css_computed_ساڄو(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_ساڄو_AUTO:
		fprintf(وهڪرو, "right: auto ");
		اكسر;
	حالة CSS_ساڄو_سيٽ:
		fprintf(وهڪرو, "right: ");

		dump_css_unit(وهڪرو, len1, unit1);

		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* table-layout */
	val = css_computed_جدول_layout(style);
	بدل (val) {
	حالة CSS_جدول_LAYOUT_AUTO:
		fprintf(وهڪرو, "table-layout: auto ");
		اكسر;
	حالة CSS_جدول_LAYOUT_FIXED:
		fprintf(وهڪرو, "table-layout: fixed ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* text-align */
	val = css_computed_متن_align(style);
	بدل (val) {
	حالة CSS_متن_ALIGN_کاٻو:
		fprintf(وهڪرو, "text-align: left ");
		اكسر;
	حالة CSS_متن_ALIGN_ساڄو:
		fprintf(وهڪرو, "text-align: right ");
		اكسر;
	حالة CSS_متن_ALIGN_CENTER:
		fprintf(وهڪرو, "text-align: center ");
		اكسر;
	حالة CSS_متن_ALIGN_JUSTIFY:
		fprintf(وهڪرو, "text-align: justify ");
		اكسر;
	حالة CSS_متن_ALIGN_DEFAULT:
		fprintf(وهڪرو, "text-align: default ");
		اكسر;
	حالة CSS_متن_ALIGN_LIBCSS_کاٻو:
		fprintf(وهڪرو, "text-align: -libcss-left ");
		اكسر;
	حالة CSS_متن_ALIGN_LIBCSS_CENTER:
		fprintf(وهڪرو, "text-align: -libcss-center ");
		اكسر;
	حالة CSS_متن_ALIGN_LIBCSS_ساڄو:
		fprintf(وهڪرو, "text-align: -libcss-right ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* text-decoration */
	val = css_computed_متن_decoration(style);
	إذا (val == CSS_متن_DECORATION_NONE) {
		fprintf(وهڪرو, "text-decoration: none ");
	} وإلا {
		fprintf(وهڪرو, "text-decoration:");

		إذا (val & CSS_متن_DECORATION_BLINK) {
			fprintf(وهڪرو, " blink");
		}
		إذا (val & CSS_متن_DECORATION_سٽ_THROUGH) {
			fprintf(وهڪرو, " line-through");
		}
		إذا (val & CSS_متن_DECORATION_OVERLINE) {
			fprintf(وهڪرو, " overline");
		}
		إذا (val & CSS_متن_DECORATION_UNDERLINE) {
			fprintf(وهڪرو, " underline");
		}

		fprintf(وهڪرو, " ");
	}

	/* text-indent */
	val = css_computed_متن_indent(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_متن_INDENT_سيٽ:
		fprintf(وهڪرو, "text-indent: ");

		dump_css_unit(وهڪرو, len1, unit1);

		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* text-transform */
	val = css_computed_متن_transform(style);
	بدل (val) {
	حالة CSS_متن_TRANSFORM_CAPITALIZE:
		fprintf(وهڪرو, "text-transform: capitalize ");
		اكسر;
	حالة CSS_متن_TRANSFORM_UPPERCASE:
		fprintf(وهڪرو, "text-transform: uppercase ");
		اكسر;
	حالة CSS_متن_TRANSFORM_LOWERCASE:
		fprintf(وهڪرو, "text-transform: lowercase ");
		اكسر;
	حالة CSS_متن_TRANSFORM_NONE:
		fprintf(وهڪرو, "text-transform: none ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/*چوٽي*/
	val = css_computed_چوٽي(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_چوٽي_AUTO:
		fprintf(وهڪرو, "top: auto ");
		اكسر;
	حالة CSS_چوٽي_سيٽ:
		fprintf(وهڪرو, "top: ");

		dump_css_unit(وهڪرو, len1, unit1);

		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* unicode-bidi */
	val = css_computed_unicode_bidi(style);
	بدل (val) {
	حالة CSS_UNICODE_BIDI_عام:
		fprintf(وهڪرو, "unicode-bidi: normal ");
		اكسر;
	حالة CSS_UNICODE_BIDI_EMBED:
		fprintf(وهڪرو, "unicode-bidi: embed ");
		اكسر;
	حالة CSS_UNICODE_BIDI_BIDI_OVERRIDE:
		fprintf(وهڪرو, "unicode-bidi: bidi-override ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* vertical-align */
	val = css_computed_vertical_align(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_VERTICAL_ALIGN_BASELINE:
		fprintf(وهڪرو, "vertical-align: baseline ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_SUB:
		fprintf(وهڪرو, "vertical-align: sub ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_SUPER:
		fprintf(وهڪرو, "vertical-align: super ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_چوٽي:
		fprintf(وهڪرو, "vertical-align: top ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_متن_چوٽي:
		fprintf(وهڪرو, "vertical-align: text-top ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_وچ:
		fprintf(وهڪرو, "vertical-align: middle ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_BOTTOM:
		fprintf(وهڪرو, "vertical-align: bottom ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_متن_BOTTOM:
		fprintf(وهڪرو, "vertical-align: text-bottom ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_سيٽ:
		fprintf(وهڪرو, "vertical-align: ");

		dump_css_unit(وهڪرو, len1, unit1);

		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* visibility */
	val = css_computed_visibility(style);
	بدل (val) {
	حالة CSS_VISIBILITY_VISIBLE:
		fprintf(وهڪرو, "visibility: visible ");
		اكسر;
	حالة CSS_VISIBILITY_HIDDEN:
		fprintf(وهڪرو, "visibility: hidden ");
		اكسر;
	حالة CSS_VISIBILITY_COLLAPSE:
		fprintf(وهڪرو, "visibility: collapse ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* white-space */
	val = css_computed_white_space(style);
	بدل (val) {
	حالة CSS_WHITE_SPACE_عام:
		fprintf(وهڪرو, "white-space: normal ");
		اكسر;
	حالة CSS_WHITE_SPACE_PRE:
		fprintf(وهڪرو, "white-space: pre ");
		اكسر;
	حالة CSS_WHITE_SPACE_NOWRAP:
		fprintf(وهڪرو, "white-space: nowrap ");
		اكسر;
	حالة CSS_WHITE_SPACE_PRE_WRAP:
		fprintf(وهڪرو, "white-space: pre-wrap ");
		اكسر;
	حالة CSS_WHITE_SPACE_PRE_سٽ:
		fprintf(وهڪرو, "white-space: pre-line ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/*ويڪر*/
	val = css_computed_ويڪر(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_ويڪر_AUTO:
		fprintf(وهڪرو, "width: auto ");
		اكسر;
	حالة CSS_ويڪر_سيٽ:
		fprintf(وهڪرو, "width: ");

		dump_css_unit(وهڪرو, len1, unit1);

		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* word-spacing */
	val = css_computed_لفظ_spacing(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_لفظ_SPACING_عام:
		fprintf(وهڪرو, "word-spacing: normal ");
		اكسر;
	حالة CSS_لفظ_SPACING_سيٽ:
		fprintf(وهڪرو, "word-spacing: ");

		dump_css_unit(وهڪرو, len1, unit1);

		fprintf(وهڪرو, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* z-index */
	val = css_computed_z_اشاريو(style, &zindex);
	بدل (val) {
	حالة CSS_Z_اشاريو_AUTO:
		fprintf(وهڪرو, "z-index: auto ");
		اكسر;
	حالة CSS_Z_اشاريو_سيٽ:
		fprintf(وهڪرو, "z-index: %"PRId32" ", zindex);
		اكسر;
	افتراضي:
		اكسر;
	}

	fprintf(وهڪرو, "}");
}
