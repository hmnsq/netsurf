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
ساكن فراغ dump_css_fixed(FILE *جریان, css_fixed f)
{
#تعریف_کن NSCSS_ABS(x) (uint32_t)((x) < 0 ? -(x) : (x))
	uint32_t uintpart = FIXTOINT(NSCSS_ABS(f));
	/* + 500 to ensure round to nearest (division will truncate) */
	uint32_t fracpart = ((NSCSS_ABS(f) & 0x3ff) * 1000 + 500) / (1 << 10);
#لغو_تعریف NSCSS_ABS

	fprintf(جریان, "%s%"PRIu32".%03"PRIu32, f < 0 ? "-" : "", uintpart, fracpart);
}

/**
 * Dump a numeric value to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 */
ساكن فراغ dump_css_عدد(FILE *جریان, css_fixed val)
{
	إذا (INTTOFIX(FIXTOINT(val)) == val)
		fprintf(جریان, "%"PRId32, FIXTOINT(val));
	وإلا
		dump_css_fixed(جریان, val);
}

/**
 * Dump a dimension to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 * \param unit    Unit to write
 */
ساكن فراغ dump_css_unit(FILE *جریان, css_fixed val, css_unit unit)
{
	dump_css_عدد(جریان, val);

	بدل (unit) {
	حالة CSS_UNIT_PX:
		fprintf(جریان, "px");
		اكسر;
	حالة CSS_UNIT_EX:
		fprintf(جریان, "ex");
		اكسر;
	حالة CSS_UNIT_EM:
		fprintf(جریان, "em");
		اكسر;
	حالة CSS_UNIT_درون:
		fprintf(جریان, "in");
		اكسر;
	حالة CSS_UNIT_CM:
		fprintf(جریان, "cm");
		اكسر;
	حالة CSS_UNIT_MM:
		fprintf(جریان, "mm");
		اكسر;
	حالة CSS_UNIT_PT:
		fprintf(جریان, "pt");
		اكسر;
	حالة CSS_UNIT_PC:
		fprintf(جریان, "pc");
		اكسر;
	حالة CSS_UNIT_PCT:
		fprintf(جریان, "%%");
		اكسر;
	حالة CSS_UNIT_DEG:
		fprintf(جریان, "deg");
		اكسر;
	حالة CSS_UNIT_GRAD:
		fprintf(جریان, "grad");
		اكسر;
	حالة CSS_UNIT_RAD:
		fprintf(جریان, "rad");
		اكسر;
	حالة CSS_UNIT_MS:
		fprintf(جریان, "ms");
		اكسر;
	حالة CSS_UNIT_S:
		fprintf(جریان, "s");
		اكسر;
	حالة CSS_UNIT_HZ:
		fprintf(جریان, "Hz");
		اكسر;
	حالة CSS_UNIT_KHZ:
		fprintf(جریان, "kHz");
		اكسر;
	حالة CSS_UNIT_CH:
		fprintf(جریان, "ch");
		اكسر;
	حالة CSS_UNIT_REM:
		fprintf(جریان, "rem");
		اكسر;
	حالة CSS_UNIT_LH:
		fprintf(جریان, "lh");
		اكسر;
	حالة CSS_UNIT_VH:
		fprintf(جریان, "vh");
		اكسر;
	حالة CSS_UNIT_VW:
		fprintf(جریان, "vw");
		اكسر;
	حالة CSS_UNIT_VI:
		fprintf(جریان, "vi");
		اكسر;
	حالة CSS_UNIT_VB:
		fprintf(جریان, "vb");
		اكسر;
	حالة CSS_UNIT_VMIN:
		fprintf(جریان, "vmin");
		اكسر;
	حالة CSS_UNIT_VMAX:
		fprintf(جریان, "vmax");
		اكسر;
	حالة CSS_UNIT_Q:
		fprintf(جریان, "q");
		اكسر;
	حالة CSS_UNIT_CALC:
		fprintf(جریان, "calc()");
		اكسر;
	}
}

/* exported interface documented in content/handlers/css/dump.h */
فراغ nscss_dump_computed_style(FILE *جریان, ثابت css_computed_style *style)
{
	uint8_t val;
	css_color color = 0;
	lwc_string *url = عدم;
	css_fixed len1 = 0, len2 = 0;
	css_unit unit1 = CSS_UNIT_PX, unit2 = CSS_UNIT_PX;
	css_computed_clip_rect rect = { 0, 0, 0, 0, CSS_UNIT_PX, CSS_UNIT_PX,
					CSS_UNIT_PX, CSS_UNIT_PX, true, true,
					true, true };
	ثابت css_computed_content_قلم *content = عدم;
	ثابت css_computed_counter *counter = عدم;
	lwc_string **string_list = عدم;
	int32_t zindex = 0;

	fprintf(جریان, "{ ");

	/* background-attachment */
	val = css_computed_background_attachment(style);
	بدل (val) {
	حالة CSS_BACKGROUND_ATTACHMENT_FIXED:
		fprintf(جریان, "background-attachment: fixed ");
		اكسر;
	حالة CSS_BACKGROUND_ATTACHMENT_SCROLL:
		fprintf(جریان, "background-attachment: scroll ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* background-color */
	val = css_computed_background_color(style, &color);
	بدل (val) {
	حالة CSS_BACKGROUND_COLOR_COLOR:
		fprintf(جریان, "background-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* background-image */
	val = css_computed_background_image(style, &url);
	إذا (val == CSS_BACKGROUND_IMAGE_IMAGE && url != عدم) {
		fprintf(جریان, "background-image: url('%.*s') ",
				(صحيح) lwc_string_درازی(url),
				lwc_string_داده(url));
	} وإلا إذا (val == CSS_BACKGROUND_IMAGE_NONE) {
		fprintf(جریان, "background-image: none ");
	}

	/* background-position */
	val = css_computed_background_position(style, &len1, &unit1,
			&len2, &unit2);
	إذا (val == CSS_BACKGROUND_POSITION_مجموعه) {
		fprintf(جریان, "background-position: ");
		dump_css_unit(جریان, len1, unit1);
		fprintf(جریان, " ");
		dump_css_unit(جریان, len2, unit2);
		fprintf(جریان, " ");
	}

	/* background-repeat */
	val = css_computed_background_repeat(style);
	بدل (val) {
	حالة CSS_BACKGROUND_REPEAT_REPEAT_X:
		fprintf(جریان, "background-repeat: repeat-x ");
		اكسر;
	حالة CSS_BACKGROUND_REPEAT_REPEAT_Y:
		fprintf(جریان, "background-repeat: repeat-y ");
		اكسر;
	حالة CSS_BACKGROUND_REPEAT_REPEAT:
		fprintf(جریان, "background-repeat: repeat ");
		اكسر;
	حالة CSS_BACKGROUND_REPEAT_NO_REPEAT:
		fprintf(جریان, "background-repeat: no-repeat ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-collapse */
	val = css_computed_border_collapse(style);
	بدل (val) {
	حالة CSS_BORDER_COLLAPSE_SEPARATE:
		fprintf(جریان, "border-collapse: separate ");
		اكسر;
	حالة CSS_BORDER_COLLAPSE_COLLAPSE:
		fprintf(جریان, "border-collapse: collapse ");
		اكسر;
	افتراضي:

		اكسر;
	}

	/* border-spacing */
	val = css_computed_border_spacing(style, &len1, &unit1, &len2, &unit2);
	إذا (val == CSS_BORDER_SPACING_مجموعه) {
		fprintf(جریان, "border-spacing: ");
		dump_css_unit(جریان, len1, unit1);
		fprintf(جریان, " ");
		dump_css_unit(جریان, len2, unit2);
		fprintf(جریان, " ");
	}

	/* border-top-color */
	val = css_computed_border_بالاترین_color(style, &color);
	بدل (val) {
	حالة CSS_BORDER_COLOR_COLOR:
		fprintf(جریان, "border-top-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-right-color */
	val = css_computed_border_راست_color(style, &color);
	بدل (val) {
	حالة CSS_BORDER_COLOR_COLOR:
		fprintf(جریان, "border-right-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-bottom-color */
	val = css_computed_border_bottom_color(style, &color);
	بدل (val) {
	حالة CSS_BORDER_COLOR_COLOR:
		fprintf(جریان, "border-bottom-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-left-color */
	val = css_computed_border_چپ_color(style, &color);
	بدل (val) {
	حالة CSS_BORDER_COLOR_COLOR:
		fprintf(جریان, "border-left-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-top-style */
	val = css_computed_border_بالاترین_style(style);
	بدل (val) {
	حالة CSS_BORDER_STYLE_NONE:
		fprintf(جریان, "border-top-style: none ");
		اكسر;
	حالة CSS_BORDER_STYLE_HIDDEN:
		fprintf(جریان, "border-top-style: hidden ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOTTED:
		fprintf(جریان, "border-top-style: dotted ");
		اكسر;
	حالة CSS_BORDER_STYLE_DASHED:
		fprintf(جریان, "border-top-style: dashed ");
		اكسر;
	حالة CSS_BORDER_STYLE_SOLID:
		fprintf(جریان, "border-top-style: solid ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOUBLE:
		fprintf(جریان, "border-top-style: double ");
		اكسر;
	حالة CSS_BORDER_STYLE_GROOVE:
		fprintf(جریان, "border-top-style: groove ");
		اكسر;
	حالة CSS_BORDER_STYLE_RIDGE:
		fprintf(جریان, "border-top-style: ridge ");
		اكسر;
	حالة CSS_BORDER_STYLE_INSET:
		fprintf(جریان, "border-top-style: inset ");
		اكسر;
	حالة CSS_BORDER_STYLE_OUTSET:
		fprintf(جریان, "border-top-style: outset ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-right-style */
	val = css_computed_border_راست_style(style);
	بدل (val) {
	حالة CSS_BORDER_STYLE_NONE:
		fprintf(جریان, "border-right-style: none ");
		اكسر;
	حالة CSS_BORDER_STYLE_HIDDEN:
		fprintf(جریان, "border-right-style: hidden ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOTTED:
		fprintf(جریان, "border-right-style: dotted ");
		اكسر;
	حالة CSS_BORDER_STYLE_DASHED:
		fprintf(جریان, "border-right-style: dashed ");
		اكسر;
	حالة CSS_BORDER_STYLE_SOLID:
		fprintf(جریان, "border-right-style: solid ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOUBLE:
		fprintf(جریان, "border-right-style: double ");
		اكسر;
	حالة CSS_BORDER_STYLE_GROOVE:
		fprintf(جریان, "border-right-style: groove ");
		اكسر;
	حالة CSS_BORDER_STYLE_RIDGE:
		fprintf(جریان, "border-right-style: ridge ");
		اكسر;
	حالة CSS_BORDER_STYLE_INSET:
		fprintf(جریان, "border-right-style: inset ");
		اكسر;
	حالة CSS_BORDER_STYLE_OUTSET:
		fprintf(جریان, "border-right-style: outset ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-bottom-style */
	val = css_computed_border_bottom_style(style);
	بدل (val) {
	حالة CSS_BORDER_STYLE_NONE:
		fprintf(جریان, "border-bottom-style: none ");
		اكسر;
	حالة CSS_BORDER_STYLE_HIDDEN:
		fprintf(جریان, "border-bottom-style: hidden ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOTTED:
		fprintf(جریان, "border-bottom-style: dotted ");
		اكسر;
	حالة CSS_BORDER_STYLE_DASHED:
		fprintf(جریان, "border-bottom-style: dashed ");
		اكسر;
	حالة CSS_BORDER_STYLE_SOLID:
		fprintf(جریان, "border-bottom-style: solid ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOUBLE:
		fprintf(جریان, "border-bottom-style: double ");
		اكسر;
	حالة CSS_BORDER_STYLE_GROOVE:
		fprintf(جریان, "border-bottom-style: groove ");
		اكسر;
	حالة CSS_BORDER_STYLE_RIDGE:
		fprintf(جریان, "border-bottom-style: ridge ");
		اكسر;
	حالة CSS_BORDER_STYLE_INSET:
		fprintf(جریان, "border-bottom-style: inset ");
		اكسر;
	حالة CSS_BORDER_STYLE_OUTSET:
		fprintf(جریان, "border-bottom-style: outset ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-left-style */
	val = css_computed_border_چپ_style(style);
	بدل (val) {
	حالة CSS_BORDER_STYLE_NONE:
		fprintf(جریان, "border-left-style: none ");
		اكسر;
	حالة CSS_BORDER_STYLE_HIDDEN:
		fprintf(جریان, "border-left-style: hidden ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOTTED:
		fprintf(جریان, "border-left-style: dotted ");
		اكسر;
	حالة CSS_BORDER_STYLE_DASHED:
		fprintf(جریان, "border-left-style: dashed ");
		اكسر;
	حالة CSS_BORDER_STYLE_SOLID:
		fprintf(جریان, "border-left-style: solid ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOUBLE:
		fprintf(جریان, "border-left-style: double ");
		اكسر;
	حالة CSS_BORDER_STYLE_GROOVE:
		fprintf(جریان, "border-left-style: groove ");
		اكسر;
	حالة CSS_BORDER_STYLE_RIDGE:
		fprintf(جریان, "border-left-style: ridge ");
		اكسر;
	حالة CSS_BORDER_STYLE_INSET:
		fprintf(جریان, "border-left-style: inset ");
		اكسر;
	حالة CSS_BORDER_STYLE_OUTSET:
		fprintf(جریان, "border-left-style: outset ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-top-width */
	val = css_computed_border_بالاترین_عرض(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_BORDER_عرض_THIN:
		fprintf(جریان, "border-top-width: thin ");
		اكسر;
	حالة CSS_BORDER_عرض_MEDIUM:
		fprintf(جریان, "border-top-width: medium ");
		اكسر;
	حالة CSS_BORDER_عرض_THICK:
		fprintf(جریان, "border-top-width: thick ");
		اكسر;
	حالة CSS_BORDER_عرض_عرض:
		fprintf(جریان, "border-top-width: ");
		dump_css_unit(جریان, len1, unit1);
		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-right-width */
	val = css_computed_border_راست_عرض(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_BORDER_عرض_THIN:
		fprintf(جریان, "border-right-width: thin ");
		اكسر;
	حالة CSS_BORDER_عرض_MEDIUM:
		fprintf(جریان, "border-right-width: medium ");
		اكسر;
	حالة CSS_BORDER_عرض_THICK:
		fprintf(جریان, "border-right-width: thick ");
		اكسر;
	حالة CSS_BORDER_عرض_عرض:
		fprintf(جریان, "border-right-width: ");
		dump_css_unit(جریان, len1, unit1);
		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-bottom-width */
	val = css_computed_border_bottom_عرض(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_BORDER_عرض_THIN:
		fprintf(جریان, "border-bottom-width: thin ");
		اكسر;
	حالة CSS_BORDER_عرض_MEDIUM:
		fprintf(جریان, "border-bottom-width: medium ");
		اكسر;
	حالة CSS_BORDER_عرض_THICK:
		fprintf(جریان, "border-bottom-width: thick ");
		اكسر;
	حالة CSS_BORDER_عرض_عرض:
		fprintf(جریان, "border-bottom-width: ");
		dump_css_unit(جریان, len1, unit1);
		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-left-width */
	val = css_computed_border_چپ_عرض(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_BORDER_عرض_THIN:
		fprintf(جریان, "border-left-width: thin ");
		اكسر;
	حالة CSS_BORDER_عرض_MEDIUM:
		fprintf(جریان, "border-left-width: medium ");
		اكسر;
	حالة CSS_BORDER_عرض_THICK:
		fprintf(جریان, "border-left-width: thick ");
		اكسر;
	حالة CSS_BORDER_عرض_عرض:
		fprintf(جریان, "border-left-width: ");
		dump_css_unit(جریان, len1, unit1);
		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* bottom */
	val = css_computed_bottom(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_BOTTOM_AUTO:
		fprintf(جریان, "bottom: auto ");
		اكسر;
	حالة CSS_BOTTOM_مجموعه:
		fprintf(جریان, "bottom: ");
		dump_css_unit(جریان, len1, unit1);
		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* caption-side */
	val = css_computed_caption_side(style);
	بدل (val) {
	حالة CSS_CAPTION_SIDE_بالاترین:
		fprintf(جریان, "caption_side: top ");
		اكسر;
	حالة CSS_CAPTION_SIDE_BOTTOM:
		fprintf(جریان, "caption_side: bottom ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* clear */
	val = css_computed_clear(style);
	بدل (val) {
	حالة CSS_CLEAR_NONE:
		fprintf(جریان, "clear: none ");
		اكسر;
	حالة CSS_CLEAR_چپ:
		fprintf(جریان, "clear: left ");
		اكسر;
	حالة CSS_CLEAR_راست:
		fprintf(جریان, "clear: right ");
		اكسر;
	حالة CSS_CLEAR_BOTH:
		fprintf(جریان, "clear: both ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* clip */
	val = css_computed_clip(style, &rect);
	بدل (val) {
	حالة CSS_CLIP_AUTO:
		fprintf(جریان, "clip: auto ");
		اكسر;
	حالة CSS_CLIP_RECT:
		fprintf(جریان, "clip: rect( ");

		إذا (rect.بالاترین_auto)
			fprintf(جریان, "auto");
		وإلا
			dump_css_unit(جریان, rect.بالاترین, rect.tunit);
		fprintf(جریان, ", ");

		إذا (rect.راست_auto)
			fprintf(جریان, "auto");
		وإلا
			dump_css_unit(جریان, rect.راست, rect.runit);
		fprintf(جریان, ", ");

		إذا (rect.bottom_auto)
			fprintf(جریان, "auto");
		وإلا
			dump_css_unit(جریان, rect.bottom, rect.bunit);
		fprintf(جریان, ", ");

		إذا (rect.چپ_auto)
			fprintf(جریان, "auto");
		وإلا
			dump_css_unit(جریان, rect.چپ, rect.lunit);
		fprintf(جریان, ") ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* color */
	val = css_computed_color(style, &color);
	إذا (val == CSS_COLOR_COLOR) {
		fprintf(جریان, "color: #%08"PRIx32" ", color);
	}

	/* content */
	val = css_computed_content(style, &content);
	بدل (val) {
	حالة CSS_CONTENT_NONE:
		fprintf(جریان, "content: none ");
		اكسر;
	حالة CSS_CONTENT_نرمال:
		fprintf(جریان, "content: normal ");
		اكسر;
	حالة CSS_CONTENT_مجموعه:
		fprintf(جریان, "content:");

		طالما (content->type != CSS_COMPUTED_CONTENT_NONE) {
			fprintf(جریان, " ");

			بدل (content->type) {
			حالة CSS_COMPUTED_CONTENT_STRING:
				fprintf(جریان,	"\"%.*s\"",
						(صحيح) lwc_string_درازی(
						content->داده.string),
						lwc_string_داده(
						content->داده.string));
				اكسر;
			حالة CSS_COMPUTED_CONTENT_URI:
				fprintf(جریان,	"uri(\"%.*s\")",
						(صحيح) lwc_string_درازی(
						content->داده.uri),
						lwc_string_داده(
						content->داده.uri));
				اكسر;
			حالة CSS_COMPUTED_CONTENT_COUNTER:
				fprintf(جریان, "counter(%.*s)",
						(صحيح) lwc_string_درازی(
						content->داده.counter.name),
						lwc_string_داده(
						content->داده.counter.name));
				اكسر;
			حالة CSS_COMPUTED_CONTENT_COUNTERS:
				fprintf(جریان, "counters(%.*s, \"%.*s\")",
						(صحيح) lwc_string_درازی(
						content->داده.counters.name),
						lwc_string_داده(
						content->داده.counters.name),
						(صحيح) lwc_string_درازی(
						content->داده.counters.sep),
						lwc_string_داده(
						content->داده.counters.sep));
				اكسر;
			حالة CSS_COMPUTED_CONTENT_ATTR:
				fprintf(جریان, "attr(%.*s)",
						(صحيح) lwc_string_درازی(
						content->داده.attr),
						lwc_string_داده(
						content->داده.attr));
				اكسر;
			حالة CSS_COMPUTED_CONTENT_OPEN_QUOTE:
				fprintf(جریان, "open-quote");
				اكسر;
			حالة CSS_COMPUTED_CONTENT_CLOSE_QUOTE:
				fprintf(جریان, "close-quote");
				اكسر;
			حالة CSS_COMPUTED_CONTENT_NO_OPEN_QUOTE:
				fprintf(جریان, "no-open-quote");
				اكسر;
			حالة CSS_COMPUTED_CONTENT_NO_CLOSE_QUOTE:
				fprintf(جریان, "no-close-quote");
				اكسر;
			}

			content++;
		}

		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* counter-increment */
	val = css_computed_counter_increment(style, &counter);
	إذا ((val == CSS_COUNTER_INCREMENT_NONE) || (counter == عدم)) {
		fprintf(جریان, "counter-increment: none ");
	} وإلا {
		fprintf(جریان, "counter-increment:");

		طالما (counter->name != عدم) {
			fprintf(جریان, " %.*s ",
					(صحيح) lwc_string_درازی(counter->name),
					lwc_string_داده(counter->name));

			dump_css_fixed(جریان, counter->مقدار);

			counter++;
		}

		fprintf(جریان, " ");
	}

	/* counter-reset */
	val = css_computed_counter_reset(style, &counter);
	إذا ((val == CSS_COUNTER_RESET_NONE) || (counter == عدم)) {
		fprintf(جریان, "counter-reset: none ");
	} وإلا {
		fprintf(جریان, "counter-reset:");

		طالما (counter->name != عدم) {
			fprintf(جریان, " %.*s ",
					(صحيح) lwc_string_درازی(counter->name),
					lwc_string_داده(counter->name));

			dump_css_fixed(جریان, counter->مقدار);

			counter++;
		}

		fprintf(جریان, " ");
	}

	/* cursor */
	val = css_computed_cursor(style, &string_list);
	fprintf(جریان, "cursor:");

	إذا (string_list != عدم) {
		طالما (*string_list != عدم) {
			fprintf(جریان, " url\"%.*s\")",
					(صحيح) lwc_string_درازی(*string_list),
					lwc_string_داده(*string_list));

			string_list++;
		}
	}
	بدل (val) {
	حالة CSS_CURSOR_AUTO:
		fprintf(جریان, " auto ");
		اكسر;
	حالة CSS_CURSOR_CROSSHAIR:
		fprintf(جریان, " crosshair ");
		اكسر;
	حالة CSS_CURSOR_DEFAULT:
		fprintf(جریان, " default ");
		اكسر;
	حالة CSS_CURSOR_اشاره_گر:
		fprintf(جریان, " pointer ");
		اكسر;
	حالة CSS_CURSOR_MOVE:
		fprintf(جریان, " move ");
		اكسر;
	حالة CSS_CURSOR_E_RESIZE:
		fprintf(جریان, " e-resize ");
		اكسر;
	حالة CSS_CURSOR_NE_RESIZE:
		fprintf(جریان, " ne-resize ");
		اكسر;
	حالة CSS_CURSOR_NW_RESIZE:
		fprintf(جریان, " nw-resize ");
		اكسر;
	حالة CSS_CURSOR_N_RESIZE:
		fprintf(جریان, " n-resize ");
		اكسر;
	حالة CSS_CURSOR_SE_RESIZE:
		fprintf(جریان, " se-resize ");
		اكسر;
	حالة CSS_CURSOR_SW_RESIZE:
		fprintf(جریان, " sw-resize ");
		اكسر;
	حالة CSS_CURSOR_S_RESIZE:
		fprintf(جریان, " s-resize ");
		اكسر;
	حالة CSS_CURSOR_W_RESIZE:
		fprintf(جریان, " w-resize ");
		اكسر;
	حالة CSS_CURSOR_متن:
		fprintf(جریان, " text ");
		اكسر;
	حالة CSS_CURSOR_WAIT:
		fprintf(جریان, " wait ");
		اكسر;
	حالة CSS_CURSOR_HELP:
		fprintf(جریان, " help ");
		اكسر;
	حالة CSS_CURSOR_پیشرفت:
		fprintf(جریان, " progress ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* direction */
	val = css_computed_direction(style);
	بدل (val) {
	حالة CSS_DIRECTION_LTR:
		fprintf(جریان, "direction: ltr ");
		اكسر;
	حالة CSS_DIRECTION_RTL:
		fprintf(جریان, "direction: rtl ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* display */
	val = ns_computed_display_static(style);
	بدل (val) {
	حالة CSS_DISPLAY_INLINE:
		fprintf(جریان, "display: inline ");
		اكسر;
	حالة CSS_DISPLAY_بلوک:
		fprintf(جریان, "display: block ");
		اكسر;
	حالة CSS_DISPLAY_LIST_قلم:
		fprintf(جریان, "display: list-item ");
		اكسر;
	حالة CSS_DISPLAY_پیاپی_درون:
		fprintf(جریان, "display: run-in ");
		اكسر;
	حالة CSS_DISPLAY_INLINE_بلوک:
		fprintf(جریان, "display: inline-block ");
		اكسر;
	حالة CSS_DISPLAY_جدول:
		fprintf(جریان, "display: table ");
		اكسر;
	حالة CSS_DISPLAY_INLINE_جدول:
		fprintf(جریان, "display: inline-table ");
		اكسر;
	حالة CSS_DISPLAY_جدول_ردیف_گروه:
		fprintf(جریان, "display: table-row-group ");
		اكسر;
	حالة CSS_DISPLAY_جدول_سرآیند_گروه:
		fprintf(جریان, "display: table-header-group ");
		اكسر;
	حالة CSS_DISPLAY_جدول_FOOTER_گروه:
		fprintf(جریان, "display: table-footer-group ");
		اكسر;
	حالة CSS_DISPLAY_جدول_ردیف:
		fprintf(جریان, "display: table-row ");
		اكسر;
	حالة CSS_DISPLAY_جدول_ستونی_گروه:
		fprintf(جریان, "display: table-column-group ");
		اكسر;
	حالة CSS_DISPLAY_جدول_ستونی:
		fprintf(جریان, "display: table-column ");
		اكسر;
	حالة CSS_DISPLAY_جدول_CELL:
		fprintf(جریان, "display: table-cell ");
		اكسر;
	حالة CSS_DISPLAY_جدول_CAPTION:
		fprintf(جریان, "display: table-caption ");
		اكسر;
	حالة CSS_DISPLAY_NONE:
		fprintf(جریان, "display: none ");
		اكسر;
	حالة CSS_DISPLAY_FLEX:
		fprintf(جریان, "display: flex ");
		اكسر;
	حالة CSS_DISPLAY_INLINE_FLEX:
		fprintf(جریان, "display: inline-flex ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* empty-cells */
	val = css_computed_خالی_cells(style);
	بدل (val) {
	حالة CSS_خالی_CELLS_SHOW:
		fprintf(جریان, "empty-cells: show ");
		اكسر;
	حالة CSS_خالی_CELLS_HIDE:
		fprintf(جریان, "empty-cells: hide ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* float */
	val = css_computed_float(style);
	بدل (val) {
	حالة CSS_FLOAT_چپ:
		fprintf(جریان, "float: left ");
		اكسر;
	حالة CSS_FLOAT_راست:
		fprintf(جریان, "float: right ");
		اكسر;
	حالة CSS_FLOAT_NONE:
		fprintf(جریان, "float: none ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* font-family */
	val = css_computed_font_family(style, &string_list);
	إذا (val != CSS_FONT_FAMILY_INHERIT) {
		fprintf(جریان, "font-family:");

		إذا (string_list != عدم) {
			طالما (*string_list != عدم) {
				fprintf(جریان, " \"%.*s\"",
					(صحيح) lwc_string_درازی(*string_list),
					lwc_string_داده(*string_list));

				string_list++;
			}
		}
		بدل (val) {
		حالة CSS_FONT_FAMILY_SERIF:
			fprintf(جریان, " serif ");
			اكسر;
		حالة CSS_FONT_FAMILY_SANS_SERIF:
			fprintf(جریان, " sans-serif ");
			اكسر;
		حالة CSS_FONT_FAMILY_CURSIVE:
			fprintf(جریان, " cursive ");
			اكسر;
		حالة CSS_FONT_FAMILY_FANTASY:
			fprintf(جریان, " fantasy ");
			اكسر;
		حالة CSS_FONT_FAMILY_MONOSPACE:
			fprintf(جریان, " monospace ");
			اكسر;
		}
	}

	/* font-size */
	val = css_computed_font_اندازه(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_FONT_اندازه_XX_SMALL:
		fprintf(جریان, "font-size: xx-small ");
		اكسر;
	حالة CSS_FONT_اندازه_X_SMALL:
		fprintf(جریان, "font-size: x-small ");
		اكسر;
	حالة CSS_FONT_اندازه_SMALL:
		fprintf(جریان, "font-size: small ");
		اكسر;
	حالة CSS_FONT_اندازه_MEDIUM:
		fprintf(جریان, "font-size: medium ");
		اكسر;
	حالة CSS_FONT_اندازه_LARGE:
		fprintf(جریان, "font-size: large ");
		اكسر;
	حالة CSS_FONT_اندازه_X_LARGE:
		fprintf(جریان, "font-size: x-large ");
		اكسر;
	حالة CSS_FONT_اندازه_XX_LARGE:
		fprintf(جریان, "font-size: xx-large ");
		اكسر;
	حالة CSS_FONT_اندازه_LARGER:
		fprintf(جریان, "font-size: larger ");
		اكسر;
	حالة CSS_FONT_اندازه_SMALLER:
		fprintf(جریان, "font-size: smaller ");
		اكسر;
	حالة CSS_FONT_اندازه_DIMENSION:
		fprintf(جریان, "font-size: ");

		dump_css_unit(جریان, len1, unit1);

		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* font-style */
	val = css_computed_font_style(style);
	بدل (val) {
	حالة CSS_FONT_STYLE_نرمال:
		fprintf(جریان, "font-style: normal ");
		اكسر;
	حالة CSS_FONT_STYLE_ITALIC:
		fprintf(جریان, "font-style: italic ");
		اكسر;
	حالة CSS_FONT_STYLE_OBLIQUE:
		fprintf(جریان, "font-style: oblique ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* font-variant */
	val = css_computed_font_variant(style);
	بدل (val) {
	حالة CSS_FONT_VARIANT_نرمال:
		fprintf(جریان, "font-variant: normal ");
		اكسر;
	حالة CSS_FONT_VARIANT_SMALL_CAPS:
		fprintf(جریان, "font-variant: small-caps ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* font-weight */
	val = css_computed_font_وزن(style);
	بدل (val) {
	حالة CSS_FONT_وزن_نرمال:
		fprintf(جریان, "font-weight: normal ");
		اكسر;
	حالة CSS_FONT_وزن_BOLD:
		fprintf(جریان, "font-weight: bold ");
		اكسر;
	حالة CSS_FONT_وزن_BOLDER:
		fprintf(جریان, "font-weight: bolder ");
		اكسر;
	حالة CSS_FONT_وزن_LIGHTER:
		fprintf(جریان, "font-weight: lighter ");
		اكسر;
	حالة CSS_FONT_وزن_100:
		fprintf(جریان, "font-weight: 100 ");
		اكسر;
	حالة CSS_FONT_وزن_200:
		fprintf(جریان, "font-weight: 200 ");
		اكسر;
	حالة CSS_FONT_وزن_300:
		fprintf(جریان, "font-weight: 300 ");
		اكسر;
	حالة CSS_FONT_وزن_400:
		fprintf(جریان, "font-weight: 400 ");
		اكسر;
	حالة CSS_FONT_وزن_500:
		fprintf(جریان, "font-weight: 500 ");
		اكسر;
	حالة CSS_FONT_وزن_600:
		fprintf(جریان, "font-weight: 600 ");
		اكسر;
	حالة CSS_FONT_وزن_700:
		fprintf(جریان, "font-weight: 700 ");
		اكسر;
	حالة CSS_FONT_وزن_800:
		fprintf(جریان, "font-weight: 800 ");
		اكسر;
	حالة CSS_FONT_وزن_900:
		fprintf(جریان, "font-weight: 900 ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/*ارتفاع*/
	val = css_computed_ارتفاع(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_ارتفاع_AUTO:
		fprintf(جریان, "height: auto ");
		اكسر;
	حالة CSS_ارتفاع_مجموعه:
		fprintf(جریان, "height: ");

		dump_css_unit(جریان, len1, unit1);

		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/*چپ*/
	val = css_computed_چپ(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_چپ_AUTO:
		fprintf(جریان, "left: auto ");
		اكسر;
	حالة CSS_چپ_مجموعه:
		fprintf(جریان, "left: ");

		dump_css_unit(جریان, len1, unit1);

		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* letter-spacing */
	val = css_computed_letter_spacing(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_LETTER_SPACING_نرمال:
		fprintf(جریان, "letter-spacing: normal ");
		اكسر;
	حالة CSS_LETTER_SPACING_مجموعه:
		fprintf(جریان, "letter-spacing: ");

		dump_css_unit(جریان, len1, unit1);

		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* line-height */
	val = css_computed_سطر_ارتفاع(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_سطر_ارتفاع_نرمال:
		fprintf(جریان, "line-height: normal ");
		اكسر;
	حالة CSS_سطر_ارتفاع_عدد:
		fprintf(جریان, "line-height: ");

		dump_css_fixed(جریان, len1);

		fprintf(جریان, " ");
		اكسر;
	حالة CSS_سطر_ارتفاع_DIMENSION:
		fprintf(جریان, "line-height: ");

		dump_css_unit(جریان, len1, unit1);

		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* list-style-image */
	val = css_computed_list_style_image(style, &url);
	إذا (url != عدم) {
		fprintf(جریان, "list-style-image: url('%.*s') ",
				(صحيح) lwc_string_درازی(url),
				lwc_string_داده(url));
	} وإلا إذا (val == CSS_LIST_STYLE_IMAGE_NONE) {
		fprintf(جریان, "list-style-image: none ");
	}

	/* list-style-position */
	val = css_computed_list_style_position(style);
	بدل (val) {
	حالة CSS_LIST_STYLE_POSITION_INSIDE:
		fprintf(جریان, "list-style-position: inside ");
		اكسر;
	حالة CSS_LIST_STYLE_POSITION_OUTSIDE:
		fprintf(جریان, "list-style-position: outside ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* list-style-type */
	val = css_computed_list_style_type(style);
	بدل (val) {
	حالة CSS_LIST_STYLE_TYPE_DISC:
		fprintf(جریان, "list-style-type: disc ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_CIRCLE:
		fprintf(جریان, "list-style-type: circle ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_مربع:
		fprintf(جریان, "list-style-type: square ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_DECIMAL:
		fprintf(جریان, "list-style-type: decimal ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_DECIMAL_LEADING_ZERO:
		fprintf(جریان, "list-style-type: decimal-leading-zero ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_پایینی_ROMAN:
		fprintf(جریان, "list-style-type: lower-roman ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_فوقانی_ROMAN:
		fprintf(جریان, "list-style-type: upper-roman ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_پایینی_GREEK:
		fprintf(جریان, "list-style-type: lower-greek ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_پایینی_LATIN:
		fprintf(جریان, "list-style-type: lower-latin ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_فوقانی_LATIN:
		fprintf(جریان, "list-style-type: upper-latin ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_ARMENIAN:
		fprintf(جریان, "list-style-type: armenian ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_GEORGIAN:
		fprintf(جریان, "list-style-type: georgian ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_پایینی_ALPHA:
		fprintf(جریان, "list-style-type: lower-alpha ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_فوقانی_ALPHA:
		fprintf(جریان, "list-style-type: upper-alpha ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_NONE:
		fprintf(جریان, "list-style-type: none ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* margin-top */
	val = css_computed_margin_بالاترین(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MARGIN_AUTO:
		fprintf(جریان, "margin-top: auto ");
		اكسر;
	حالة CSS_MARGIN_مجموعه:
		fprintf(جریان, "margin-top: ");

		dump_css_unit(جریان, len1, unit1);

		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* margin-right */
	val = css_computed_margin_راست(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MARGIN_AUTO:
		fprintf(جریان, "margin-right: auto ");
		اكسر;
	حالة CSS_MARGIN_مجموعه:
		fprintf(جریان, "margin-right: ");

		dump_css_unit(جریان, len1, unit1);

		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* margin-bottom */
	val = css_computed_margin_bottom(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MARGIN_AUTO:
		fprintf(جریان, "margin-bottom: auto ");
		اكسر;
	حالة CSS_MARGIN_مجموعه:
		fprintf(جریان, "margin-bottom: ");

		dump_css_unit(جریان, len1, unit1);

		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* margin-left */
	val = css_computed_margin_چپ(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MARGIN_AUTO:
		fprintf(جریان, "margin-left: auto ");
		اكسر;
	حالة CSS_MARGIN_مجموعه:
		fprintf(جریان, "margin-left: ");

		dump_css_unit(جریان, len1, unit1);

		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* max-height */
	val = css_computed_max_ارتفاع(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MAX_ارتفاع_NONE:
		fprintf(جریان, "max-height: none ");
		اكسر;
	حالة CSS_MAX_ارتفاع_مجموعه:
		fprintf(جریان, "max-height: ");

		dump_css_unit(جریان, len1, unit1);

		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* max-width */
	val = css_computed_max_عرض(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MAX_عرض_NONE:
		fprintf(جریان, "max-width: none ");
		اكسر;
	حالة CSS_MAX_عرض_مجموعه:
		fprintf(جریان, "max-width: ");

		dump_css_unit(جریان, len1, unit1);

		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* min-height */
	val = ns_computed_min_ارتفاع(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MIN_ارتفاع_مجموعه:
		fprintf(جریان, "min-height: ");

		dump_css_unit(جریان, len1, unit1);

		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* min-width */
	val = ns_computed_min_عرض(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MIN_عرض_مجموعه:
		fprintf(جریان, "min-width: ");

		dump_css_unit(جریان, len1, unit1);

		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* opacity */
	val = css_computed_opacity(style, &len1);
	بدل (val) {
	حالة CSS_OPACITY_مجموعه:
		fprintf(جریان, "opacity: ");

		dump_css_fixed(جریان, len1);

		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* outline-color */
	val = css_computed_outline_color(style, &color);
	بدل (val) {
	حالة CSS_OUTLINE_COLOR_INVERT:
		fprintf(جریان, "outline-color: invert ");
		اكسر;
	حالة CSS_OUTLINE_COLOR_COLOR:
		fprintf(جریان, "outline-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* outline-style */
	val = css_computed_outline_style(style);
	بدل (val) {
	حالة CSS_OUTLINE_STYLE_NONE:
		fprintf(جریان, "outline-style: none ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_DOTTED:
		fprintf(جریان, "outline-style: dotted ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_DASHED:
		fprintf(جریان, "outline-style: dashed ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_SOLID:
		fprintf(جریان, "outline-style: solid ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_DOUBLE:
		fprintf(جریان, "outline-style: double ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_GROOVE:
		fprintf(جریان, "outline-style: groove ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_RIDGE:
		fprintf(جریان, "outline-style: ridge ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_INSET:
		fprintf(جریان, "outline-style: inset ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_OUTSET:
		fprintf(جریان, "outline-style: outset ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* outline-width */
	val = css_computed_outline_عرض(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_OUTLINE_عرض_THIN:
		fprintf(جریان, "outline-width: thin ");
		اكسر;
	حالة CSS_OUTLINE_عرض_MEDIUM:
		fprintf(جریان, "outline-width: medium ");
		اكسر;
	حالة CSS_OUTLINE_عرض_THICK:
		fprintf(جریان, "outline-width: thick ");
		اكسر;
	حالة CSS_OUTLINE_عرض_عرض:
		fprintf(جریان, "outline-width: ");

		dump_css_unit(جریان, len1, unit1);

		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/*سرریز*/
	val = css_computed_سرریز_x(style);
	بدل (val) {
	حالة CSS_سرریز_VISIBLE:
		fprintf(جریان, "overflow-x: visible ");
		اكسر;
	حالة CSS_سرریز_HIDDEN:
		fprintf(جریان, "overflow-x: hidden ");
		اكسر;
	حالة CSS_سرریز_SCROLL:
		fprintf(جریان, "overflow-x: scroll ");
		اكسر;
	حالة CSS_سرریز_AUTO:
		fprintf(جریان, "overflow-x auto ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/*سرریز*/
	val = css_computed_سرریز_y(style);
	بدل (val) {
	حالة CSS_سرریز_VISIBLE:
		fprintf(جریان, "overflow-y: visible ");
		اكسر;
	حالة CSS_سرریز_HIDDEN:
		fprintf(جریان, "overflow-y: hidden ");
		اكسر;
	حالة CSS_سرریز_SCROLL:
		fprintf(جریان, "overflow-y: scroll ");
		اكسر;
	حالة CSS_سرریز_AUTO:
		fprintf(جریان, "overflow-y: auto ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* padding-top */
	val = css_computed_padding_بالاترین(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_PADDING_مجموعه:
		fprintf(جریان, "padding-top: ");

		dump_css_unit(جریان, len1, unit1);

		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* padding-right */
	val = css_computed_padding_راست(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_PADDING_مجموعه:
		fprintf(جریان, "padding-right: ");

		dump_css_unit(جریان, len1, unit1);

		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* padding-bottom */
	val = css_computed_padding_bottom(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_PADDING_مجموعه:
		fprintf(جریان, "padding-bottom: ");

		dump_css_unit(جریان, len1, unit1);

		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* padding-left */
	val = css_computed_padding_چپ(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_PADDING_مجموعه:
		fprintf(جریان, "padding-left: ");

		dump_css_unit(جریان, len1, unit1);

		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* position */
	val = css_computed_position(style);
	بدل (val) {
	حالة CSS_POSITION_STATIC:
		fprintf(جریان, "position: static ");
		اكسر;
	حالة CSS_POSITION_RELATIVE:
		fprintf(جریان, "position: relative ");
		اكسر;
	حالة CSS_POSITION_ABSOLUTE:
		fprintf(جریان, "position: absolute ");
		اكسر;
	حالة CSS_POSITION_FIXED:
		fprintf(جریان, "position: fixed ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* quotes */
	val = css_computed_quotes(style, &string_list);
	إذا (val == CSS_QUOTES_STRING && string_list != عدم) {
		fprintf(جریان, "quotes:");

		طالما (*string_list != عدم) {
			fprintf(جریان, " \"%.*s\"",
				(صحيح) lwc_string_درازی(*string_list),
				lwc_string_داده(*string_list));

			string_list++;
		}

		fprintf(جریان, " ");
	} وإلا {
		بدل (val) {
		حالة CSS_QUOTES_NONE:
			fprintf(جریان, "quotes: none ");
			اكسر;
		افتراضي:
			اكسر;
		}
	}

	/*راست*/
	val = css_computed_راست(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_راست_AUTO:
		fprintf(جریان, "right: auto ");
		اكسر;
	حالة CSS_راست_مجموعه:
		fprintf(جریان, "right: ");

		dump_css_unit(جریان, len1, unit1);

		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* table-layout */
	val = css_computed_جدول_layout(style);
	بدل (val) {
	حالة CSS_جدول_LAYOUT_AUTO:
		fprintf(جریان, "table-layout: auto ");
		اكسر;
	حالة CSS_جدول_LAYOUT_FIXED:
		fprintf(جریان, "table-layout: fixed ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* text-align */
	val = css_computed_متن_align(style);
	بدل (val) {
	حالة CSS_متن_ALIGN_چپ:
		fprintf(جریان, "text-align: left ");
		اكسر;
	حالة CSS_متن_ALIGN_راست:
		fprintf(جریان, "text-align: right ");
		اكسر;
	حالة CSS_متن_ALIGN_CENTER:
		fprintf(جریان, "text-align: center ");
		اكسر;
	حالة CSS_متن_ALIGN_JUSTIFY:
		fprintf(جریان, "text-align: justify ");
		اكسر;
	حالة CSS_متن_ALIGN_DEFAULT:
		fprintf(جریان, "text-align: default ");
		اكسر;
	حالة CSS_متن_ALIGN_LIBCSS_چپ:
		fprintf(جریان, "text-align: -libcss-left ");
		اكسر;
	حالة CSS_متن_ALIGN_LIBCSS_CENTER:
		fprintf(جریان, "text-align: -libcss-center ");
		اكسر;
	حالة CSS_متن_ALIGN_LIBCSS_راست:
		fprintf(جریان, "text-align: -libcss-right ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* text-decoration */
	val = css_computed_متن_decoration(style);
	إذا (val == CSS_متن_DECORATION_NONE) {
		fprintf(جریان, "text-decoration: none ");
	} وإلا {
		fprintf(جریان, "text-decoration:");

		إذا (val & CSS_متن_DECORATION_BLINK) {
			fprintf(جریان, " blink");
		}
		إذا (val & CSS_متن_DECORATION_سطر_THROUGH) {
			fprintf(جریان, " line-through");
		}
		إذا (val & CSS_متن_DECORATION_OVERLINE) {
			fprintf(جریان, " overline");
		}
		إذا (val & CSS_متن_DECORATION_UNDERLINE) {
			fprintf(جریان, " underline");
		}

		fprintf(جریان, " ");
	}

	/* text-indent */
	val = css_computed_متن_indent(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_متن_INDENT_مجموعه:
		fprintf(جریان, "text-indent: ");

		dump_css_unit(جریان, len1, unit1);

		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* text-transform */
	val = css_computed_متن_transform(style);
	بدل (val) {
	حالة CSS_متن_TRANSFORM_CAPITALIZE:
		fprintf(جریان, "text-transform: capitalize ");
		اكسر;
	حالة CSS_متن_TRANSFORM_UPPERCASE:
		fprintf(جریان, "text-transform: uppercase ");
		اكسر;
	حالة CSS_متن_TRANSFORM_LOWERCASE:
		fprintf(جریان, "text-transform: lowercase ");
		اكسر;
	حالة CSS_متن_TRANSFORM_NONE:
		fprintf(جریان, "text-transform: none ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/*بالاترین*/
	val = css_computed_بالاترین(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_بالاترین_AUTO:
		fprintf(جریان, "top: auto ");
		اكسر;
	حالة CSS_بالاترین_مجموعه:
		fprintf(جریان, "top: ");

		dump_css_unit(جریان, len1, unit1);

		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* unicode-bidi */
	val = css_computed_unicode_bidi(style);
	بدل (val) {
	حالة CSS_UNICODE_BIDI_نرمال:
		fprintf(جریان, "unicode-bidi: normal ");
		اكسر;
	حالة CSS_UNICODE_BIDI_EMBED:
		fprintf(جریان, "unicode-bidi: embed ");
		اكسر;
	حالة CSS_UNICODE_BIDI_BIDI_OVERRIDE:
		fprintf(جریان, "unicode-bidi: bidi-override ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* vertical-align */
	val = css_computed_vertical_align(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_VERTICAL_ALIGN_BASELINE:
		fprintf(جریان, "vertical-align: baseline ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_SUB:
		fprintf(جریان, "vertical-align: sub ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_SUPER:
		fprintf(جریان, "vertical-align: super ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_بالاترین:
		fprintf(جریان, "vertical-align: top ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_متن_بالاترین:
		fprintf(جریان, "vertical-align: text-top ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_میانه:
		fprintf(جریان, "vertical-align: middle ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_BOTTOM:
		fprintf(جریان, "vertical-align: bottom ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_متن_BOTTOM:
		fprintf(جریان, "vertical-align: text-bottom ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_مجموعه:
		fprintf(جریان, "vertical-align: ");

		dump_css_unit(جریان, len1, unit1);

		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* visibility */
	val = css_computed_visibility(style);
	بدل (val) {
	حالة CSS_VISIBILITY_VISIBLE:
		fprintf(جریان, "visibility: visible ");
		اكسر;
	حالة CSS_VISIBILITY_HIDDEN:
		fprintf(جریان, "visibility: hidden ");
		اكسر;
	حالة CSS_VISIBILITY_COLLAPSE:
		fprintf(جریان, "visibility: collapse ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* white-space */
	val = css_computed_white_space(style);
	بدل (val) {
	حالة CSS_WHITE_SPACE_نرمال:
		fprintf(جریان, "white-space: normal ");
		اكسر;
	حالة CSS_WHITE_SPACE_PRE:
		fprintf(جریان, "white-space: pre ");
		اكسر;
	حالة CSS_WHITE_SPACE_NOWRAP:
		fprintf(جریان, "white-space: nowrap ");
		اكسر;
	حالة CSS_WHITE_SPACE_PRE_WRAP:
		fprintf(جریان, "white-space: pre-wrap ");
		اكسر;
	حالة CSS_WHITE_SPACE_PRE_سطر:
		fprintf(جریان, "white-space: pre-line ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/*عرض*/
	val = css_computed_عرض(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_عرض_AUTO:
		fprintf(جریان, "width: auto ");
		اكسر;
	حالة CSS_عرض_مجموعه:
		fprintf(جریان, "width: ");

		dump_css_unit(جریان, len1, unit1);

		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* word-spacing */
	val = css_computed_کلمه_spacing(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_کلمه_SPACING_نرمال:
		fprintf(جریان, "word-spacing: normal ");
		اكسر;
	حالة CSS_کلمه_SPACING_مجموعه:
		fprintf(جریان, "word-spacing: ");

		dump_css_unit(جریان, len1, unit1);

		fprintf(جریان, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* z-index */
	val = css_computed_z_شاخص(style, &zindex);
	بدل (val) {
	حالة CSS_Z_شاخص_AUTO:
		fprintf(جریان, "z-index: auto ");
		اكسر;
	حالة CSS_Z_شاخص_مجموعه:
		fprintf(جریان, "z-index: %"PRId32" ", zindex);
		اكسر;
	افتراضي:
		اكسر;
	}

	fprintf(جریان, "}");
}
