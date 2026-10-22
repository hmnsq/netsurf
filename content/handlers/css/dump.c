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
ساكن فراغ dump_css_fixed(FILE *بهیر, css_fixed f)
{
#تعریف_کړه NSCSS_ABS(x) (uint32_t)((x) < 0 ? -(x) : (x))
	uint32_t uintpart = FIXTOINT(NSCSS_ABS(f));
	/* + 500 to ensure round to nearest (division will truncate) */
	uint32_t fracpart = ((NSCSS_ABS(f) & 0x3ff) * 1000 + 500) / (1 << 10);
#تعریف_لغوه NSCSS_ABS

	fprintf(بهیر, "%s%"PRIu32".%03"PRIu32, f < 0 ? "-" : "", uintpart, fracpart);
}

/**
 * Dump a numeric value to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 */
ساكن فراغ dump_css_شمېره(FILE *بهیر, css_fixed val)
{
	إذا (INTTOFIX(FIXTOINT(val)) == val)
		fprintf(بهیر, "%"PRId32, FIXTOINT(val));
	وإلا
		dump_css_fixed(بهیر, val);
}

/**
 * Dump a dimension to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 * \param unit    Unit to write
 */
ساكن فراغ dump_css_unit(FILE *بهیر, css_fixed val, css_unit unit)
{
	dump_css_شمېره(بهیر, val);

	بدل (unit) {
	حالة CSS_UNIT_PX:
		fprintf(بهیر, "px");
		اكسر;
	حالة CSS_UNIT_EX:
		fprintf(بهیر, "ex");
		اكسر;
	حالة CSS_UNIT_EM:
		fprintf(بهیر, "em");
		اكسر;
	حالة CSS_UNIT_دننه:
		fprintf(بهیر, "in");
		اكسر;
	حالة CSS_UNIT_CM:
		fprintf(بهیر, "cm");
		اكسر;
	حالة CSS_UNIT_MM:
		fprintf(بهیر, "mm");
		اكسر;
	حالة CSS_UNIT_PT:
		fprintf(بهیر, "pt");
		اكسر;
	حالة CSS_UNIT_PC:
		fprintf(بهیر, "pc");
		اكسر;
	حالة CSS_UNIT_PCT:
		fprintf(بهیر, "%%");
		اكسر;
	حالة CSS_UNIT_DEG:
		fprintf(بهیر, "deg");
		اكسر;
	حالة CSS_UNIT_GRAD:
		fprintf(بهیر, "grad");
		اكسر;
	حالة CSS_UNIT_RAD:
		fprintf(بهیر, "rad");
		اكسر;
	حالة CSS_UNIT_MS:
		fprintf(بهیر, "ms");
		اكسر;
	حالة CSS_UNIT_S:
		fprintf(بهیر, "s");
		اكسر;
	حالة CSS_UNIT_HZ:
		fprintf(بهیر, "Hz");
		اكسر;
	حالة CSS_UNIT_KHZ:
		fprintf(بهیر, "kHz");
		اكسر;
	حالة CSS_UNIT_CH:
		fprintf(بهیر, "ch");
		اكسر;
	حالة CSS_UNIT_REM:
		fprintf(بهیر, "rem");
		اكسر;
	حالة CSS_UNIT_LH:
		fprintf(بهیر, "lh");
		اكسر;
	حالة CSS_UNIT_VH:
		fprintf(بهیر, "vh");
		اكسر;
	حالة CSS_UNIT_VW:
		fprintf(بهیر, "vw");
		اكسر;
	حالة CSS_UNIT_VI:
		fprintf(بهیر, "vi");
		اكسر;
	حالة CSS_UNIT_VB:
		fprintf(بهیر, "vb");
		اكسر;
	حالة CSS_UNIT_VMIN:
		fprintf(بهیر, "vmin");
		اكسر;
	حالة CSS_UNIT_VMAX:
		fprintf(بهیر, "vmax");
		اكسر;
	حالة CSS_UNIT_Q:
		fprintf(بهیر, "q");
		اكسر;
	حالة CSS_UNIT_CALC:
		fprintf(بهیر, "calc()");
		اكسر;
	}
}

/* exported interface documented in content/handlers/css/dump.h */
فراغ nscss_dump_computed_style(FILE *بهیر, ثابت css_computed_style *style)
{
	uint8_t val;
	css_color color = 0;
	lwc_string *url = عدم;
	css_fixed len1 = 0, len2 = 0;
	css_unit unit1 = CSS_UNIT_PX, unit2 = CSS_UNIT_PX;
	css_computed_clip_rect rect = { 0, 0, 0, 0, CSS_UNIT_PX, CSS_UNIT_PX,
					CSS_UNIT_PX, CSS_UNIT_PX, true, true,
					true, true };
	ثابت css_computed_content_توکی *content = عدم;
	ثابت css_computed_counter *counter = عدم;
	lwc_string **string_list = عدم;
	int32_t zindex = 0;

	fprintf(بهیر, "{ ");

	/* background-attachment */
	val = css_computed_background_attachment(style);
	بدل (val) {
	حالة CSS_BACKGROUND_ATTACHMENT_FIXED:
		fprintf(بهیر, "background-attachment: fixed ");
		اكسر;
	حالة CSS_BACKGROUND_ATTACHMENT_SCROLL:
		fprintf(بهیر, "background-attachment: scroll ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* background-color */
	val = css_computed_background_color(style, &color);
	بدل (val) {
	حالة CSS_BACKGROUND_COLOR_COLOR:
		fprintf(بهیر, "background-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* background-image */
	val = css_computed_background_image(style, &url);
	إذا (val == CSS_BACKGROUND_IMAGE_IMAGE && url != عدم) {
		fprintf(بهیر, "background-image: url('%.*s') ",
				(صحيح) lwc_string_اوږدتیا(url),
				lwc_string_معلومات(url));
	} وإلا إذا (val == CSS_BACKGROUND_IMAGE_NONE) {
		fprintf(بهیر, "background-image: none ");
	}

	/* background-position */
	val = css_computed_background_position(style, &len1, &unit1,
			&len2, &unit2);
	إذا (val == CSS_BACKGROUND_POSITION_سېټ) {
		fprintf(بهیر, "background-position: ");
		dump_css_unit(بهیر, len1, unit1);
		fprintf(بهیر, " ");
		dump_css_unit(بهیر, len2, unit2);
		fprintf(بهیر, " ");
	}

	/* background-repeat */
	val = css_computed_background_repeat(style);
	بدل (val) {
	حالة CSS_BACKGROUND_REPEAT_REPEAT_X:
		fprintf(بهیر, "background-repeat: repeat-x ");
		اكسر;
	حالة CSS_BACKGROUND_REPEAT_REPEAT_Y:
		fprintf(بهیر, "background-repeat: repeat-y ");
		اكسر;
	حالة CSS_BACKGROUND_REPEAT_REPEAT:
		fprintf(بهیر, "background-repeat: repeat ");
		اكسر;
	حالة CSS_BACKGROUND_REPEAT_NO_REPEAT:
		fprintf(بهیر, "background-repeat: no-repeat ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-collapse */
	val = css_computed_border_collapse(style);
	بدل (val) {
	حالة CSS_BORDER_COLLAPSE_SEPARATE:
		fprintf(بهیر, "border-collapse: separate ");
		اكسر;
	حالة CSS_BORDER_COLLAPSE_COLLAPSE:
		fprintf(بهیر, "border-collapse: collapse ");
		اكسر;
	افتراضي:

		اكسر;
	}

	/* border-spacing */
	val = css_computed_border_spacing(style, &len1, &unit1, &len2, &unit2);
	إذا (val == CSS_BORDER_SPACING_سېټ) {
		fprintf(بهیر, "border-spacing: ");
		dump_css_unit(بهیر, len1, unit1);
		fprintf(بهیر, " ");
		dump_css_unit(بهیر, len2, unit2);
		fprintf(بهیر, " ");
	}

	/* border-top-color */
	val = css_computed_border_سر_color(style, &color);
	بدل (val) {
	حالة CSS_BORDER_COLOR_COLOR:
		fprintf(بهیر, "border-top-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-right-color */
	val = css_computed_border_ښی_color(style, &color);
	بدل (val) {
	حالة CSS_BORDER_COLOR_COLOR:
		fprintf(بهیر, "border-right-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-bottom-color */
	val = css_computed_border_bottom_color(style, &color);
	بدل (val) {
	حالة CSS_BORDER_COLOR_COLOR:
		fprintf(بهیر, "border-bottom-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-left-color */
	val = css_computed_border_کیڼ_color(style, &color);
	بدل (val) {
	حالة CSS_BORDER_COLOR_COLOR:
		fprintf(بهیر, "border-left-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-top-style */
	val = css_computed_border_سر_style(style);
	بدل (val) {
	حالة CSS_BORDER_STYLE_NONE:
		fprintf(بهیر, "border-top-style: none ");
		اكسر;
	حالة CSS_BORDER_STYLE_HIDDEN:
		fprintf(بهیر, "border-top-style: hidden ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOTTED:
		fprintf(بهیر, "border-top-style: dotted ");
		اكسر;
	حالة CSS_BORDER_STYLE_DASHED:
		fprintf(بهیر, "border-top-style: dashed ");
		اكسر;
	حالة CSS_BORDER_STYLE_SOLID:
		fprintf(بهیر, "border-top-style: solid ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOUBLE:
		fprintf(بهیر, "border-top-style: double ");
		اكسر;
	حالة CSS_BORDER_STYLE_GROOVE:
		fprintf(بهیر, "border-top-style: groove ");
		اكسر;
	حالة CSS_BORDER_STYLE_RIDGE:
		fprintf(بهیر, "border-top-style: ridge ");
		اكسر;
	حالة CSS_BORDER_STYLE_INSET:
		fprintf(بهیر, "border-top-style: inset ");
		اكسر;
	حالة CSS_BORDER_STYLE_OUTSET:
		fprintf(بهیر, "border-top-style: outset ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-right-style */
	val = css_computed_border_ښی_style(style);
	بدل (val) {
	حالة CSS_BORDER_STYLE_NONE:
		fprintf(بهیر, "border-right-style: none ");
		اكسر;
	حالة CSS_BORDER_STYLE_HIDDEN:
		fprintf(بهیر, "border-right-style: hidden ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOTTED:
		fprintf(بهیر, "border-right-style: dotted ");
		اكسر;
	حالة CSS_BORDER_STYLE_DASHED:
		fprintf(بهیر, "border-right-style: dashed ");
		اكسر;
	حالة CSS_BORDER_STYLE_SOLID:
		fprintf(بهیر, "border-right-style: solid ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOUBLE:
		fprintf(بهیر, "border-right-style: double ");
		اكسر;
	حالة CSS_BORDER_STYLE_GROOVE:
		fprintf(بهیر, "border-right-style: groove ");
		اكسر;
	حالة CSS_BORDER_STYLE_RIDGE:
		fprintf(بهیر, "border-right-style: ridge ");
		اكسر;
	حالة CSS_BORDER_STYLE_INSET:
		fprintf(بهیر, "border-right-style: inset ");
		اكسر;
	حالة CSS_BORDER_STYLE_OUTSET:
		fprintf(بهیر, "border-right-style: outset ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-bottom-style */
	val = css_computed_border_bottom_style(style);
	بدل (val) {
	حالة CSS_BORDER_STYLE_NONE:
		fprintf(بهیر, "border-bottom-style: none ");
		اكسر;
	حالة CSS_BORDER_STYLE_HIDDEN:
		fprintf(بهیر, "border-bottom-style: hidden ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOTTED:
		fprintf(بهیر, "border-bottom-style: dotted ");
		اكسر;
	حالة CSS_BORDER_STYLE_DASHED:
		fprintf(بهیر, "border-bottom-style: dashed ");
		اكسر;
	حالة CSS_BORDER_STYLE_SOLID:
		fprintf(بهیر, "border-bottom-style: solid ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOUBLE:
		fprintf(بهیر, "border-bottom-style: double ");
		اكسر;
	حالة CSS_BORDER_STYLE_GROOVE:
		fprintf(بهیر, "border-bottom-style: groove ");
		اكسر;
	حالة CSS_BORDER_STYLE_RIDGE:
		fprintf(بهیر, "border-bottom-style: ridge ");
		اكسر;
	حالة CSS_BORDER_STYLE_INSET:
		fprintf(بهیر, "border-bottom-style: inset ");
		اكسر;
	حالة CSS_BORDER_STYLE_OUTSET:
		fprintf(بهیر, "border-bottom-style: outset ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-left-style */
	val = css_computed_border_کیڼ_style(style);
	بدل (val) {
	حالة CSS_BORDER_STYLE_NONE:
		fprintf(بهیر, "border-left-style: none ");
		اكسر;
	حالة CSS_BORDER_STYLE_HIDDEN:
		fprintf(بهیر, "border-left-style: hidden ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOTTED:
		fprintf(بهیر, "border-left-style: dotted ");
		اكسر;
	حالة CSS_BORDER_STYLE_DASHED:
		fprintf(بهیر, "border-left-style: dashed ");
		اكسر;
	حالة CSS_BORDER_STYLE_SOLID:
		fprintf(بهیر, "border-left-style: solid ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOUBLE:
		fprintf(بهیر, "border-left-style: double ");
		اكسر;
	حالة CSS_BORDER_STYLE_GROOVE:
		fprintf(بهیر, "border-left-style: groove ");
		اكسر;
	حالة CSS_BORDER_STYLE_RIDGE:
		fprintf(بهیر, "border-left-style: ridge ");
		اكسر;
	حالة CSS_BORDER_STYLE_INSET:
		fprintf(بهیر, "border-left-style: inset ");
		اكسر;
	حالة CSS_BORDER_STYLE_OUTSET:
		fprintf(بهیر, "border-left-style: outset ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-top-width */
	val = css_computed_border_سر_پلنوالی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_BORDER_پلنوالی_THIN:
		fprintf(بهیر, "border-top-width: thin ");
		اكسر;
	حالة CSS_BORDER_پلنوالی_MEDIUM:
		fprintf(بهیر, "border-top-width: medium ");
		اكسر;
	حالة CSS_BORDER_پلنوالی_THICK:
		fprintf(بهیر, "border-top-width: thick ");
		اكسر;
	حالة CSS_BORDER_پلنوالی_پلنوالی:
		fprintf(بهیر, "border-top-width: ");
		dump_css_unit(بهیر, len1, unit1);
		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-right-width */
	val = css_computed_border_ښی_پلنوالی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_BORDER_پلنوالی_THIN:
		fprintf(بهیر, "border-right-width: thin ");
		اكسر;
	حالة CSS_BORDER_پلنوالی_MEDIUM:
		fprintf(بهیر, "border-right-width: medium ");
		اكسر;
	حالة CSS_BORDER_پلنوالی_THICK:
		fprintf(بهیر, "border-right-width: thick ");
		اكسر;
	حالة CSS_BORDER_پلنوالی_پلنوالی:
		fprintf(بهیر, "border-right-width: ");
		dump_css_unit(بهیر, len1, unit1);
		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-bottom-width */
	val = css_computed_border_bottom_پلنوالی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_BORDER_پلنوالی_THIN:
		fprintf(بهیر, "border-bottom-width: thin ");
		اكسر;
	حالة CSS_BORDER_پلنوالی_MEDIUM:
		fprintf(بهیر, "border-bottom-width: medium ");
		اكسر;
	حالة CSS_BORDER_پلنوالی_THICK:
		fprintf(بهیر, "border-bottom-width: thick ");
		اكسر;
	حالة CSS_BORDER_پلنوالی_پلنوالی:
		fprintf(بهیر, "border-bottom-width: ");
		dump_css_unit(بهیر, len1, unit1);
		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-left-width */
	val = css_computed_border_کیڼ_پلنوالی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_BORDER_پلنوالی_THIN:
		fprintf(بهیر, "border-left-width: thin ");
		اكسر;
	حالة CSS_BORDER_پلنوالی_MEDIUM:
		fprintf(بهیر, "border-left-width: medium ");
		اكسر;
	حالة CSS_BORDER_پلنوالی_THICK:
		fprintf(بهیر, "border-left-width: thick ");
		اكسر;
	حالة CSS_BORDER_پلنوالی_پلنوالی:
		fprintf(بهیر, "border-left-width: ");
		dump_css_unit(بهیر, len1, unit1);
		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* bottom */
	val = css_computed_bottom(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_BOTTOM_AUTO:
		fprintf(بهیر, "bottom: auto ");
		اكسر;
	حالة CSS_BOTTOM_سېټ:
		fprintf(بهیر, "bottom: ");
		dump_css_unit(بهیر, len1, unit1);
		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* caption-side */
	val = css_computed_caption_side(style);
	بدل (val) {
	حالة CSS_CAPTION_SIDE_سر:
		fprintf(بهیر, "caption_side: top ");
		اكسر;
	حالة CSS_CAPTION_SIDE_BOTTOM:
		fprintf(بهیر, "caption_side: bottom ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* clear */
	val = css_computed_clear(style);
	بدل (val) {
	حالة CSS_CLEAR_NONE:
		fprintf(بهیر, "clear: none ");
		اكسر;
	حالة CSS_CLEAR_کیڼ:
		fprintf(بهیر, "clear: left ");
		اكسر;
	حالة CSS_CLEAR_ښی:
		fprintf(بهیر, "clear: right ");
		اكسر;
	حالة CSS_CLEAR_BOTH:
		fprintf(بهیر, "clear: both ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* clip */
	val = css_computed_clip(style, &rect);
	بدل (val) {
	حالة CSS_CLIP_AUTO:
		fprintf(بهیر, "clip: auto ");
		اكسر;
	حالة CSS_CLIP_RECT:
		fprintf(بهیر, "clip: rect( ");

		إذا (rect.سر_auto)
			fprintf(بهیر, "auto");
		وإلا
			dump_css_unit(بهیر, rect.سر, rect.tunit);
		fprintf(بهیر, ", ");

		إذا (rect.ښی_auto)
			fprintf(بهیر, "auto");
		وإلا
			dump_css_unit(بهیر, rect.ښی, rect.runit);
		fprintf(بهیر, ", ");

		إذا (rect.bottom_auto)
			fprintf(بهیر, "auto");
		وإلا
			dump_css_unit(بهیر, rect.bottom, rect.bunit);
		fprintf(بهیر, ", ");

		إذا (rect.کیڼ_auto)
			fprintf(بهیر, "auto");
		وإلا
			dump_css_unit(بهیر, rect.کیڼ, rect.lunit);
		fprintf(بهیر, ") ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* color */
	val = css_computed_color(style, &color);
	إذا (val == CSS_COLOR_COLOR) {
		fprintf(بهیر, "color: #%08"PRIx32" ", color);
	}

	/* content */
	val = css_computed_content(style, &content);
	بدل (val) {
	حالة CSS_CONTENT_NONE:
		fprintf(بهیر, "content: none ");
		اكسر;
	حالة CSS_CONTENT_نورمال:
		fprintf(بهیر, "content: normal ");
		اكسر;
	حالة CSS_CONTENT_سېټ:
		fprintf(بهیر, "content:");

		طالما (content->type != CSS_COMPUTED_CONTENT_NONE) {
			fprintf(بهیر, " ");

			بدل (content->type) {
			حالة CSS_COMPUTED_CONTENT_STRING:
				fprintf(بهیر,	"\"%.*s\"",
						(صحيح) lwc_string_اوږدتیا(
						content->معلومات.string),
						lwc_string_معلومات(
						content->معلومات.string));
				اكسر;
			حالة CSS_COMPUTED_CONTENT_URI:
				fprintf(بهیر,	"uri(\"%.*s\")",
						(صحيح) lwc_string_اوږدتیا(
						content->معلومات.uri),
						lwc_string_معلومات(
						content->معلومات.uri));
				اكسر;
			حالة CSS_COMPUTED_CONTENT_COUNTER:
				fprintf(بهیر, "counter(%.*s)",
						(صحيح) lwc_string_اوږدتیا(
						content->معلومات.counter.name),
						lwc_string_معلومات(
						content->معلومات.counter.name));
				اكسر;
			حالة CSS_COMPUTED_CONTENT_COUNTERS:
				fprintf(بهیر, "counters(%.*s, \"%.*s\")",
						(صحيح) lwc_string_اوږدتیا(
						content->معلومات.counters.name),
						lwc_string_معلومات(
						content->معلومات.counters.name),
						(صحيح) lwc_string_اوږدتیا(
						content->معلومات.counters.sep),
						lwc_string_معلومات(
						content->معلومات.counters.sep));
				اكسر;
			حالة CSS_COMPUTED_CONTENT_ATTR:
				fprintf(بهیر, "attr(%.*s)",
						(صحيح) lwc_string_اوږدتیا(
						content->معلومات.attr),
						lwc_string_معلومات(
						content->معلومات.attr));
				اكسر;
			حالة CSS_COMPUTED_CONTENT_OPEN_QUOTE:
				fprintf(بهیر, "open-quote");
				اكسر;
			حالة CSS_COMPUTED_CONTENT_CLOSE_QUOTE:
				fprintf(بهیر, "close-quote");
				اكسر;
			حالة CSS_COMPUTED_CONTENT_NO_OPEN_QUOTE:
				fprintf(بهیر, "no-open-quote");
				اكسر;
			حالة CSS_COMPUTED_CONTENT_NO_CLOSE_QUOTE:
				fprintf(بهیر, "no-close-quote");
				اكسر;
			}

			content++;
		}

		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* counter-increment */
	val = css_computed_counter_increment(style, &counter);
	إذا ((val == CSS_COUNTER_INCREMENT_NONE) || (counter == عدم)) {
		fprintf(بهیر, "counter-increment: none ");
	} وإلا {
		fprintf(بهیر, "counter-increment:");

		طالما (counter->name != عدم) {
			fprintf(بهیر, " %.*s ",
					(صحيح) lwc_string_اوږدتیا(counter->name),
					lwc_string_معلومات(counter->name));

			dump_css_fixed(بهیر, counter->ارزښت);

			counter++;
		}

		fprintf(بهیر, " ");
	}

	/* counter-reset */
	val = css_computed_counter_reset(style, &counter);
	إذا ((val == CSS_COUNTER_RESET_NONE) || (counter == عدم)) {
		fprintf(بهیر, "counter-reset: none ");
	} وإلا {
		fprintf(بهیر, "counter-reset:");

		طالما (counter->name != عدم) {
			fprintf(بهیر, " %.*s ",
					(صحيح) lwc_string_اوږدتیا(counter->name),
					lwc_string_معلومات(counter->name));

			dump_css_fixed(بهیر, counter->ارزښت);

			counter++;
		}

		fprintf(بهیر, " ");
	}

	/* cursor */
	val = css_computed_cursor(style, &string_list);
	fprintf(بهیر, "cursor:");

	إذا (string_list != عدم) {
		طالما (*string_list != عدم) {
			fprintf(بهیر, " url\"%.*s\")",
					(صحيح) lwc_string_اوږدتیا(*string_list),
					lwc_string_معلومات(*string_list));

			string_list++;
		}
	}
	بدل (val) {
	حالة CSS_CURSOR_AUTO:
		fprintf(بهیر, " auto ");
		اكسر;
	حالة CSS_CURSOR_CROSSHAIR:
		fprintf(بهیر, " crosshair ");
		اكسر;
	حالة CSS_CURSOR_DEFAULT:
		fprintf(بهیر, " default ");
		اكسر;
	حالة CSS_CURSOR_اشاره_ګر:
		fprintf(بهیر, " pointer ");
		اكسر;
	حالة CSS_CURSOR_MOVE:
		fprintf(بهیر, " move ");
		اكسر;
	حالة CSS_CURSOR_E_RESIZE:
		fprintf(بهیر, " e-resize ");
		اكسر;
	حالة CSS_CURSOR_NE_RESIZE:
		fprintf(بهیر, " ne-resize ");
		اكسر;
	حالة CSS_CURSOR_NW_RESIZE:
		fprintf(بهیر, " nw-resize ");
		اكسر;
	حالة CSS_CURSOR_N_RESIZE:
		fprintf(بهیر, " n-resize ");
		اكسر;
	حالة CSS_CURSOR_SE_RESIZE:
		fprintf(بهیر, " se-resize ");
		اكسر;
	حالة CSS_CURSOR_SW_RESIZE:
		fprintf(بهیر, " sw-resize ");
		اكسر;
	حالة CSS_CURSOR_S_RESIZE:
		fprintf(بهیر, " s-resize ");
		اكسر;
	حالة CSS_CURSOR_W_RESIZE:
		fprintf(بهیر, " w-resize ");
		اكسر;
	حالة CSS_CURSOR_متن:
		fprintf(بهیر, " text ");
		اكسر;
	حالة CSS_CURSOR_WAIT:
		fprintf(بهیر, " wait ");
		اكسر;
	حالة CSS_CURSOR_HELP:
		fprintf(بهیر, " help ");
		اكسر;
	حالة CSS_CURSOR_پرمختګ:
		fprintf(بهیر, " progress ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* direction */
	val = css_computed_direction(style);
	بدل (val) {
	حالة CSS_DIRECTION_LTR:
		fprintf(بهیر, "direction: ltr ");
		اكسر;
	حالة CSS_DIRECTION_RTL:
		fprintf(بهیر, "direction: rtl ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* display */
	val = ns_computed_display_static(style);
	بدل (val) {
	حالة CSS_DISPLAY_INLINE:
		fprintf(بهیر, "display: inline ");
		اكسر;
	حالة CSS_DISPLAY_بلاک:
		fprintf(بهیر, "display: block ");
		اكسر;
	حالة CSS_DISPLAY_LIST_توکی:
		fprintf(بهیر, "display: list-item ");
		اكسر;
	حالة CSS_DISPLAY_پرلپسې_دننه:
		fprintf(بهیر, "display: run-in ");
		اكسر;
	حالة CSS_DISPLAY_INLINE_بلاک:
		fprintf(بهیر, "display: inline-block ");
		اكسر;
	حالة CSS_DISPLAY_جدول:
		fprintf(بهیر, "display: table ");
		اكسر;
	حالة CSS_DISPLAY_INLINE_جدول:
		fprintf(بهیر, "display: inline-table ");
		اكسر;
	حالة CSS_DISPLAY_جدول_لیکه_ډله:
		fprintf(بهیر, "display: table-row-group ");
		اكسر;
	حالة CSS_DISPLAY_جدول_سرلیک_ډله:
		fprintf(بهیر, "display: table-header-group ");
		اكسر;
	حالة CSS_DISPLAY_جدول_FOOTER_ډله:
		fprintf(بهیر, "display: table-footer-group ");
		اكسر;
	حالة CSS_DISPLAY_جدول_لیکه:
		fprintf(بهیر, "display: table-row ");
		اكسر;
	حالة CSS_DISPLAY_جدول_ستنه_ډله:
		fprintf(بهیر, "display: table-column-group ");
		اكسر;
	حالة CSS_DISPLAY_جدول_ستنه:
		fprintf(بهیر, "display: table-column ");
		اكسر;
	حالة CSS_DISPLAY_جدول_CELL:
		fprintf(بهیر, "display: table-cell ");
		اكسر;
	حالة CSS_DISPLAY_جدول_CAPTION:
		fprintf(بهیر, "display: table-caption ");
		اكسر;
	حالة CSS_DISPLAY_NONE:
		fprintf(بهیر, "display: none ");
		اكسر;
	حالة CSS_DISPLAY_FLEX:
		fprintf(بهیر, "display: flex ");
		اكسر;
	حالة CSS_DISPLAY_INLINE_FLEX:
		fprintf(بهیر, "display: inline-flex ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* empty-cells */
	val = css_computed_تشه_cells(style);
	بدل (val) {
	حالة CSS_تشه_CELLS_SHOW:
		fprintf(بهیر, "empty-cells: show ");
		اكسر;
	حالة CSS_تشه_CELLS_HIDE:
		fprintf(بهیر, "empty-cells: hide ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* float */
	val = css_computed_float(style);
	بدل (val) {
	حالة CSS_FLOAT_کیڼ:
		fprintf(بهیر, "float: left ");
		اكسر;
	حالة CSS_FLOAT_ښی:
		fprintf(بهیر, "float: right ");
		اكسر;
	حالة CSS_FLOAT_NONE:
		fprintf(بهیر, "float: none ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* font-family */
	val = css_computed_font_family(style, &string_list);
	إذا (val != CSS_FONT_FAMILY_INHERIT) {
		fprintf(بهیر, "font-family:");

		إذا (string_list != عدم) {
			طالما (*string_list != عدم) {
				fprintf(بهیر, " \"%.*s\"",
					(صحيح) lwc_string_اوږدتیا(*string_list),
					lwc_string_معلومات(*string_list));

				string_list++;
			}
		}
		بدل (val) {
		حالة CSS_FONT_FAMILY_SERIF:
			fprintf(بهیر, " serif ");
			اكسر;
		حالة CSS_FONT_FAMILY_SANS_SERIF:
			fprintf(بهیر, " sans-serif ");
			اكسر;
		حالة CSS_FONT_FAMILY_CURSIVE:
			fprintf(بهیر, " cursive ");
			اكسر;
		حالة CSS_FONT_FAMILY_FANTASY:
			fprintf(بهیر, " fantasy ");
			اكسر;
		حالة CSS_FONT_FAMILY_MONOSPACE:
			fprintf(بهیر, " monospace ");
			اكسر;
		}
	}

	/* font-size */
	val = css_computed_font_اندازه(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_FONT_اندازه_XX_SMALL:
		fprintf(بهیر, "font-size: xx-small ");
		اكسر;
	حالة CSS_FONT_اندازه_X_SMALL:
		fprintf(بهیر, "font-size: x-small ");
		اكسر;
	حالة CSS_FONT_اندازه_SMALL:
		fprintf(بهیر, "font-size: small ");
		اكسر;
	حالة CSS_FONT_اندازه_MEDIUM:
		fprintf(بهیر, "font-size: medium ");
		اكسر;
	حالة CSS_FONT_اندازه_LARGE:
		fprintf(بهیر, "font-size: large ");
		اكسر;
	حالة CSS_FONT_اندازه_X_LARGE:
		fprintf(بهیر, "font-size: x-large ");
		اكسر;
	حالة CSS_FONT_اندازه_XX_LARGE:
		fprintf(بهیر, "font-size: xx-large ");
		اكسر;
	حالة CSS_FONT_اندازه_LARGER:
		fprintf(بهیر, "font-size: larger ");
		اكسر;
	حالة CSS_FONT_اندازه_SMALLER:
		fprintf(بهیر, "font-size: smaller ");
		اكسر;
	حالة CSS_FONT_اندازه_DIMENSION:
		fprintf(بهیر, "font-size: ");

		dump_css_unit(بهیر, len1, unit1);

		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* font-style */
	val = css_computed_font_style(style);
	بدل (val) {
	حالة CSS_FONT_STYLE_نورمال:
		fprintf(بهیر, "font-style: normal ");
		اكسر;
	حالة CSS_FONT_STYLE_ITALIC:
		fprintf(بهیر, "font-style: italic ");
		اكسر;
	حالة CSS_FONT_STYLE_OBLIQUE:
		fprintf(بهیر, "font-style: oblique ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* font-variant */
	val = css_computed_font_variant(style);
	بدل (val) {
	حالة CSS_FONT_VARIANT_نورمال:
		fprintf(بهیر, "font-variant: normal ");
		اكسر;
	حالة CSS_FONT_VARIANT_SMALL_CAPS:
		fprintf(بهیر, "font-variant: small-caps ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* font-weight */
	val = css_computed_font_وزن(style);
	بدل (val) {
	حالة CSS_FONT_وزن_نورمال:
		fprintf(بهیر, "font-weight: normal ");
		اكسر;
	حالة CSS_FONT_وزن_BOLD:
		fprintf(بهیر, "font-weight: bold ");
		اكسر;
	حالة CSS_FONT_وزن_BOLDER:
		fprintf(بهیر, "font-weight: bolder ");
		اكسر;
	حالة CSS_FONT_وزن_LIGHTER:
		fprintf(بهیر, "font-weight: lighter ");
		اكسر;
	حالة CSS_FONT_وزن_100:
		fprintf(بهیر, "font-weight: 100 ");
		اكسر;
	حالة CSS_FONT_وزن_200:
		fprintf(بهیر, "font-weight: 200 ");
		اكسر;
	حالة CSS_FONT_وزن_300:
		fprintf(بهیر, "font-weight: 300 ");
		اكسر;
	حالة CSS_FONT_وزن_400:
		fprintf(بهیر, "font-weight: 400 ");
		اكسر;
	حالة CSS_FONT_وزن_500:
		fprintf(بهیر, "font-weight: 500 ");
		اكسر;
	حالة CSS_FONT_وزن_600:
		fprintf(بهیر, "font-weight: 600 ");
		اكسر;
	حالة CSS_FONT_وزن_700:
		fprintf(بهیر, "font-weight: 700 ");
		اكسر;
	حالة CSS_FONT_وزن_800:
		fprintf(بهیر, "font-weight: 800 ");
		اكسر;
	حالة CSS_FONT_وزن_900:
		fprintf(بهیر, "font-weight: 900 ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/*لوړوالی*/
	val = css_computed_لوړوالی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_لوړوالی_AUTO:
		fprintf(بهیر, "height: auto ");
		اكسر;
	حالة CSS_لوړوالی_سېټ:
		fprintf(بهیر, "height: ");

		dump_css_unit(بهیر, len1, unit1);

		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/*کیڼ*/
	val = css_computed_کیڼ(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_کیڼ_AUTO:
		fprintf(بهیر, "left: auto ");
		اكسر;
	حالة CSS_کیڼ_سېټ:
		fprintf(بهیر, "left: ");

		dump_css_unit(بهیر, len1, unit1);

		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* letter-spacing */
	val = css_computed_letter_spacing(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_LETTER_SPACING_نورمال:
		fprintf(بهیر, "letter-spacing: normal ");
		اكسر;
	حالة CSS_LETTER_SPACING_سېټ:
		fprintf(بهیر, "letter-spacing: ");

		dump_css_unit(بهیر, len1, unit1);

		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* line-height */
	val = css_computed_کرښه_لوړوالی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_کرښه_لوړوالی_نورمال:
		fprintf(بهیر, "line-height: normal ");
		اكسر;
	حالة CSS_کرښه_لوړوالی_شمېره:
		fprintf(بهیر, "line-height: ");

		dump_css_fixed(بهیر, len1);

		fprintf(بهیر, " ");
		اكسر;
	حالة CSS_کرښه_لوړوالی_DIMENSION:
		fprintf(بهیر, "line-height: ");

		dump_css_unit(بهیر, len1, unit1);

		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* list-style-image */
	val = css_computed_list_style_image(style, &url);
	إذا (url != عدم) {
		fprintf(بهیر, "list-style-image: url('%.*s') ",
				(صحيح) lwc_string_اوږدتیا(url),
				lwc_string_معلومات(url));
	} وإلا إذا (val == CSS_LIST_STYLE_IMAGE_NONE) {
		fprintf(بهیر, "list-style-image: none ");
	}

	/* list-style-position */
	val = css_computed_list_style_position(style);
	بدل (val) {
	حالة CSS_LIST_STYLE_POSITION_INSIDE:
		fprintf(بهیر, "list-style-position: inside ");
		اكسر;
	حالة CSS_LIST_STYLE_POSITION_OUTSIDE:
		fprintf(بهیر, "list-style-position: outside ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* list-style-type */
	val = css_computed_list_style_type(style);
	بدل (val) {
	حالة CSS_LIST_STYLE_TYPE_DISC:
		fprintf(بهیر, "list-style-type: disc ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_CIRCLE:
		fprintf(بهیر, "list-style-type: circle ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_مربع:
		fprintf(بهیر, "list-style-type: square ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_DECIMAL:
		fprintf(بهیر, "list-style-type: decimal ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_DECIMAL_LEADING_ZERO:
		fprintf(بهیر, "list-style-type: decimal-leading-zero ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_ښکتنی_ROMAN:
		fprintf(بهیر, "list-style-type: lower-roman ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_پاسنی_ROMAN:
		fprintf(بهیر, "list-style-type: upper-roman ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_ښکتنی_GREEK:
		fprintf(بهیر, "list-style-type: lower-greek ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_ښکتنی_LATIN:
		fprintf(بهیر, "list-style-type: lower-latin ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_پاسنی_LATIN:
		fprintf(بهیر, "list-style-type: upper-latin ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_ARMENIAN:
		fprintf(بهیر, "list-style-type: armenian ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_GEORGIAN:
		fprintf(بهیر, "list-style-type: georgian ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_ښکتنی_ALPHA:
		fprintf(بهیر, "list-style-type: lower-alpha ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_پاسنی_ALPHA:
		fprintf(بهیر, "list-style-type: upper-alpha ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_NONE:
		fprintf(بهیر, "list-style-type: none ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* margin-top */
	val = css_computed_margin_سر(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MARGIN_AUTO:
		fprintf(بهیر, "margin-top: auto ");
		اكسر;
	حالة CSS_MARGIN_سېټ:
		fprintf(بهیر, "margin-top: ");

		dump_css_unit(بهیر, len1, unit1);

		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* margin-right */
	val = css_computed_margin_ښی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MARGIN_AUTO:
		fprintf(بهیر, "margin-right: auto ");
		اكسر;
	حالة CSS_MARGIN_سېټ:
		fprintf(بهیر, "margin-right: ");

		dump_css_unit(بهیر, len1, unit1);

		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* margin-bottom */
	val = css_computed_margin_bottom(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MARGIN_AUTO:
		fprintf(بهیر, "margin-bottom: auto ");
		اكسر;
	حالة CSS_MARGIN_سېټ:
		fprintf(بهیر, "margin-bottom: ");

		dump_css_unit(بهیر, len1, unit1);

		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* margin-left */
	val = css_computed_margin_کیڼ(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MARGIN_AUTO:
		fprintf(بهیر, "margin-left: auto ");
		اكسر;
	حالة CSS_MARGIN_سېټ:
		fprintf(بهیر, "margin-left: ");

		dump_css_unit(بهیر, len1, unit1);

		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* max-height */
	val = css_computed_max_لوړوالی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MAX_لوړوالی_NONE:
		fprintf(بهیر, "max-height: none ");
		اكسر;
	حالة CSS_MAX_لوړوالی_سېټ:
		fprintf(بهیر, "max-height: ");

		dump_css_unit(بهیر, len1, unit1);

		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* max-width */
	val = css_computed_max_پلنوالی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MAX_پلنوالی_NONE:
		fprintf(بهیر, "max-width: none ");
		اكسر;
	حالة CSS_MAX_پلنوالی_سېټ:
		fprintf(بهیر, "max-width: ");

		dump_css_unit(بهیر, len1, unit1);

		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* min-height */
	val = ns_computed_min_لوړوالی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MIN_لوړوالی_سېټ:
		fprintf(بهیر, "min-height: ");

		dump_css_unit(بهیر, len1, unit1);

		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* min-width */
	val = ns_computed_min_پلنوالی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MIN_پلنوالی_سېټ:
		fprintf(بهیر, "min-width: ");

		dump_css_unit(بهیر, len1, unit1);

		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* opacity */
	val = css_computed_opacity(style, &len1);
	بدل (val) {
	حالة CSS_OPACITY_سېټ:
		fprintf(بهیر, "opacity: ");

		dump_css_fixed(بهیر, len1);

		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* outline-color */
	val = css_computed_outline_color(style, &color);
	بدل (val) {
	حالة CSS_OUTLINE_COLOR_INVERT:
		fprintf(بهیر, "outline-color: invert ");
		اكسر;
	حالة CSS_OUTLINE_COLOR_COLOR:
		fprintf(بهیر, "outline-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* outline-style */
	val = css_computed_outline_style(style);
	بدل (val) {
	حالة CSS_OUTLINE_STYLE_NONE:
		fprintf(بهیر, "outline-style: none ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_DOTTED:
		fprintf(بهیر, "outline-style: dotted ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_DASHED:
		fprintf(بهیر, "outline-style: dashed ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_SOLID:
		fprintf(بهیر, "outline-style: solid ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_DOUBLE:
		fprintf(بهیر, "outline-style: double ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_GROOVE:
		fprintf(بهیر, "outline-style: groove ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_RIDGE:
		fprintf(بهیر, "outline-style: ridge ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_INSET:
		fprintf(بهیر, "outline-style: inset ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_OUTSET:
		fprintf(بهیر, "outline-style: outset ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* outline-width */
	val = css_computed_outline_پلنوالی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_OUTLINE_پلنوالی_THIN:
		fprintf(بهیر, "outline-width: thin ");
		اكسر;
	حالة CSS_OUTLINE_پلنوالی_MEDIUM:
		fprintf(بهیر, "outline-width: medium ");
		اكسر;
	حالة CSS_OUTLINE_پلنوالی_THICK:
		fprintf(بهیر, "outline-width: thick ");
		اكسر;
	حالة CSS_OUTLINE_پلنوالی_پلنوالی:
		fprintf(بهیر, "outline-width: ");

		dump_css_unit(بهیر, len1, unit1);

		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/*ډکېدنه*/
	val = css_computed_ډکېدنه_x(style);
	بدل (val) {
	حالة CSS_ډکېدنه_VISIBLE:
		fprintf(بهیر, "overflow-x: visible ");
		اكسر;
	حالة CSS_ډکېدنه_HIDDEN:
		fprintf(بهیر, "overflow-x: hidden ");
		اكسر;
	حالة CSS_ډکېدنه_SCROLL:
		fprintf(بهیر, "overflow-x: scroll ");
		اكسر;
	حالة CSS_ډکېدنه_AUTO:
		fprintf(بهیر, "overflow-x auto ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/*ډکېدنه*/
	val = css_computed_ډکېدنه_y(style);
	بدل (val) {
	حالة CSS_ډکېدنه_VISIBLE:
		fprintf(بهیر, "overflow-y: visible ");
		اكسر;
	حالة CSS_ډکېدنه_HIDDEN:
		fprintf(بهیر, "overflow-y: hidden ");
		اكسر;
	حالة CSS_ډکېدنه_SCROLL:
		fprintf(بهیر, "overflow-y: scroll ");
		اكسر;
	حالة CSS_ډکېدنه_AUTO:
		fprintf(بهیر, "overflow-y: auto ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* padding-top */
	val = css_computed_padding_سر(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_PADDING_سېټ:
		fprintf(بهیر, "padding-top: ");

		dump_css_unit(بهیر, len1, unit1);

		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* padding-right */
	val = css_computed_padding_ښی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_PADDING_سېټ:
		fprintf(بهیر, "padding-right: ");

		dump_css_unit(بهیر, len1, unit1);

		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* padding-bottom */
	val = css_computed_padding_bottom(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_PADDING_سېټ:
		fprintf(بهیر, "padding-bottom: ");

		dump_css_unit(بهیر, len1, unit1);

		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* padding-left */
	val = css_computed_padding_کیڼ(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_PADDING_سېټ:
		fprintf(بهیر, "padding-left: ");

		dump_css_unit(بهیر, len1, unit1);

		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* position */
	val = css_computed_position(style);
	بدل (val) {
	حالة CSS_POSITION_STATIC:
		fprintf(بهیر, "position: static ");
		اكسر;
	حالة CSS_POSITION_RELATIVE:
		fprintf(بهیر, "position: relative ");
		اكسر;
	حالة CSS_POSITION_ABSOLUTE:
		fprintf(بهیر, "position: absolute ");
		اكسر;
	حالة CSS_POSITION_FIXED:
		fprintf(بهیر, "position: fixed ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* quotes */
	val = css_computed_quotes(style, &string_list);
	إذا (val == CSS_QUOTES_STRING && string_list != عدم) {
		fprintf(بهیر, "quotes:");

		طالما (*string_list != عدم) {
			fprintf(بهیر, " \"%.*s\"",
				(صحيح) lwc_string_اوږدتیا(*string_list),
				lwc_string_معلومات(*string_list));

			string_list++;
		}

		fprintf(بهیر, " ");
	} وإلا {
		بدل (val) {
		حالة CSS_QUOTES_NONE:
			fprintf(بهیر, "quotes: none ");
			اكسر;
		افتراضي:
			اكسر;
		}
	}

	/*ښی*/
	val = css_computed_ښی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_ښی_AUTO:
		fprintf(بهیر, "right: auto ");
		اكسر;
	حالة CSS_ښی_سېټ:
		fprintf(بهیر, "right: ");

		dump_css_unit(بهیر, len1, unit1);

		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* table-layout */
	val = css_computed_جدول_layout(style);
	بدل (val) {
	حالة CSS_جدول_LAYOUT_AUTO:
		fprintf(بهیر, "table-layout: auto ");
		اكسر;
	حالة CSS_جدول_LAYOUT_FIXED:
		fprintf(بهیر, "table-layout: fixed ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* text-align */
	val = css_computed_متن_align(style);
	بدل (val) {
	حالة CSS_متن_ALIGN_کیڼ:
		fprintf(بهیر, "text-align: left ");
		اكسر;
	حالة CSS_متن_ALIGN_ښی:
		fprintf(بهیر, "text-align: right ");
		اكسر;
	حالة CSS_متن_ALIGN_CENTER:
		fprintf(بهیر, "text-align: center ");
		اكسر;
	حالة CSS_متن_ALIGN_JUSTIFY:
		fprintf(بهیر, "text-align: justify ");
		اكسر;
	حالة CSS_متن_ALIGN_DEFAULT:
		fprintf(بهیر, "text-align: default ");
		اكسر;
	حالة CSS_متن_ALIGN_LIBCSS_کیڼ:
		fprintf(بهیر, "text-align: -libcss-left ");
		اكسر;
	حالة CSS_متن_ALIGN_LIBCSS_CENTER:
		fprintf(بهیر, "text-align: -libcss-center ");
		اكسر;
	حالة CSS_متن_ALIGN_LIBCSS_ښی:
		fprintf(بهیر, "text-align: -libcss-right ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* text-decoration */
	val = css_computed_متن_decoration(style);
	إذا (val == CSS_متن_DECORATION_NONE) {
		fprintf(بهیر, "text-decoration: none ");
	} وإلا {
		fprintf(بهیر, "text-decoration:");

		إذا (val & CSS_متن_DECORATION_BLINK) {
			fprintf(بهیر, " blink");
		}
		إذا (val & CSS_متن_DECORATION_کرښه_THROUGH) {
			fprintf(بهیر, " line-through");
		}
		إذا (val & CSS_متن_DECORATION_OVERLINE) {
			fprintf(بهیر, " overline");
		}
		إذا (val & CSS_متن_DECORATION_UNDERLINE) {
			fprintf(بهیر, " underline");
		}

		fprintf(بهیر, " ");
	}

	/* text-indent */
	val = css_computed_متن_indent(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_متن_INDENT_سېټ:
		fprintf(بهیر, "text-indent: ");

		dump_css_unit(بهیر, len1, unit1);

		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* text-transform */
	val = css_computed_متن_transform(style);
	بدل (val) {
	حالة CSS_متن_TRANSFORM_CAPITALIZE:
		fprintf(بهیر, "text-transform: capitalize ");
		اكسر;
	حالة CSS_متن_TRANSFORM_UPPERCASE:
		fprintf(بهیر, "text-transform: uppercase ");
		اكسر;
	حالة CSS_متن_TRANSFORM_LOWERCASE:
		fprintf(بهیر, "text-transform: lowercase ");
		اكسر;
	حالة CSS_متن_TRANSFORM_NONE:
		fprintf(بهیر, "text-transform: none ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/*سر*/
	val = css_computed_سر(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_سر_AUTO:
		fprintf(بهیر, "top: auto ");
		اكسر;
	حالة CSS_سر_سېټ:
		fprintf(بهیر, "top: ");

		dump_css_unit(بهیر, len1, unit1);

		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* unicode-bidi */
	val = css_computed_unicode_bidi(style);
	بدل (val) {
	حالة CSS_UNICODE_BIDI_نورمال:
		fprintf(بهیر, "unicode-bidi: normal ");
		اكسر;
	حالة CSS_UNICODE_BIDI_EMBED:
		fprintf(بهیر, "unicode-bidi: embed ");
		اكسر;
	حالة CSS_UNICODE_BIDI_BIDI_OVERRIDE:
		fprintf(بهیر, "unicode-bidi: bidi-override ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* vertical-align */
	val = css_computed_vertical_align(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_VERTICAL_ALIGN_BASELINE:
		fprintf(بهیر, "vertical-align: baseline ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_SUB:
		fprintf(بهیر, "vertical-align: sub ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_SUPER:
		fprintf(بهیر, "vertical-align: super ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_سر:
		fprintf(بهیر, "vertical-align: top ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_متن_سر:
		fprintf(بهیر, "vertical-align: text-top ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_منځ:
		fprintf(بهیر, "vertical-align: middle ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_BOTTOM:
		fprintf(بهیر, "vertical-align: bottom ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_متن_BOTTOM:
		fprintf(بهیر, "vertical-align: text-bottom ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_سېټ:
		fprintf(بهیر, "vertical-align: ");

		dump_css_unit(بهیر, len1, unit1);

		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* visibility */
	val = css_computed_visibility(style);
	بدل (val) {
	حالة CSS_VISIBILITY_VISIBLE:
		fprintf(بهیر, "visibility: visible ");
		اكسر;
	حالة CSS_VISIBILITY_HIDDEN:
		fprintf(بهیر, "visibility: hidden ");
		اكسر;
	حالة CSS_VISIBILITY_COLLAPSE:
		fprintf(بهیر, "visibility: collapse ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* white-space */
	val = css_computed_white_space(style);
	بدل (val) {
	حالة CSS_WHITE_SPACE_نورمال:
		fprintf(بهیر, "white-space: normal ");
		اكسر;
	حالة CSS_WHITE_SPACE_PRE:
		fprintf(بهیر, "white-space: pre ");
		اكسر;
	حالة CSS_WHITE_SPACE_NOWRAP:
		fprintf(بهیر, "white-space: nowrap ");
		اكسر;
	حالة CSS_WHITE_SPACE_PRE_WRAP:
		fprintf(بهیر, "white-space: pre-wrap ");
		اكسر;
	حالة CSS_WHITE_SPACE_PRE_کرښه:
		fprintf(بهیر, "white-space: pre-line ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/*پلنوالی*/
	val = css_computed_پلنوالی(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_پلنوالی_AUTO:
		fprintf(بهیر, "width: auto ");
		اكسر;
	حالة CSS_پلنوالی_سېټ:
		fprintf(بهیر, "width: ");

		dump_css_unit(بهیر, len1, unit1);

		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* word-spacing */
	val = css_computed_کلمه_spacing(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_کلمه_SPACING_نورمال:
		fprintf(بهیر, "word-spacing: normal ");
		اكسر;
	حالة CSS_کلمه_SPACING_سېټ:
		fprintf(بهیر, "word-spacing: ");

		dump_css_unit(بهیر, len1, unit1);

		fprintf(بهیر, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* z-index */
	val = css_computed_z_شاخص(style, &zindex);
	بدل (val) {
	حالة CSS_Z_شاخص_AUTO:
		fprintf(بهیر, "z-index: auto ");
		اكسر;
	حالة CSS_Z_شاخص_سېټ:
		fprintf(بهیر, "z-index: %"PRId32" ", zindex);
		اكسر;
	افتراضي:
		اكسر;
	}

	fprintf(بهیر, "}");
}
