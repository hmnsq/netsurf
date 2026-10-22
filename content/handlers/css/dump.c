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
स्थिर शून्य dump_css_fixed(खाता *stream, css_fixed f)
{
#define NSCSS_ABS(x) (uint32_t)((x) < 0 ? -(x) : (x))
	uint32_t uintpart = FIXTOINT(NSCSS_ABS(f));
	/* + 500 to ensure round to nearest (division will truncate) */
	uint32_t fracpart = ((NSCSS_ABS(f) & 0x3ff) * 1000 + 500) / (1 << 10);
#undef NSCSS_ABS

	ख_लिखो(stream, "%s%"PRIu32".%03"PRIu32, f < 0 ? "-" : "", uintpart, fracpart);
}

/**
 * Dump a numeric value to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 */
स्थिर शून्य dump_css_सङ्ख्या(खाता *stream, css_fixed val)
{
	यदि (INTTOFIX(FIXTOINT(val)) == val)
		ख_लिखो(stream, "%"PRId32, FIXTOINT(val));
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
स्थिर शून्य dump_css_unit(खाता *stream, css_fixed val, css_unit unit)
{
	dump_css_सङ्ख्या(stream, val);

	विकल्प (unit) {
	पक्ष CSS_UNIT_PX:
		ख_लिखो(stream, "px");
		रोक;
	पक्ष CSS_UNIT_EX:
		ख_लिखो(stream, "ex");
		रोक;
	पक्ष CSS_UNIT_EM:
		ख_लिखो(stream, "em");
		रोक;
	पक्ष CSS_UNIT_IN:
		ख_लिखो(stream, "in");
		रोक;
	पक्ष CSS_UNIT_CM:
		ख_लिखो(stream, "cm");
		रोक;
	पक्ष CSS_UNIT_MM:
		ख_लिखो(stream, "mm");
		रोक;
	पक्ष CSS_UNIT_PT:
		ख_लिखो(stream, "pt");
		रोक;
	पक्ष CSS_UNIT_PC:
		ख_लिखो(stream, "pc");
		रोक;
	पक्ष CSS_UNIT_PCT:
		ख_लिखो(stream, "%%");
		रोक;
	पक्ष CSS_UNIT_DEG:
		ख_लिखो(stream, "deg");
		रोक;
	पक्ष CSS_UNIT_GRAD:
		ख_लिखो(stream, "grad");
		रोक;
	पक्ष CSS_UNIT_RAD:
		ख_लिखो(stream, "rad");
		रोक;
	पक्ष CSS_UNIT_MS:
		ख_लिखो(stream, "ms");
		रोक;
	पक्ष CSS_UNIT_S:
		ख_लिखो(stream, "s");
		रोक;
	पक्ष CSS_UNIT_HZ:
		ख_लिखो(stream, "Hz");
		रोक;
	पक्ष CSS_UNIT_KHZ:
		ख_लिखो(stream, "kHz");
		रोक;
	पक्ष CSS_UNIT_CH:
		ख_लिखो(stream, "ch");
		रोक;
	पक्ष CSS_UNIT_REM:
		ख_लिखो(stream, "rem");
		रोक;
	पक्ष CSS_UNIT_LH:
		ख_लिखो(stream, "lh");
		रोक;
	पक्ष CSS_UNIT_VH:
		ख_लिखो(stream, "vh");
		रोक;
	पक्ष CSS_UNIT_VW:
		ख_लिखो(stream, "vw");
		रोक;
	पक्ष CSS_UNIT_VI:
		ख_लिखो(stream, "vi");
		रोक;
	पक्ष CSS_UNIT_VB:
		ख_लिखो(stream, "vb");
		रोक;
	पक्ष CSS_UNIT_VMIN:
		ख_लिखो(stream, "vmin");
		रोक;
	पक्ष CSS_UNIT_VMAX:
		ख_लिखो(stream, "vmax");
		रोक;
	पक्ष CSS_UNIT_Q:
		ख_लिखो(stream, "q");
		रोक;
	पक्ष CSS_UNIT_CALC:
		ख_लिखो(stream, "calc()");
		रोक;
	}
}

/* exported interface documented in content/handlers/css/dump.h */
शून्य nscss_dump_computed_style(खाता *stream, अचर css_computed_style *style)
{
	uint8_t val;
	css_color color = 0;
	lwc_string *url = रिक्त;
	css_fixed len1 = 0, len2 = 0;
	css_unit unit1 = CSS_UNIT_PX, unit2 = CSS_UNIT_PX;
	css_computed_clip_rect rect = { 0, 0, 0, 0, CSS_UNIT_PX, CSS_UNIT_PX,
					CSS_UNIT_PX, CSS_UNIT_PX, true, true,
					true, true };
	अचर css_computed_content_item *content = रिक्त;
	अचर css_computed_counter *counter = रिक्त;
	lwc_string **string_list = रिक्त;
	int32_t zindex = 0;

	ख_लिखो(stream, "{ ");

	/* background-attachment */
	val = css_computed_background_attachment(style);
	विकल्प (val) {
	पक्ष CSS_BACKGROUND_ATTACHMENT_FIXED:
		ख_लिखो(stream, "background-attachment: fixed ");
		रोक;
	पक्ष CSS_BACKGROUND_ATTACHMENT_SCROLL:
		ख_लिखो(stream, "background-attachment: scroll ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* background-color */
	val = css_computed_background_color(style, &color);
	विकल्प (val) {
	पक्ष CSS_BACKGROUND_COLOR_COLOR:
		ख_लिखो(stream, "background-color: #%08"PRIx32" ", color);
		रोक;
	स्वभाव:
		रोक;
	}

	/* background-image */
	val = css_computed_background_image(style, &url);
	यदि (val == CSS_BACKGROUND_IMAGE_IMAGE && url != रिक्त) {
		ख_लिखो(stream, "background-image: url('%.*s') ",
				(पूर्णाङ्क) lwc_string_length(url),
				lwc_string_data(url));
	} अन्यथा यदि (val == CSS_BACKGROUND_IMAGE_NONE) {
		ख_लिखो(stream, "background-image: none ");
	}

	/* background-position */
	val = css_computed_background_position(style, &len1, &unit1,
			&len2, &unit2);
	यदि (val == CSS_BACKGROUND_POSITION_SET) {
		ख_लिखो(stream, "background-position: ");
		dump_css_unit(stream, len1, unit1);
		ख_लिखो(stream, " ");
		dump_css_unit(stream, len2, unit2);
		ख_लिखो(stream, " ");
	}

	/* background-repeat */
	val = css_computed_background_repeat(style);
	विकल्प (val) {
	पक्ष CSS_BACKGROUND_REPEAT_REPEAT_X:
		ख_लिखो(stream, "background-repeat: repeat-x ");
		रोक;
	पक्ष CSS_BACKGROUND_REPEAT_REPEAT_Y:
		ख_लिखो(stream, "background-repeat: repeat-y ");
		रोक;
	पक्ष CSS_BACKGROUND_REPEAT_REPEAT:
		ख_लिखो(stream, "background-repeat: repeat ");
		रोक;
	पक्ष CSS_BACKGROUND_REPEAT_NO_REPEAT:
		ख_लिखो(stream, "background-repeat: no-repeat ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* border-collapse */
	val = css_computed_border_collapse(style);
	विकल्प (val) {
	पक्ष CSS_BORDER_COLLAPSE_SEPARATE:
		ख_लिखो(stream, "border-collapse: separate ");
		रोक;
	पक्ष CSS_BORDER_COLLAPSE_COLLAPSE:
		ख_लिखो(stream, "border-collapse: collapse ");
		रोक;
	स्वभाव:

		रोक;
	}

	/* border-spacing */
	val = css_computed_border_spacing(style, &len1, &unit1, &len2, &unit2);
	यदि (val == CSS_BORDER_SPACING_SET) {
		ख_लिखो(stream, "border-spacing: ");
		dump_css_unit(stream, len1, unit1);
		ख_लिखो(stream, " ");
		dump_css_unit(stream, len2, unit2);
		ख_लिखो(stream, " ");
	}

	/* border-top-color */
	val = css_computed_border_शीर्ष_color(style, &color);
	विकल्प (val) {
	पक्ष CSS_BORDER_COLOR_COLOR:
		ख_लिखो(stream, "border-top-color: #%08"PRIx32" ", color);
		रोक;
	स्वभाव:
		रोक;
	}

	/* border-right-color */
	val = css_computed_border_दायाँ_color(style, &color);
	विकल्प (val) {
	पक्ष CSS_BORDER_COLOR_COLOR:
		ख_लिखो(stream, "border-right-color: #%08"PRIx32" ", color);
		रोक;
	स्वभाव:
		रोक;
	}

	/* border-bottom-color */
	val = css_computed_border_bottom_color(style, &color);
	विकल्प (val) {
	पक्ष CSS_BORDER_COLOR_COLOR:
		ख_लिखो(stream, "border-bottom-color: #%08"PRIx32" ", color);
		रोक;
	स्वभाव:
		रोक;
	}

	/* border-left-color */
	val = css_computed_border_बायाँ_color(style, &color);
	विकल्प (val) {
	पक्ष CSS_BORDER_COLOR_COLOR:
		ख_लिखो(stream, "border-left-color: #%08"PRIx32" ", color);
		रोक;
	स्वभाव:
		रोक;
	}

	/* border-top-style */
	val = css_computed_border_शीर्ष_style(style);
	विकल्प (val) {
	पक्ष CSS_BORDER_STYLE_NONE:
		ख_लिखो(stream, "border-top-style: none ");
		रोक;
	पक्ष CSS_BORDER_STYLE_HIDDEN:
		ख_लिखो(stream, "border-top-style: hidden ");
		रोक;
	पक्ष CSS_BORDER_STYLE_DOTTED:
		ख_लिखो(stream, "border-top-style: dotted ");
		रोक;
	पक्ष CSS_BORDER_STYLE_DASHED:
		ख_लिखो(stream, "border-top-style: dashed ");
		रोक;
	पक्ष CSS_BORDER_STYLE_SOLID:
		ख_लिखो(stream, "border-top-style: solid ");
		रोक;
	पक्ष CSS_BORDER_STYLE_DOUBLE:
		ख_लिखो(stream, "border-top-style: double ");
		रोक;
	पक्ष CSS_BORDER_STYLE_GROOVE:
		ख_लिखो(stream, "border-top-style: groove ");
		रोक;
	पक्ष CSS_BORDER_STYLE_RIDGE:
		ख_लिखो(stream, "border-top-style: ridge ");
		रोक;
	पक्ष CSS_BORDER_STYLE_INSET:
		ख_लिखो(stream, "border-top-style: inset ");
		रोक;
	पक्ष CSS_BORDER_STYLE_OUTSET:
		ख_लिखो(stream, "border-top-style: outset ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* border-right-style */
	val = css_computed_border_दायाँ_style(style);
	विकल्प (val) {
	पक्ष CSS_BORDER_STYLE_NONE:
		ख_लिखो(stream, "border-right-style: none ");
		रोक;
	पक्ष CSS_BORDER_STYLE_HIDDEN:
		ख_लिखो(stream, "border-right-style: hidden ");
		रोक;
	पक्ष CSS_BORDER_STYLE_DOTTED:
		ख_लिखो(stream, "border-right-style: dotted ");
		रोक;
	पक्ष CSS_BORDER_STYLE_DASHED:
		ख_लिखो(stream, "border-right-style: dashed ");
		रोक;
	पक्ष CSS_BORDER_STYLE_SOLID:
		ख_लिखो(stream, "border-right-style: solid ");
		रोक;
	पक्ष CSS_BORDER_STYLE_DOUBLE:
		ख_लिखो(stream, "border-right-style: double ");
		रोक;
	पक्ष CSS_BORDER_STYLE_GROOVE:
		ख_लिखो(stream, "border-right-style: groove ");
		रोक;
	पक्ष CSS_BORDER_STYLE_RIDGE:
		ख_लिखो(stream, "border-right-style: ridge ");
		रोक;
	पक्ष CSS_BORDER_STYLE_INSET:
		ख_लिखो(stream, "border-right-style: inset ");
		रोक;
	पक्ष CSS_BORDER_STYLE_OUTSET:
		ख_लिखो(stream, "border-right-style: outset ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* border-bottom-style */
	val = css_computed_border_bottom_style(style);
	विकल्प (val) {
	पक्ष CSS_BORDER_STYLE_NONE:
		ख_लिखो(stream, "border-bottom-style: none ");
		रोक;
	पक्ष CSS_BORDER_STYLE_HIDDEN:
		ख_लिखो(stream, "border-bottom-style: hidden ");
		रोक;
	पक्ष CSS_BORDER_STYLE_DOTTED:
		ख_लिखो(stream, "border-bottom-style: dotted ");
		रोक;
	पक्ष CSS_BORDER_STYLE_DASHED:
		ख_लिखो(stream, "border-bottom-style: dashed ");
		रोक;
	पक्ष CSS_BORDER_STYLE_SOLID:
		ख_लिखो(stream, "border-bottom-style: solid ");
		रोक;
	पक्ष CSS_BORDER_STYLE_DOUBLE:
		ख_लिखो(stream, "border-bottom-style: double ");
		रोक;
	पक्ष CSS_BORDER_STYLE_GROOVE:
		ख_लिखो(stream, "border-bottom-style: groove ");
		रोक;
	पक्ष CSS_BORDER_STYLE_RIDGE:
		ख_लिखो(stream, "border-bottom-style: ridge ");
		रोक;
	पक्ष CSS_BORDER_STYLE_INSET:
		ख_लिखो(stream, "border-bottom-style: inset ");
		रोक;
	पक्ष CSS_BORDER_STYLE_OUTSET:
		ख_लिखो(stream, "border-bottom-style: outset ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* border-left-style */
	val = css_computed_border_बायाँ_style(style);
	विकल्प (val) {
	पक्ष CSS_BORDER_STYLE_NONE:
		ख_लिखो(stream, "border-left-style: none ");
		रोक;
	पक्ष CSS_BORDER_STYLE_HIDDEN:
		ख_लिखो(stream, "border-left-style: hidden ");
		रोक;
	पक्ष CSS_BORDER_STYLE_DOTTED:
		ख_लिखो(stream, "border-left-style: dotted ");
		रोक;
	पक्ष CSS_BORDER_STYLE_DASHED:
		ख_लिखो(stream, "border-left-style: dashed ");
		रोक;
	पक्ष CSS_BORDER_STYLE_SOLID:
		ख_लिखो(stream, "border-left-style: solid ");
		रोक;
	पक्ष CSS_BORDER_STYLE_DOUBLE:
		ख_लिखो(stream, "border-left-style: double ");
		रोक;
	पक्ष CSS_BORDER_STYLE_GROOVE:
		ख_लिखो(stream, "border-left-style: groove ");
		रोक;
	पक्ष CSS_BORDER_STYLE_RIDGE:
		ख_लिखो(stream, "border-left-style: ridge ");
		रोक;
	पक्ष CSS_BORDER_STYLE_INSET:
		ख_लिखो(stream, "border-left-style: inset ");
		रोक;
	पक्ष CSS_BORDER_STYLE_OUTSET:
		ख_लिखो(stream, "border-left-style: outset ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* border-top-width */
	val = css_computed_border_शीर्ष_width(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_BORDER_WIDTH_THIN:
		ख_लिखो(stream, "border-top-width: thin ");
		रोक;
	पक्ष CSS_BORDER_WIDTH_MEDIUM:
		ख_लिखो(stream, "border-top-width: medium ");
		रोक;
	पक्ष CSS_BORDER_WIDTH_THICK:
		ख_लिखो(stream, "border-top-width: thick ");
		रोक;
	पक्ष CSS_BORDER_WIDTH_WIDTH:
		ख_लिखो(stream, "border-top-width: ");
		dump_css_unit(stream, len1, unit1);
		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* border-right-width */
	val = css_computed_border_दायाँ_width(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_BORDER_WIDTH_THIN:
		ख_लिखो(stream, "border-right-width: thin ");
		रोक;
	पक्ष CSS_BORDER_WIDTH_MEDIUM:
		ख_लिखो(stream, "border-right-width: medium ");
		रोक;
	पक्ष CSS_BORDER_WIDTH_THICK:
		ख_लिखो(stream, "border-right-width: thick ");
		रोक;
	पक्ष CSS_BORDER_WIDTH_WIDTH:
		ख_लिखो(stream, "border-right-width: ");
		dump_css_unit(stream, len1, unit1);
		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* border-bottom-width */
	val = css_computed_border_bottom_width(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_BORDER_WIDTH_THIN:
		ख_लिखो(stream, "border-bottom-width: thin ");
		रोक;
	पक्ष CSS_BORDER_WIDTH_MEDIUM:
		ख_लिखो(stream, "border-bottom-width: medium ");
		रोक;
	पक्ष CSS_BORDER_WIDTH_THICK:
		ख_लिखो(stream, "border-bottom-width: thick ");
		रोक;
	पक्ष CSS_BORDER_WIDTH_WIDTH:
		ख_लिखो(stream, "border-bottom-width: ");
		dump_css_unit(stream, len1, unit1);
		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* border-left-width */
	val = css_computed_border_बायाँ_width(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_BORDER_WIDTH_THIN:
		ख_लिखो(stream, "border-left-width: thin ");
		रोक;
	पक्ष CSS_BORDER_WIDTH_MEDIUM:
		ख_लिखो(stream, "border-left-width: medium ");
		रोक;
	पक्ष CSS_BORDER_WIDTH_THICK:
		ख_लिखो(stream, "border-left-width: thick ");
		रोक;
	पक्ष CSS_BORDER_WIDTH_WIDTH:
		ख_लिखो(stream, "border-left-width: ");
		dump_css_unit(stream, len1, unit1);
		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* bottom */
	val = css_computed_bottom(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_BOTTOM_AUTO:
		ख_लिखो(stream, "bottom: auto ");
		रोक;
	पक्ष CSS_BOTTOM_SET:
		ख_लिखो(stream, "bottom: ");
		dump_css_unit(stream, len1, unit1);
		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* caption-side */
	val = css_computed_caption_side(style);
	विकल्प (val) {
	पक्ष CSS_CAPTION_SIDE_शीर्ष:
		ख_लिखो(stream, "caption_side: top ");
		रोक;
	पक्ष CSS_CAPTION_SIDE_BOTTOM:
		ख_लिखो(stream, "caption_side: bottom ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* clear */
	val = css_computed_clear(style);
	विकल्प (val) {
	पक्ष CSS_CLEAR_NONE:
		ख_लिखो(stream, "clear: none ");
		रोक;
	पक्ष CSS_CLEAR_बायाँ:
		ख_लिखो(stream, "clear: left ");
		रोक;
	पक्ष CSS_CLEAR_दायाँ:
		ख_लिखो(stream, "clear: right ");
		रोक;
	पक्ष CSS_CLEAR_BOTH:
		ख_लिखो(stream, "clear: both ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* clip */
	val = css_computed_clip(style, &rect);
	विकल्प (val) {
	पक्ष CSS_CLIP_AUTO:
		ख_लिखो(stream, "clip: auto ");
		रोक;
	पक्ष CSS_CLIP_RECT:
		ख_लिखो(stream, "clip: rect( ");

		यदि (rect.शीर्ष_auto)
			ख_लिखो(stream, "auto");
		अन्यथा
			dump_css_unit(stream, rect.शीर्ष, rect.tunit);
		ख_लिखो(stream, ", ");

		यदि (rect.दायाँ_auto)
			ख_लिखो(stream, "auto");
		अन्यथा
			dump_css_unit(stream, rect.दायाँ, rect.runit);
		ख_लिखो(stream, ", ");

		यदि (rect.bottom_auto)
			ख_लिखो(stream, "auto");
		अन्यथा
			dump_css_unit(stream, rect.bottom, rect.bunit);
		ख_लिखो(stream, ", ");

		यदि (rect.बायाँ_auto)
			ख_लिखो(stream, "auto");
		अन्यथा
			dump_css_unit(stream, rect.बायाँ, rect.lunit);
		ख_लिखो(stream, ") ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* color */
	val = css_computed_color(style, &color);
	यदि (val == CSS_COLOR_COLOR) {
		ख_लिखो(stream, "color: #%08"PRIx32" ", color);
	}

	/* content */
	val = css_computed_content(style, &content);
	विकल्प (val) {
	पक्ष CSS_CONTENT_NONE:
		ख_लिखो(stream, "content: none ");
		रोक;
	पक्ष CSS_CONTENT_NORMAL:
		ख_लिखो(stream, "content: normal ");
		रोक;
	पक्ष CSS_CONTENT_SET:
		ख_लिखो(stream, "content:");

		जबसम्म (content->type != CSS_COMPUTED_CONTENT_NONE) {
			ख_लिखो(stream, " ");

			विकल्प (content->type) {
			पक्ष CSS_COMPUTED_CONTENT_STRING:
				ख_लिखो(stream,	"\"%.*s\"",
						(पूर्णाङ्क) lwc_string_length(
						content->data.string),
						lwc_string_data(
						content->data.string));
				रोक;
			पक्ष CSS_COMPUTED_CONTENT_URI:
				ख_लिखो(stream,	"uri(\"%.*s\")",
						(पूर्णाङ्क) lwc_string_length(
						content->data.uri),
						lwc_string_data(
						content->data.uri));
				रोक;
			पक्ष CSS_COMPUTED_CONTENT_COUNTER:
				ख_लिखो(stream, "counter(%.*s)",
						(पूर्णाङ्क) lwc_string_length(
						content->data.counter.name),
						lwc_string_data(
						content->data.counter.name));
				रोक;
			पक्ष CSS_COMPUTED_CONTENT_COUNTERS:
				ख_लिखो(stream, "counters(%.*s, \"%.*s\")",
						(पूर्णाङ्क) lwc_string_length(
						content->data.counters.name),
						lwc_string_data(
						content->data.counters.name),
						(पूर्णाङ्क) lwc_string_length(
						content->data.counters.sep),
						lwc_string_data(
						content->data.counters.sep));
				रोक;
			पक्ष CSS_COMPUTED_CONTENT_ATTR:
				ख_लिखो(stream, "attr(%.*s)",
						(पूर्णाङ्क) lwc_string_length(
						content->data.attr),
						lwc_string_data(
						content->data.attr));
				रोक;
			पक्ष CSS_COMPUTED_CONTENT_OPEN_QUOTE:
				ख_लिखो(stream, "open-quote");
				रोक;
			पक्ष CSS_COMPUTED_CONTENT_CLOSE_QUOTE:
				ख_लिखो(stream, "close-quote");
				रोक;
			पक्ष CSS_COMPUTED_CONTENT_NO_OPEN_QUOTE:
				ख_लिखो(stream, "no-open-quote");
				रोक;
			पक्ष CSS_COMPUTED_CONTENT_NO_CLOSE_QUOTE:
				ख_लिखो(stream, "no-close-quote");
				रोक;
			}

			content++;
		}

		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* counter-increment */
	val = css_computed_counter_increment(style, &counter);
	यदि ((val == CSS_COUNTER_INCREMENT_NONE) || (counter == रिक्त)) {
		ख_लिखो(stream, "counter-increment: none ");
	} अन्यथा {
		ख_लिखो(stream, "counter-increment:");

		जबसम्म (counter->name != रिक्त) {
			ख_लिखो(stream, " %.*s ",
					(पूर्णाङ्क) lwc_string_length(counter->name),
					lwc_string_data(counter->name));

			dump_css_fixed(stream, counter->मान);

			counter++;
		}

		ख_लिखो(stream, " ");
	}

	/* counter-reset */
	val = css_computed_counter_reset(style, &counter);
	यदि ((val == CSS_COUNTER_RESET_NONE) || (counter == रिक्त)) {
		ख_लिखो(stream, "counter-reset: none ");
	} अन्यथा {
		ख_लिखो(stream, "counter-reset:");

		जबसम्म (counter->name != रिक्त) {
			ख_लिखो(stream, " %.*s ",
					(पूर्णाङ्क) lwc_string_length(counter->name),
					lwc_string_data(counter->name));

			dump_css_fixed(stream, counter->मान);

			counter++;
		}

		ख_लिखो(stream, " ");
	}

	/* cursor */
	val = css_computed_cursor(style, &string_list);
	ख_लिखो(stream, "cursor:");

	यदि (string_list != रिक्त) {
		जबसम्म (*string_list != रिक्त) {
			ख_लिखो(stream, " url\"%.*s\")",
					(पूर्णाङ्क) lwc_string_length(*string_list),
					lwc_string_data(*string_list));

			string_list++;
		}
	}
	विकल्प (val) {
	पक्ष CSS_CURSOR_AUTO:
		ख_लिखो(stream, " auto ");
		रोक;
	पक्ष CSS_CURSOR_CROSSHAIR:
		ख_लिखो(stream, " crosshair ");
		रोक;
	पक्ष CSS_CURSOR_DEFAULT:
		ख_लिखो(stream, " default ");
		रोक;
	पक्ष CSS_CURSOR_POINTER:
		ख_लिखो(stream, " pointer ");
		रोक;
	पक्ष CSS_CURSOR_MOVE:
		ख_लिखो(stream, " move ");
		रोक;
	पक्ष CSS_CURSOR_E_RESIZE:
		ख_लिखो(stream, " e-resize ");
		रोक;
	पक्ष CSS_CURSOR_NE_RESIZE:
		ख_लिखो(stream, " ne-resize ");
		रोक;
	पक्ष CSS_CURSOR_NW_RESIZE:
		ख_लिखो(stream, " nw-resize ");
		रोक;
	पक्ष CSS_CURSOR_N_RESIZE:
		ख_लिखो(stream, " n-resize ");
		रोक;
	पक्ष CSS_CURSOR_SE_RESIZE:
		ख_लिखो(stream, " se-resize ");
		रोक;
	पक्ष CSS_CURSOR_SW_RESIZE:
		ख_लिखो(stream, " sw-resize ");
		रोक;
	पक्ष CSS_CURSOR_S_RESIZE:
		ख_लिखो(stream, " s-resize ");
		रोक;
	पक्ष CSS_CURSOR_W_RESIZE:
		ख_लिखो(stream, " w-resize ");
		रोक;
	पक्ष CSS_CURSOR_TEXT:
		ख_लिखो(stream, " text ");
		रोक;
	पक्ष CSS_CURSOR_WAIT:
		ख_लिखो(stream, " wait ");
		रोक;
	पक्ष CSS_CURSOR_HELP:
		ख_लिखो(stream, " help ");
		रोक;
	पक्ष CSS_CURSOR_PROGRESS:
		ख_लिखो(stream, " progress ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* direction */
	val = css_computed_direction(style);
	विकल्प (val) {
	पक्ष CSS_DIRECTION_LTR:
		ख_लिखो(stream, "direction: ltr ");
		रोक;
	पक्ष CSS_DIRECTION_RTL:
		ख_लिखो(stream, "direction: rtl ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* display */
	val = ns_computed_display_static(style);
	विकल्प (val) {
	पक्ष CSS_DISPLAY_INLINE:
		ख_लिखो(stream, "display: inline ");
		रोक;
	पक्ष CSS_DISPLAY_BLOCK:
		ख_लिखो(stream, "display: block ");
		रोक;
	पक्ष CSS_DISPLAY_LIST_ITEM:
		ख_लिखो(stream, "display: list-item ");
		रोक;
	पक्ष CSS_DISPLAY_RUN_IN:
		ख_लिखो(stream, "display: run-in ");
		रोक;
	पक्ष CSS_DISPLAY_INLINE_BLOCK:
		ख_लिखो(stream, "display: inline-block ");
		रोक;
	पक्ष CSS_DISPLAY_TABLE:
		ख_लिखो(stream, "display: table ");
		रोक;
	पक्ष CSS_DISPLAY_INLINE_TABLE:
		ख_लिखो(stream, "display: inline-table ");
		रोक;
	पक्ष CSS_DISPLAY_TABLE_ROW_GROUP:
		ख_लिखो(stream, "display: table-row-group ");
		रोक;
	पक्ष CSS_DISPLAY_TABLE_HEADER_GROUP:
		ख_लिखो(stream, "display: table-header-group ");
		रोक;
	पक्ष CSS_DISPLAY_TABLE_FOOTER_GROUP:
		ख_लिखो(stream, "display: table-footer-group ");
		रोक;
	पक्ष CSS_DISPLAY_TABLE_ROW:
		ख_लिखो(stream, "display: table-row ");
		रोक;
	पक्ष CSS_DISPLAY_TABLE_COLUMN_GROUP:
		ख_लिखो(stream, "display: table-column-group ");
		रोक;
	पक्ष CSS_DISPLAY_TABLE_COLUMN:
		ख_लिखो(stream, "display: table-column ");
		रोक;
	पक्ष CSS_DISPLAY_TABLE_CELL:
		ख_लिखो(stream, "display: table-cell ");
		रोक;
	पक्ष CSS_DISPLAY_TABLE_CAPTION:
		ख_लिखो(stream, "display: table-caption ");
		रोक;
	पक्ष CSS_DISPLAY_NONE:
		ख_लिखो(stream, "display: none ");
		रोक;
	पक्ष CSS_DISPLAY_FLEX:
		ख_लिखो(stream, "display: flex ");
		रोक;
	पक्ष CSS_DISPLAY_INLINE_FLEX:
		ख_लिखो(stream, "display: inline-flex ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* empty-cells */
	val = css_computed_empty_cells(style);
	विकल्प (val) {
	पक्ष CSS_EMPTY_CELLS_SHOW:
		ख_लिखो(stream, "empty-cells: show ");
		रोक;
	पक्ष CSS_EMPTY_CELLS_HIDE:
		ख_लिखो(stream, "empty-cells: hide ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* float */
	val = css_computed_float(style);
	विकल्प (val) {
	पक्ष CSS_FLOAT_बायाँ:
		ख_लिखो(stream, "float: left ");
		रोक;
	पक्ष CSS_FLOAT_दायाँ:
		ख_लिखो(stream, "float: right ");
		रोक;
	पक्ष CSS_FLOAT_NONE:
		ख_लिखो(stream, "float: none ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* font-family */
	val = css_computed_font_family(style, &string_list);
	यदि (val != CSS_FONT_FAMILY_INHERIT) {
		ख_लिखो(stream, "font-family:");

		यदि (string_list != रिक्त) {
			जबसम्म (*string_list != रिक्त) {
				ख_लिखो(stream, " \"%.*s\"",
					(पूर्णाङ्क) lwc_string_length(*string_list),
					lwc_string_data(*string_list));

				string_list++;
			}
		}
		विकल्प (val) {
		पक्ष CSS_FONT_FAMILY_SERIF:
			ख_लिखो(stream, " serif ");
			रोक;
		पक्ष CSS_FONT_FAMILY_SANS_SERIF:
			ख_लिखो(stream, " sans-serif ");
			रोक;
		पक्ष CSS_FONT_FAMILY_CURSIVE:
			ख_लिखो(stream, " cursive ");
			रोक;
		पक्ष CSS_FONT_FAMILY_FANTASY:
			ख_लिखो(stream, " fantasy ");
			रोक;
		पक्ष CSS_FONT_FAMILY_MONOSPACE:
			ख_लिखो(stream, " monospace ");
			रोक;
		}
	}

	/* font-size */
	val = css_computed_font_size(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_FONT_SIZE_XX_SMALL:
		ख_लिखो(stream, "font-size: xx-small ");
		रोक;
	पक्ष CSS_FONT_SIZE_X_SMALL:
		ख_लिखो(stream, "font-size: x-small ");
		रोक;
	पक्ष CSS_FONT_SIZE_SMALL:
		ख_लिखो(stream, "font-size: small ");
		रोक;
	पक्ष CSS_FONT_SIZE_MEDIUM:
		ख_लिखो(stream, "font-size: medium ");
		रोक;
	पक्ष CSS_FONT_SIZE_LARGE:
		ख_लिखो(stream, "font-size: large ");
		रोक;
	पक्ष CSS_FONT_SIZE_X_LARGE:
		ख_लिखो(stream, "font-size: x-large ");
		रोक;
	पक्ष CSS_FONT_SIZE_XX_LARGE:
		ख_लिखो(stream, "font-size: xx-large ");
		रोक;
	पक्ष CSS_FONT_SIZE_LARGER:
		ख_लिखो(stream, "font-size: larger ");
		रोक;
	पक्ष CSS_FONT_SIZE_SMALLER:
		ख_लिखो(stream, "font-size: smaller ");
		रोक;
	पक्ष CSS_FONT_SIZE_DIMENSION:
		ख_लिखो(stream, "font-size: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* font-style */
	val = css_computed_font_style(style);
	विकल्प (val) {
	पक्ष CSS_FONT_STYLE_NORMAL:
		ख_लिखो(stream, "font-style: normal ");
		रोक;
	पक्ष CSS_FONT_STYLE_ITALIC:
		ख_लिखो(stream, "font-style: italic ");
		रोक;
	पक्ष CSS_FONT_STYLE_OBLIQUE:
		ख_लिखो(stream, "font-style: oblique ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* font-variant */
	val = css_computed_font_variant(style);
	विकल्प (val) {
	पक्ष CSS_FONT_VARIANT_NORMAL:
		ख_लिखो(stream, "font-variant: normal ");
		रोक;
	पक्ष CSS_FONT_VARIANT_SMALL_CAPS:
		ख_लिखो(stream, "font-variant: small-caps ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* font-weight */
	val = css_computed_font_weight(style);
	विकल्प (val) {
	पक्ष CSS_FONT_WEIGHT_NORMAL:
		ख_लिखो(stream, "font-weight: normal ");
		रोक;
	पक्ष CSS_FONT_WEIGHT_BOLD:
		ख_लिखो(stream, "font-weight: bold ");
		रोक;
	पक्ष CSS_FONT_WEIGHT_BOLDER:
		ख_लिखो(stream, "font-weight: bolder ");
		रोक;
	पक्ष CSS_FONT_WEIGHT_LIGHTER:
		ख_लिखो(stream, "font-weight: lighter ");
		रोक;
	पक्ष CSS_FONT_WEIGHT_100:
		ख_लिखो(stream, "font-weight: 100 ");
		रोक;
	पक्ष CSS_FONT_WEIGHT_200:
		ख_लिखो(stream, "font-weight: 200 ");
		रोक;
	पक्ष CSS_FONT_WEIGHT_300:
		ख_लिखो(stream, "font-weight: 300 ");
		रोक;
	पक्ष CSS_FONT_WEIGHT_400:
		ख_लिखो(stream, "font-weight: 400 ");
		रोक;
	पक्ष CSS_FONT_WEIGHT_500:
		ख_लिखो(stream, "font-weight: 500 ");
		रोक;
	पक्ष CSS_FONT_WEIGHT_600:
		ख_लिखो(stream, "font-weight: 600 ");
		रोक;
	पक्ष CSS_FONT_WEIGHT_700:
		ख_लिखो(stream, "font-weight: 700 ");
		रोक;
	पक्ष CSS_FONT_WEIGHT_800:
		ख_लिखो(stream, "font-weight: 800 ");
		रोक;
	पक्ष CSS_FONT_WEIGHT_900:
		ख_लिखो(stream, "font-weight: 900 ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* height */
	val = css_computed_height(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_HEIGHT_AUTO:
		ख_लिखो(stream, "height: auto ");
		रोक;
	पक्ष CSS_HEIGHT_SET:
		ख_लिखो(stream, "height: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/*बायाँ*/
	val = css_computed_बायाँ(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_बायाँ_AUTO:
		ख_लिखो(stream, "left: auto ");
		रोक;
	पक्ष CSS_बायाँ_SET:
		ख_लिखो(stream, "left: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* letter-spacing */
	val = css_computed_letter_spacing(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_LETTER_SPACING_NORMAL:
		ख_लिखो(stream, "letter-spacing: normal ");
		रोक;
	पक्ष CSS_LETTER_SPACING_SET:
		ख_लिखो(stream, "letter-spacing: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* line-height */
	val = css_computed_पङ्क्ति_height(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_पङ्क्ति_HEIGHT_NORMAL:
		ख_लिखो(stream, "line-height: normal ");
		रोक;
	पक्ष CSS_पङ्क्ति_HEIGHT_सङ्ख्या:
		ख_लिखो(stream, "line-height: ");

		dump_css_fixed(stream, len1);

		ख_लिखो(stream, " ");
		रोक;
	पक्ष CSS_पङ्क्ति_HEIGHT_DIMENSION:
		ख_लिखो(stream, "line-height: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* list-style-image */
	val = css_computed_list_style_image(style, &url);
	यदि (url != रिक्त) {
		ख_लिखो(stream, "list-style-image: url('%.*s') ",
				(पूर्णाङ्क) lwc_string_length(url),
				lwc_string_data(url));
	} अन्यथा यदि (val == CSS_LIST_STYLE_IMAGE_NONE) {
		ख_लिखो(stream, "list-style-image: none ");
	}

	/* list-style-position */
	val = css_computed_list_style_position(style);
	विकल्प (val) {
	पक्ष CSS_LIST_STYLE_POSITION_INSIDE:
		ख_लिखो(stream, "list-style-position: inside ");
		रोक;
	पक्ष CSS_LIST_STYLE_POSITION_OUTSIDE:
		ख_लिखो(stream, "list-style-position: outside ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* list-style-type */
	val = css_computed_list_style_type(style);
	विकल्प (val) {
	पक्ष CSS_LIST_STYLE_TYPE_DISC:
		ख_लिखो(stream, "list-style-type: disc ");
		रोक;
	पक्ष CSS_LIST_STYLE_TYPE_CIRCLE:
		ख_लिखो(stream, "list-style-type: circle ");
		रोक;
	पक्ष CSS_LIST_STYLE_TYPE_SQUARE:
		ख_लिखो(stream, "list-style-type: square ");
		रोक;
	पक्ष CSS_LIST_STYLE_TYPE_DECIMAL:
		ख_लिखो(stream, "list-style-type: decimal ");
		रोक;
	पक्ष CSS_LIST_STYLE_TYPE_DECIMAL_LEADING_ZERO:
		ख_लिखो(stream, "list-style-type: decimal-leading-zero ");
		रोक;
	पक्ष CSS_LIST_STYLE_TYPE_LOWER_ROMAN:
		ख_लिखो(stream, "list-style-type: lower-roman ");
		रोक;
	पक्ष CSS_LIST_STYLE_TYPE_UPPER_ROMAN:
		ख_लिखो(stream, "list-style-type: upper-roman ");
		रोक;
	पक्ष CSS_LIST_STYLE_TYPE_LOWER_GREEK:
		ख_लिखो(stream, "list-style-type: lower-greek ");
		रोक;
	पक्ष CSS_LIST_STYLE_TYPE_LOWER_LATIN:
		ख_लिखो(stream, "list-style-type: lower-latin ");
		रोक;
	पक्ष CSS_LIST_STYLE_TYPE_UPPER_LATIN:
		ख_लिखो(stream, "list-style-type: upper-latin ");
		रोक;
	पक्ष CSS_LIST_STYLE_TYPE_ARMENIAN:
		ख_लिखो(stream, "list-style-type: armenian ");
		रोक;
	पक्ष CSS_LIST_STYLE_TYPE_GEORGIAN:
		ख_लिखो(stream, "list-style-type: georgian ");
		रोक;
	पक्ष CSS_LIST_STYLE_TYPE_LOWER_ALPHA:
		ख_लिखो(stream, "list-style-type: lower-alpha ");
		रोक;
	पक्ष CSS_LIST_STYLE_TYPE_UPPER_ALPHA:
		ख_लिखो(stream, "list-style-type: upper-alpha ");
		रोक;
	पक्ष CSS_LIST_STYLE_TYPE_NONE:
		ख_लिखो(stream, "list-style-type: none ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* margin-top */
	val = css_computed_margin_शीर्ष(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_MARGIN_AUTO:
		ख_लिखो(stream, "margin-top: auto ");
		रोक;
	पक्ष CSS_MARGIN_SET:
		ख_लिखो(stream, "margin-top: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* margin-right */
	val = css_computed_margin_दायाँ(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_MARGIN_AUTO:
		ख_लिखो(stream, "margin-right: auto ");
		रोक;
	पक्ष CSS_MARGIN_SET:
		ख_लिखो(stream, "margin-right: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* margin-bottom */
	val = css_computed_margin_bottom(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_MARGIN_AUTO:
		ख_लिखो(stream, "margin-bottom: auto ");
		रोक;
	पक्ष CSS_MARGIN_SET:
		ख_लिखो(stream, "margin-bottom: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* margin-left */
	val = css_computed_margin_बायाँ(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_MARGIN_AUTO:
		ख_लिखो(stream, "margin-left: auto ");
		रोक;
	पक्ष CSS_MARGIN_SET:
		ख_लिखो(stream, "margin-left: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* max-height */
	val = css_computed_max_height(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_MAX_HEIGHT_NONE:
		ख_लिखो(stream, "max-height: none ");
		रोक;
	पक्ष CSS_MAX_HEIGHT_SET:
		ख_लिखो(stream, "max-height: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* max-width */
	val = css_computed_max_width(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_MAX_WIDTH_NONE:
		ख_लिखो(stream, "max-width: none ");
		रोक;
	पक्ष CSS_MAX_WIDTH_SET:
		ख_लिखो(stream, "max-width: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* min-height */
	val = ns_computed_min_height(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_MIN_HEIGHT_SET:
		ख_लिखो(stream, "min-height: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* min-width */
	val = ns_computed_min_width(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_MIN_WIDTH_SET:
		ख_लिखो(stream, "min-width: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* opacity */
	val = css_computed_opacity(style, &len1);
	विकल्प (val) {
	पक्ष CSS_OPACITY_SET:
		ख_लिखो(stream, "opacity: ");

		dump_css_fixed(stream, len1);

		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* outline-color */
	val = css_computed_outline_color(style, &color);
	विकल्प (val) {
	पक्ष CSS_OUTLINE_COLOR_INVERT:
		ख_लिखो(stream, "outline-color: invert ");
		रोक;
	पक्ष CSS_OUTLINE_COLOR_COLOR:
		ख_लिखो(stream, "outline-color: #%08"PRIx32" ", color);
		रोक;
	स्वभाव:
		रोक;
	}

	/* outline-style */
	val = css_computed_outline_style(style);
	विकल्प (val) {
	पक्ष CSS_OUTLINE_STYLE_NONE:
		ख_लिखो(stream, "outline-style: none ");
		रोक;
	पक्ष CSS_OUTLINE_STYLE_DOTTED:
		ख_लिखो(stream, "outline-style: dotted ");
		रोक;
	पक्ष CSS_OUTLINE_STYLE_DASHED:
		ख_लिखो(stream, "outline-style: dashed ");
		रोक;
	पक्ष CSS_OUTLINE_STYLE_SOLID:
		ख_लिखो(stream, "outline-style: solid ");
		रोक;
	पक्ष CSS_OUTLINE_STYLE_DOUBLE:
		ख_लिखो(stream, "outline-style: double ");
		रोक;
	पक्ष CSS_OUTLINE_STYLE_GROOVE:
		ख_लिखो(stream, "outline-style: groove ");
		रोक;
	पक्ष CSS_OUTLINE_STYLE_RIDGE:
		ख_लिखो(stream, "outline-style: ridge ");
		रोक;
	पक्ष CSS_OUTLINE_STYLE_INSET:
		ख_लिखो(stream, "outline-style: inset ");
		रोक;
	पक्ष CSS_OUTLINE_STYLE_OUTSET:
		ख_लिखो(stream, "outline-style: outset ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* outline-width */
	val = css_computed_outline_width(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_OUTLINE_WIDTH_THIN:
		ख_लिखो(stream, "outline-width: thin ");
		रोक;
	पक्ष CSS_OUTLINE_WIDTH_MEDIUM:
		ख_लिखो(stream, "outline-width: medium ");
		रोक;
	पक्ष CSS_OUTLINE_WIDTH_THICK:
		ख_लिखो(stream, "outline-width: thick ");
		रोक;
	पक्ष CSS_OUTLINE_WIDTH_WIDTH:
		ख_लिखो(stream, "outline-width: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* overflow */
	val = css_computed_overflow_x(style);
	विकल्प (val) {
	पक्ष CSS_OVERFLOW_VISIBLE:
		ख_लिखो(stream, "overflow-x: visible ");
		रोक;
	पक्ष CSS_OVERFLOW_HIDDEN:
		ख_लिखो(stream, "overflow-x: hidden ");
		रोक;
	पक्ष CSS_OVERFLOW_SCROLL:
		ख_लिखो(stream, "overflow-x: scroll ");
		रोक;
	पक्ष CSS_OVERFLOW_AUTO:
		ख_लिखो(stream, "overflow-x auto ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* overflow */
	val = css_computed_overflow_y(style);
	विकल्प (val) {
	पक्ष CSS_OVERFLOW_VISIBLE:
		ख_लिखो(stream, "overflow-y: visible ");
		रोक;
	पक्ष CSS_OVERFLOW_HIDDEN:
		ख_लिखो(stream, "overflow-y: hidden ");
		रोक;
	पक्ष CSS_OVERFLOW_SCROLL:
		ख_लिखो(stream, "overflow-y: scroll ");
		रोक;
	पक्ष CSS_OVERFLOW_AUTO:
		ख_लिखो(stream, "overflow-y: auto ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* padding-top */
	val = css_computed_padding_शीर्ष(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_PADDING_SET:
		ख_लिखो(stream, "padding-top: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* padding-right */
	val = css_computed_padding_दायाँ(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_PADDING_SET:
		ख_लिखो(stream, "padding-right: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* padding-bottom */
	val = css_computed_padding_bottom(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_PADDING_SET:
		ख_लिखो(stream, "padding-bottom: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* padding-left */
	val = css_computed_padding_बायाँ(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_PADDING_SET:
		ख_लिखो(stream, "padding-left: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* position */
	val = css_computed_position(style);
	विकल्प (val) {
	पक्ष CSS_POSITION_STATIC:
		ख_लिखो(stream, "position: static ");
		रोक;
	पक्ष CSS_POSITION_RELATIVE:
		ख_लिखो(stream, "position: relative ");
		रोक;
	पक्ष CSS_POSITION_ABSOLUTE:
		ख_लिखो(stream, "position: absolute ");
		रोक;
	पक्ष CSS_POSITION_FIXED:
		ख_लिखो(stream, "position: fixed ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* quotes */
	val = css_computed_quotes(style, &string_list);
	यदि (val == CSS_QUOTES_STRING && string_list != रिक्त) {
		ख_लिखो(stream, "quotes:");

		जबसम्म (*string_list != रिक्त) {
			ख_लिखो(stream, " \"%.*s\"",
				(पूर्णाङ्क) lwc_string_length(*string_list),
				lwc_string_data(*string_list));

			string_list++;
		}

		ख_लिखो(stream, " ");
	} अन्यथा {
		विकल्प (val) {
		पक्ष CSS_QUOTES_NONE:
			ख_लिखो(stream, "quotes: none ");
			रोक;
		स्वभाव:
			रोक;
		}
	}

	/*दायाँ*/
	val = css_computed_दायाँ(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_दायाँ_AUTO:
		ख_लिखो(stream, "right: auto ");
		रोक;
	पक्ष CSS_दायाँ_SET:
		ख_लिखो(stream, "right: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* table-layout */
	val = css_computed_table_layout(style);
	विकल्प (val) {
	पक्ष CSS_TABLE_LAYOUT_AUTO:
		ख_लिखो(stream, "table-layout: auto ");
		रोक;
	पक्ष CSS_TABLE_LAYOUT_FIXED:
		ख_लिखो(stream, "table-layout: fixed ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* text-align */
	val = css_computed_text_align(style);
	विकल्प (val) {
	पक्ष CSS_TEXT_ALIGN_बायाँ:
		ख_लिखो(stream, "text-align: left ");
		रोक;
	पक्ष CSS_TEXT_ALIGN_दायाँ:
		ख_लिखो(stream, "text-align: right ");
		रोक;
	पक्ष CSS_TEXT_ALIGN_CENTER:
		ख_लिखो(stream, "text-align: center ");
		रोक;
	पक्ष CSS_TEXT_ALIGN_JUSTIFY:
		ख_लिखो(stream, "text-align: justify ");
		रोक;
	पक्ष CSS_TEXT_ALIGN_DEFAULT:
		ख_लिखो(stream, "text-align: default ");
		रोक;
	पक्ष CSS_TEXT_ALIGN_LIBCSS_बायाँ:
		ख_लिखो(stream, "text-align: -libcss-left ");
		रोक;
	पक्ष CSS_TEXT_ALIGN_LIBCSS_CENTER:
		ख_लिखो(stream, "text-align: -libcss-center ");
		रोक;
	पक्ष CSS_TEXT_ALIGN_LIBCSS_दायाँ:
		ख_लिखो(stream, "text-align: -libcss-right ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* text-decoration */
	val = css_computed_text_decoration(style);
	यदि (val == CSS_TEXT_DECORATION_NONE) {
		ख_लिखो(stream, "text-decoration: none ");
	} अन्यथा {
		ख_लिखो(stream, "text-decoration:");

		यदि (val & CSS_TEXT_DECORATION_BLINK) {
			ख_लिखो(stream, " blink");
		}
		यदि (val & CSS_TEXT_DECORATION_पङ्क्ति_THROUGH) {
			ख_लिखो(stream, " line-through");
		}
		यदि (val & CSS_TEXT_DECORATION_OVERLINE) {
			ख_लिखो(stream, " overline");
		}
		यदि (val & CSS_TEXT_DECORATION_UNDERLINE) {
			ख_लिखो(stream, " underline");
		}

		ख_लिखो(stream, " ");
	}

	/* text-indent */
	val = css_computed_text_indent(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_TEXT_INDENT_SET:
		ख_लिखो(stream, "text-indent: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* text-transform */
	val = css_computed_text_transform(style);
	विकल्प (val) {
	पक्ष CSS_TEXT_TRANSFORM_CAPITALIZE:
		ख_लिखो(stream, "text-transform: capitalize ");
		रोक;
	पक्ष CSS_TEXT_TRANSFORM_UPPERCASE:
		ख_लिखो(stream, "text-transform: uppercase ");
		रोक;
	पक्ष CSS_TEXT_TRANSFORM_LOWERCASE:
		ख_लिखो(stream, "text-transform: lowercase ");
		रोक;
	पक्ष CSS_TEXT_TRANSFORM_NONE:
		ख_लिखो(stream, "text-transform: none ");
		रोक;
	स्वभाव:
		रोक;
	}

	/*शीर्ष*/
	val = css_computed_शीर्ष(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_शीर्ष_AUTO:
		ख_लिखो(stream, "top: auto ");
		रोक;
	पक्ष CSS_शीर्ष_SET:
		ख_लिखो(stream, "top: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* unicode-bidi */
	val = css_computed_unicode_bidi(style);
	विकल्प (val) {
	पक्ष CSS_UNICODE_BIDI_NORMAL:
		ख_लिखो(stream, "unicode-bidi: normal ");
		रोक;
	पक्ष CSS_UNICODE_BIDI_EMBED:
		ख_लिखो(stream, "unicode-bidi: embed ");
		रोक;
	पक्ष CSS_UNICODE_BIDI_BIDI_OVERRIDE:
		ख_लिखो(stream, "unicode-bidi: bidi-override ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* vertical-align */
	val = css_computed_vertical_align(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_VERTICAL_ALIGN_BASELINE:
		ख_लिखो(stream, "vertical-align: baseline ");
		रोक;
	पक्ष CSS_VERTICAL_ALIGN_SUB:
		ख_लिखो(stream, "vertical-align: sub ");
		रोक;
	पक्ष CSS_VERTICAL_ALIGN_SUPER:
		ख_लिखो(stream, "vertical-align: super ");
		रोक;
	पक्ष CSS_VERTICAL_ALIGN_शीर्ष:
		ख_लिखो(stream, "vertical-align: top ");
		रोक;
	पक्ष CSS_VERTICAL_ALIGN_TEXT_शीर्ष:
		ख_लिखो(stream, "vertical-align: text-top ");
		रोक;
	पक्ष CSS_VERTICAL_ALIGN_बीच:
		ख_लिखो(stream, "vertical-align: middle ");
		रोक;
	पक्ष CSS_VERTICAL_ALIGN_BOTTOM:
		ख_लिखो(stream, "vertical-align: bottom ");
		रोक;
	पक्ष CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
		ख_लिखो(stream, "vertical-align: text-bottom ");
		रोक;
	पक्ष CSS_VERTICAL_ALIGN_SET:
		ख_लिखो(stream, "vertical-align: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* visibility */
	val = css_computed_visibility(style);
	विकल्प (val) {
	पक्ष CSS_VISIBILITY_VISIBLE:
		ख_लिखो(stream, "visibility: visible ");
		रोक;
	पक्ष CSS_VISIBILITY_HIDDEN:
		ख_लिखो(stream, "visibility: hidden ");
		रोक;
	पक्ष CSS_VISIBILITY_COLLAPSE:
		ख_लिखो(stream, "visibility: collapse ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* white-space */
	val = css_computed_white_space(style);
	विकल्प (val) {
	पक्ष CSS_WHITE_SPACE_NORMAL:
		ख_लिखो(stream, "white-space: normal ");
		रोक;
	पक्ष CSS_WHITE_SPACE_PRE:
		ख_लिखो(stream, "white-space: pre ");
		रोक;
	पक्ष CSS_WHITE_SPACE_NOWRAP:
		ख_लिखो(stream, "white-space: nowrap ");
		रोक;
	पक्ष CSS_WHITE_SPACE_PRE_WRAP:
		ख_लिखो(stream, "white-space: pre-wrap ");
		रोक;
	पक्ष CSS_WHITE_SPACE_PRE_पङ्क्ति:
		ख_लिखो(stream, "white-space: pre-line ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* width */
	val = css_computed_width(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_WIDTH_AUTO:
		ख_लिखो(stream, "width: auto ");
		रोक;
	पक्ष CSS_WIDTH_SET:
		ख_लिखो(stream, "width: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* word-spacing */
	val = css_computed_word_spacing(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_WORD_SPACING_NORMAL:
		ख_लिखो(stream, "word-spacing: normal ");
		रोक;
	पक्ष CSS_WORD_SPACING_SET:
		ख_लिखो(stream, "word-spacing: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		रोक;
	स्वभाव:
		रोक;
	}

	/* z-index */
	val = css_computed_z_index(style, &zindex);
	विकल्प (val) {
	पक्ष CSS_Z_INDEX_AUTO:
		ख_लिखो(stream, "z-index: auto ");
		रोक;
	पक्ष CSS_Z_INDEX_SET:
		ख_लिखो(stream, "z-index: %"PRId32" ", zindex);
		रोक;
	स्वभाव:
		रोक;
	}

	ख_लिखो(stream, "}");
}
