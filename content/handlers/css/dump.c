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
स्थिर शून्य dump_css_संख्या(खाता *stream, css_fixed val)
{
	जर (INTTOFIX(FIXTOINT(val)) == val)
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
	dump_css_संख्या(stream, val);

	निवड (unit) {
	स्थिती CSS_UNIT_PX:
		ख_लिखो(stream, "px");
		थांबा;
	स्थिती CSS_UNIT_EX:
		ख_लिखो(stream, "ex");
		थांबा;
	स्थिती CSS_UNIT_EM:
		ख_लिखो(stream, "em");
		थांबा;
	स्थिती CSS_UNIT_IN:
		ख_लिखो(stream, "in");
		थांबा;
	स्थिती CSS_UNIT_CM:
		ख_लिखो(stream, "cm");
		थांबा;
	स्थिती CSS_UNIT_MM:
		ख_लिखो(stream, "mm");
		थांबा;
	स्थिती CSS_UNIT_PT:
		ख_लिखो(stream, "pt");
		थांबा;
	स्थिती CSS_UNIT_PC:
		ख_लिखो(stream, "pc");
		थांबा;
	स्थिती CSS_UNIT_PCT:
		ख_लिखो(stream, "%%");
		थांबा;
	स्थिती CSS_UNIT_DEG:
		ख_लिखो(stream, "deg");
		थांबा;
	स्थिती CSS_UNIT_GRAD:
		ख_लिखो(stream, "grad");
		थांबा;
	स्थिती CSS_UNIT_RAD:
		ख_लिखो(stream, "rad");
		थांबा;
	स्थिती CSS_UNIT_MS:
		ख_लिखो(stream, "ms");
		थांबा;
	स्थिती CSS_UNIT_S:
		ख_लिखो(stream, "s");
		थांबा;
	स्थिती CSS_UNIT_HZ:
		ख_लिखो(stream, "Hz");
		थांबा;
	स्थिती CSS_UNIT_KHZ:
		ख_लिखो(stream, "kHz");
		थांबा;
	स्थिती CSS_UNIT_CH:
		ख_लिखो(stream, "ch");
		थांबा;
	स्थिती CSS_UNIT_REM:
		ख_लिखो(stream, "rem");
		थांबा;
	स्थिती CSS_UNIT_LH:
		ख_लिखो(stream, "lh");
		थांबा;
	स्थिती CSS_UNIT_VH:
		ख_लिखो(stream, "vh");
		थांबा;
	स्थिती CSS_UNIT_VW:
		ख_लिखो(stream, "vw");
		थांबा;
	स्थिती CSS_UNIT_VI:
		ख_लिखो(stream, "vi");
		थांबा;
	स्थिती CSS_UNIT_VB:
		ख_लिखो(stream, "vb");
		थांबा;
	स्थिती CSS_UNIT_VMIN:
		ख_लिखो(stream, "vmin");
		थांबा;
	स्थिती CSS_UNIT_VMAX:
		ख_लिखो(stream, "vmax");
		थांबा;
	स्थिती CSS_UNIT_Q:
		ख_लिखो(stream, "q");
		थांबा;
	स्थिती CSS_UNIT_CALC:
		ख_लिखो(stream, "calc()");
		थांबा;
	}
}

/* exported interface documented in content/handlers/css/dump.h */
शून्य nscss_dump_computed_style(खाता *stream, स्थिरांक css_computed_style *style)
{
	uint8_t val;
	css_color color = 0;
	lwc_string *url = रिक्त;
	css_fixed len1 = 0, len2 = 0;
	css_unit unit1 = CSS_UNIT_PX, unit2 = CSS_UNIT_PX;
	css_computed_clip_rect rect = { 0, 0, 0, 0, CSS_UNIT_PX, CSS_UNIT_PX,
					CSS_UNIT_PX, CSS_UNIT_PX, true, true,
					true, true };
	स्थिरांक css_computed_content_item *content = रिक्त;
	स्थिरांक css_computed_counter *counter = रिक्त;
	lwc_string **string_list = रिक्त;
	int32_t zindex = 0;

	ख_लिखो(stream, "{ ");

	/* background-attachment */
	val = css_computed_background_attachment(style);
	निवड (val) {
	स्थिती CSS_BACKGROUND_ATTACHMENT_FIXED:
		ख_लिखो(stream, "background-attachment: fixed ");
		थांबा;
	स्थिती CSS_BACKGROUND_ATTACHMENT_SCROLL:
		ख_लिखो(stream, "background-attachment: scroll ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* background-color */
	val = css_computed_background_color(style, &color);
	निवड (val) {
	स्थिती CSS_BACKGROUND_COLOR_COLOR:
		ख_लिखो(stream, "background-color: #%08"PRIx32" ", color);
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* background-image */
	val = css_computed_background_image(style, &url);
	जर (val == CSS_BACKGROUND_IMAGE_IMAGE && url != रिक्त) {
		ख_लिखो(stream, "background-image: url('%.*s') ",
				(पूर्णांक) lwc_string_length(url),
				lwc_string_data(url));
	} अन्यथा जर (val == CSS_BACKGROUND_IMAGE_NONE) {
		ख_लिखो(stream, "background-image: none ");
	}

	/* background-position */
	val = css_computed_background_position(style, &len1, &unit1,
			&len2, &unit2);
	जर (val == CSS_BACKGROUND_POSITION_SET) {
		ख_लिखो(stream, "background-position: ");
		dump_css_unit(stream, len1, unit1);
		ख_लिखो(stream, " ");
		dump_css_unit(stream, len2, unit2);
		ख_लिखो(stream, " ");
	}

	/* background-repeat */
	val = css_computed_background_repeat(style);
	निवड (val) {
	स्थिती CSS_BACKGROUND_REPEAT_REPEAT_X:
		ख_लिखो(stream, "background-repeat: repeat-x ");
		थांबा;
	स्थिती CSS_BACKGROUND_REPEAT_REPEAT_Y:
		ख_लिखो(stream, "background-repeat: repeat-y ");
		थांबा;
	स्थिती CSS_BACKGROUND_REPEAT_REPEAT:
		ख_लिखो(stream, "background-repeat: repeat ");
		थांबा;
	स्थिती CSS_BACKGROUND_REPEAT_NO_REPEAT:
		ख_लिखो(stream, "background-repeat: no-repeat ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* border-collapse */
	val = css_computed_border_collapse(style);
	निवड (val) {
	स्थिती CSS_BORDER_COLLAPSE_SEPARATE:
		ख_लिखो(stream, "border-collapse: separate ");
		थांबा;
	स्थिती CSS_BORDER_COLLAPSE_COLLAPSE:
		ख_लिखो(stream, "border-collapse: collapse ");
		थांबा;
	पूर्वनिर्धारित:

		थांबा;
	}

	/* border-spacing */
	val = css_computed_border_spacing(style, &len1, &unit1, &len2, &unit2);
	जर (val == CSS_BORDER_SPACING_SET) {
		ख_लिखो(stream, "border-spacing: ");
		dump_css_unit(stream, len1, unit1);
		ख_लिखो(stream, " ");
		dump_css_unit(stream, len2, unit2);
		ख_लिखो(stream, " ");
	}

	/* border-top-color */
	val = css_computed_border_शीर्ष_color(style, &color);
	निवड (val) {
	स्थिती CSS_BORDER_COLOR_COLOR:
		ख_लिखो(stream, "border-top-color: #%08"PRIx32" ", color);
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* border-right-color */
	val = css_computed_border_उजवा_color(style, &color);
	निवड (val) {
	स्थिती CSS_BORDER_COLOR_COLOR:
		ख_लिखो(stream, "border-right-color: #%08"PRIx32" ", color);
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* border-bottom-color */
	val = css_computed_border_bottom_color(style, &color);
	निवड (val) {
	स्थिती CSS_BORDER_COLOR_COLOR:
		ख_लिखो(stream, "border-bottom-color: #%08"PRIx32" ", color);
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* border-left-color */
	val = css_computed_border_डावा_color(style, &color);
	निवड (val) {
	स्थिती CSS_BORDER_COLOR_COLOR:
		ख_लिखो(stream, "border-left-color: #%08"PRIx32" ", color);
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* border-top-style */
	val = css_computed_border_शीर्ष_style(style);
	निवड (val) {
	स्थिती CSS_BORDER_STYLE_NONE:
		ख_लिखो(stream, "border-top-style: none ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_HIDDEN:
		ख_लिखो(stream, "border-top-style: hidden ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_DOTTED:
		ख_लिखो(stream, "border-top-style: dotted ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_DASHED:
		ख_लिखो(stream, "border-top-style: dashed ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_SOLID:
		ख_लिखो(stream, "border-top-style: solid ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_DOUBLE:
		ख_लिखो(stream, "border-top-style: double ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_GROOVE:
		ख_लिखो(stream, "border-top-style: groove ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_RIDGE:
		ख_लिखो(stream, "border-top-style: ridge ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_INSET:
		ख_लिखो(stream, "border-top-style: inset ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_OUTSET:
		ख_लिखो(stream, "border-top-style: outset ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* border-right-style */
	val = css_computed_border_उजवा_style(style);
	निवड (val) {
	स्थिती CSS_BORDER_STYLE_NONE:
		ख_लिखो(stream, "border-right-style: none ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_HIDDEN:
		ख_लिखो(stream, "border-right-style: hidden ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_DOTTED:
		ख_लिखो(stream, "border-right-style: dotted ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_DASHED:
		ख_लिखो(stream, "border-right-style: dashed ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_SOLID:
		ख_लिखो(stream, "border-right-style: solid ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_DOUBLE:
		ख_लिखो(stream, "border-right-style: double ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_GROOVE:
		ख_लिखो(stream, "border-right-style: groove ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_RIDGE:
		ख_लिखो(stream, "border-right-style: ridge ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_INSET:
		ख_लिखो(stream, "border-right-style: inset ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_OUTSET:
		ख_लिखो(stream, "border-right-style: outset ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* border-bottom-style */
	val = css_computed_border_bottom_style(style);
	निवड (val) {
	स्थिती CSS_BORDER_STYLE_NONE:
		ख_लिखो(stream, "border-bottom-style: none ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_HIDDEN:
		ख_लिखो(stream, "border-bottom-style: hidden ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_DOTTED:
		ख_लिखो(stream, "border-bottom-style: dotted ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_DASHED:
		ख_लिखो(stream, "border-bottom-style: dashed ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_SOLID:
		ख_लिखो(stream, "border-bottom-style: solid ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_DOUBLE:
		ख_लिखो(stream, "border-bottom-style: double ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_GROOVE:
		ख_लिखो(stream, "border-bottom-style: groove ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_RIDGE:
		ख_लिखो(stream, "border-bottom-style: ridge ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_INSET:
		ख_लिखो(stream, "border-bottom-style: inset ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_OUTSET:
		ख_लिखो(stream, "border-bottom-style: outset ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* border-left-style */
	val = css_computed_border_डावा_style(style);
	निवड (val) {
	स्थिती CSS_BORDER_STYLE_NONE:
		ख_लिखो(stream, "border-left-style: none ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_HIDDEN:
		ख_लिखो(stream, "border-left-style: hidden ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_DOTTED:
		ख_लिखो(stream, "border-left-style: dotted ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_DASHED:
		ख_लिखो(stream, "border-left-style: dashed ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_SOLID:
		ख_लिखो(stream, "border-left-style: solid ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_DOUBLE:
		ख_लिखो(stream, "border-left-style: double ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_GROOVE:
		ख_लिखो(stream, "border-left-style: groove ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_RIDGE:
		ख_लिखो(stream, "border-left-style: ridge ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_INSET:
		ख_लिखो(stream, "border-left-style: inset ");
		थांबा;
	स्थिती CSS_BORDER_STYLE_OUTSET:
		ख_लिखो(stream, "border-left-style: outset ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* border-top-width */
	val = css_computed_border_शीर्ष_width(style, &len1, &unit1);
	निवड (val) {
	स्थिती CSS_BORDER_WIDTH_THIN:
		ख_लिखो(stream, "border-top-width: thin ");
		थांबा;
	स्थिती CSS_BORDER_WIDTH_MEDIUM:
		ख_लिखो(stream, "border-top-width: medium ");
		थांबा;
	स्थिती CSS_BORDER_WIDTH_THICK:
		ख_लिखो(stream, "border-top-width: thick ");
		थांबा;
	स्थिती CSS_BORDER_WIDTH_WIDTH:
		ख_लिखो(stream, "border-top-width: ");
		dump_css_unit(stream, len1, unit1);
		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* border-right-width */
	val = css_computed_border_उजवा_width(style, &len1, &unit1);
	निवड (val) {
	स्थिती CSS_BORDER_WIDTH_THIN:
		ख_लिखो(stream, "border-right-width: thin ");
		थांबा;
	स्थिती CSS_BORDER_WIDTH_MEDIUM:
		ख_लिखो(stream, "border-right-width: medium ");
		थांबा;
	स्थिती CSS_BORDER_WIDTH_THICK:
		ख_लिखो(stream, "border-right-width: thick ");
		थांबा;
	स्थिती CSS_BORDER_WIDTH_WIDTH:
		ख_लिखो(stream, "border-right-width: ");
		dump_css_unit(stream, len1, unit1);
		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* border-bottom-width */
	val = css_computed_border_bottom_width(style, &len1, &unit1);
	निवड (val) {
	स्थिती CSS_BORDER_WIDTH_THIN:
		ख_लिखो(stream, "border-bottom-width: thin ");
		थांबा;
	स्थिती CSS_BORDER_WIDTH_MEDIUM:
		ख_लिखो(stream, "border-bottom-width: medium ");
		थांबा;
	स्थिती CSS_BORDER_WIDTH_THICK:
		ख_लिखो(stream, "border-bottom-width: thick ");
		थांबा;
	स्थिती CSS_BORDER_WIDTH_WIDTH:
		ख_लिखो(stream, "border-bottom-width: ");
		dump_css_unit(stream, len1, unit1);
		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* border-left-width */
	val = css_computed_border_डावा_width(style, &len1, &unit1);
	निवड (val) {
	स्थिती CSS_BORDER_WIDTH_THIN:
		ख_लिखो(stream, "border-left-width: thin ");
		थांबा;
	स्थिती CSS_BORDER_WIDTH_MEDIUM:
		ख_लिखो(stream, "border-left-width: medium ");
		थांबा;
	स्थिती CSS_BORDER_WIDTH_THICK:
		ख_लिखो(stream, "border-left-width: thick ");
		थांबा;
	स्थिती CSS_BORDER_WIDTH_WIDTH:
		ख_लिखो(stream, "border-left-width: ");
		dump_css_unit(stream, len1, unit1);
		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* bottom */
	val = css_computed_bottom(style, &len1, &unit1);
	निवड (val) {
	स्थिती CSS_BOTTOM_AUTO:
		ख_लिखो(stream, "bottom: auto ");
		थांबा;
	स्थिती CSS_BOTTOM_SET:
		ख_लिखो(stream, "bottom: ");
		dump_css_unit(stream, len1, unit1);
		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* caption-side */
	val = css_computed_caption_side(style);
	निवड (val) {
	स्थिती CSS_CAPTION_SIDE_शीर्ष:
		ख_लिखो(stream, "caption_side: top ");
		थांबा;
	स्थिती CSS_CAPTION_SIDE_BOTTOM:
		ख_लिखो(stream, "caption_side: bottom ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* clear */
	val = css_computed_clear(style);
	निवड (val) {
	स्थिती CSS_CLEAR_NONE:
		ख_लिखो(stream, "clear: none ");
		थांबा;
	स्थिती CSS_CLEAR_डावा:
		ख_लिखो(stream, "clear: left ");
		थांबा;
	स्थिती CSS_CLEAR_उजवा:
		ख_लिखो(stream, "clear: right ");
		थांबा;
	स्थिती CSS_CLEAR_BOTH:
		ख_लिखो(stream, "clear: both ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* clip */
	val = css_computed_clip(style, &rect);
	निवड (val) {
	स्थिती CSS_CLIP_AUTO:
		ख_लिखो(stream, "clip: auto ");
		थांबा;
	स्थिती CSS_CLIP_RECT:
		ख_लिखो(stream, "clip: rect( ");

		जर (rect.शीर्ष_auto)
			ख_लिखो(stream, "auto");
		अन्यथा
			dump_css_unit(stream, rect.शीर्ष, rect.tunit);
		ख_लिखो(stream, ", ");

		जर (rect.उजवा_auto)
			ख_लिखो(stream, "auto");
		अन्यथा
			dump_css_unit(stream, rect.उजवा, rect.runit);
		ख_लिखो(stream, ", ");

		जर (rect.bottom_auto)
			ख_लिखो(stream, "auto");
		अन्यथा
			dump_css_unit(stream, rect.bottom, rect.bunit);
		ख_लिखो(stream, ", ");

		जर (rect.डावा_auto)
			ख_लिखो(stream, "auto");
		अन्यथा
			dump_css_unit(stream, rect.डावा, rect.lunit);
		ख_लिखो(stream, ") ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* color */
	val = css_computed_color(style, &color);
	जर (val == CSS_COLOR_COLOR) {
		ख_लिखो(stream, "color: #%08"PRIx32" ", color);
	}

	/* content */
	val = css_computed_content(style, &content);
	निवड (val) {
	स्थिती CSS_CONTENT_NONE:
		ख_लिखो(stream, "content: none ");
		थांबा;
	स्थिती CSS_CONTENT_NORMAL:
		ख_लिखो(stream, "content: normal ");
		थांबा;
	स्थिती CSS_CONTENT_SET:
		ख_लिखो(stream, "content:");

		पर्यंत (content->type != CSS_COMPUTED_CONTENT_NONE) {
			ख_लिखो(stream, " ");

			निवड (content->type) {
			स्थिती CSS_COMPUTED_CONTENT_STRING:
				ख_लिखो(stream,	"\"%.*s\"",
						(पूर्णांक) lwc_string_length(
						content->data.string),
						lwc_string_data(
						content->data.string));
				थांबा;
			स्थिती CSS_COMPUTED_CONTENT_URI:
				ख_लिखो(stream,	"uri(\"%.*s\")",
						(पूर्णांक) lwc_string_length(
						content->data.uri),
						lwc_string_data(
						content->data.uri));
				थांबा;
			स्थिती CSS_COMPUTED_CONTENT_COUNTER:
				ख_लिखो(stream, "counter(%.*s)",
						(पूर्णांक) lwc_string_length(
						content->data.counter.name),
						lwc_string_data(
						content->data.counter.name));
				थांबा;
			स्थिती CSS_COMPUTED_CONTENT_COUNTERS:
				ख_लिखो(stream, "counters(%.*s, \"%.*s\")",
						(पूर्णांक) lwc_string_length(
						content->data.counters.name),
						lwc_string_data(
						content->data.counters.name),
						(पूर्णांक) lwc_string_length(
						content->data.counters.sep),
						lwc_string_data(
						content->data.counters.sep));
				थांबा;
			स्थिती CSS_COMPUTED_CONTENT_ATTR:
				ख_लिखो(stream, "attr(%.*s)",
						(पूर्णांक) lwc_string_length(
						content->data.attr),
						lwc_string_data(
						content->data.attr));
				थांबा;
			स्थिती CSS_COMPUTED_CONTENT_OPEN_QUOTE:
				ख_लिखो(stream, "open-quote");
				थांबा;
			स्थिती CSS_COMPUTED_CONTENT_CLOSE_QUOTE:
				ख_लिखो(stream, "close-quote");
				थांबा;
			स्थिती CSS_COMPUTED_CONTENT_NO_OPEN_QUOTE:
				ख_लिखो(stream, "no-open-quote");
				थांबा;
			स्थिती CSS_COMPUTED_CONTENT_NO_CLOSE_QUOTE:
				ख_लिखो(stream, "no-close-quote");
				थांबा;
			}

			content++;
		}

		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* counter-increment */
	val = css_computed_counter_increment(style, &counter);
	जर ((val == CSS_COUNTER_INCREMENT_NONE) || (counter == रिक्त)) {
		ख_लिखो(stream, "counter-increment: none ");
	} अन्यथा {
		ख_लिखो(stream, "counter-increment:");

		पर्यंत (counter->name != रिक्त) {
			ख_लिखो(stream, " %.*s ",
					(पूर्णांक) lwc_string_length(counter->name),
					lwc_string_data(counter->name));

			dump_css_fixed(stream, counter->मूल्य);

			counter++;
		}

		ख_लिखो(stream, " ");
	}

	/* counter-reset */
	val = css_computed_counter_reset(style, &counter);
	जर ((val == CSS_COUNTER_RESET_NONE) || (counter == रिक्त)) {
		ख_लिखो(stream, "counter-reset: none ");
	} अन्यथा {
		ख_लिखो(stream, "counter-reset:");

		पर्यंत (counter->name != रिक्त) {
			ख_लिखो(stream, " %.*s ",
					(पूर्णांक) lwc_string_length(counter->name),
					lwc_string_data(counter->name));

			dump_css_fixed(stream, counter->मूल्य);

			counter++;
		}

		ख_लिखो(stream, " ");
	}

	/* cursor */
	val = css_computed_cursor(style, &string_list);
	ख_लिखो(stream, "cursor:");

	जर (string_list != रिक्त) {
		पर्यंत (*string_list != रिक्त) {
			ख_लिखो(stream, " url\"%.*s\")",
					(पूर्णांक) lwc_string_length(*string_list),
					lwc_string_data(*string_list));

			string_list++;
		}
	}
	निवड (val) {
	स्थिती CSS_CURSOR_AUTO:
		ख_लिखो(stream, " auto ");
		थांबा;
	स्थिती CSS_CURSOR_CROSSHAIR:
		ख_लिखो(stream, " crosshair ");
		थांबा;
	स्थिती CSS_CURSOR_DEFAULT:
		ख_लिखो(stream, " default ");
		थांबा;
	स्थिती CSS_CURSOR_POINTER:
		ख_लिखो(stream, " pointer ");
		थांबा;
	स्थिती CSS_CURSOR_MOVE:
		ख_लिखो(stream, " move ");
		थांबा;
	स्थिती CSS_CURSOR_E_RESIZE:
		ख_लिखो(stream, " e-resize ");
		थांबा;
	स्थिती CSS_CURSOR_NE_RESIZE:
		ख_लिखो(stream, " ne-resize ");
		थांबा;
	स्थिती CSS_CURSOR_NW_RESIZE:
		ख_लिखो(stream, " nw-resize ");
		थांबा;
	स्थिती CSS_CURSOR_N_RESIZE:
		ख_लिखो(stream, " n-resize ");
		थांबा;
	स्थिती CSS_CURSOR_SE_RESIZE:
		ख_लिखो(stream, " se-resize ");
		थांबा;
	स्थिती CSS_CURSOR_SW_RESIZE:
		ख_लिखो(stream, " sw-resize ");
		थांबा;
	स्थिती CSS_CURSOR_S_RESIZE:
		ख_लिखो(stream, " s-resize ");
		थांबा;
	स्थिती CSS_CURSOR_W_RESIZE:
		ख_लिखो(stream, " w-resize ");
		थांबा;
	स्थिती CSS_CURSOR_TEXT:
		ख_लिखो(stream, " text ");
		थांबा;
	स्थिती CSS_CURSOR_WAIT:
		ख_लिखो(stream, " wait ");
		थांबा;
	स्थिती CSS_CURSOR_HELP:
		ख_लिखो(stream, " help ");
		थांबा;
	स्थिती CSS_CURSOR_PROGRESS:
		ख_लिखो(stream, " progress ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* direction */
	val = css_computed_direction(style);
	निवड (val) {
	स्थिती CSS_DIRECTION_LTR:
		ख_लिखो(stream, "direction: ltr ");
		थांबा;
	स्थिती CSS_DIRECTION_RTL:
		ख_लिखो(stream, "direction: rtl ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* display */
	val = ns_computed_display_static(style);
	निवड (val) {
	स्थिती CSS_DISPLAY_INLINE:
		ख_लिखो(stream, "display: inline ");
		थांबा;
	स्थिती CSS_DISPLAY_BLOCK:
		ख_लिखो(stream, "display: block ");
		थांबा;
	स्थिती CSS_DISPLAY_LIST_ITEM:
		ख_लिखो(stream, "display: list-item ");
		थांबा;
	स्थिती CSS_DISPLAY_RUN_IN:
		ख_लिखो(stream, "display: run-in ");
		थांबा;
	स्थिती CSS_DISPLAY_INLINE_BLOCK:
		ख_लिखो(stream, "display: inline-block ");
		थांबा;
	स्थिती CSS_DISPLAY_TABLE:
		ख_लिखो(stream, "display: table ");
		थांबा;
	स्थिती CSS_DISPLAY_INLINE_TABLE:
		ख_लिखो(stream, "display: inline-table ");
		थांबा;
	स्थिती CSS_DISPLAY_TABLE_ROW_GROUP:
		ख_लिखो(stream, "display: table-row-group ");
		थांबा;
	स्थिती CSS_DISPLAY_TABLE_HEADER_GROUP:
		ख_लिखो(stream, "display: table-header-group ");
		थांबा;
	स्थिती CSS_DISPLAY_TABLE_FOOTER_GROUP:
		ख_लिखो(stream, "display: table-footer-group ");
		थांबा;
	स्थिती CSS_DISPLAY_TABLE_ROW:
		ख_लिखो(stream, "display: table-row ");
		थांबा;
	स्थिती CSS_DISPLAY_TABLE_COLUMN_GROUP:
		ख_लिखो(stream, "display: table-column-group ");
		थांबा;
	स्थिती CSS_DISPLAY_TABLE_COLUMN:
		ख_लिखो(stream, "display: table-column ");
		थांबा;
	स्थिती CSS_DISPLAY_TABLE_CELL:
		ख_लिखो(stream, "display: table-cell ");
		थांबा;
	स्थिती CSS_DISPLAY_TABLE_CAPTION:
		ख_लिखो(stream, "display: table-caption ");
		थांबा;
	स्थिती CSS_DISPLAY_NONE:
		ख_लिखो(stream, "display: none ");
		थांबा;
	स्थिती CSS_DISPLAY_FLEX:
		ख_लिखो(stream, "display: flex ");
		थांबा;
	स्थिती CSS_DISPLAY_INLINE_FLEX:
		ख_लिखो(stream, "display: inline-flex ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* empty-cells */
	val = css_computed_empty_cells(style);
	निवड (val) {
	स्थिती CSS_EMPTY_CELLS_SHOW:
		ख_लिखो(stream, "empty-cells: show ");
		थांबा;
	स्थिती CSS_EMPTY_CELLS_HIDE:
		ख_लिखो(stream, "empty-cells: hide ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* float */
	val = css_computed_float(style);
	निवड (val) {
	स्थिती CSS_FLOAT_डावा:
		ख_लिखो(stream, "float: left ");
		थांबा;
	स्थिती CSS_FLOAT_उजवा:
		ख_लिखो(stream, "float: right ");
		थांबा;
	स्थिती CSS_FLOAT_NONE:
		ख_लिखो(stream, "float: none ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* font-family */
	val = css_computed_font_family(style, &string_list);
	जर (val != CSS_FONT_FAMILY_INHERIT) {
		ख_लिखो(stream, "font-family:");

		जर (string_list != रिक्त) {
			पर्यंत (*string_list != रिक्त) {
				ख_लिखो(stream, " \"%.*s\"",
					(पूर्णांक) lwc_string_length(*string_list),
					lwc_string_data(*string_list));

				string_list++;
			}
		}
		निवड (val) {
		स्थिती CSS_FONT_FAMILY_SERIF:
			ख_लिखो(stream, " serif ");
			थांबा;
		स्थिती CSS_FONT_FAMILY_SANS_SERIF:
			ख_लिखो(stream, " sans-serif ");
			थांबा;
		स्थिती CSS_FONT_FAMILY_CURSIVE:
			ख_लिखो(stream, " cursive ");
			थांबा;
		स्थिती CSS_FONT_FAMILY_FANTASY:
			ख_लिखो(stream, " fantasy ");
			थांबा;
		स्थिती CSS_FONT_FAMILY_MONOSPACE:
			ख_लिखो(stream, " monospace ");
			थांबा;
		}
	}

	/* font-size */
	val = css_computed_font_size(style, &len1, &unit1);
	निवड (val) {
	स्थिती CSS_FONT_SIZE_XX_SMALL:
		ख_लिखो(stream, "font-size: xx-small ");
		थांबा;
	स्थिती CSS_FONT_SIZE_X_SMALL:
		ख_लिखो(stream, "font-size: x-small ");
		थांबा;
	स्थिती CSS_FONT_SIZE_SMALL:
		ख_लिखो(stream, "font-size: small ");
		थांबा;
	स्थिती CSS_FONT_SIZE_MEDIUM:
		ख_लिखो(stream, "font-size: medium ");
		थांबा;
	स्थिती CSS_FONT_SIZE_LARGE:
		ख_लिखो(stream, "font-size: large ");
		थांबा;
	स्थिती CSS_FONT_SIZE_X_LARGE:
		ख_लिखो(stream, "font-size: x-large ");
		थांबा;
	स्थिती CSS_FONT_SIZE_XX_LARGE:
		ख_लिखो(stream, "font-size: xx-large ");
		थांबा;
	स्थिती CSS_FONT_SIZE_LARGER:
		ख_लिखो(stream, "font-size: larger ");
		थांबा;
	स्थिती CSS_FONT_SIZE_SMALLER:
		ख_लिखो(stream, "font-size: smaller ");
		थांबा;
	स्थिती CSS_FONT_SIZE_DIMENSION:
		ख_लिखो(stream, "font-size: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* font-style */
	val = css_computed_font_style(style);
	निवड (val) {
	स्थिती CSS_FONT_STYLE_NORMAL:
		ख_लिखो(stream, "font-style: normal ");
		थांबा;
	स्थिती CSS_FONT_STYLE_ITALIC:
		ख_लिखो(stream, "font-style: italic ");
		थांबा;
	स्थिती CSS_FONT_STYLE_OBLIQUE:
		ख_लिखो(stream, "font-style: oblique ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* font-variant */
	val = css_computed_font_variant(style);
	निवड (val) {
	स्थिती CSS_FONT_VARIANT_NORMAL:
		ख_लिखो(stream, "font-variant: normal ");
		थांबा;
	स्थिती CSS_FONT_VARIANT_SMALL_CAPS:
		ख_लिखो(stream, "font-variant: small-caps ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* font-weight */
	val = css_computed_font_weight(style);
	निवड (val) {
	स्थिती CSS_FONT_WEIGHT_NORMAL:
		ख_लिखो(stream, "font-weight: normal ");
		थांबा;
	स्थिती CSS_FONT_WEIGHT_BOLD:
		ख_लिखो(stream, "font-weight: bold ");
		थांबा;
	स्थिती CSS_FONT_WEIGHT_BOLDER:
		ख_लिखो(stream, "font-weight: bolder ");
		थांबा;
	स्थिती CSS_FONT_WEIGHT_LIGHTER:
		ख_लिखो(stream, "font-weight: lighter ");
		थांबा;
	स्थिती CSS_FONT_WEIGHT_100:
		ख_लिखो(stream, "font-weight: 100 ");
		थांबा;
	स्थिती CSS_FONT_WEIGHT_200:
		ख_लिखो(stream, "font-weight: 200 ");
		थांबा;
	स्थिती CSS_FONT_WEIGHT_300:
		ख_लिखो(stream, "font-weight: 300 ");
		थांबा;
	स्थिती CSS_FONT_WEIGHT_400:
		ख_लिखो(stream, "font-weight: 400 ");
		थांबा;
	स्थिती CSS_FONT_WEIGHT_500:
		ख_लिखो(stream, "font-weight: 500 ");
		थांबा;
	स्थिती CSS_FONT_WEIGHT_600:
		ख_लिखो(stream, "font-weight: 600 ");
		थांबा;
	स्थिती CSS_FONT_WEIGHT_700:
		ख_लिखो(stream, "font-weight: 700 ");
		थांबा;
	स्थिती CSS_FONT_WEIGHT_800:
		ख_लिखो(stream, "font-weight: 800 ");
		थांबा;
	स्थिती CSS_FONT_WEIGHT_900:
		ख_लिखो(stream, "font-weight: 900 ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* height */
	val = css_computed_height(style, &len1, &unit1);
	निवड (val) {
	स्थिती CSS_HEIGHT_AUTO:
		ख_लिखो(stream, "height: auto ");
		थांबा;
	स्थिती CSS_HEIGHT_SET:
		ख_लिखो(stream, "height: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/*डावा*/
	val = css_computed_डावा(style, &len1, &unit1);
	निवड (val) {
	स्थिती CSS_डावा_AUTO:
		ख_लिखो(stream, "left: auto ");
		थांबा;
	स्थिती CSS_डावा_SET:
		ख_लिखो(stream, "left: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* letter-spacing */
	val = css_computed_letter_spacing(style, &len1, &unit1);
	निवड (val) {
	स्थिती CSS_LETTER_SPACING_NORMAL:
		ख_लिखो(stream, "letter-spacing: normal ");
		थांबा;
	स्थिती CSS_LETTER_SPACING_SET:
		ख_लिखो(stream, "letter-spacing: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* line-height */
	val = css_computed_ओळ_height(style, &len1, &unit1);
	निवड (val) {
	स्थिती CSS_ओळ_HEIGHT_NORMAL:
		ख_लिखो(stream, "line-height: normal ");
		थांबा;
	स्थिती CSS_ओळ_HEIGHT_संख्या:
		ख_लिखो(stream, "line-height: ");

		dump_css_fixed(stream, len1);

		ख_लिखो(stream, " ");
		थांबा;
	स्थिती CSS_ओळ_HEIGHT_DIMENSION:
		ख_लिखो(stream, "line-height: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* list-style-image */
	val = css_computed_list_style_image(style, &url);
	जर (url != रिक्त) {
		ख_लिखो(stream, "list-style-image: url('%.*s') ",
				(पूर्णांक) lwc_string_length(url),
				lwc_string_data(url));
	} अन्यथा जर (val == CSS_LIST_STYLE_IMAGE_NONE) {
		ख_लिखो(stream, "list-style-image: none ");
	}

	/* list-style-position */
	val = css_computed_list_style_position(style);
	निवड (val) {
	स्थिती CSS_LIST_STYLE_POSITION_INSIDE:
		ख_लिखो(stream, "list-style-position: inside ");
		थांबा;
	स्थिती CSS_LIST_STYLE_POSITION_OUTSIDE:
		ख_लिखो(stream, "list-style-position: outside ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* list-style-type */
	val = css_computed_list_style_type(style);
	निवड (val) {
	स्थिती CSS_LIST_STYLE_TYPE_DISC:
		ख_लिखो(stream, "list-style-type: disc ");
		थांबा;
	स्थिती CSS_LIST_STYLE_TYPE_CIRCLE:
		ख_लिखो(stream, "list-style-type: circle ");
		थांबा;
	स्थिती CSS_LIST_STYLE_TYPE_SQUARE:
		ख_लिखो(stream, "list-style-type: square ");
		थांबा;
	स्थिती CSS_LIST_STYLE_TYPE_DECIMAL:
		ख_लिखो(stream, "list-style-type: decimal ");
		थांबा;
	स्थिती CSS_LIST_STYLE_TYPE_DECIMAL_LEADING_ZERO:
		ख_लिखो(stream, "list-style-type: decimal-leading-zero ");
		थांबा;
	स्थिती CSS_LIST_STYLE_TYPE_LOWER_ROMAN:
		ख_लिखो(stream, "list-style-type: lower-roman ");
		थांबा;
	स्थिती CSS_LIST_STYLE_TYPE_UPPER_ROMAN:
		ख_लिखो(stream, "list-style-type: upper-roman ");
		थांबा;
	स्थिती CSS_LIST_STYLE_TYPE_LOWER_GREEK:
		ख_लिखो(stream, "list-style-type: lower-greek ");
		थांबा;
	स्थिती CSS_LIST_STYLE_TYPE_LOWER_LATIN:
		ख_लिखो(stream, "list-style-type: lower-latin ");
		थांबा;
	स्थिती CSS_LIST_STYLE_TYPE_UPPER_LATIN:
		ख_लिखो(stream, "list-style-type: upper-latin ");
		थांबा;
	स्थिती CSS_LIST_STYLE_TYPE_ARMENIAN:
		ख_लिखो(stream, "list-style-type: armenian ");
		थांबा;
	स्थिती CSS_LIST_STYLE_TYPE_GEORGIAN:
		ख_लिखो(stream, "list-style-type: georgian ");
		थांबा;
	स्थिती CSS_LIST_STYLE_TYPE_LOWER_ALPHA:
		ख_लिखो(stream, "list-style-type: lower-alpha ");
		थांबा;
	स्थिती CSS_LIST_STYLE_TYPE_UPPER_ALPHA:
		ख_लिखो(stream, "list-style-type: upper-alpha ");
		थांबा;
	स्थिती CSS_LIST_STYLE_TYPE_NONE:
		ख_लिखो(stream, "list-style-type: none ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* margin-top */
	val = css_computed_margin_शीर्ष(style, &len1, &unit1);
	निवड (val) {
	स्थिती CSS_MARGIN_AUTO:
		ख_लिखो(stream, "margin-top: auto ");
		थांबा;
	स्थिती CSS_MARGIN_SET:
		ख_लिखो(stream, "margin-top: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* margin-right */
	val = css_computed_margin_उजवा(style, &len1, &unit1);
	निवड (val) {
	स्थिती CSS_MARGIN_AUTO:
		ख_लिखो(stream, "margin-right: auto ");
		थांबा;
	स्थिती CSS_MARGIN_SET:
		ख_लिखो(stream, "margin-right: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* margin-bottom */
	val = css_computed_margin_bottom(style, &len1, &unit1);
	निवड (val) {
	स्थिती CSS_MARGIN_AUTO:
		ख_लिखो(stream, "margin-bottom: auto ");
		थांबा;
	स्थिती CSS_MARGIN_SET:
		ख_लिखो(stream, "margin-bottom: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* margin-left */
	val = css_computed_margin_डावा(style, &len1, &unit1);
	निवड (val) {
	स्थिती CSS_MARGIN_AUTO:
		ख_लिखो(stream, "margin-left: auto ");
		थांबा;
	स्थिती CSS_MARGIN_SET:
		ख_लिखो(stream, "margin-left: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* max-height */
	val = css_computed_max_height(style, &len1, &unit1);
	निवड (val) {
	स्थिती CSS_MAX_HEIGHT_NONE:
		ख_लिखो(stream, "max-height: none ");
		थांबा;
	स्थिती CSS_MAX_HEIGHT_SET:
		ख_लिखो(stream, "max-height: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* max-width */
	val = css_computed_max_width(style, &len1, &unit1);
	निवड (val) {
	स्थिती CSS_MAX_WIDTH_NONE:
		ख_लिखो(stream, "max-width: none ");
		थांबा;
	स्थिती CSS_MAX_WIDTH_SET:
		ख_लिखो(stream, "max-width: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* min-height */
	val = ns_computed_min_height(style, &len1, &unit1);
	निवड (val) {
	स्थिती CSS_MIN_HEIGHT_SET:
		ख_लिखो(stream, "min-height: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* min-width */
	val = ns_computed_min_width(style, &len1, &unit1);
	निवड (val) {
	स्थिती CSS_MIN_WIDTH_SET:
		ख_लिखो(stream, "min-width: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* opacity */
	val = css_computed_opacity(style, &len1);
	निवड (val) {
	स्थिती CSS_OPACITY_SET:
		ख_लिखो(stream, "opacity: ");

		dump_css_fixed(stream, len1);

		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* outline-color */
	val = css_computed_outline_color(style, &color);
	निवड (val) {
	स्थिती CSS_OUTLINE_COLOR_INVERT:
		ख_लिखो(stream, "outline-color: invert ");
		थांबा;
	स्थिती CSS_OUTLINE_COLOR_COLOR:
		ख_लिखो(stream, "outline-color: #%08"PRIx32" ", color);
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* outline-style */
	val = css_computed_outline_style(style);
	निवड (val) {
	स्थिती CSS_OUTLINE_STYLE_NONE:
		ख_लिखो(stream, "outline-style: none ");
		थांबा;
	स्थिती CSS_OUTLINE_STYLE_DOTTED:
		ख_लिखो(stream, "outline-style: dotted ");
		थांबा;
	स्थिती CSS_OUTLINE_STYLE_DASHED:
		ख_लिखो(stream, "outline-style: dashed ");
		थांबा;
	स्थिती CSS_OUTLINE_STYLE_SOLID:
		ख_लिखो(stream, "outline-style: solid ");
		थांबा;
	स्थिती CSS_OUTLINE_STYLE_DOUBLE:
		ख_लिखो(stream, "outline-style: double ");
		थांबा;
	स्थिती CSS_OUTLINE_STYLE_GROOVE:
		ख_लिखो(stream, "outline-style: groove ");
		थांबा;
	स्थिती CSS_OUTLINE_STYLE_RIDGE:
		ख_लिखो(stream, "outline-style: ridge ");
		थांबा;
	स्थिती CSS_OUTLINE_STYLE_INSET:
		ख_लिखो(stream, "outline-style: inset ");
		थांबा;
	स्थिती CSS_OUTLINE_STYLE_OUTSET:
		ख_लिखो(stream, "outline-style: outset ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* outline-width */
	val = css_computed_outline_width(style, &len1, &unit1);
	निवड (val) {
	स्थिती CSS_OUTLINE_WIDTH_THIN:
		ख_लिखो(stream, "outline-width: thin ");
		थांबा;
	स्थिती CSS_OUTLINE_WIDTH_MEDIUM:
		ख_लिखो(stream, "outline-width: medium ");
		थांबा;
	स्थिती CSS_OUTLINE_WIDTH_THICK:
		ख_लिखो(stream, "outline-width: thick ");
		थांबा;
	स्थिती CSS_OUTLINE_WIDTH_WIDTH:
		ख_लिखो(stream, "outline-width: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* overflow */
	val = css_computed_overflow_x(style);
	निवड (val) {
	स्थिती CSS_OVERFLOW_VISIBLE:
		ख_लिखो(stream, "overflow-x: visible ");
		थांबा;
	स्थिती CSS_OVERFLOW_HIDDEN:
		ख_लिखो(stream, "overflow-x: hidden ");
		थांबा;
	स्थिती CSS_OVERFLOW_SCROLL:
		ख_लिखो(stream, "overflow-x: scroll ");
		थांबा;
	स्थिती CSS_OVERFLOW_AUTO:
		ख_लिखो(stream, "overflow-x auto ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* overflow */
	val = css_computed_overflow_y(style);
	निवड (val) {
	स्थिती CSS_OVERFLOW_VISIBLE:
		ख_लिखो(stream, "overflow-y: visible ");
		थांबा;
	स्थिती CSS_OVERFLOW_HIDDEN:
		ख_लिखो(stream, "overflow-y: hidden ");
		थांबा;
	स्थिती CSS_OVERFLOW_SCROLL:
		ख_लिखो(stream, "overflow-y: scroll ");
		थांबा;
	स्थिती CSS_OVERFLOW_AUTO:
		ख_लिखो(stream, "overflow-y: auto ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* padding-top */
	val = css_computed_padding_शीर्ष(style, &len1, &unit1);
	निवड (val) {
	स्थिती CSS_PADDING_SET:
		ख_लिखो(stream, "padding-top: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* padding-right */
	val = css_computed_padding_उजवा(style, &len1, &unit1);
	निवड (val) {
	स्थिती CSS_PADDING_SET:
		ख_लिखो(stream, "padding-right: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* padding-bottom */
	val = css_computed_padding_bottom(style, &len1, &unit1);
	निवड (val) {
	स्थिती CSS_PADDING_SET:
		ख_लिखो(stream, "padding-bottom: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* padding-left */
	val = css_computed_padding_डावा(style, &len1, &unit1);
	निवड (val) {
	स्थिती CSS_PADDING_SET:
		ख_लिखो(stream, "padding-left: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* position */
	val = css_computed_position(style);
	निवड (val) {
	स्थिती CSS_POSITION_STATIC:
		ख_लिखो(stream, "position: static ");
		थांबा;
	स्थिती CSS_POSITION_RELATIVE:
		ख_लिखो(stream, "position: relative ");
		थांबा;
	स्थिती CSS_POSITION_ABSOLUTE:
		ख_लिखो(stream, "position: absolute ");
		थांबा;
	स्थिती CSS_POSITION_FIXED:
		ख_लिखो(stream, "position: fixed ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* quotes */
	val = css_computed_quotes(style, &string_list);
	जर (val == CSS_QUOTES_STRING && string_list != रिक्त) {
		ख_लिखो(stream, "quotes:");

		पर्यंत (*string_list != रिक्त) {
			ख_लिखो(stream, " \"%.*s\"",
				(पूर्णांक) lwc_string_length(*string_list),
				lwc_string_data(*string_list));

			string_list++;
		}

		ख_लिखो(stream, " ");
	} अन्यथा {
		निवड (val) {
		स्थिती CSS_QUOTES_NONE:
			ख_लिखो(stream, "quotes: none ");
			थांबा;
		पूर्वनिर्धारित:
			थांबा;
		}
	}

	/*उजवा*/
	val = css_computed_उजवा(style, &len1, &unit1);
	निवड (val) {
	स्थिती CSS_उजवा_AUTO:
		ख_लिखो(stream, "right: auto ");
		थांबा;
	स्थिती CSS_उजवा_SET:
		ख_लिखो(stream, "right: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* table-layout */
	val = css_computed_table_layout(style);
	निवड (val) {
	स्थिती CSS_TABLE_LAYOUT_AUTO:
		ख_लिखो(stream, "table-layout: auto ");
		थांबा;
	स्थिती CSS_TABLE_LAYOUT_FIXED:
		ख_लिखो(stream, "table-layout: fixed ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* text-align */
	val = css_computed_text_align(style);
	निवड (val) {
	स्थिती CSS_TEXT_ALIGN_डावा:
		ख_लिखो(stream, "text-align: left ");
		थांबा;
	स्थिती CSS_TEXT_ALIGN_उजवा:
		ख_लिखो(stream, "text-align: right ");
		थांबा;
	स्थिती CSS_TEXT_ALIGN_CENTER:
		ख_लिखो(stream, "text-align: center ");
		थांबा;
	स्थिती CSS_TEXT_ALIGN_JUSTIFY:
		ख_लिखो(stream, "text-align: justify ");
		थांबा;
	स्थिती CSS_TEXT_ALIGN_DEFAULT:
		ख_लिखो(stream, "text-align: default ");
		थांबा;
	स्थिती CSS_TEXT_ALIGN_LIBCSS_डावा:
		ख_लिखो(stream, "text-align: -libcss-left ");
		थांबा;
	स्थिती CSS_TEXT_ALIGN_LIBCSS_CENTER:
		ख_लिखो(stream, "text-align: -libcss-center ");
		थांबा;
	स्थिती CSS_TEXT_ALIGN_LIBCSS_उजवा:
		ख_लिखो(stream, "text-align: -libcss-right ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* text-decoration */
	val = css_computed_text_decoration(style);
	जर (val == CSS_TEXT_DECORATION_NONE) {
		ख_लिखो(stream, "text-decoration: none ");
	} अन्यथा {
		ख_लिखो(stream, "text-decoration:");

		जर (val & CSS_TEXT_DECORATION_BLINK) {
			ख_लिखो(stream, " blink");
		}
		जर (val & CSS_TEXT_DECORATION_ओळ_THROUGH) {
			ख_लिखो(stream, " line-through");
		}
		जर (val & CSS_TEXT_DECORATION_OVERLINE) {
			ख_लिखो(stream, " overline");
		}
		जर (val & CSS_TEXT_DECORATION_UNDERLINE) {
			ख_लिखो(stream, " underline");
		}

		ख_लिखो(stream, " ");
	}

	/* text-indent */
	val = css_computed_text_indent(style, &len1, &unit1);
	निवड (val) {
	स्थिती CSS_TEXT_INDENT_SET:
		ख_लिखो(stream, "text-indent: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* text-transform */
	val = css_computed_text_transform(style);
	निवड (val) {
	स्थिती CSS_TEXT_TRANSFORM_CAPITALIZE:
		ख_लिखो(stream, "text-transform: capitalize ");
		थांबा;
	स्थिती CSS_TEXT_TRANSFORM_UPPERCASE:
		ख_लिखो(stream, "text-transform: uppercase ");
		थांबा;
	स्थिती CSS_TEXT_TRANSFORM_LOWERCASE:
		ख_लिखो(stream, "text-transform: lowercase ");
		थांबा;
	स्थिती CSS_TEXT_TRANSFORM_NONE:
		ख_लिखो(stream, "text-transform: none ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/*शीर्ष*/
	val = css_computed_शीर्ष(style, &len1, &unit1);
	निवड (val) {
	स्थिती CSS_शीर्ष_AUTO:
		ख_लिखो(stream, "top: auto ");
		थांबा;
	स्थिती CSS_शीर्ष_SET:
		ख_लिखो(stream, "top: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* unicode-bidi */
	val = css_computed_unicode_bidi(style);
	निवड (val) {
	स्थिती CSS_UNICODE_BIDI_NORMAL:
		ख_लिखो(stream, "unicode-bidi: normal ");
		थांबा;
	स्थिती CSS_UNICODE_BIDI_EMBED:
		ख_लिखो(stream, "unicode-bidi: embed ");
		थांबा;
	स्थिती CSS_UNICODE_BIDI_BIDI_OVERRIDE:
		ख_लिखो(stream, "unicode-bidi: bidi-override ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* vertical-align */
	val = css_computed_vertical_align(style, &len1, &unit1);
	निवड (val) {
	स्थिती CSS_VERTICAL_ALIGN_BASELINE:
		ख_लिखो(stream, "vertical-align: baseline ");
		थांबा;
	स्थिती CSS_VERTICAL_ALIGN_SUB:
		ख_लिखो(stream, "vertical-align: sub ");
		थांबा;
	स्थिती CSS_VERTICAL_ALIGN_SUPER:
		ख_लिखो(stream, "vertical-align: super ");
		थांबा;
	स्थिती CSS_VERTICAL_ALIGN_शीर्ष:
		ख_लिखो(stream, "vertical-align: top ");
		थांबा;
	स्थिती CSS_VERTICAL_ALIGN_TEXT_शीर्ष:
		ख_लिखो(stream, "vertical-align: text-top ");
		थांबा;
	स्थिती CSS_VERTICAL_ALIGN_मधला:
		ख_लिखो(stream, "vertical-align: middle ");
		थांबा;
	स्थिती CSS_VERTICAL_ALIGN_BOTTOM:
		ख_लिखो(stream, "vertical-align: bottom ");
		थांबा;
	स्थिती CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
		ख_लिखो(stream, "vertical-align: text-bottom ");
		थांबा;
	स्थिती CSS_VERTICAL_ALIGN_SET:
		ख_लिखो(stream, "vertical-align: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* visibility */
	val = css_computed_visibility(style);
	निवड (val) {
	स्थिती CSS_VISIBILITY_VISIBLE:
		ख_लिखो(stream, "visibility: visible ");
		थांबा;
	स्थिती CSS_VISIBILITY_HIDDEN:
		ख_लिखो(stream, "visibility: hidden ");
		थांबा;
	स्थिती CSS_VISIBILITY_COLLAPSE:
		ख_लिखो(stream, "visibility: collapse ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* white-space */
	val = css_computed_white_space(style);
	निवड (val) {
	स्थिती CSS_WHITE_SPACE_NORMAL:
		ख_लिखो(stream, "white-space: normal ");
		थांबा;
	स्थिती CSS_WHITE_SPACE_PRE:
		ख_लिखो(stream, "white-space: pre ");
		थांबा;
	स्थिती CSS_WHITE_SPACE_NOWRAP:
		ख_लिखो(stream, "white-space: nowrap ");
		थांबा;
	स्थिती CSS_WHITE_SPACE_PRE_WRAP:
		ख_लिखो(stream, "white-space: pre-wrap ");
		थांबा;
	स्थिती CSS_WHITE_SPACE_PRE_ओळ:
		ख_लिखो(stream, "white-space: pre-line ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* width */
	val = css_computed_width(style, &len1, &unit1);
	निवड (val) {
	स्थिती CSS_WIDTH_AUTO:
		ख_लिखो(stream, "width: auto ");
		थांबा;
	स्थिती CSS_WIDTH_SET:
		ख_लिखो(stream, "width: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* word-spacing */
	val = css_computed_word_spacing(style, &len1, &unit1);
	निवड (val) {
	स्थिती CSS_WORD_SPACING_NORMAL:
		ख_लिखो(stream, "word-spacing: normal ");
		थांबा;
	स्थिती CSS_WORD_SPACING_SET:
		ख_लिखो(stream, "word-spacing: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	/* z-index */
	val = css_computed_z_index(style, &zindex);
	निवड (val) {
	स्थिती CSS_Z_INDEX_AUTO:
		ख_लिखो(stream, "z-index: auto ");
		थांबा;
	स्थिती CSS_Z_INDEX_SET:
		ख_लिखो(stream, "z-index: %"PRId32" ", zindex);
		थांबा;
	पूर्वनिर्धारित:
		थांबा;
	}

	ख_लिखो(stream, "}");
}
