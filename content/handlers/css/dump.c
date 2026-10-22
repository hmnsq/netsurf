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
ایستا تهی dump_css_fixed(FILE *stream, css_fixed f)
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
ایستا تهی dump_css_عدد(FILE *stream, css_fixed val)
{
	اگر (INTTOFIX(FIXTOINT(val)) == val)
		fprintf(stream, "%"PRId32, FIXTOINT(val));
	وگرنه
		dump_css_fixed(stream, val);
}

/**
 * Dump a dimension to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 * \param unit    Unit to write
 */
ایستا تهی dump_css_unit(FILE *stream, css_fixed val, css_unit unit)
{
	dump_css_عدد(stream, val);

	انتخاب (unit) {
	حالت CSS_UNIT_PX:
		fprintf(stream, "px");
		بشکن;
	حالت CSS_UNIT_EX:
		fprintf(stream, "ex");
		بشکن;
	حالت CSS_UNIT_EM:
		fprintf(stream, "em");
		بشکن;
	حالت CSS_UNIT_IN:
		fprintf(stream, "in");
		بشکن;
	حالت CSS_UNIT_CM:
		fprintf(stream, "cm");
		بشکن;
	حالت CSS_UNIT_MM:
		fprintf(stream, "mm");
		بشکن;
	حالت CSS_UNIT_PT:
		fprintf(stream, "pt");
		بشکن;
	حالت CSS_UNIT_PC:
		fprintf(stream, "pc");
		بشکن;
	حالت CSS_UNIT_PCT:
		fprintf(stream, "%%");
		بشکن;
	حالت CSS_UNIT_DEG:
		fprintf(stream, "deg");
		بشکن;
	حالت CSS_UNIT_GRAD:
		fprintf(stream, "grad");
		بشکن;
	حالت CSS_UNIT_RAD:
		fprintf(stream, "rad");
		بشکن;
	حالت CSS_UNIT_MS:
		fprintf(stream, "ms");
		بشکن;
	حالت CSS_UNIT_S:
		fprintf(stream, "s");
		بشکن;
	حالت CSS_UNIT_HZ:
		fprintf(stream, "Hz");
		بشکن;
	حالت CSS_UNIT_KHZ:
		fprintf(stream, "kHz");
		بشکن;
	حالت CSS_UNIT_CH:
		fprintf(stream, "ch");
		بشکن;
	حالت CSS_UNIT_REM:
		fprintf(stream, "rem");
		بشکن;
	حالت CSS_UNIT_LH:
		fprintf(stream, "lh");
		بشکن;
	حالت CSS_UNIT_VH:
		fprintf(stream, "vh");
		بشکن;
	حالت CSS_UNIT_VW:
		fprintf(stream, "vw");
		بشکن;
	حالت CSS_UNIT_VI:
		fprintf(stream, "vi");
		بشکن;
	حالت CSS_UNIT_VB:
		fprintf(stream, "vb");
		بشکن;
	حالت CSS_UNIT_VMIN:
		fprintf(stream, "vmin");
		بشکن;
	حالت CSS_UNIT_VMAX:
		fprintf(stream, "vmax");
		بشکن;
	حالت CSS_UNIT_Q:
		fprintf(stream, "q");
		بشکن;
	حالت CSS_UNIT_CALC:
		fprintf(stream, "calc()");
		بشکن;
	}
}

/* exported interface documented in content/handlers/css/dump.h */
تهی nscss_dump_computed_style(FILE *stream, ثابت css_computed_style *style)
{
	uint8_t val;
	css_color color = 0;
	lwc_string *url = پوچ;
	css_fixed len1 = 0, len2 = 0;
	css_unit unit1 = CSS_UNIT_PX, unit2 = CSS_UNIT_PX;
	css_computed_clip_rect rect = { 0, 0, 0, 0, CSS_UNIT_PX, CSS_UNIT_PX,
					CSS_UNIT_PX, CSS_UNIT_PX, true, true,
					true, true };
	ثابت css_computed_content_item *content = پوچ;
	ثابت css_computed_counter *counter = پوچ;
	lwc_string **string_list = پوچ;
	int32_t zindex = 0;

	fprintf(stream, "{ ");

	/* background-attachment */
	val = css_computed_background_attachment(style);
	انتخاب (val) {
	حالت CSS_BACKGROUND_ATTACHMENT_FIXED:
		fprintf(stream, "background-attachment: fixed ");
		بشکن;
	حالت CSS_BACKGROUND_ATTACHMENT_SCROLL:
		fprintf(stream, "background-attachment: scroll ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* background-color */
	val = css_computed_background_color(style, &color);
	انتخاب (val) {
	حالت CSS_BACKGROUND_COLOR_COLOR:
		fprintf(stream, "background-color: #%08"PRIx32" ", color);
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* background-image */
	val = css_computed_background_image(style, &url);
	اگر (val == CSS_BACKGROUND_IMAGE_IMAGE && url != پوچ) {
		fprintf(stream, "background-image: url('%.*s') ",
				(صحیح) lwc_string_length(url),
				lwc_string_data(url));
	} وگرنه اگر (val == CSS_BACKGROUND_IMAGE_NONE) {
		fprintf(stream, "background-image: none ");
	}

	/* background-position */
	val = css_computed_background_position(style, &len1, &unit1,
			&len2, &unit2);
	اگر (val == CSS_BACKGROUND_POSITION_SET) {
		fprintf(stream, "background-position: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		dump_css_unit(stream, len2, unit2);
		fprintf(stream, " ");
	}

	/* background-repeat */
	val = css_computed_background_repeat(style);
	انتخاب (val) {
	حالت CSS_BACKGROUND_REPEAT_REPEAT_X:
		fprintf(stream, "background-repeat: repeat-x ");
		بشکن;
	حالت CSS_BACKGROUND_REPEAT_REPEAT_Y:
		fprintf(stream, "background-repeat: repeat-y ");
		بشکن;
	حالت CSS_BACKGROUND_REPEAT_REPEAT:
		fprintf(stream, "background-repeat: repeat ");
		بشکن;
	حالت CSS_BACKGROUND_REPEAT_NO_REPEAT:
		fprintf(stream, "background-repeat: no-repeat ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* border-collapse */
	val = css_computed_border_collapse(style);
	انتخاب (val) {
	حالت CSS_BORDER_COLLAPSE_SEPARATE:
		fprintf(stream, "border-collapse: separate ");
		بشکن;
	حالت CSS_BORDER_COLLAPSE_COLLAPSE:
		fprintf(stream, "border-collapse: collapse ");
		بشکن;
	پیشفرض:

		بشکن;
	}

	/* border-spacing */
	val = css_computed_border_spacing(style, &len1, &unit1, &len2, &unit2);
	اگر (val == CSS_BORDER_SPACING_SET) {
		fprintf(stream, "border-spacing: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		dump_css_unit(stream, len2, unit2);
		fprintf(stream, " ");
	}

	/* border-top-color */
	val = css_computed_border_بالاترین_color(style, &color);
	انتخاب (val) {
	حالت CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-top-color: #%08"PRIx32" ", color);
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* border-right-color */
	val = css_computed_border_راست_color(style, &color);
	انتخاب (val) {
	حالت CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-right-color: #%08"PRIx32" ", color);
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* border-bottom-color */
	val = css_computed_border_bottom_color(style, &color);
	انتخاب (val) {
	حالت CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-bottom-color: #%08"PRIx32" ", color);
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* border-left-color */
	val = css_computed_border_چپ_color(style, &color);
	انتخاب (val) {
	حالت CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-left-color: #%08"PRIx32" ", color);
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* border-top-style */
	val = css_computed_border_بالاترین_style(style);
	انتخاب (val) {
	حالت CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-top-style: none ");
		بشکن;
	حالت CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-top-style: hidden ");
		بشکن;
	حالت CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-top-style: dotted ");
		بشکن;
	حالت CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-top-style: dashed ");
		بشکن;
	حالت CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-top-style: solid ");
		بشکن;
	حالت CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-top-style: double ");
		بشکن;
	حالت CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-top-style: groove ");
		بشکن;
	حالت CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-top-style: ridge ");
		بشکن;
	حالت CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-top-style: inset ");
		بشکن;
	حالت CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-top-style: outset ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* border-right-style */
	val = css_computed_border_راست_style(style);
	انتخاب (val) {
	حالت CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-right-style: none ");
		بشکن;
	حالت CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-right-style: hidden ");
		بشکن;
	حالت CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-right-style: dotted ");
		بشکن;
	حالت CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-right-style: dashed ");
		بشکن;
	حالت CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-right-style: solid ");
		بشکن;
	حالت CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-right-style: double ");
		بشکن;
	حالت CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-right-style: groove ");
		بشکن;
	حالت CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-right-style: ridge ");
		بشکن;
	حالت CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-right-style: inset ");
		بشکن;
	حالت CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-right-style: outset ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* border-bottom-style */
	val = css_computed_border_bottom_style(style);
	انتخاب (val) {
	حالت CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-bottom-style: none ");
		بشکن;
	حالت CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-bottom-style: hidden ");
		بشکن;
	حالت CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-bottom-style: dotted ");
		بشکن;
	حالت CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-bottom-style: dashed ");
		بشکن;
	حالت CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-bottom-style: solid ");
		بشکن;
	حالت CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-bottom-style: double ");
		بشکن;
	حالت CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-bottom-style: groove ");
		بشکن;
	حالت CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-bottom-style: ridge ");
		بشکن;
	حالت CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-bottom-style: inset ");
		بشکن;
	حالت CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-bottom-style: outset ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* border-left-style */
	val = css_computed_border_چپ_style(style);
	انتخاب (val) {
	حالت CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-left-style: none ");
		بشکن;
	حالت CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-left-style: hidden ");
		بشکن;
	حالت CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-left-style: dotted ");
		بشکن;
	حالت CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-left-style: dashed ");
		بشکن;
	حالت CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-left-style: solid ");
		بشکن;
	حالت CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-left-style: double ");
		بشکن;
	حالت CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-left-style: groove ");
		بشکن;
	حالت CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-left-style: ridge ");
		بشکن;
	حالت CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-left-style: inset ");
		بشکن;
	حالت CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-left-style: outset ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* border-top-width */
	val = css_computed_border_بالاترین_width(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-top-width: thin ");
		بشکن;
	حالت CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-top-width: medium ");
		بشکن;
	حالت CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-top-width: thick ");
		بشکن;
	حالت CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-top-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* border-right-width */
	val = css_computed_border_راست_width(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-right-width: thin ");
		بشکن;
	حالت CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-right-width: medium ");
		بشکن;
	حالت CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-right-width: thick ");
		بشکن;
	حالت CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-right-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* border-bottom-width */
	val = css_computed_border_bottom_width(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-bottom-width: thin ");
		بشکن;
	حالت CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-bottom-width: medium ");
		بشکن;
	حالت CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-bottom-width: thick ");
		بشکن;
	حالت CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-bottom-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* border-left-width */
	val = css_computed_border_چپ_width(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-left-width: thin ");
		بشکن;
	حالت CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-left-width: medium ");
		بشکن;
	حالت CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-left-width: thick ");
		بشکن;
	حالت CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-left-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* bottom */
	val = css_computed_bottom(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_BOTTOM_AUTO:
		fprintf(stream, "bottom: auto ");
		بشکن;
	حالت CSS_BOTTOM_SET:
		fprintf(stream, "bottom: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* caption-side */
	val = css_computed_caption_side(style);
	انتخاب (val) {
	حالت CSS_CAPTION_SIDE_بالاترین:
		fprintf(stream, "caption_side: top ");
		بشکن;
	حالت CSS_CAPTION_SIDE_BOTTOM:
		fprintf(stream, "caption_side: bottom ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* clear */
	val = css_computed_clear(style);
	انتخاب (val) {
	حالت CSS_CLEAR_NONE:
		fprintf(stream, "clear: none ");
		بشکن;
	حالت CSS_CLEAR_چپ:
		fprintf(stream, "clear: left ");
		بشکن;
	حالت CSS_CLEAR_راست:
		fprintf(stream, "clear: right ");
		بشکن;
	حالت CSS_CLEAR_BOTH:
		fprintf(stream, "clear: both ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* clip */
	val = css_computed_clip(style, &rect);
	انتخاب (val) {
	حالت CSS_CLIP_AUTO:
		fprintf(stream, "clip: auto ");
		بشکن;
	حالت CSS_CLIP_RECT:
		fprintf(stream, "clip: rect( ");

		اگر (rect.بالاترین_auto)
			fprintf(stream, "auto");
		وگرنه
			dump_css_unit(stream, rect.بالاترین, rect.tunit);
		fprintf(stream, ", ");

		اگر (rect.راست_auto)
			fprintf(stream, "auto");
		وگرنه
			dump_css_unit(stream, rect.راست, rect.runit);
		fprintf(stream, ", ");

		اگر (rect.bottom_auto)
			fprintf(stream, "auto");
		وگرنه
			dump_css_unit(stream, rect.bottom, rect.bunit);
		fprintf(stream, ", ");

		اگر (rect.چپ_auto)
			fprintf(stream, "auto");
		وگرنه
			dump_css_unit(stream, rect.چپ, rect.lunit);
		fprintf(stream, ") ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* color */
	val = css_computed_color(style, &color);
	اگر (val == CSS_COLOR_COLOR) {
		fprintf(stream, "color: #%08"PRIx32" ", color);
	}

	/* content */
	val = css_computed_content(style, &content);
	انتخاب (val) {
	حالت CSS_CONTENT_NONE:
		fprintf(stream, "content: none ");
		بشکن;
	حالت CSS_CONTENT_NORMAL:
		fprintf(stream, "content: normal ");
		بشکن;
	حالت CSS_CONTENT_SET:
		fprintf(stream, "content:");

		وقتی (content->type != CSS_COMPUTED_CONTENT_NONE) {
			fprintf(stream, " ");

			انتخاب (content->type) {
			حالت CSS_COMPUTED_CONTENT_STRING:
				fprintf(stream,	"\"%.*s\"",
						(صحیح) lwc_string_length(
						content->data.string),
						lwc_string_data(
						content->data.string));
				بشکن;
			حالت CSS_COMPUTED_CONTENT_URI:
				fprintf(stream,	"uri(\"%.*s\")",
						(صحیح) lwc_string_length(
						content->data.uri),
						lwc_string_data(
						content->data.uri));
				بشکن;
			حالت CSS_COMPUTED_CONTENT_COUNTER:
				fprintf(stream, "counter(%.*s)",
						(صحیح) lwc_string_length(
						content->data.counter.name),
						lwc_string_data(
						content->data.counter.name));
				بشکن;
			حالت CSS_COMPUTED_CONTENT_COUNTERS:
				fprintf(stream, "counters(%.*s, \"%.*s\")",
						(صحیح) lwc_string_length(
						content->data.counters.name),
						lwc_string_data(
						content->data.counters.name),
						(صحیح) lwc_string_length(
						content->data.counters.sep),
						lwc_string_data(
						content->data.counters.sep));
				بشکن;
			حالت CSS_COMPUTED_CONTENT_ATTR:
				fprintf(stream, "attr(%.*s)",
						(صحیح) lwc_string_length(
						content->data.attr),
						lwc_string_data(
						content->data.attr));
				بشکن;
			حالت CSS_COMPUTED_CONTENT_OPEN_QUOTE:
				fprintf(stream, "open-quote");
				بشکن;
			حالت CSS_COMPUTED_CONTENT_CLOSE_QUOTE:
				fprintf(stream, "close-quote");
				بشکن;
			حالت CSS_COMPUTED_CONTENT_NO_OPEN_QUOTE:
				fprintf(stream, "no-open-quote");
				بشکن;
			حالت CSS_COMPUTED_CONTENT_NO_CLOSE_QUOTE:
				fprintf(stream, "no-close-quote");
				بشکن;
			}

			content++;
		}

		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* counter-increment */
	val = css_computed_counter_increment(style, &counter);
	اگر ((val == CSS_COUNTER_INCREMENT_NONE) || (counter == پوچ)) {
		fprintf(stream, "counter-increment: none ");
	} وگرنه {
		fprintf(stream, "counter-increment:");

		وقتی (counter->name != پوچ) {
			fprintf(stream, " %.*s ",
					(صحیح) lwc_string_length(counter->name),
					lwc_string_data(counter->name));

			dump_css_fixed(stream, counter->مقدار);

			counter++;
		}

		fprintf(stream, " ");
	}

	/* counter-reset */
	val = css_computed_counter_reset(style, &counter);
	اگر ((val == CSS_COUNTER_RESET_NONE) || (counter == پوچ)) {
		fprintf(stream, "counter-reset: none ");
	} وگرنه {
		fprintf(stream, "counter-reset:");

		وقتی (counter->name != پوچ) {
			fprintf(stream, " %.*s ",
					(صحیح) lwc_string_length(counter->name),
					lwc_string_data(counter->name));

			dump_css_fixed(stream, counter->مقدار);

			counter++;
		}

		fprintf(stream, " ");
	}

	/* cursor */
	val = css_computed_cursor(style, &string_list);
	fprintf(stream, "cursor:");

	اگر (string_list != پوچ) {
		وقتی (*string_list != پوچ) {
			fprintf(stream, " url\"%.*s\")",
					(صحیح) lwc_string_length(*string_list),
					lwc_string_data(*string_list));

			string_list++;
		}
	}
	انتخاب (val) {
	حالت CSS_CURSOR_AUTO:
		fprintf(stream, " auto ");
		بشکن;
	حالت CSS_CURSOR_CROSSHAIR:
		fprintf(stream, " crosshair ");
		بشکن;
	حالت CSS_CURSOR_DEFAULT:
		fprintf(stream, " default ");
		بشکن;
	حالت CSS_CURSOR_POINTER:
		fprintf(stream, " pointer ");
		بشکن;
	حالت CSS_CURSOR_MOVE:
		fprintf(stream, " move ");
		بشکن;
	حالت CSS_CURSOR_E_RESIZE:
		fprintf(stream, " e-resize ");
		بشکن;
	حالت CSS_CURSOR_NE_RESIZE:
		fprintf(stream, " ne-resize ");
		بشکن;
	حالت CSS_CURSOR_NW_RESIZE:
		fprintf(stream, " nw-resize ");
		بشکن;
	حالت CSS_CURSOR_N_RESIZE:
		fprintf(stream, " n-resize ");
		بشکن;
	حالت CSS_CURSOR_SE_RESIZE:
		fprintf(stream, " se-resize ");
		بشکن;
	حالت CSS_CURSOR_SW_RESIZE:
		fprintf(stream, " sw-resize ");
		بشکن;
	حالت CSS_CURSOR_S_RESIZE:
		fprintf(stream, " s-resize ");
		بشکن;
	حالت CSS_CURSOR_W_RESIZE:
		fprintf(stream, " w-resize ");
		بشکن;
	حالت CSS_CURSOR_TEXT:
		fprintf(stream, " text ");
		بشکن;
	حالت CSS_CURSOR_WAIT:
		fprintf(stream, " wait ");
		بشکن;
	حالت CSS_CURSOR_HELP:
		fprintf(stream, " help ");
		بشکن;
	حالت CSS_CURSOR_PROGRESS:
		fprintf(stream, " progress ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* direction */
	val = css_computed_direction(style);
	انتخاب (val) {
	حالت CSS_DIRECTION_LTR:
		fprintf(stream, "direction: ltr ");
		بشکن;
	حالت CSS_DIRECTION_RTL:
		fprintf(stream, "direction: rtl ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* display */
	val = ns_computed_display_static(style);
	انتخاب (val) {
	حالت CSS_DISPLAY_INLINE:
		fprintf(stream, "display: inline ");
		بشکن;
	حالت CSS_DISPLAY_BLOCK:
		fprintf(stream, "display: block ");
		بشکن;
	حالت CSS_DISPLAY_LIST_ITEM:
		fprintf(stream, "display: list-item ");
		بشکن;
	حالت CSS_DISPLAY_RUN_IN:
		fprintf(stream, "display: run-in ");
		بشکن;
	حالت CSS_DISPLAY_INLINE_BLOCK:
		fprintf(stream, "display: inline-block ");
		بشکن;
	حالت CSS_DISPLAY_TABLE:
		fprintf(stream, "display: table ");
		بشکن;
	حالت CSS_DISPLAY_INLINE_TABLE:
		fprintf(stream, "display: inline-table ");
		بشکن;
	حالت CSS_DISPLAY_TABLE_ROW_GROUP:
		fprintf(stream, "display: table-row-group ");
		بشکن;
	حالت CSS_DISPLAY_TABLE_HEADER_GROUP:
		fprintf(stream, "display: table-header-group ");
		بشکن;
	حالت CSS_DISPLAY_TABLE_FOOTER_GROUP:
		fprintf(stream, "display: table-footer-group ");
		بشکن;
	حالت CSS_DISPLAY_TABLE_ROW:
		fprintf(stream, "display: table-row ");
		بشکن;
	حالت CSS_DISPLAY_TABLE_COLUMN_GROUP:
		fprintf(stream, "display: table-column-group ");
		بشکن;
	حالت CSS_DISPLAY_TABLE_COLUMN:
		fprintf(stream, "display: table-column ");
		بشکن;
	حالت CSS_DISPLAY_TABLE_CELL:
		fprintf(stream, "display: table-cell ");
		بشکن;
	حالت CSS_DISPLAY_TABLE_CAPTION:
		fprintf(stream, "display: table-caption ");
		بشکن;
	حالت CSS_DISPLAY_NONE:
		fprintf(stream, "display: none ");
		بشکن;
	حالت CSS_DISPLAY_FLEX:
		fprintf(stream, "display: flex ");
		بشکن;
	حالت CSS_DISPLAY_INLINE_FLEX:
		fprintf(stream, "display: inline-flex ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* empty-cells */
	val = css_computed_empty_cells(style);
	انتخاب (val) {
	حالت CSS_EMPTY_CELLS_SHOW:
		fprintf(stream, "empty-cells: show ");
		بشکن;
	حالت CSS_EMPTY_CELLS_HIDE:
		fprintf(stream, "empty-cells: hide ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* float */
	val = css_computed_float(style);
	انتخاب (val) {
	حالت CSS_FLOAT_چپ:
		fprintf(stream, "float: left ");
		بشکن;
	حالت CSS_FLOAT_راست:
		fprintf(stream, "float: right ");
		بشکن;
	حالت CSS_FLOAT_NONE:
		fprintf(stream, "float: none ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* font-family */
	val = css_computed_font_family(style, &string_list);
	اگر (val != CSS_FONT_FAMILY_INHERIT) {
		fprintf(stream, "font-family:");

		اگر (string_list != پوچ) {
			وقتی (*string_list != پوچ) {
				fprintf(stream, " \"%.*s\"",
					(صحیح) lwc_string_length(*string_list),
					lwc_string_data(*string_list));

				string_list++;
			}
		}
		انتخاب (val) {
		حالت CSS_FONT_FAMILY_SERIF:
			fprintf(stream, " serif ");
			بشکن;
		حالت CSS_FONT_FAMILY_SANS_SERIF:
			fprintf(stream, " sans-serif ");
			بشکن;
		حالت CSS_FONT_FAMILY_CURSIVE:
			fprintf(stream, " cursive ");
			بشکن;
		حالت CSS_FONT_FAMILY_FANTASY:
			fprintf(stream, " fantasy ");
			بشکن;
		حالت CSS_FONT_FAMILY_MONOSPACE:
			fprintf(stream, " monospace ");
			بشکن;
		}
	}

	/* font-size */
	val = css_computed_font_size(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_FONT_SIZE_XX_SMALL:
		fprintf(stream, "font-size: xx-small ");
		بشکن;
	حالت CSS_FONT_SIZE_X_SMALL:
		fprintf(stream, "font-size: x-small ");
		بشکن;
	حالت CSS_FONT_SIZE_SMALL:
		fprintf(stream, "font-size: small ");
		بشکن;
	حالت CSS_FONT_SIZE_MEDIUM:
		fprintf(stream, "font-size: medium ");
		بشکن;
	حالت CSS_FONT_SIZE_LARGE:
		fprintf(stream, "font-size: large ");
		بشکن;
	حالت CSS_FONT_SIZE_X_LARGE:
		fprintf(stream, "font-size: x-large ");
		بشکن;
	حالت CSS_FONT_SIZE_XX_LARGE:
		fprintf(stream, "font-size: xx-large ");
		بشکن;
	حالت CSS_FONT_SIZE_LARGER:
		fprintf(stream, "font-size: larger ");
		بشکن;
	حالت CSS_FONT_SIZE_SMALLER:
		fprintf(stream, "font-size: smaller ");
		بشکن;
	حالت CSS_FONT_SIZE_DIMENSION:
		fprintf(stream, "font-size: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* font-style */
	val = css_computed_font_style(style);
	انتخاب (val) {
	حالت CSS_FONT_STYLE_NORMAL:
		fprintf(stream, "font-style: normal ");
		بشکن;
	حالت CSS_FONT_STYLE_ITALIC:
		fprintf(stream, "font-style: italic ");
		بشکن;
	حالت CSS_FONT_STYLE_OBLIQUE:
		fprintf(stream, "font-style: oblique ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* font-variant */
	val = css_computed_font_variant(style);
	انتخاب (val) {
	حالت CSS_FONT_VARIANT_NORMAL:
		fprintf(stream, "font-variant: normal ");
		بشکن;
	حالت CSS_FONT_VARIANT_SMALL_CAPS:
		fprintf(stream, "font-variant: small-caps ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* font-weight */
	val = css_computed_font_weight(style);
	انتخاب (val) {
	حالت CSS_FONT_WEIGHT_NORMAL:
		fprintf(stream, "font-weight: normal ");
		بشکن;
	حالت CSS_FONT_WEIGHT_BOLD:
		fprintf(stream, "font-weight: bold ");
		بشکن;
	حالت CSS_FONT_WEIGHT_BOLDER:
		fprintf(stream, "font-weight: bolder ");
		بشکن;
	حالت CSS_FONT_WEIGHT_LIGHTER:
		fprintf(stream, "font-weight: lighter ");
		بشکن;
	حالت CSS_FONT_WEIGHT_100:
		fprintf(stream, "font-weight: 100 ");
		بشکن;
	حالت CSS_FONT_WEIGHT_200:
		fprintf(stream, "font-weight: 200 ");
		بشکن;
	حالت CSS_FONT_WEIGHT_300:
		fprintf(stream, "font-weight: 300 ");
		بشکن;
	حالت CSS_FONT_WEIGHT_400:
		fprintf(stream, "font-weight: 400 ");
		بشکن;
	حالت CSS_FONT_WEIGHT_500:
		fprintf(stream, "font-weight: 500 ");
		بشکن;
	حالت CSS_FONT_WEIGHT_600:
		fprintf(stream, "font-weight: 600 ");
		بشکن;
	حالت CSS_FONT_WEIGHT_700:
		fprintf(stream, "font-weight: 700 ");
		بشکن;
	حالت CSS_FONT_WEIGHT_800:
		fprintf(stream, "font-weight: 800 ");
		بشکن;
	حالت CSS_FONT_WEIGHT_900:
		fprintf(stream, "font-weight: 900 ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* height */
	val = css_computed_height(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_HEIGHT_AUTO:
		fprintf(stream, "height: auto ");
		بشکن;
	حالت CSS_HEIGHT_SET:
		fprintf(stream, "height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/*چپ*/
	val = css_computed_چپ(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_چپ_AUTO:
		fprintf(stream, "left: auto ");
		بشکن;
	حالت CSS_چپ_SET:
		fprintf(stream, "left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* letter-spacing */
	val = css_computed_letter_spacing(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_LETTER_SPACING_NORMAL:
		fprintf(stream, "letter-spacing: normal ");
		بشکن;
	حالت CSS_LETTER_SPACING_SET:
		fprintf(stream, "letter-spacing: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* line-height */
	val = css_computed_سطر_height(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_سطر_HEIGHT_NORMAL:
		fprintf(stream, "line-height: normal ");
		بشکن;
	حالت CSS_سطر_HEIGHT_عدد:
		fprintf(stream, "line-height: ");

		dump_css_fixed(stream, len1);

		fprintf(stream, " ");
		بشکن;
	حالت CSS_سطر_HEIGHT_DIMENSION:
		fprintf(stream, "line-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* list-style-image */
	val = css_computed_list_style_image(style, &url);
	اگر (url != پوچ) {
		fprintf(stream, "list-style-image: url('%.*s') ",
				(صحیح) lwc_string_length(url),
				lwc_string_data(url));
	} وگرنه اگر (val == CSS_LIST_STYLE_IMAGE_NONE) {
		fprintf(stream, "list-style-image: none ");
	}

	/* list-style-position */
	val = css_computed_list_style_position(style);
	انتخاب (val) {
	حالت CSS_LIST_STYLE_POSITION_INSIDE:
		fprintf(stream, "list-style-position: inside ");
		بشکن;
	حالت CSS_LIST_STYLE_POSITION_OUTSIDE:
		fprintf(stream, "list-style-position: outside ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* list-style-type */
	val = css_computed_list_style_type(style);
	انتخاب (val) {
	حالت CSS_LIST_STYLE_TYPE_DISC:
		fprintf(stream, "list-style-type: disc ");
		بشکن;
	حالت CSS_LIST_STYLE_TYPE_CIRCLE:
		fprintf(stream, "list-style-type: circle ");
		بشکن;
	حالت CSS_LIST_STYLE_TYPE_SQUARE:
		fprintf(stream, "list-style-type: square ");
		بشکن;
	حالت CSS_LIST_STYLE_TYPE_DECIMAL:
		fprintf(stream, "list-style-type: decimal ");
		بشکن;
	حالت CSS_LIST_STYLE_TYPE_DECIMAL_LEADING_ZERO:
		fprintf(stream, "list-style-type: decimal-leading-zero ");
		بشکن;
	حالت CSS_LIST_STYLE_TYPE_LOWER_ROMAN:
		fprintf(stream, "list-style-type: lower-roman ");
		بشکن;
	حالت CSS_LIST_STYLE_TYPE_UPPER_ROMAN:
		fprintf(stream, "list-style-type: upper-roman ");
		بشکن;
	حالت CSS_LIST_STYLE_TYPE_LOWER_GREEK:
		fprintf(stream, "list-style-type: lower-greek ");
		بشکن;
	حالت CSS_LIST_STYLE_TYPE_LOWER_LATIN:
		fprintf(stream, "list-style-type: lower-latin ");
		بشکن;
	حالت CSS_LIST_STYLE_TYPE_UPPER_LATIN:
		fprintf(stream, "list-style-type: upper-latin ");
		بشکن;
	حالت CSS_LIST_STYLE_TYPE_ARMENIAN:
		fprintf(stream, "list-style-type: armenian ");
		بشکن;
	حالت CSS_LIST_STYLE_TYPE_GEORGIAN:
		fprintf(stream, "list-style-type: georgian ");
		بشکن;
	حالت CSS_LIST_STYLE_TYPE_LOWER_ALPHA:
		fprintf(stream, "list-style-type: lower-alpha ");
		بشکن;
	حالت CSS_LIST_STYLE_TYPE_UPPER_ALPHA:
		fprintf(stream, "list-style-type: upper-alpha ");
		بشکن;
	حالت CSS_LIST_STYLE_TYPE_NONE:
		fprintf(stream, "list-style-type: none ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* margin-top */
	val = css_computed_margin_بالاترین(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_MARGIN_AUTO:
		fprintf(stream, "margin-top: auto ");
		بشکن;
	حالت CSS_MARGIN_SET:
		fprintf(stream, "margin-top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* margin-right */
	val = css_computed_margin_راست(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_MARGIN_AUTO:
		fprintf(stream, "margin-right: auto ");
		بشکن;
	حالت CSS_MARGIN_SET:
		fprintf(stream, "margin-right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* margin-bottom */
	val = css_computed_margin_bottom(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_MARGIN_AUTO:
		fprintf(stream, "margin-bottom: auto ");
		بشکن;
	حالت CSS_MARGIN_SET:
		fprintf(stream, "margin-bottom: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* margin-left */
	val = css_computed_margin_چپ(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_MARGIN_AUTO:
		fprintf(stream, "margin-left: auto ");
		بشکن;
	حالت CSS_MARGIN_SET:
		fprintf(stream, "margin-left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* max-height */
	val = css_computed_max_height(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_MAX_HEIGHT_NONE:
		fprintf(stream, "max-height: none ");
		بشکن;
	حالت CSS_MAX_HEIGHT_SET:
		fprintf(stream, "max-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* max-width */
	val = css_computed_max_width(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_MAX_WIDTH_NONE:
		fprintf(stream, "max-width: none ");
		بشکن;
	حالت CSS_MAX_WIDTH_SET:
		fprintf(stream, "max-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* min-height */
	val = ns_computed_min_height(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_MIN_HEIGHT_SET:
		fprintf(stream, "min-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* min-width */
	val = ns_computed_min_width(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_MIN_WIDTH_SET:
		fprintf(stream, "min-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* opacity */
	val = css_computed_opacity(style, &len1);
	انتخاب (val) {
	حالت CSS_OPACITY_SET:
		fprintf(stream, "opacity: ");

		dump_css_fixed(stream, len1);

		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* outline-color */
	val = css_computed_outline_color(style, &color);
	انتخاب (val) {
	حالت CSS_OUTLINE_COLOR_INVERT:
		fprintf(stream, "outline-color: invert ");
		بشکن;
	حالت CSS_OUTLINE_COLOR_COLOR:
		fprintf(stream, "outline-color: #%08"PRIx32" ", color);
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* outline-style */
	val = css_computed_outline_style(style);
	انتخاب (val) {
	حالت CSS_OUTLINE_STYLE_NONE:
		fprintf(stream, "outline-style: none ");
		بشکن;
	حالت CSS_OUTLINE_STYLE_DOTTED:
		fprintf(stream, "outline-style: dotted ");
		بشکن;
	حالت CSS_OUTLINE_STYLE_DASHED:
		fprintf(stream, "outline-style: dashed ");
		بشکن;
	حالت CSS_OUTLINE_STYLE_SOLID:
		fprintf(stream, "outline-style: solid ");
		بشکن;
	حالت CSS_OUTLINE_STYLE_DOUBLE:
		fprintf(stream, "outline-style: double ");
		بشکن;
	حالت CSS_OUTLINE_STYLE_GROOVE:
		fprintf(stream, "outline-style: groove ");
		بشکن;
	حالت CSS_OUTLINE_STYLE_RIDGE:
		fprintf(stream, "outline-style: ridge ");
		بشکن;
	حالت CSS_OUTLINE_STYLE_INSET:
		fprintf(stream, "outline-style: inset ");
		بشکن;
	حالت CSS_OUTLINE_STYLE_OUTSET:
		fprintf(stream, "outline-style: outset ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* outline-width */
	val = css_computed_outline_width(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_OUTLINE_WIDTH_THIN:
		fprintf(stream, "outline-width: thin ");
		بشکن;
	حالت CSS_OUTLINE_WIDTH_MEDIUM:
		fprintf(stream, "outline-width: medium ");
		بشکن;
	حالت CSS_OUTLINE_WIDTH_THICK:
		fprintf(stream, "outline-width: thick ");
		بشکن;
	حالت CSS_OUTLINE_WIDTH_WIDTH:
		fprintf(stream, "outline-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* overflow */
	val = css_computed_overflow_x(style);
	انتخاب (val) {
	حالت CSS_OVERFLOW_VISIBLE:
		fprintf(stream, "overflow-x: visible ");
		بشکن;
	حالت CSS_OVERFLOW_HIDDEN:
		fprintf(stream, "overflow-x: hidden ");
		بشکن;
	حالت CSS_OVERFLOW_SCROLL:
		fprintf(stream, "overflow-x: scroll ");
		بشکن;
	حالت CSS_OVERFLOW_AUTO:
		fprintf(stream, "overflow-x auto ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* overflow */
	val = css_computed_overflow_y(style);
	انتخاب (val) {
	حالت CSS_OVERFLOW_VISIBLE:
		fprintf(stream, "overflow-y: visible ");
		بشکن;
	حالت CSS_OVERFLOW_HIDDEN:
		fprintf(stream, "overflow-y: hidden ");
		بشکن;
	حالت CSS_OVERFLOW_SCROLL:
		fprintf(stream, "overflow-y: scroll ");
		بشکن;
	حالت CSS_OVERFLOW_AUTO:
		fprintf(stream, "overflow-y: auto ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* padding-top */
	val = css_computed_padding_بالاترین(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_PADDING_SET:
		fprintf(stream, "padding-top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* padding-right */
	val = css_computed_padding_راست(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_PADDING_SET:
		fprintf(stream, "padding-right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* padding-bottom */
	val = css_computed_padding_bottom(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_PADDING_SET:
		fprintf(stream, "padding-bottom: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* padding-left */
	val = css_computed_padding_چپ(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_PADDING_SET:
		fprintf(stream, "padding-left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* position */
	val = css_computed_position(style);
	انتخاب (val) {
	حالت CSS_POSITION_STATIC:
		fprintf(stream, "position: static ");
		بشکن;
	حالت CSS_POSITION_RELATIVE:
		fprintf(stream, "position: relative ");
		بشکن;
	حالت CSS_POSITION_ABSOLUTE:
		fprintf(stream, "position: absolute ");
		بشکن;
	حالت CSS_POSITION_FIXED:
		fprintf(stream, "position: fixed ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* quotes */
	val = css_computed_quotes(style, &string_list);
	اگر (val == CSS_QUOTES_STRING && string_list != پوچ) {
		fprintf(stream, "quotes:");

		وقتی (*string_list != پوچ) {
			fprintf(stream, " \"%.*s\"",
				(صحیح) lwc_string_length(*string_list),
				lwc_string_data(*string_list));

			string_list++;
		}

		fprintf(stream, " ");
	} وگرنه {
		انتخاب (val) {
		حالت CSS_QUOTES_NONE:
			fprintf(stream, "quotes: none ");
			بشکن;
		پیشفرض:
			بشکن;
		}
	}

	/*راست*/
	val = css_computed_راست(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_راست_AUTO:
		fprintf(stream, "right: auto ");
		بشکن;
	حالت CSS_راست_SET:
		fprintf(stream, "right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* table-layout */
	val = css_computed_table_layout(style);
	انتخاب (val) {
	حالت CSS_TABLE_LAYOUT_AUTO:
		fprintf(stream, "table-layout: auto ");
		بشکن;
	حالت CSS_TABLE_LAYOUT_FIXED:
		fprintf(stream, "table-layout: fixed ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* text-align */
	val = css_computed_text_align(style);
	انتخاب (val) {
	حالت CSS_TEXT_ALIGN_چپ:
		fprintf(stream, "text-align: left ");
		بشکن;
	حالت CSS_TEXT_ALIGN_راست:
		fprintf(stream, "text-align: right ");
		بشکن;
	حالت CSS_TEXT_ALIGN_CENTER:
		fprintf(stream, "text-align: center ");
		بشکن;
	حالت CSS_TEXT_ALIGN_JUSTIFY:
		fprintf(stream, "text-align: justify ");
		بشکن;
	حالت CSS_TEXT_ALIGN_DEFAULT:
		fprintf(stream, "text-align: default ");
		بشکن;
	حالت CSS_TEXT_ALIGN_LIBCSS_چپ:
		fprintf(stream, "text-align: -libcss-left ");
		بشکن;
	حالت CSS_TEXT_ALIGN_LIBCSS_CENTER:
		fprintf(stream, "text-align: -libcss-center ");
		بشکن;
	حالت CSS_TEXT_ALIGN_LIBCSS_راست:
		fprintf(stream, "text-align: -libcss-right ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* text-decoration */
	val = css_computed_text_decoration(style);
	اگر (val == CSS_TEXT_DECORATION_NONE) {
		fprintf(stream, "text-decoration: none ");
	} وگرنه {
		fprintf(stream, "text-decoration:");

		اگر (val & CSS_TEXT_DECORATION_BLINK) {
			fprintf(stream, " blink");
		}
		اگر (val & CSS_TEXT_DECORATION_سطر_THROUGH) {
			fprintf(stream, " line-through");
		}
		اگر (val & CSS_TEXT_DECORATION_OVERLINE) {
			fprintf(stream, " overline");
		}
		اگر (val & CSS_TEXT_DECORATION_UNDERLINE) {
			fprintf(stream, " underline");
		}

		fprintf(stream, " ");
	}

	/* text-indent */
	val = css_computed_text_indent(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_TEXT_INDENT_SET:
		fprintf(stream, "text-indent: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* text-transform */
	val = css_computed_text_transform(style);
	انتخاب (val) {
	حالت CSS_TEXT_TRANSFORM_CAPITALIZE:
		fprintf(stream, "text-transform: capitalize ");
		بشکن;
	حالت CSS_TEXT_TRANSFORM_UPPERCASE:
		fprintf(stream, "text-transform: uppercase ");
		بشکن;
	حالت CSS_TEXT_TRANSFORM_LOWERCASE:
		fprintf(stream, "text-transform: lowercase ");
		بشکن;
	حالت CSS_TEXT_TRANSFORM_NONE:
		fprintf(stream, "text-transform: none ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/*بالاترین*/
	val = css_computed_بالاترین(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_بالاترین_AUTO:
		fprintf(stream, "top: auto ");
		بشکن;
	حالت CSS_بالاترین_SET:
		fprintf(stream, "top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* unicode-bidi */
	val = css_computed_unicode_bidi(style);
	انتخاب (val) {
	حالت CSS_UNICODE_BIDI_NORMAL:
		fprintf(stream, "unicode-bidi: normal ");
		بشکن;
	حالت CSS_UNICODE_BIDI_EMBED:
		fprintf(stream, "unicode-bidi: embed ");
		بشکن;
	حالت CSS_UNICODE_BIDI_BIDI_OVERRIDE:
		fprintf(stream, "unicode-bidi: bidi-override ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* vertical-align */
	val = css_computed_vertical_align(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_VERTICAL_ALIGN_BASELINE:
		fprintf(stream, "vertical-align: baseline ");
		بشکن;
	حالت CSS_VERTICAL_ALIGN_SUB:
		fprintf(stream, "vertical-align: sub ");
		بشکن;
	حالت CSS_VERTICAL_ALIGN_SUPER:
		fprintf(stream, "vertical-align: super ");
		بشکن;
	حالت CSS_VERTICAL_ALIGN_بالاترین:
		fprintf(stream, "vertical-align: top ");
		بشکن;
	حالت CSS_VERTICAL_ALIGN_TEXT_بالاترین:
		fprintf(stream, "vertical-align: text-top ");
		بشکن;
	حالت CSS_VERTICAL_ALIGN_میانه:
		fprintf(stream, "vertical-align: middle ");
		بشکن;
	حالت CSS_VERTICAL_ALIGN_BOTTOM:
		fprintf(stream, "vertical-align: bottom ");
		بشکن;
	حالت CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
		fprintf(stream, "vertical-align: text-bottom ");
		بشکن;
	حالت CSS_VERTICAL_ALIGN_SET:
		fprintf(stream, "vertical-align: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* visibility */
	val = css_computed_visibility(style);
	انتخاب (val) {
	حالت CSS_VISIBILITY_VISIBLE:
		fprintf(stream, "visibility: visible ");
		بشکن;
	حالت CSS_VISIBILITY_HIDDEN:
		fprintf(stream, "visibility: hidden ");
		بشکن;
	حالت CSS_VISIBILITY_COLLAPSE:
		fprintf(stream, "visibility: collapse ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* white-space */
	val = css_computed_white_space(style);
	انتخاب (val) {
	حالت CSS_WHITE_SPACE_NORMAL:
		fprintf(stream, "white-space: normal ");
		بشکن;
	حالت CSS_WHITE_SPACE_PRE:
		fprintf(stream, "white-space: pre ");
		بشکن;
	حالت CSS_WHITE_SPACE_NOWRAP:
		fprintf(stream, "white-space: nowrap ");
		بشکن;
	حالت CSS_WHITE_SPACE_PRE_WRAP:
		fprintf(stream, "white-space: pre-wrap ");
		بشکن;
	حالت CSS_WHITE_SPACE_PRE_سطر:
		fprintf(stream, "white-space: pre-line ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* width */
	val = css_computed_width(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_WIDTH_AUTO:
		fprintf(stream, "width: auto ");
		بشکن;
	حالت CSS_WIDTH_SET:
		fprintf(stream, "width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* word-spacing */
	val = css_computed_word_spacing(style, &len1, &unit1);
	انتخاب (val) {
	حالت CSS_WORD_SPACING_NORMAL:
		fprintf(stream, "word-spacing: normal ");
		بشکن;
	حالت CSS_WORD_SPACING_SET:
		fprintf(stream, "word-spacing: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* z-index */
	val = css_computed_z_index(style, &zindex);
	انتخاب (val) {
	حالت CSS_Z_INDEX_AUTO:
		fprintf(stream, "z-index: auto ");
		بشکن;
	حالت CSS_Z_INDEX_SET:
		fprintf(stream, "z-index: %"PRId32" ", zindex);
		بشکن;
	پیشفرض:
		بشکن;
	}

	fprintf(stream, "}");
}
