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
ساكن فراغ dump_css_fixed(FILE *stream, css_fixed f)
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
ساكن فراغ dump_css_ژمار(FILE *stream, css_fixed val)
{
	إذا (INTTOFIX(FIXTOINT(val)) == val)
		fprintf(stream, "%"PRId32, FIXTOINT(val));
	وإلا
		dump_css_fixed(stream, val);
}

/**
 * Dump a dimension to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 * \param unit    Unit to write
 */
ساكن فراغ dump_css_unit(FILE *stream, css_fixed val, css_unit unit)
{
	dump_css_ژمار(stream, val);

	بدل (unit) {
	حالة CSS_UNIT_PX:
		fprintf(stream, "px");
		اكسر;
	حالة CSS_UNIT_EX:
		fprintf(stream, "ex");
		اكسر;
	حالة CSS_UNIT_EM:
		fprintf(stream, "em");
		اكسر;
	حالة CSS_UNIT_IN:
		fprintf(stream, "in");
		اكسر;
	حالة CSS_UNIT_CM:
		fprintf(stream, "cm");
		اكسر;
	حالة CSS_UNIT_MM:
		fprintf(stream, "mm");
		اكسر;
	حالة CSS_UNIT_PT:
		fprintf(stream, "pt");
		اكسر;
	حالة CSS_UNIT_PC:
		fprintf(stream, "pc");
		اكسر;
	حالة CSS_UNIT_PCT:
		fprintf(stream, "%%");
		اكسر;
	حالة CSS_UNIT_DEG:
		fprintf(stream, "deg");
		اكسر;
	حالة CSS_UNIT_GRAD:
		fprintf(stream, "grad");
		اكسر;
	حالة CSS_UNIT_RAD:
		fprintf(stream, "rad");
		اكسر;
	حالة CSS_UNIT_MS:
		fprintf(stream, "ms");
		اكسر;
	حالة CSS_UNIT_S:
		fprintf(stream, "s");
		اكسر;
	حالة CSS_UNIT_HZ:
		fprintf(stream, "Hz");
		اكسر;
	حالة CSS_UNIT_KHZ:
		fprintf(stream, "kHz");
		اكسر;
	حالة CSS_UNIT_CH:
		fprintf(stream, "ch");
		اكسر;
	حالة CSS_UNIT_REM:
		fprintf(stream, "rem");
		اكسر;
	حالة CSS_UNIT_LH:
		fprintf(stream, "lh");
		اكسر;
	حالة CSS_UNIT_VH:
		fprintf(stream, "vh");
		اكسر;
	حالة CSS_UNIT_VW:
		fprintf(stream, "vw");
		اكسر;
	حالة CSS_UNIT_VI:
		fprintf(stream, "vi");
		اكسر;
	حالة CSS_UNIT_VB:
		fprintf(stream, "vb");
		اكسر;
	حالة CSS_UNIT_VMIN:
		fprintf(stream, "vmin");
		اكسر;
	حالة CSS_UNIT_VMAX:
		fprintf(stream, "vmax");
		اكسر;
	حالة CSS_UNIT_Q:
		fprintf(stream, "q");
		اكسر;
	حالة CSS_UNIT_CALC:
		fprintf(stream, "calc()");
		اكسر;
	}
}

/* exported interface documented in content/handlers/css/dump.h */
فراغ nscss_dump_computed_style(FILE *stream, ثابت css_computed_style *style)
{
	uint8_t val;
	css_color color = 0;
	lwc_string *url = عدم;
	css_fixed len1 = 0, len2 = 0;
	css_unit unit1 = CSS_UNIT_PX, unit2 = CSS_UNIT_PX;
	css_computed_clip_rect rect = { 0, 0, 0, 0, CSS_UNIT_PX, CSS_UNIT_PX,
					CSS_UNIT_PX, CSS_UNIT_PX, true, true,
					true, true };
	ثابت css_computed_content_item *content = عدم;
	ثابت css_computed_counter *counter = عدم;
	lwc_string **string_list = عدم;
	int32_t zindex = 0;

	fprintf(stream, "{ ");

	/* background-attachment */
	val = css_computed_background_attachment(style);
	بدل (val) {
	حالة CSS_BACKGROUND_ATTACHMENT_FIXED:
		fprintf(stream, "background-attachment: fixed ");
		اكسر;
	حالة CSS_BACKGROUND_ATTACHMENT_SCROLL:
		fprintf(stream, "background-attachment: scroll ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* background-color */
	val = css_computed_background_color(style, &color);
	بدل (val) {
	حالة CSS_BACKGROUND_COLOR_COLOR:
		fprintf(stream, "background-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* background-image */
	val = css_computed_background_image(style, &url);
	إذا (val == CSS_BACKGROUND_IMAGE_IMAGE && url != عدم) {
		fprintf(stream, "background-image: url('%.*s') ",
				(صحيح) lwc_string_length(url),
				lwc_string_data(url));
	} وإلا إذا (val == CSS_BACKGROUND_IMAGE_NONE) {
		fprintf(stream, "background-image: none ");
	}

	/* background-position */
	val = css_computed_background_position(style, &len1, &unit1,
			&len2, &unit2);
	إذا (val == CSS_BACKGROUND_POSITION_SET) {
		fprintf(stream, "background-position: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		dump_css_unit(stream, len2, unit2);
		fprintf(stream, " ");
	}

	/* background-repeat */
	val = css_computed_background_repeat(style);
	بدل (val) {
	حالة CSS_BACKGROUND_REPEAT_REPEAT_X:
		fprintf(stream, "background-repeat: repeat-x ");
		اكسر;
	حالة CSS_BACKGROUND_REPEAT_REPEAT_Y:
		fprintf(stream, "background-repeat: repeat-y ");
		اكسر;
	حالة CSS_BACKGROUND_REPEAT_REPEAT:
		fprintf(stream, "background-repeat: repeat ");
		اكسر;
	حالة CSS_BACKGROUND_REPEAT_NO_REPEAT:
		fprintf(stream, "background-repeat: no-repeat ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-collapse */
	val = css_computed_border_collapse(style);
	بدل (val) {
	حالة CSS_BORDER_COLLAPSE_SEPARATE:
		fprintf(stream, "border-collapse: separate ");
		اكسر;
	حالة CSS_BORDER_COLLAPSE_COLLAPSE:
		fprintf(stream, "border-collapse: collapse ");
		اكسر;
	افتراضي:

		اكسر;
	}

	/* border-spacing */
	val = css_computed_border_spacing(style, &len1, &unit1, &len2, &unit2);
	إذا (val == CSS_BORDER_SPACING_SET) {
		fprintf(stream, "border-spacing: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		dump_css_unit(stream, len2, unit2);
		fprintf(stream, " ");
	}

	/* border-top-color */
	val = css_computed_border_لووتکە_color(style, &color);
	بدل (val) {
	حالة CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-top-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-right-color */
	val = css_computed_border_ڕاست_color(style, &color);
	بدل (val) {
	حالة CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-right-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-bottom-color */
	val = css_computed_border_bottom_color(style, &color);
	بدل (val) {
	حالة CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-bottom-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-left-color */
	val = css_computed_border_چەپ_color(style, &color);
	بدل (val) {
	حالة CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-left-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-top-style */
	val = css_computed_border_لووتکە_style(style);
	بدل (val) {
	حالة CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-top-style: none ");
		اكسر;
	حالة CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-top-style: hidden ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-top-style: dotted ");
		اكسر;
	حالة CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-top-style: dashed ");
		اكسر;
	حالة CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-top-style: solid ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-top-style: double ");
		اكسر;
	حالة CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-top-style: groove ");
		اكسر;
	حالة CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-top-style: ridge ");
		اكسر;
	حالة CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-top-style: inset ");
		اكسر;
	حالة CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-top-style: outset ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-right-style */
	val = css_computed_border_ڕاست_style(style);
	بدل (val) {
	حالة CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-right-style: none ");
		اكسر;
	حالة CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-right-style: hidden ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-right-style: dotted ");
		اكسر;
	حالة CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-right-style: dashed ");
		اكسر;
	حالة CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-right-style: solid ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-right-style: double ");
		اكسر;
	حالة CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-right-style: groove ");
		اكسر;
	حالة CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-right-style: ridge ");
		اكسر;
	حالة CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-right-style: inset ");
		اكسر;
	حالة CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-right-style: outset ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-bottom-style */
	val = css_computed_border_bottom_style(style);
	بدل (val) {
	حالة CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-bottom-style: none ");
		اكسر;
	حالة CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-bottom-style: hidden ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-bottom-style: dotted ");
		اكسر;
	حالة CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-bottom-style: dashed ");
		اكسر;
	حالة CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-bottom-style: solid ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-bottom-style: double ");
		اكسر;
	حالة CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-bottom-style: groove ");
		اكسر;
	حالة CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-bottom-style: ridge ");
		اكسر;
	حالة CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-bottom-style: inset ");
		اكسر;
	حالة CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-bottom-style: outset ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-left-style */
	val = css_computed_border_چەپ_style(style);
	بدل (val) {
	حالة CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-left-style: none ");
		اكسر;
	حالة CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-left-style: hidden ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-left-style: dotted ");
		اكسر;
	حالة CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-left-style: dashed ");
		اكسر;
	حالة CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-left-style: solid ");
		اكسر;
	حالة CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-left-style: double ");
		اكسر;
	حالة CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-left-style: groove ");
		اكسر;
	حالة CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-left-style: ridge ");
		اكسر;
	حالة CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-left-style: inset ");
		اكسر;
	حالة CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-left-style: outset ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-top-width */
	val = css_computed_border_لووتکە_width(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-top-width: thin ");
		اكسر;
	حالة CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-top-width: medium ");
		اكسر;
	حالة CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-top-width: thick ");
		اكسر;
	حالة CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-top-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-right-width */
	val = css_computed_border_ڕاست_width(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-right-width: thin ");
		اكسر;
	حالة CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-right-width: medium ");
		اكسر;
	حالة CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-right-width: thick ");
		اكسر;
	حالة CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-right-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-bottom-width */
	val = css_computed_border_bottom_width(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-bottom-width: thin ");
		اكسر;
	حالة CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-bottom-width: medium ");
		اكسر;
	حالة CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-bottom-width: thick ");
		اكسر;
	حالة CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-bottom-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* border-left-width */
	val = css_computed_border_چەپ_width(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-left-width: thin ");
		اكسر;
	حالة CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-left-width: medium ");
		اكسر;
	حالة CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-left-width: thick ");
		اكسر;
	حالة CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-left-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* bottom */
	val = css_computed_bottom(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_BOTTOM_AUTO:
		fprintf(stream, "bottom: auto ");
		اكسر;
	حالة CSS_BOTTOM_SET:
		fprintf(stream, "bottom: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* caption-side */
	val = css_computed_caption_side(style);
	بدل (val) {
	حالة CSS_CAPTION_SIDE_لووتکە:
		fprintf(stream, "caption_side: top ");
		اكسر;
	حالة CSS_CAPTION_SIDE_BOTTOM:
		fprintf(stream, "caption_side: bottom ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* clear */
	val = css_computed_clear(style);
	بدل (val) {
	حالة CSS_CLEAR_NONE:
		fprintf(stream, "clear: none ");
		اكسر;
	حالة CSS_CLEAR_چەپ:
		fprintf(stream, "clear: left ");
		اكسر;
	حالة CSS_CLEAR_ڕاست:
		fprintf(stream, "clear: right ");
		اكسر;
	حالة CSS_CLEAR_BOTH:
		fprintf(stream, "clear: both ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* clip */
	val = css_computed_clip(style, &rect);
	بدل (val) {
	حالة CSS_CLIP_AUTO:
		fprintf(stream, "clip: auto ");
		اكسر;
	حالة CSS_CLIP_RECT:
		fprintf(stream, "clip: rect( ");

		إذا (rect.لووتکە_auto)
			fprintf(stream, "auto");
		وإلا
			dump_css_unit(stream, rect.لووتکە, rect.tunit);
		fprintf(stream, ", ");

		إذا (rect.ڕاست_auto)
			fprintf(stream, "auto");
		وإلا
			dump_css_unit(stream, rect.ڕاست, rect.runit);
		fprintf(stream, ", ");

		إذا (rect.bottom_auto)
			fprintf(stream, "auto");
		وإلا
			dump_css_unit(stream, rect.bottom, rect.bunit);
		fprintf(stream, ", ");

		إذا (rect.چەپ_auto)
			fprintf(stream, "auto");
		وإلا
			dump_css_unit(stream, rect.چەپ, rect.lunit);
		fprintf(stream, ") ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* color */
	val = css_computed_color(style, &color);
	إذا (val == CSS_COLOR_COLOR) {
		fprintf(stream, "color: #%08"PRIx32" ", color);
	}

	/* content */
	val = css_computed_content(style, &content);
	بدل (val) {
	حالة CSS_CONTENT_NONE:
		fprintf(stream, "content: none ");
		اكسر;
	حالة CSS_CONTENT_NORMAL:
		fprintf(stream, "content: normal ");
		اكسر;
	حالة CSS_CONTENT_SET:
		fprintf(stream, "content:");

		طالما (content->type != CSS_COMPUTED_CONTENT_NONE) {
			fprintf(stream, " ");

			بدل (content->type) {
			حالة CSS_COMPUTED_CONTENT_STRING:
				fprintf(stream,	"\"%.*s\"",
						(صحيح) lwc_string_length(
						content->data.string),
						lwc_string_data(
						content->data.string));
				اكسر;
			حالة CSS_COMPUTED_CONTENT_URI:
				fprintf(stream,	"uri(\"%.*s\")",
						(صحيح) lwc_string_length(
						content->data.uri),
						lwc_string_data(
						content->data.uri));
				اكسر;
			حالة CSS_COMPUTED_CONTENT_COUNTER:
				fprintf(stream, "counter(%.*s)",
						(صحيح) lwc_string_length(
						content->data.counter.name),
						lwc_string_data(
						content->data.counter.name));
				اكسر;
			حالة CSS_COMPUTED_CONTENT_COUNTERS:
				fprintf(stream, "counters(%.*s, \"%.*s\")",
						(صحيح) lwc_string_length(
						content->data.counters.name),
						lwc_string_data(
						content->data.counters.name),
						(صحيح) lwc_string_length(
						content->data.counters.sep),
						lwc_string_data(
						content->data.counters.sep));
				اكسر;
			حالة CSS_COMPUTED_CONTENT_ATTR:
				fprintf(stream, "attr(%.*s)",
						(صحيح) lwc_string_length(
						content->data.attr),
						lwc_string_data(
						content->data.attr));
				اكسر;
			حالة CSS_COMPUTED_CONTENT_OPEN_QUOTE:
				fprintf(stream, "open-quote");
				اكسر;
			حالة CSS_COMPUTED_CONTENT_CLOSE_QUOTE:
				fprintf(stream, "close-quote");
				اكسر;
			حالة CSS_COMPUTED_CONTENT_NO_OPEN_QUOTE:
				fprintf(stream, "no-open-quote");
				اكسر;
			حالة CSS_COMPUTED_CONTENT_NO_CLOSE_QUOTE:
				fprintf(stream, "no-close-quote");
				اكسر;
			}

			content++;
		}

		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* counter-increment */
	val = css_computed_counter_increment(style, &counter);
	إذا ((val == CSS_COUNTER_INCREMENT_NONE) || (counter == عدم)) {
		fprintf(stream, "counter-increment: none ");
	} وإلا {
		fprintf(stream, "counter-increment:");

		طالما (counter->name != عدم) {
			fprintf(stream, " %.*s ",
					(صحيح) lwc_string_length(counter->name),
					lwc_string_data(counter->name));

			dump_css_fixed(stream, counter->بەها);

			counter++;
		}

		fprintf(stream, " ");
	}

	/* counter-reset */
	val = css_computed_counter_reset(style, &counter);
	إذا ((val == CSS_COUNTER_RESET_NONE) || (counter == عدم)) {
		fprintf(stream, "counter-reset: none ");
	} وإلا {
		fprintf(stream, "counter-reset:");

		طالما (counter->name != عدم) {
			fprintf(stream, " %.*s ",
					(صحيح) lwc_string_length(counter->name),
					lwc_string_data(counter->name));

			dump_css_fixed(stream, counter->بەها);

			counter++;
		}

		fprintf(stream, " ");
	}

	/* cursor */
	val = css_computed_cursor(style, &string_list);
	fprintf(stream, "cursor:");

	إذا (string_list != عدم) {
		طالما (*string_list != عدم) {
			fprintf(stream, " url\"%.*s\")",
					(صحيح) lwc_string_length(*string_list),
					lwc_string_data(*string_list));

			string_list++;
		}
	}
	بدل (val) {
	حالة CSS_CURSOR_AUTO:
		fprintf(stream, " auto ");
		اكسر;
	حالة CSS_CURSOR_CROSSHAIR:
		fprintf(stream, " crosshair ");
		اكسر;
	حالة CSS_CURSOR_DEFAULT:
		fprintf(stream, " default ");
		اكسر;
	حالة CSS_CURSOR_POINTER:
		fprintf(stream, " pointer ");
		اكسر;
	حالة CSS_CURSOR_MOVE:
		fprintf(stream, " move ");
		اكسر;
	حالة CSS_CURSOR_E_RESIZE:
		fprintf(stream, " e-resize ");
		اكسر;
	حالة CSS_CURSOR_NE_RESIZE:
		fprintf(stream, " ne-resize ");
		اكسر;
	حالة CSS_CURSOR_NW_RESIZE:
		fprintf(stream, " nw-resize ");
		اكسر;
	حالة CSS_CURSOR_N_RESIZE:
		fprintf(stream, " n-resize ");
		اكسر;
	حالة CSS_CURSOR_SE_RESIZE:
		fprintf(stream, " se-resize ");
		اكسر;
	حالة CSS_CURSOR_SW_RESIZE:
		fprintf(stream, " sw-resize ");
		اكسر;
	حالة CSS_CURSOR_S_RESIZE:
		fprintf(stream, " s-resize ");
		اكسر;
	حالة CSS_CURSOR_W_RESIZE:
		fprintf(stream, " w-resize ");
		اكسر;
	حالة CSS_CURSOR_TEXT:
		fprintf(stream, " text ");
		اكسر;
	حالة CSS_CURSOR_WAIT:
		fprintf(stream, " wait ");
		اكسر;
	حالة CSS_CURSOR_HELP:
		fprintf(stream, " help ");
		اكسر;
	حالة CSS_CURSOR_PROGRESS:
		fprintf(stream, " progress ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* direction */
	val = css_computed_direction(style);
	بدل (val) {
	حالة CSS_DIRECTION_LTR:
		fprintf(stream, "direction: ltr ");
		اكسر;
	حالة CSS_DIRECTION_RTL:
		fprintf(stream, "direction: rtl ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* display */
	val = ns_computed_display_static(style);
	بدل (val) {
	حالة CSS_DISPLAY_INLINE:
		fprintf(stream, "display: inline ");
		اكسر;
	حالة CSS_DISPLAY_BLOCK:
		fprintf(stream, "display: block ");
		اكسر;
	حالة CSS_DISPLAY_LIST_ITEM:
		fprintf(stream, "display: list-item ");
		اكسر;
	حالة CSS_DISPLAY_RUN_IN:
		fprintf(stream, "display: run-in ");
		اكسر;
	حالة CSS_DISPLAY_INLINE_BLOCK:
		fprintf(stream, "display: inline-block ");
		اكسر;
	حالة CSS_DISPLAY_TABLE:
		fprintf(stream, "display: table ");
		اكسر;
	حالة CSS_DISPLAY_INLINE_TABLE:
		fprintf(stream, "display: inline-table ");
		اكسر;
	حالة CSS_DISPLAY_TABLE_ROW_GROUP:
		fprintf(stream, "display: table-row-group ");
		اكسر;
	حالة CSS_DISPLAY_TABLE_HEADER_GROUP:
		fprintf(stream, "display: table-header-group ");
		اكسر;
	حالة CSS_DISPLAY_TABLE_FOOTER_GROUP:
		fprintf(stream, "display: table-footer-group ");
		اكسر;
	حالة CSS_DISPLAY_TABLE_ROW:
		fprintf(stream, "display: table-row ");
		اكسر;
	حالة CSS_DISPLAY_TABLE_COLUMN_GROUP:
		fprintf(stream, "display: table-column-group ");
		اكسر;
	حالة CSS_DISPLAY_TABLE_COLUMN:
		fprintf(stream, "display: table-column ");
		اكسر;
	حالة CSS_DISPLAY_TABLE_CELL:
		fprintf(stream, "display: table-cell ");
		اكسر;
	حالة CSS_DISPLAY_TABLE_CAPTION:
		fprintf(stream, "display: table-caption ");
		اكسر;
	حالة CSS_DISPLAY_NONE:
		fprintf(stream, "display: none ");
		اكسر;
	حالة CSS_DISPLAY_FLEX:
		fprintf(stream, "display: flex ");
		اكسر;
	حالة CSS_DISPLAY_INLINE_FLEX:
		fprintf(stream, "display: inline-flex ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* empty-cells */
	val = css_computed_empty_cells(style);
	بدل (val) {
	حالة CSS_EMPTY_CELLS_SHOW:
		fprintf(stream, "empty-cells: show ");
		اكسر;
	حالة CSS_EMPTY_CELLS_HIDE:
		fprintf(stream, "empty-cells: hide ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* float */
	val = css_computed_float(style);
	بدل (val) {
	حالة CSS_FLOAT_چەپ:
		fprintf(stream, "float: left ");
		اكسر;
	حالة CSS_FLOAT_ڕاست:
		fprintf(stream, "float: right ");
		اكسر;
	حالة CSS_FLOAT_NONE:
		fprintf(stream, "float: none ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* font-family */
	val = css_computed_font_family(style, &string_list);
	إذا (val != CSS_FONT_FAMILY_INHERIT) {
		fprintf(stream, "font-family:");

		إذا (string_list != عدم) {
			طالما (*string_list != عدم) {
				fprintf(stream, " \"%.*s\"",
					(صحيح) lwc_string_length(*string_list),
					lwc_string_data(*string_list));

				string_list++;
			}
		}
		بدل (val) {
		حالة CSS_FONT_FAMILY_SERIF:
			fprintf(stream, " serif ");
			اكسر;
		حالة CSS_FONT_FAMILY_SANS_SERIF:
			fprintf(stream, " sans-serif ");
			اكسر;
		حالة CSS_FONT_FAMILY_CURSIVE:
			fprintf(stream, " cursive ");
			اكسر;
		حالة CSS_FONT_FAMILY_FANTASY:
			fprintf(stream, " fantasy ");
			اكسر;
		حالة CSS_FONT_FAMILY_MONOSPACE:
			fprintf(stream, " monospace ");
			اكسر;
		}
	}

	/* font-size */
	val = css_computed_font_size(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_FONT_SIZE_XX_SMALL:
		fprintf(stream, "font-size: xx-small ");
		اكسر;
	حالة CSS_FONT_SIZE_X_SMALL:
		fprintf(stream, "font-size: x-small ");
		اكسر;
	حالة CSS_FONT_SIZE_SMALL:
		fprintf(stream, "font-size: small ");
		اكسر;
	حالة CSS_FONT_SIZE_MEDIUM:
		fprintf(stream, "font-size: medium ");
		اكسر;
	حالة CSS_FONT_SIZE_LARGE:
		fprintf(stream, "font-size: large ");
		اكسر;
	حالة CSS_FONT_SIZE_X_LARGE:
		fprintf(stream, "font-size: x-large ");
		اكسر;
	حالة CSS_FONT_SIZE_XX_LARGE:
		fprintf(stream, "font-size: xx-large ");
		اكسر;
	حالة CSS_FONT_SIZE_LARGER:
		fprintf(stream, "font-size: larger ");
		اكسر;
	حالة CSS_FONT_SIZE_SMALLER:
		fprintf(stream, "font-size: smaller ");
		اكسر;
	حالة CSS_FONT_SIZE_DIMENSION:
		fprintf(stream, "font-size: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* font-style */
	val = css_computed_font_style(style);
	بدل (val) {
	حالة CSS_FONT_STYLE_NORMAL:
		fprintf(stream, "font-style: normal ");
		اكسر;
	حالة CSS_FONT_STYLE_ITALIC:
		fprintf(stream, "font-style: italic ");
		اكسر;
	حالة CSS_FONT_STYLE_OBLIQUE:
		fprintf(stream, "font-style: oblique ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* font-variant */
	val = css_computed_font_variant(style);
	بدل (val) {
	حالة CSS_FONT_VARIANT_NORMAL:
		fprintf(stream, "font-variant: normal ");
		اكسر;
	حالة CSS_FONT_VARIANT_SMALL_CAPS:
		fprintf(stream, "font-variant: small-caps ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* font-weight */
	val = css_computed_font_weight(style);
	بدل (val) {
	حالة CSS_FONT_WEIGHT_NORMAL:
		fprintf(stream, "font-weight: normal ");
		اكسر;
	حالة CSS_FONT_WEIGHT_BOLD:
		fprintf(stream, "font-weight: bold ");
		اكسر;
	حالة CSS_FONT_WEIGHT_BOLDER:
		fprintf(stream, "font-weight: bolder ");
		اكسر;
	حالة CSS_FONT_WEIGHT_LIGHTER:
		fprintf(stream, "font-weight: lighter ");
		اكسر;
	حالة CSS_FONT_WEIGHT_100:
		fprintf(stream, "font-weight: 100 ");
		اكسر;
	حالة CSS_FONT_WEIGHT_200:
		fprintf(stream, "font-weight: 200 ");
		اكسر;
	حالة CSS_FONT_WEIGHT_300:
		fprintf(stream, "font-weight: 300 ");
		اكسر;
	حالة CSS_FONT_WEIGHT_400:
		fprintf(stream, "font-weight: 400 ");
		اكسر;
	حالة CSS_FONT_WEIGHT_500:
		fprintf(stream, "font-weight: 500 ");
		اكسر;
	حالة CSS_FONT_WEIGHT_600:
		fprintf(stream, "font-weight: 600 ");
		اكسر;
	حالة CSS_FONT_WEIGHT_700:
		fprintf(stream, "font-weight: 700 ");
		اكسر;
	حالة CSS_FONT_WEIGHT_800:
		fprintf(stream, "font-weight: 800 ");
		اكسر;
	حالة CSS_FONT_WEIGHT_900:
		fprintf(stream, "font-weight: 900 ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* height */
	val = css_computed_height(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_HEIGHT_AUTO:
		fprintf(stream, "height: auto ");
		اكسر;
	حالة CSS_HEIGHT_SET:
		fprintf(stream, "height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/*چەپ*/
	val = css_computed_چەپ(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_چەپ_AUTO:
		fprintf(stream, "left: auto ");
		اكسر;
	حالة CSS_چەپ_SET:
		fprintf(stream, "left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* letter-spacing */
	val = css_computed_letter_spacing(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_LETTER_SPACING_NORMAL:
		fprintf(stream, "letter-spacing: normal ");
		اكسر;
	حالة CSS_LETTER_SPACING_SET:
		fprintf(stream, "letter-spacing: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* line-height */
	val = css_computed_هێڵ_height(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_هێڵ_HEIGHT_NORMAL:
		fprintf(stream, "line-height: normal ");
		اكسر;
	حالة CSS_هێڵ_HEIGHT_ژمار:
		fprintf(stream, "line-height: ");

		dump_css_fixed(stream, len1);

		fprintf(stream, " ");
		اكسر;
	حالة CSS_هێڵ_HEIGHT_DIMENSION:
		fprintf(stream, "line-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* list-style-image */
	val = css_computed_list_style_image(style, &url);
	إذا (url != عدم) {
		fprintf(stream, "list-style-image: url('%.*s') ",
				(صحيح) lwc_string_length(url),
				lwc_string_data(url));
	} وإلا إذا (val == CSS_LIST_STYLE_IMAGE_NONE) {
		fprintf(stream, "list-style-image: none ");
	}

	/* list-style-position */
	val = css_computed_list_style_position(style);
	بدل (val) {
	حالة CSS_LIST_STYLE_POSITION_INSIDE:
		fprintf(stream, "list-style-position: inside ");
		اكسر;
	حالة CSS_LIST_STYLE_POSITION_OUTSIDE:
		fprintf(stream, "list-style-position: outside ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* list-style-type */
	val = css_computed_list_style_type(style);
	بدل (val) {
	حالة CSS_LIST_STYLE_TYPE_DISC:
		fprintf(stream, "list-style-type: disc ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_CIRCLE:
		fprintf(stream, "list-style-type: circle ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_SQUARE:
		fprintf(stream, "list-style-type: square ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_DECIMAL:
		fprintf(stream, "list-style-type: decimal ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_DECIMAL_LEADING_ZERO:
		fprintf(stream, "list-style-type: decimal-leading-zero ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_LOWER_ROMAN:
		fprintf(stream, "list-style-type: lower-roman ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_UPPER_ROMAN:
		fprintf(stream, "list-style-type: upper-roman ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_LOWER_GREEK:
		fprintf(stream, "list-style-type: lower-greek ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_LOWER_LATIN:
		fprintf(stream, "list-style-type: lower-latin ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_UPPER_LATIN:
		fprintf(stream, "list-style-type: upper-latin ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_ARMENIAN:
		fprintf(stream, "list-style-type: armenian ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_GEORGIAN:
		fprintf(stream, "list-style-type: georgian ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_LOWER_ALPHA:
		fprintf(stream, "list-style-type: lower-alpha ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_UPPER_ALPHA:
		fprintf(stream, "list-style-type: upper-alpha ");
		اكسر;
	حالة CSS_LIST_STYLE_TYPE_NONE:
		fprintf(stream, "list-style-type: none ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* margin-top */
	val = css_computed_margin_لووتکە(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MARGIN_AUTO:
		fprintf(stream, "margin-top: auto ");
		اكسر;
	حالة CSS_MARGIN_SET:
		fprintf(stream, "margin-top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* margin-right */
	val = css_computed_margin_ڕاست(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MARGIN_AUTO:
		fprintf(stream, "margin-right: auto ");
		اكسر;
	حالة CSS_MARGIN_SET:
		fprintf(stream, "margin-right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* margin-bottom */
	val = css_computed_margin_bottom(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MARGIN_AUTO:
		fprintf(stream, "margin-bottom: auto ");
		اكسر;
	حالة CSS_MARGIN_SET:
		fprintf(stream, "margin-bottom: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* margin-left */
	val = css_computed_margin_چەپ(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MARGIN_AUTO:
		fprintf(stream, "margin-left: auto ");
		اكسر;
	حالة CSS_MARGIN_SET:
		fprintf(stream, "margin-left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* max-height */
	val = css_computed_max_height(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MAX_HEIGHT_NONE:
		fprintf(stream, "max-height: none ");
		اكسر;
	حالة CSS_MAX_HEIGHT_SET:
		fprintf(stream, "max-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* max-width */
	val = css_computed_max_width(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MAX_WIDTH_NONE:
		fprintf(stream, "max-width: none ");
		اكسر;
	حالة CSS_MAX_WIDTH_SET:
		fprintf(stream, "max-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* min-height */
	val = ns_computed_min_height(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MIN_HEIGHT_SET:
		fprintf(stream, "min-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* min-width */
	val = ns_computed_min_width(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_MIN_WIDTH_SET:
		fprintf(stream, "min-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* opacity */
	val = css_computed_opacity(style, &len1);
	بدل (val) {
	حالة CSS_OPACITY_SET:
		fprintf(stream, "opacity: ");

		dump_css_fixed(stream, len1);

		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* outline-color */
	val = css_computed_outline_color(style, &color);
	بدل (val) {
	حالة CSS_OUTLINE_COLOR_INVERT:
		fprintf(stream, "outline-color: invert ");
		اكسر;
	حالة CSS_OUTLINE_COLOR_COLOR:
		fprintf(stream, "outline-color: #%08"PRIx32" ", color);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* outline-style */
	val = css_computed_outline_style(style);
	بدل (val) {
	حالة CSS_OUTLINE_STYLE_NONE:
		fprintf(stream, "outline-style: none ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_DOTTED:
		fprintf(stream, "outline-style: dotted ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_DASHED:
		fprintf(stream, "outline-style: dashed ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_SOLID:
		fprintf(stream, "outline-style: solid ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_DOUBLE:
		fprintf(stream, "outline-style: double ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_GROOVE:
		fprintf(stream, "outline-style: groove ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_RIDGE:
		fprintf(stream, "outline-style: ridge ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_INSET:
		fprintf(stream, "outline-style: inset ");
		اكسر;
	حالة CSS_OUTLINE_STYLE_OUTSET:
		fprintf(stream, "outline-style: outset ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* outline-width */
	val = css_computed_outline_width(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_OUTLINE_WIDTH_THIN:
		fprintf(stream, "outline-width: thin ");
		اكسر;
	حالة CSS_OUTLINE_WIDTH_MEDIUM:
		fprintf(stream, "outline-width: medium ");
		اكسر;
	حالة CSS_OUTLINE_WIDTH_THICK:
		fprintf(stream, "outline-width: thick ");
		اكسر;
	حالة CSS_OUTLINE_WIDTH_WIDTH:
		fprintf(stream, "outline-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* overflow */
	val = css_computed_overflow_x(style);
	بدل (val) {
	حالة CSS_OVERFLOW_VISIBLE:
		fprintf(stream, "overflow-x: visible ");
		اكسر;
	حالة CSS_OVERFLOW_HIDDEN:
		fprintf(stream, "overflow-x: hidden ");
		اكسر;
	حالة CSS_OVERFLOW_SCROLL:
		fprintf(stream, "overflow-x: scroll ");
		اكسر;
	حالة CSS_OVERFLOW_AUTO:
		fprintf(stream, "overflow-x auto ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* overflow */
	val = css_computed_overflow_y(style);
	بدل (val) {
	حالة CSS_OVERFLOW_VISIBLE:
		fprintf(stream, "overflow-y: visible ");
		اكسر;
	حالة CSS_OVERFLOW_HIDDEN:
		fprintf(stream, "overflow-y: hidden ");
		اكسر;
	حالة CSS_OVERFLOW_SCROLL:
		fprintf(stream, "overflow-y: scroll ");
		اكسر;
	حالة CSS_OVERFLOW_AUTO:
		fprintf(stream, "overflow-y: auto ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* padding-top */
	val = css_computed_padding_لووتکە(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_PADDING_SET:
		fprintf(stream, "padding-top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* padding-right */
	val = css_computed_padding_ڕاست(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_PADDING_SET:
		fprintf(stream, "padding-right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* padding-bottom */
	val = css_computed_padding_bottom(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_PADDING_SET:
		fprintf(stream, "padding-bottom: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* padding-left */
	val = css_computed_padding_چەپ(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_PADDING_SET:
		fprintf(stream, "padding-left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* position */
	val = css_computed_position(style);
	بدل (val) {
	حالة CSS_POSITION_STATIC:
		fprintf(stream, "position: static ");
		اكسر;
	حالة CSS_POSITION_RELATIVE:
		fprintf(stream, "position: relative ");
		اكسر;
	حالة CSS_POSITION_ABSOLUTE:
		fprintf(stream, "position: absolute ");
		اكسر;
	حالة CSS_POSITION_FIXED:
		fprintf(stream, "position: fixed ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* quotes */
	val = css_computed_quotes(style, &string_list);
	إذا (val == CSS_QUOTES_STRING && string_list != عدم) {
		fprintf(stream, "quotes:");

		طالما (*string_list != عدم) {
			fprintf(stream, " \"%.*s\"",
				(صحيح) lwc_string_length(*string_list),
				lwc_string_data(*string_list));

			string_list++;
		}

		fprintf(stream, " ");
	} وإلا {
		بدل (val) {
		حالة CSS_QUOTES_NONE:
			fprintf(stream, "quotes: none ");
			اكسر;
		افتراضي:
			اكسر;
		}
	}

	/*ڕاست*/
	val = css_computed_ڕاست(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_ڕاست_AUTO:
		fprintf(stream, "right: auto ");
		اكسر;
	حالة CSS_ڕاست_SET:
		fprintf(stream, "right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* table-layout */
	val = css_computed_table_layout(style);
	بدل (val) {
	حالة CSS_TABLE_LAYOUT_AUTO:
		fprintf(stream, "table-layout: auto ");
		اكسر;
	حالة CSS_TABLE_LAYOUT_FIXED:
		fprintf(stream, "table-layout: fixed ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* text-align */
	val = css_computed_text_align(style);
	بدل (val) {
	حالة CSS_TEXT_ALIGN_چەپ:
		fprintf(stream, "text-align: left ");
		اكسر;
	حالة CSS_TEXT_ALIGN_ڕاست:
		fprintf(stream, "text-align: right ");
		اكسر;
	حالة CSS_TEXT_ALIGN_CENTER:
		fprintf(stream, "text-align: center ");
		اكسر;
	حالة CSS_TEXT_ALIGN_JUSTIFY:
		fprintf(stream, "text-align: justify ");
		اكسر;
	حالة CSS_TEXT_ALIGN_DEFAULT:
		fprintf(stream, "text-align: default ");
		اكسر;
	حالة CSS_TEXT_ALIGN_LIBCSS_چەپ:
		fprintf(stream, "text-align: -libcss-left ");
		اكسر;
	حالة CSS_TEXT_ALIGN_LIBCSS_CENTER:
		fprintf(stream, "text-align: -libcss-center ");
		اكسر;
	حالة CSS_TEXT_ALIGN_LIBCSS_ڕاست:
		fprintf(stream, "text-align: -libcss-right ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* text-decoration */
	val = css_computed_text_decoration(style);
	إذا (val == CSS_TEXT_DECORATION_NONE) {
		fprintf(stream, "text-decoration: none ");
	} وإلا {
		fprintf(stream, "text-decoration:");

		إذا (val & CSS_TEXT_DECORATION_BLINK) {
			fprintf(stream, " blink");
		}
		إذا (val & CSS_TEXT_DECORATION_هێڵ_THROUGH) {
			fprintf(stream, " line-through");
		}
		إذا (val & CSS_TEXT_DECORATION_OVERLINE) {
			fprintf(stream, " overline");
		}
		إذا (val & CSS_TEXT_DECORATION_UNDERLINE) {
			fprintf(stream, " underline");
		}

		fprintf(stream, " ");
	}

	/* text-indent */
	val = css_computed_text_indent(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_TEXT_INDENT_SET:
		fprintf(stream, "text-indent: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* text-transform */
	val = css_computed_text_transform(style);
	بدل (val) {
	حالة CSS_TEXT_TRANSFORM_CAPITALIZE:
		fprintf(stream, "text-transform: capitalize ");
		اكسر;
	حالة CSS_TEXT_TRANSFORM_UPPERCASE:
		fprintf(stream, "text-transform: uppercase ");
		اكسر;
	حالة CSS_TEXT_TRANSFORM_LOWERCASE:
		fprintf(stream, "text-transform: lowercase ");
		اكسر;
	حالة CSS_TEXT_TRANSFORM_NONE:
		fprintf(stream, "text-transform: none ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/*لووتکە*/
	val = css_computed_لووتکە(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_لووتکە_AUTO:
		fprintf(stream, "top: auto ");
		اكسر;
	حالة CSS_لووتکە_SET:
		fprintf(stream, "top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* unicode-bidi */
	val = css_computed_unicode_bidi(style);
	بدل (val) {
	حالة CSS_UNICODE_BIDI_NORMAL:
		fprintf(stream, "unicode-bidi: normal ");
		اكسر;
	حالة CSS_UNICODE_BIDI_EMBED:
		fprintf(stream, "unicode-bidi: embed ");
		اكسر;
	حالة CSS_UNICODE_BIDI_BIDI_OVERRIDE:
		fprintf(stream, "unicode-bidi: bidi-override ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* vertical-align */
	val = css_computed_vertical_align(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_VERTICAL_ALIGN_BASELINE:
		fprintf(stream, "vertical-align: baseline ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_SUB:
		fprintf(stream, "vertical-align: sub ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_SUPER:
		fprintf(stream, "vertical-align: super ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_لووتکە:
		fprintf(stream, "vertical-align: top ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_TEXT_لووتکە:
		fprintf(stream, "vertical-align: text-top ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_ناوەڕاست:
		fprintf(stream, "vertical-align: middle ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_BOTTOM:
		fprintf(stream, "vertical-align: bottom ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
		fprintf(stream, "vertical-align: text-bottom ");
		اكسر;
	حالة CSS_VERTICAL_ALIGN_SET:
		fprintf(stream, "vertical-align: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* visibility */
	val = css_computed_visibility(style);
	بدل (val) {
	حالة CSS_VISIBILITY_VISIBLE:
		fprintf(stream, "visibility: visible ");
		اكسر;
	حالة CSS_VISIBILITY_HIDDEN:
		fprintf(stream, "visibility: hidden ");
		اكسر;
	حالة CSS_VISIBILITY_COLLAPSE:
		fprintf(stream, "visibility: collapse ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* white-space */
	val = css_computed_white_space(style);
	بدل (val) {
	حالة CSS_WHITE_SPACE_NORMAL:
		fprintf(stream, "white-space: normal ");
		اكسر;
	حالة CSS_WHITE_SPACE_PRE:
		fprintf(stream, "white-space: pre ");
		اكسر;
	حالة CSS_WHITE_SPACE_NOWRAP:
		fprintf(stream, "white-space: nowrap ");
		اكسر;
	حالة CSS_WHITE_SPACE_PRE_WRAP:
		fprintf(stream, "white-space: pre-wrap ");
		اكسر;
	حالة CSS_WHITE_SPACE_PRE_هێڵ:
		fprintf(stream, "white-space: pre-line ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* width */
	val = css_computed_width(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_WIDTH_AUTO:
		fprintf(stream, "width: auto ");
		اكسر;
	حالة CSS_WIDTH_SET:
		fprintf(stream, "width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* word-spacing */
	val = css_computed_word_spacing(style, &len1, &unit1);
	بدل (val) {
	حالة CSS_WORD_SPACING_NORMAL:
		fprintf(stream, "word-spacing: normal ");
		اكسر;
	حالة CSS_WORD_SPACING_SET:
		fprintf(stream, "word-spacing: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		اكسر;
	افتراضي:
		اكسر;
	}

	/* z-index */
	val = css_computed_z_index(style, &zindex);
	بدل (val) {
	حالة CSS_Z_INDEX_AUTO:
		fprintf(stream, "z-index: auto ");
		اكسر;
	حالة CSS_Z_INDEX_SET:
		fprintf(stream, "z-index: %"PRId32" ", zindex);
		اكسر;
	افتراضي:
		اكسر;
	}

	fprintf(stream, "}");
}
