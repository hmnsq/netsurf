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

#अन्तर्भाव <stdio.h>
#अन्तर्भाव <libcss/libcss.h>

#अन्तर्भाव "netsurf/inttypes.h"

#अन्तर्भाव "css/dump.h"
#अन्तर्भाव "css/utils.h"

/**
 * Dump a fixed point value to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param f       Value to write
 */
स्थिर शून्य dump_css_fixed(FILE *stream, css_fixed f)
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
स्थिर शून्य dump_css_संख्या(FILE *stream, css_fixed val)
{
	यदि (INTTOFIX(FIXTOINT(val)) == val)
		fprintf(stream, "%"PRId32, FIXTOINT(val));
	अन्यथा
		dump_css_fixed(stream, val);
}

/**
 * Dump a dimension to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 * \param unit    Unit to write
 */
स्थिर शून्य dump_css_unit(FILE *stream, css_fixed val, css_unit unit)
{
	dump_css_संख्या(stream, val);

	विकल्प (unit) {
	पक्ष CSS_UNIT_PX:
		fprintf(stream, "px");
		भङ्ग;
	पक्ष CSS_UNIT_EX:
		fprintf(stream, "ex");
		भङ्ग;
	पक्ष CSS_UNIT_EM:
		fprintf(stream, "em");
		भङ्ग;
	पक्ष CSS_UNIT_IN:
		fprintf(stream, "in");
		भङ्ग;
	पक्ष CSS_UNIT_CM:
		fprintf(stream, "cm");
		भङ्ग;
	पक्ष CSS_UNIT_MM:
		fprintf(stream, "mm");
		भङ्ग;
	पक्ष CSS_UNIT_PT:
		fprintf(stream, "pt");
		भङ्ग;
	पक्ष CSS_UNIT_PC:
		fprintf(stream, "pc");
		भङ्ग;
	पक्ष CSS_UNIT_PCT:
		fprintf(stream, "%%");
		भङ्ग;
	पक्ष CSS_UNIT_DEG:
		fprintf(stream, "deg");
		भङ्ग;
	पक्ष CSS_UNIT_GRAD:
		fprintf(stream, "grad");
		भङ्ग;
	पक्ष CSS_UNIT_RAD:
		fprintf(stream, "rad");
		भङ्ग;
	पक्ष CSS_UNIT_MS:
		fprintf(stream, "ms");
		भङ्ग;
	पक्ष CSS_UNIT_S:
		fprintf(stream, "s");
		भङ्ग;
	पक्ष CSS_UNIT_HZ:
		fprintf(stream, "Hz");
		भङ्ग;
	पक्ष CSS_UNIT_KHZ:
		fprintf(stream, "kHz");
		भङ्ग;
	पक्ष CSS_UNIT_CH:
		fprintf(stream, "ch");
		भङ्ग;
	पक्ष CSS_UNIT_REM:
		fprintf(stream, "rem");
		भङ्ग;
	पक्ष CSS_UNIT_LH:
		fprintf(stream, "lh");
		भङ्ग;
	पक्ष CSS_UNIT_VH:
		fprintf(stream, "vh");
		भङ्ग;
	पक्ष CSS_UNIT_VW:
		fprintf(stream, "vw");
		भङ्ग;
	पक्ष CSS_UNIT_VI:
		fprintf(stream, "vi");
		भङ्ग;
	पक्ष CSS_UNIT_VB:
		fprintf(stream, "vb");
		भङ्ग;
	पक्ष CSS_UNIT_VMIN:
		fprintf(stream, "vmin");
		भङ्ग;
	पक्ष CSS_UNIT_VMAX:
		fprintf(stream, "vmax");
		भङ्ग;
	पक्ष CSS_UNIT_Q:
		fprintf(stream, "q");
		भङ्ग;
	पक्ष CSS_UNIT_CALC:
		fprintf(stream, "calc()");
		भङ्ग;
	}
}

/* exported interface documented in content/handlers/css/dump.h */
शून्य nscss_dump_computed_style(FILE *stream, ध्रुव css_computed_style *style)
{
	uint8_t val;
	css_color color = 0;
	lwc_string *url = रिक्त;
	css_fixed len1 = 0, len2 = 0;
	css_unit unit1 = CSS_UNIT_PX, unit2 = CSS_UNIT_PX;
	css_computed_clip_rect rect = { 0, 0, 0, 0, CSS_UNIT_PX, CSS_UNIT_PX,
					CSS_UNIT_PX, CSS_UNIT_PX, true, true,
					true, true };
	ध्रुव css_computed_content_item *content = रिक्त;
	ध्रुव css_computed_counter *counter = रिक्त;
	lwc_string **string_list = रिक्त;
	int32_t zindex = 0;

	fprintf(stream, "{ ");

	/* background-attachment */
	val = css_computed_background_attachment(style);
	विकल्प (val) {
	पक्ष CSS_BACKGROUND_ATTACHMENT_FIXED:
		fprintf(stream, "background-attachment: fixed ");
		भङ्ग;
	पक्ष CSS_BACKGROUND_ATTACHMENT_SCROLL:
		fprintf(stream, "background-attachment: scroll ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* background-color */
	val = css_computed_background_color(style, &color);
	विकल्प (val) {
	पक्ष CSS_BACKGROUND_COLOR_COLOR:
		fprintf(stream, "background-color: #%08"PRIx32" ", color);
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* background-image */
	val = css_computed_background_image(style, &url);
	यदि (val == CSS_BACKGROUND_IMAGE_IMAGE && url != रिक्त) {
		fprintf(stream, "background-image: url('%.*s') ",
				(पूर्णाङ्क) lwc_string_length(url),
				lwc_string_data(url));
	} अन्यथा यदि (val == CSS_BACKGROUND_IMAGE_NONE) {
		fprintf(stream, "background-image: none ");
	}

	/* background-position */
	val = css_computed_background_position(style, &len1, &unit1,
			&len2, &unit2);
	यदि (val == CSS_BACKGROUND_POSITION_SET) {
		fprintf(stream, "background-position: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		dump_css_unit(stream, len2, unit2);
		fprintf(stream, " ");
	}

	/* background-repeat */
	val = css_computed_background_repeat(style);
	विकल्प (val) {
	पक्ष CSS_BACKGROUND_REPEAT_REPEAT_X:
		fprintf(stream, "background-repeat: repeat-x ");
		भङ्ग;
	पक्ष CSS_BACKGROUND_REPEAT_REPEAT_Y:
		fprintf(stream, "background-repeat: repeat-y ");
		भङ्ग;
	पक्ष CSS_BACKGROUND_REPEAT_REPEAT:
		fprintf(stream, "background-repeat: repeat ");
		भङ्ग;
	पक्ष CSS_BACKGROUND_REPEAT_NO_REPEAT:
		fprintf(stream, "background-repeat: no-repeat ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* border-collapse */
	val = css_computed_border_collapse(style);
	विकल्प (val) {
	पक्ष CSS_BORDER_COLLAPSE_SEPARATE:
		fprintf(stream, "border-collapse: separate ");
		भङ्ग;
	पक्ष CSS_BORDER_COLLAPSE_COLLAPSE:
		fprintf(stream, "border-collapse: collapse ");
		भङ्ग;
	स्वभाव:

		भङ्ग;
	}

	/* border-spacing */
	val = css_computed_border_spacing(style, &len1, &unit1, &len2, &unit2);
	यदि (val == CSS_BORDER_SPACING_SET) {
		fprintf(stream, "border-spacing: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		dump_css_unit(stream, len2, unit2);
		fprintf(stream, " ");
	}

	/* border-top-color */
	val = css_computed_border_शीर्ष_color(style, &color);
	विकल्प (val) {
	पक्ष CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-top-color: #%08"PRIx32" ", color);
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* border-right-color */
	val = css_computed_border_उजवा_color(style, &color);
	विकल्प (val) {
	पक्ष CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-right-color: #%08"PRIx32" ", color);
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* border-bottom-color */
	val = css_computed_border_bottom_color(style, &color);
	विकल्प (val) {
	पक्ष CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-bottom-color: #%08"PRIx32" ", color);
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* border-left-color */
	val = css_computed_border_डावा_color(style, &color);
	विकल्प (val) {
	पक्ष CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-left-color: #%08"PRIx32" ", color);
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* border-top-style */
	val = css_computed_border_शीर्ष_style(style);
	विकल्प (val) {
	पक्ष CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-top-style: none ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-top-style: hidden ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-top-style: dotted ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-top-style: dashed ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-top-style: solid ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-top-style: double ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-top-style: groove ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-top-style: ridge ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-top-style: inset ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-top-style: outset ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* border-right-style */
	val = css_computed_border_उजवा_style(style);
	विकल्प (val) {
	पक्ष CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-right-style: none ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-right-style: hidden ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-right-style: dotted ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-right-style: dashed ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-right-style: solid ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-right-style: double ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-right-style: groove ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-right-style: ridge ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-right-style: inset ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-right-style: outset ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* border-bottom-style */
	val = css_computed_border_bottom_style(style);
	विकल्प (val) {
	पक्ष CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-bottom-style: none ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-bottom-style: hidden ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-bottom-style: dotted ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-bottom-style: dashed ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-bottom-style: solid ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-bottom-style: double ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-bottom-style: groove ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-bottom-style: ridge ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-bottom-style: inset ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-bottom-style: outset ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* border-left-style */
	val = css_computed_border_डावा_style(style);
	विकल्प (val) {
	पक्ष CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-left-style: none ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-left-style: hidden ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-left-style: dotted ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-left-style: dashed ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-left-style: solid ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-left-style: double ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-left-style: groove ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-left-style: ridge ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-left-style: inset ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-left-style: outset ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* border-top-width */
	val = css_computed_border_शीर्ष_width(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-top-width: thin ");
		भङ्ग;
	पक्ष CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-top-width: medium ");
		भङ्ग;
	पक्ष CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-top-width: thick ");
		भङ्ग;
	पक्ष CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-top-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* border-right-width */
	val = css_computed_border_उजवा_width(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-right-width: thin ");
		भङ्ग;
	पक्ष CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-right-width: medium ");
		भङ्ग;
	पक्ष CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-right-width: thick ");
		भङ्ग;
	पक्ष CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-right-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* border-bottom-width */
	val = css_computed_border_bottom_width(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-bottom-width: thin ");
		भङ्ग;
	पक्ष CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-bottom-width: medium ");
		भङ्ग;
	पक्ष CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-bottom-width: thick ");
		भङ्ग;
	पक्ष CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-bottom-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* border-left-width */
	val = css_computed_border_डावा_width(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-left-width: thin ");
		भङ्ग;
	पक्ष CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-left-width: medium ");
		भङ्ग;
	पक्ष CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-left-width: thick ");
		भङ्ग;
	पक्ष CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-left-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* bottom */
	val = css_computed_bottom(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_BOTTOM_AUTO:
		fprintf(stream, "bottom: auto ");
		भङ्ग;
	पक्ष CSS_BOTTOM_SET:
		fprintf(stream, "bottom: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* caption-side */
	val = css_computed_caption_side(style);
	विकल्प (val) {
	पक्ष CSS_CAPTION_SIDE_शीर्ष:
		fprintf(stream, "caption_side: top ");
		भङ्ग;
	पक्ष CSS_CAPTION_SIDE_BOTTOM:
		fprintf(stream, "caption_side: bottom ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* clear */
	val = css_computed_clear(style);
	विकल्प (val) {
	पक्ष CSS_CLEAR_NONE:
		fprintf(stream, "clear: none ");
		भङ्ग;
	पक्ष CSS_CLEAR_डावा:
		fprintf(stream, "clear: left ");
		भङ्ग;
	पक्ष CSS_CLEAR_उजवा:
		fprintf(stream, "clear: right ");
		भङ्ग;
	पक्ष CSS_CLEAR_BOTH:
		fprintf(stream, "clear: both ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* clip */
	val = css_computed_clip(style, &rect);
	विकल्प (val) {
	पक्ष CSS_CLIP_AUTO:
		fprintf(stream, "clip: auto ");
		भङ्ग;
	पक्ष CSS_CLIP_RECT:
		fprintf(stream, "clip: rect( ");

		यदि (rect.शीर्ष_auto)
			fprintf(stream, "auto");
		अन्यथा
			dump_css_unit(stream, rect.शीर्ष, rect.tunit);
		fprintf(stream, ", ");

		यदि (rect.उजवा_auto)
			fprintf(stream, "auto");
		अन्यथा
			dump_css_unit(stream, rect.उजवा, rect.runit);
		fprintf(stream, ", ");

		यदि (rect.bottom_auto)
			fprintf(stream, "auto");
		अन्यथा
			dump_css_unit(stream, rect.bottom, rect.bunit);
		fprintf(stream, ", ");

		यदि (rect.डावा_auto)
			fprintf(stream, "auto");
		अन्यथा
			dump_css_unit(stream, rect.डावा, rect.lunit);
		fprintf(stream, ") ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* color */
	val = css_computed_color(style, &color);
	यदि (val == CSS_COLOR_COLOR) {
		fprintf(stream, "color: #%08"PRIx32" ", color);
	}

	/* content */
	val = css_computed_content(style, &content);
	विकल्प (val) {
	पक्ष CSS_CONTENT_NONE:
		fprintf(stream, "content: none ");
		भङ्ग;
	पक्ष CSS_CONTENT_NORMAL:
		fprintf(stream, "content: normal ");
		भङ्ग;
	पक्ष CSS_CONTENT_SET:
		fprintf(stream, "content:");

		यावत् (content->type != CSS_COMPUTED_CONTENT_NONE) {
			fprintf(stream, " ");

			विकल्प (content->type) {
			पक्ष CSS_COMPUTED_CONTENT_STRING:
				fprintf(stream,	"\"%.*s\"",
						(पूर्णाङ्क) lwc_string_length(
						content->data.string),
						lwc_string_data(
						content->data.string));
				भङ्ग;
			पक्ष CSS_COMPUTED_CONTENT_URI:
				fprintf(stream,	"uri(\"%.*s\")",
						(पूर्णाङ्क) lwc_string_length(
						content->data.uri),
						lwc_string_data(
						content->data.uri));
				भङ्ग;
			पक्ष CSS_COMPUTED_CONTENT_COUNTER:
				fprintf(stream, "counter(%.*s)",
						(पूर्णाङ्क) lwc_string_length(
						content->data.counter.name),
						lwc_string_data(
						content->data.counter.name));
				भङ्ग;
			पक्ष CSS_COMPUTED_CONTENT_COUNTERS:
				fprintf(stream, "counters(%.*s, \"%.*s\")",
						(पूर्णाङ्क) lwc_string_length(
						content->data.counters.name),
						lwc_string_data(
						content->data.counters.name),
						(पूर्णाङ्क) lwc_string_length(
						content->data.counters.sep),
						lwc_string_data(
						content->data.counters.sep));
				भङ्ग;
			पक्ष CSS_COMPUTED_CONTENT_ATTR:
				fprintf(stream, "attr(%.*s)",
						(पूर्णाङ्क) lwc_string_length(
						content->data.attr),
						lwc_string_data(
						content->data.attr));
				भङ्ग;
			पक्ष CSS_COMPUTED_CONTENT_OPEN_QUOTE:
				fprintf(stream, "open-quote");
				भङ्ग;
			पक्ष CSS_COMPUTED_CONTENT_CLOSE_QUOTE:
				fprintf(stream, "close-quote");
				भङ्ग;
			पक्ष CSS_COMPUTED_CONTENT_NO_OPEN_QUOTE:
				fprintf(stream, "no-open-quote");
				भङ्ग;
			पक्ष CSS_COMPUTED_CONTENT_NO_CLOSE_QUOTE:
				fprintf(stream, "no-close-quote");
				भङ्ग;
			}

			content++;
		}

		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* counter-increment */
	val = css_computed_counter_increment(style, &counter);
	यदि ((val == CSS_COUNTER_INCREMENT_NONE) || (counter == रिक्त)) {
		fprintf(stream, "counter-increment: none ");
	} अन्यथा {
		fprintf(stream, "counter-increment:");

		यावत् (counter->name != रिक्त) {
			fprintf(stream, " %.*s ",
					(पूर्णाङ्क) lwc_string_length(counter->name),
					lwc_string_data(counter->name));

			dump_css_fixed(stream, counter->मूल्य);

			counter++;
		}

		fprintf(stream, " ");
	}

	/* counter-reset */
	val = css_computed_counter_reset(style, &counter);
	यदि ((val == CSS_COUNTER_RESET_NONE) || (counter == रिक्त)) {
		fprintf(stream, "counter-reset: none ");
	} अन्यथा {
		fprintf(stream, "counter-reset:");

		यावत् (counter->name != रिक्त) {
			fprintf(stream, " %.*s ",
					(पूर्णाङ्क) lwc_string_length(counter->name),
					lwc_string_data(counter->name));

			dump_css_fixed(stream, counter->मूल्य);

			counter++;
		}

		fprintf(stream, " ");
	}

	/* cursor */
	val = css_computed_cursor(style, &string_list);
	fprintf(stream, "cursor:");

	यदि (string_list != रिक्त) {
		यावत् (*string_list != रिक्त) {
			fprintf(stream, " url\"%.*s\")",
					(पूर्णाङ्क) lwc_string_length(*string_list),
					lwc_string_data(*string_list));

			string_list++;
		}
	}
	विकल्प (val) {
	पक्ष CSS_CURSOR_AUTO:
		fprintf(stream, " auto ");
		भङ्ग;
	पक्ष CSS_CURSOR_CROSSHAIR:
		fprintf(stream, " crosshair ");
		भङ्ग;
	पक्ष CSS_CURSOR_DEFAULT:
		fprintf(stream, " default ");
		भङ्ग;
	पक्ष CSS_CURSOR_POINTER:
		fprintf(stream, " pointer ");
		भङ्ग;
	पक्ष CSS_CURSOR_MOVE:
		fprintf(stream, " move ");
		भङ्ग;
	पक्ष CSS_CURSOR_E_RESIZE:
		fprintf(stream, " e-resize ");
		भङ्ग;
	पक्ष CSS_CURSOR_NE_RESIZE:
		fprintf(stream, " ne-resize ");
		भङ्ग;
	पक्ष CSS_CURSOR_NW_RESIZE:
		fprintf(stream, " nw-resize ");
		भङ्ग;
	पक्ष CSS_CURSOR_N_RESIZE:
		fprintf(stream, " n-resize ");
		भङ्ग;
	पक्ष CSS_CURSOR_SE_RESIZE:
		fprintf(stream, " se-resize ");
		भङ्ग;
	पक्ष CSS_CURSOR_SW_RESIZE:
		fprintf(stream, " sw-resize ");
		भङ्ग;
	पक्ष CSS_CURSOR_S_RESIZE:
		fprintf(stream, " s-resize ");
		भङ्ग;
	पक्ष CSS_CURSOR_W_RESIZE:
		fprintf(stream, " w-resize ");
		भङ्ग;
	पक्ष CSS_CURSOR_TEXT:
		fprintf(stream, " text ");
		भङ्ग;
	पक्ष CSS_CURSOR_WAIT:
		fprintf(stream, " wait ");
		भङ्ग;
	पक्ष CSS_CURSOR_HELP:
		fprintf(stream, " help ");
		भङ्ग;
	पक्ष CSS_CURSOR_PROGRESS:
		fprintf(stream, " progress ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* direction */
	val = css_computed_direction(style);
	विकल्प (val) {
	पक्ष CSS_DIRECTION_LTR:
		fprintf(stream, "direction: ltr ");
		भङ्ग;
	पक्ष CSS_DIRECTION_RTL:
		fprintf(stream, "direction: rtl ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* display */
	val = ns_computed_display_static(style);
	विकल्प (val) {
	पक्ष CSS_DISPLAY_INLINE:
		fprintf(stream, "display: inline ");
		भङ्ग;
	पक्ष CSS_DISPLAY_BLOCK:
		fprintf(stream, "display: block ");
		भङ्ग;
	पक्ष CSS_DISPLAY_LIST_ITEM:
		fprintf(stream, "display: list-item ");
		भङ्ग;
	पक्ष CSS_DISPLAY_RUN_IN:
		fprintf(stream, "display: run-in ");
		भङ्ग;
	पक्ष CSS_DISPLAY_INLINE_BLOCK:
		fprintf(stream, "display: inline-block ");
		भङ्ग;
	पक्ष CSS_DISPLAY_TABLE:
		fprintf(stream, "display: table ");
		भङ्ग;
	पक्ष CSS_DISPLAY_INLINE_TABLE:
		fprintf(stream, "display: inline-table ");
		भङ्ग;
	पक्ष CSS_DISPLAY_TABLE_ROW_GROUP:
		fprintf(stream, "display: table-row-group ");
		भङ्ग;
	पक्ष CSS_DISPLAY_TABLE_HEADER_GROUP:
		fprintf(stream, "display: table-header-group ");
		भङ्ग;
	पक्ष CSS_DISPLAY_TABLE_FOOTER_GROUP:
		fprintf(stream, "display: table-footer-group ");
		भङ्ग;
	पक्ष CSS_DISPLAY_TABLE_ROW:
		fprintf(stream, "display: table-row ");
		भङ्ग;
	पक्ष CSS_DISPLAY_TABLE_COLUMN_GROUP:
		fprintf(stream, "display: table-column-group ");
		भङ्ग;
	पक्ष CSS_DISPLAY_TABLE_COLUMN:
		fprintf(stream, "display: table-column ");
		भङ्ग;
	पक्ष CSS_DISPLAY_TABLE_CELL:
		fprintf(stream, "display: table-cell ");
		भङ्ग;
	पक्ष CSS_DISPLAY_TABLE_CAPTION:
		fprintf(stream, "display: table-caption ");
		भङ्ग;
	पक्ष CSS_DISPLAY_NONE:
		fprintf(stream, "display: none ");
		भङ्ग;
	पक्ष CSS_DISPLAY_FLEX:
		fprintf(stream, "display: flex ");
		भङ्ग;
	पक्ष CSS_DISPLAY_INLINE_FLEX:
		fprintf(stream, "display: inline-flex ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* empty-cells */
	val = css_computed_empty_cells(style);
	विकल्प (val) {
	पक्ष CSS_EMPTY_CELLS_SHOW:
		fprintf(stream, "empty-cells: show ");
		भङ्ग;
	पक्ष CSS_EMPTY_CELLS_HIDE:
		fprintf(stream, "empty-cells: hide ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* float */
	val = css_computed_float(style);
	विकल्प (val) {
	पक्ष CSS_FLOAT_डावा:
		fprintf(stream, "float: left ");
		भङ्ग;
	पक्ष CSS_FLOAT_उजवा:
		fprintf(stream, "float: right ");
		भङ्ग;
	पक्ष CSS_FLOAT_NONE:
		fprintf(stream, "float: none ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* font-family */
	val = css_computed_font_family(style, &string_list);
	यदि (val != CSS_FONT_FAMILY_INHERIT) {
		fprintf(stream, "font-family:");

		यदि (string_list != रिक्त) {
			यावत् (*string_list != रिक्त) {
				fprintf(stream, " \"%.*s\"",
					(पूर्णाङ्क) lwc_string_length(*string_list),
					lwc_string_data(*string_list));

				string_list++;
			}
		}
		विकल्प (val) {
		पक्ष CSS_FONT_FAMILY_SERIF:
			fprintf(stream, " serif ");
			भङ्ग;
		पक्ष CSS_FONT_FAMILY_SANS_SERIF:
			fprintf(stream, " sans-serif ");
			भङ्ग;
		पक्ष CSS_FONT_FAMILY_CURSIVE:
			fprintf(stream, " cursive ");
			भङ्ग;
		पक्ष CSS_FONT_FAMILY_FANTASY:
			fprintf(stream, " fantasy ");
			भङ्ग;
		पक्ष CSS_FONT_FAMILY_MONOSPACE:
			fprintf(stream, " monospace ");
			भङ्ग;
		}
	}

	/* font-size */
	val = css_computed_font_size(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_FONT_SIZE_XX_SMALL:
		fprintf(stream, "font-size: xx-small ");
		भङ्ग;
	पक्ष CSS_FONT_SIZE_X_SMALL:
		fprintf(stream, "font-size: x-small ");
		भङ्ग;
	पक्ष CSS_FONT_SIZE_SMALL:
		fprintf(stream, "font-size: small ");
		भङ्ग;
	पक्ष CSS_FONT_SIZE_MEDIUM:
		fprintf(stream, "font-size: medium ");
		भङ्ग;
	पक्ष CSS_FONT_SIZE_LARGE:
		fprintf(stream, "font-size: large ");
		भङ्ग;
	पक्ष CSS_FONT_SIZE_X_LARGE:
		fprintf(stream, "font-size: x-large ");
		भङ्ग;
	पक्ष CSS_FONT_SIZE_XX_LARGE:
		fprintf(stream, "font-size: xx-large ");
		भङ्ग;
	पक्ष CSS_FONT_SIZE_LARGER:
		fprintf(stream, "font-size: larger ");
		भङ्ग;
	पक्ष CSS_FONT_SIZE_SMALLER:
		fprintf(stream, "font-size: smaller ");
		भङ्ग;
	पक्ष CSS_FONT_SIZE_DIMENSION:
		fprintf(stream, "font-size: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* font-style */
	val = css_computed_font_style(style);
	विकल्प (val) {
	पक्ष CSS_FONT_STYLE_NORMAL:
		fprintf(stream, "font-style: normal ");
		भङ्ग;
	पक्ष CSS_FONT_STYLE_ITALIC:
		fprintf(stream, "font-style: italic ");
		भङ्ग;
	पक्ष CSS_FONT_STYLE_OBLIQUE:
		fprintf(stream, "font-style: oblique ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* font-variant */
	val = css_computed_font_variant(style);
	विकल्प (val) {
	पक्ष CSS_FONT_VARIANT_NORMAL:
		fprintf(stream, "font-variant: normal ");
		भङ्ग;
	पक्ष CSS_FONT_VARIANT_SMALL_CAPS:
		fprintf(stream, "font-variant: small-caps ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* font-weight */
	val = css_computed_font_weight(style);
	विकल्प (val) {
	पक्ष CSS_FONT_WEIGHT_NORMAL:
		fprintf(stream, "font-weight: normal ");
		भङ्ग;
	पक्ष CSS_FONT_WEIGHT_BOLD:
		fprintf(stream, "font-weight: bold ");
		भङ्ग;
	पक्ष CSS_FONT_WEIGHT_BOLDER:
		fprintf(stream, "font-weight: bolder ");
		भङ्ग;
	पक्ष CSS_FONT_WEIGHT_LIGHTER:
		fprintf(stream, "font-weight: lighter ");
		भङ्ग;
	पक्ष CSS_FONT_WEIGHT_100:
		fprintf(stream, "font-weight: 100 ");
		भङ्ग;
	पक्ष CSS_FONT_WEIGHT_200:
		fprintf(stream, "font-weight: 200 ");
		भङ्ग;
	पक्ष CSS_FONT_WEIGHT_300:
		fprintf(stream, "font-weight: 300 ");
		भङ्ग;
	पक्ष CSS_FONT_WEIGHT_400:
		fprintf(stream, "font-weight: 400 ");
		भङ्ग;
	पक्ष CSS_FONT_WEIGHT_500:
		fprintf(stream, "font-weight: 500 ");
		भङ्ग;
	पक्ष CSS_FONT_WEIGHT_600:
		fprintf(stream, "font-weight: 600 ");
		भङ्ग;
	पक्ष CSS_FONT_WEIGHT_700:
		fprintf(stream, "font-weight: 700 ");
		भङ्ग;
	पक्ष CSS_FONT_WEIGHT_800:
		fprintf(stream, "font-weight: 800 ");
		भङ्ग;
	पक्ष CSS_FONT_WEIGHT_900:
		fprintf(stream, "font-weight: 900 ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* height */
	val = css_computed_height(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_HEIGHT_AUTO:
		fprintf(stream, "height: auto ");
		भङ्ग;
	पक्ष CSS_HEIGHT_SET:
		fprintf(stream, "height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/*डावा*/
	val = css_computed_डावा(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_डावा_AUTO:
		fprintf(stream, "left: auto ");
		भङ्ग;
	पक्ष CSS_डावा_SET:
		fprintf(stream, "left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* letter-spacing */
	val = css_computed_letter_spacing(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_LETTER_SPACING_NORMAL:
		fprintf(stream, "letter-spacing: normal ");
		भङ्ग;
	पक्ष CSS_LETTER_SPACING_SET:
		fprintf(stream, "letter-spacing: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* line-height */
	val = css_computed_ओळ_height(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_ओळ_HEIGHT_NORMAL:
		fprintf(stream, "line-height: normal ");
		भङ्ग;
	पक्ष CSS_ओळ_HEIGHT_संख्या:
		fprintf(stream, "line-height: ");

		dump_css_fixed(stream, len1);

		fprintf(stream, " ");
		भङ्ग;
	पक्ष CSS_ओळ_HEIGHT_DIMENSION:
		fprintf(stream, "line-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* list-style-image */
	val = css_computed_list_style_image(style, &url);
	यदि (url != रिक्त) {
		fprintf(stream, "list-style-image: url('%.*s') ",
				(पूर्णाङ्क) lwc_string_length(url),
				lwc_string_data(url));
	} अन्यथा यदि (val == CSS_LIST_STYLE_IMAGE_NONE) {
		fprintf(stream, "list-style-image: none ");
	}

	/* list-style-position */
	val = css_computed_list_style_position(style);
	विकल्प (val) {
	पक्ष CSS_LIST_STYLE_POSITION_INSIDE:
		fprintf(stream, "list-style-position: inside ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_POSITION_OUTSIDE:
		fprintf(stream, "list-style-position: outside ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* list-style-type */
	val = css_computed_list_style_type(style);
	विकल्प (val) {
	पक्ष CSS_LIST_STYLE_TYPE_DISC:
		fprintf(stream, "list-style-type: disc ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_TYPE_CIRCLE:
		fprintf(stream, "list-style-type: circle ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_TYPE_SQUARE:
		fprintf(stream, "list-style-type: square ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_TYPE_DECIMAL:
		fprintf(stream, "list-style-type: decimal ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_TYPE_DECIMAL_LEADING_ZERO:
		fprintf(stream, "list-style-type: decimal-leading-zero ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_TYPE_LOWER_ROMAN:
		fprintf(stream, "list-style-type: lower-roman ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_TYPE_UPPER_ROMAN:
		fprintf(stream, "list-style-type: upper-roman ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_TYPE_LOWER_GREEK:
		fprintf(stream, "list-style-type: lower-greek ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_TYPE_LOWER_LATIN:
		fprintf(stream, "list-style-type: lower-latin ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_TYPE_UPPER_LATIN:
		fprintf(stream, "list-style-type: upper-latin ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_TYPE_ARMENIAN:
		fprintf(stream, "list-style-type: armenian ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_TYPE_GEORGIAN:
		fprintf(stream, "list-style-type: georgian ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_TYPE_LOWER_ALPHA:
		fprintf(stream, "list-style-type: lower-alpha ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_TYPE_UPPER_ALPHA:
		fprintf(stream, "list-style-type: upper-alpha ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_TYPE_NONE:
		fprintf(stream, "list-style-type: none ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* margin-top */
	val = css_computed_margin_शीर्ष(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_MARGIN_AUTO:
		fprintf(stream, "margin-top: auto ");
		भङ्ग;
	पक्ष CSS_MARGIN_SET:
		fprintf(stream, "margin-top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* margin-right */
	val = css_computed_margin_उजवा(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_MARGIN_AUTO:
		fprintf(stream, "margin-right: auto ");
		भङ्ग;
	पक्ष CSS_MARGIN_SET:
		fprintf(stream, "margin-right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* margin-bottom */
	val = css_computed_margin_bottom(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_MARGIN_AUTO:
		fprintf(stream, "margin-bottom: auto ");
		भङ्ग;
	पक्ष CSS_MARGIN_SET:
		fprintf(stream, "margin-bottom: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* margin-left */
	val = css_computed_margin_डावा(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_MARGIN_AUTO:
		fprintf(stream, "margin-left: auto ");
		भङ्ग;
	पक्ष CSS_MARGIN_SET:
		fprintf(stream, "margin-left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* max-height */
	val = css_computed_max_height(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_MAX_HEIGHT_NONE:
		fprintf(stream, "max-height: none ");
		भङ्ग;
	पक्ष CSS_MAX_HEIGHT_SET:
		fprintf(stream, "max-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* max-width */
	val = css_computed_max_width(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_MAX_WIDTH_NONE:
		fprintf(stream, "max-width: none ");
		भङ्ग;
	पक्ष CSS_MAX_WIDTH_SET:
		fprintf(stream, "max-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* min-height */
	val = ns_computed_min_height(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_MIN_HEIGHT_SET:
		fprintf(stream, "min-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* min-width */
	val = ns_computed_min_width(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_MIN_WIDTH_SET:
		fprintf(stream, "min-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* opacity */
	val = css_computed_opacity(style, &len1);
	विकल्प (val) {
	पक्ष CSS_OPACITY_SET:
		fprintf(stream, "opacity: ");

		dump_css_fixed(stream, len1);

		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* outline-color */
	val = css_computed_outline_color(style, &color);
	विकल्प (val) {
	पक्ष CSS_OUTLINE_COLOR_INVERT:
		fprintf(stream, "outline-color: invert ");
		भङ्ग;
	पक्ष CSS_OUTLINE_COLOR_COLOR:
		fprintf(stream, "outline-color: #%08"PRIx32" ", color);
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* outline-style */
	val = css_computed_outline_style(style);
	विकल्प (val) {
	पक्ष CSS_OUTLINE_STYLE_NONE:
		fprintf(stream, "outline-style: none ");
		भङ्ग;
	पक्ष CSS_OUTLINE_STYLE_DOTTED:
		fprintf(stream, "outline-style: dotted ");
		भङ्ग;
	पक्ष CSS_OUTLINE_STYLE_DASHED:
		fprintf(stream, "outline-style: dashed ");
		भङ्ग;
	पक्ष CSS_OUTLINE_STYLE_SOLID:
		fprintf(stream, "outline-style: solid ");
		भङ्ग;
	पक्ष CSS_OUTLINE_STYLE_DOUBLE:
		fprintf(stream, "outline-style: double ");
		भङ्ग;
	पक्ष CSS_OUTLINE_STYLE_GROOVE:
		fprintf(stream, "outline-style: groove ");
		भङ्ग;
	पक्ष CSS_OUTLINE_STYLE_RIDGE:
		fprintf(stream, "outline-style: ridge ");
		भङ्ग;
	पक्ष CSS_OUTLINE_STYLE_INSET:
		fprintf(stream, "outline-style: inset ");
		भङ्ग;
	पक्ष CSS_OUTLINE_STYLE_OUTSET:
		fprintf(stream, "outline-style: outset ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* outline-width */
	val = css_computed_outline_width(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_OUTLINE_WIDTH_THIN:
		fprintf(stream, "outline-width: thin ");
		भङ्ग;
	पक्ष CSS_OUTLINE_WIDTH_MEDIUM:
		fprintf(stream, "outline-width: medium ");
		भङ्ग;
	पक्ष CSS_OUTLINE_WIDTH_THICK:
		fprintf(stream, "outline-width: thick ");
		भङ्ग;
	पक्ष CSS_OUTLINE_WIDTH_WIDTH:
		fprintf(stream, "outline-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* overflow */
	val = css_computed_overflow_x(style);
	विकल्प (val) {
	पक्ष CSS_OVERFLOW_VISIBLE:
		fprintf(stream, "overflow-x: visible ");
		भङ्ग;
	पक्ष CSS_OVERFLOW_HIDDEN:
		fprintf(stream, "overflow-x: hidden ");
		भङ्ग;
	पक्ष CSS_OVERFLOW_SCROLL:
		fprintf(stream, "overflow-x: scroll ");
		भङ्ग;
	पक्ष CSS_OVERFLOW_AUTO:
		fprintf(stream, "overflow-x auto ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* overflow */
	val = css_computed_overflow_y(style);
	विकल्प (val) {
	पक्ष CSS_OVERFLOW_VISIBLE:
		fprintf(stream, "overflow-y: visible ");
		भङ्ग;
	पक्ष CSS_OVERFLOW_HIDDEN:
		fprintf(stream, "overflow-y: hidden ");
		भङ्ग;
	पक्ष CSS_OVERFLOW_SCROLL:
		fprintf(stream, "overflow-y: scroll ");
		भङ्ग;
	पक्ष CSS_OVERFLOW_AUTO:
		fprintf(stream, "overflow-y: auto ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* padding-top */
	val = css_computed_padding_शीर्ष(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_PADDING_SET:
		fprintf(stream, "padding-top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* padding-right */
	val = css_computed_padding_उजवा(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_PADDING_SET:
		fprintf(stream, "padding-right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* padding-bottom */
	val = css_computed_padding_bottom(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_PADDING_SET:
		fprintf(stream, "padding-bottom: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* padding-left */
	val = css_computed_padding_डावा(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_PADDING_SET:
		fprintf(stream, "padding-left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* position */
	val = css_computed_position(style);
	विकल्प (val) {
	पक्ष CSS_POSITION_STATIC:
		fprintf(stream, "position: static ");
		भङ्ग;
	पक्ष CSS_POSITION_RELATIVE:
		fprintf(stream, "position: relative ");
		भङ्ग;
	पक्ष CSS_POSITION_ABSOLUTE:
		fprintf(stream, "position: absolute ");
		भङ्ग;
	पक्ष CSS_POSITION_FIXED:
		fprintf(stream, "position: fixed ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* quotes */
	val = css_computed_quotes(style, &string_list);
	यदि (val == CSS_QUOTES_STRING && string_list != रिक्त) {
		fprintf(stream, "quotes:");

		यावत् (*string_list != रिक्त) {
			fprintf(stream, " \"%.*s\"",
				(पूर्णाङ्क) lwc_string_length(*string_list),
				lwc_string_data(*string_list));

			string_list++;
		}

		fprintf(stream, " ");
	} अन्यथा {
		विकल्प (val) {
		पक्ष CSS_QUOTES_NONE:
			fprintf(stream, "quotes: none ");
			भङ्ग;
		स्वभाव:
			भङ्ग;
		}
	}

	/*उजवा*/
	val = css_computed_उजवा(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_उजवा_AUTO:
		fprintf(stream, "right: auto ");
		भङ्ग;
	पक्ष CSS_उजवा_SET:
		fprintf(stream, "right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* table-layout */
	val = css_computed_table_layout(style);
	विकल्प (val) {
	पक्ष CSS_TABLE_LAYOUT_AUTO:
		fprintf(stream, "table-layout: auto ");
		भङ्ग;
	पक्ष CSS_TABLE_LAYOUT_FIXED:
		fprintf(stream, "table-layout: fixed ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* text-align */
	val = css_computed_text_align(style);
	विकल्प (val) {
	पक्ष CSS_TEXT_ALIGN_डावा:
		fprintf(stream, "text-align: left ");
		भङ्ग;
	पक्ष CSS_TEXT_ALIGN_उजवा:
		fprintf(stream, "text-align: right ");
		भङ्ग;
	पक्ष CSS_TEXT_ALIGN_CENTER:
		fprintf(stream, "text-align: center ");
		भङ्ग;
	पक्ष CSS_TEXT_ALIGN_JUSTIFY:
		fprintf(stream, "text-align: justify ");
		भङ्ग;
	पक्ष CSS_TEXT_ALIGN_DEFAULT:
		fprintf(stream, "text-align: default ");
		भङ्ग;
	पक्ष CSS_TEXT_ALIGN_LIBCSS_डावा:
		fprintf(stream, "text-align: -libcss-left ");
		भङ्ग;
	पक्ष CSS_TEXT_ALIGN_LIBCSS_CENTER:
		fprintf(stream, "text-align: -libcss-center ");
		भङ्ग;
	पक्ष CSS_TEXT_ALIGN_LIBCSS_उजवा:
		fprintf(stream, "text-align: -libcss-right ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* text-decoration */
	val = css_computed_text_decoration(style);
	यदि (val == CSS_TEXT_DECORATION_NONE) {
		fprintf(stream, "text-decoration: none ");
	} अन्यथा {
		fprintf(stream, "text-decoration:");

		यदि (val & CSS_TEXT_DECORATION_BLINK) {
			fprintf(stream, " blink");
		}
		यदि (val & CSS_TEXT_DECORATION_ओळ_THROUGH) {
			fprintf(stream, " line-through");
		}
		यदि (val & CSS_TEXT_DECORATION_OVERLINE) {
			fprintf(stream, " overline");
		}
		यदि (val & CSS_TEXT_DECORATION_UNDERLINE) {
			fprintf(stream, " underline");
		}

		fprintf(stream, " ");
	}

	/* text-indent */
	val = css_computed_text_indent(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_TEXT_INDENT_SET:
		fprintf(stream, "text-indent: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* text-transform */
	val = css_computed_text_transform(style);
	विकल्प (val) {
	पक्ष CSS_TEXT_TRANSFORM_CAPITALIZE:
		fprintf(stream, "text-transform: capitalize ");
		भङ्ग;
	पक्ष CSS_TEXT_TRANSFORM_UPPERCASE:
		fprintf(stream, "text-transform: uppercase ");
		भङ्ग;
	पक्ष CSS_TEXT_TRANSFORM_LOWERCASE:
		fprintf(stream, "text-transform: lowercase ");
		भङ्ग;
	पक्ष CSS_TEXT_TRANSFORM_NONE:
		fprintf(stream, "text-transform: none ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/*शीर्ष*/
	val = css_computed_शीर्ष(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_शीर्ष_AUTO:
		fprintf(stream, "top: auto ");
		भङ्ग;
	पक्ष CSS_शीर्ष_SET:
		fprintf(stream, "top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* unicode-bidi */
	val = css_computed_unicode_bidi(style);
	विकल्प (val) {
	पक्ष CSS_UNICODE_BIDI_NORMAL:
		fprintf(stream, "unicode-bidi: normal ");
		भङ्ग;
	पक्ष CSS_UNICODE_BIDI_EMBED:
		fprintf(stream, "unicode-bidi: embed ");
		भङ्ग;
	पक्ष CSS_UNICODE_BIDI_BIDI_OVERRIDE:
		fprintf(stream, "unicode-bidi: bidi-override ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* vertical-align */
	val = css_computed_vertical_align(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_VERTICAL_ALIGN_BASELINE:
		fprintf(stream, "vertical-align: baseline ");
		भङ्ग;
	पक्ष CSS_VERTICAL_ALIGN_SUB:
		fprintf(stream, "vertical-align: sub ");
		भङ्ग;
	पक्ष CSS_VERTICAL_ALIGN_SUPER:
		fprintf(stream, "vertical-align: super ");
		भङ्ग;
	पक्ष CSS_VERTICAL_ALIGN_शीर्ष:
		fprintf(stream, "vertical-align: top ");
		भङ्ग;
	पक्ष CSS_VERTICAL_ALIGN_TEXT_शीर्ष:
		fprintf(stream, "vertical-align: text-top ");
		भङ्ग;
	पक्ष CSS_VERTICAL_ALIGN_मधला:
		fprintf(stream, "vertical-align: middle ");
		भङ्ग;
	पक्ष CSS_VERTICAL_ALIGN_BOTTOM:
		fprintf(stream, "vertical-align: bottom ");
		भङ्ग;
	पक्ष CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
		fprintf(stream, "vertical-align: text-bottom ");
		भङ्ग;
	पक्ष CSS_VERTICAL_ALIGN_SET:
		fprintf(stream, "vertical-align: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* visibility */
	val = css_computed_visibility(style);
	विकल्प (val) {
	पक्ष CSS_VISIBILITY_VISIBLE:
		fprintf(stream, "visibility: visible ");
		भङ्ग;
	पक्ष CSS_VISIBILITY_HIDDEN:
		fprintf(stream, "visibility: hidden ");
		भङ्ग;
	पक्ष CSS_VISIBILITY_COLLAPSE:
		fprintf(stream, "visibility: collapse ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* white-space */
	val = css_computed_white_space(style);
	विकल्प (val) {
	पक्ष CSS_WHITE_SPACE_NORMAL:
		fprintf(stream, "white-space: normal ");
		भङ्ग;
	पक्ष CSS_WHITE_SPACE_PRE:
		fprintf(stream, "white-space: pre ");
		भङ्ग;
	पक्ष CSS_WHITE_SPACE_NOWRAP:
		fprintf(stream, "white-space: nowrap ");
		भङ्ग;
	पक्ष CSS_WHITE_SPACE_PRE_WRAP:
		fprintf(stream, "white-space: pre-wrap ");
		भङ्ग;
	पक्ष CSS_WHITE_SPACE_PRE_ओळ:
		fprintf(stream, "white-space: pre-line ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* width */
	val = css_computed_width(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_WIDTH_AUTO:
		fprintf(stream, "width: auto ");
		भङ्ग;
	पक्ष CSS_WIDTH_SET:
		fprintf(stream, "width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* word-spacing */
	val = css_computed_word_spacing(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_WORD_SPACING_NORMAL:
		fprintf(stream, "word-spacing: normal ");
		भङ्ग;
	पक्ष CSS_WORD_SPACING_SET:
		fprintf(stream, "word-spacing: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* z-index */
	val = css_computed_z_index(style, &zindex);
	विकल्प (val) {
	पक्ष CSS_Z_INDEX_AUTO:
		fprintf(stream, "z-index: auto ");
		भङ्ग;
	पक्ष CSS_Z_INDEX_SET:
		fprintf(stream, "z-index: %"PRId32" ", zindex);
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	fprintf(stream, "}");
}
