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

#समावेश <stdio.h>
#समावेश <libcss/libcss.h>

#समावेश "netsurf/inttypes.h"

#समावेश "css/dump.h"
#समावेश "css/utils.h"

/**
 * Dump a fixed point value to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param f       Value to write
 */
अटल व्योम dump_css_fixed(खाता *stream, css_fixed f)
{
#घोषणा NSCSS_ABS(x) (uint32_t)((x) < 0 ? -(x) : (x))
	uint32_t uintpart = FIXTOINT(NSCSS_ABS(f));
	/* + 500 to ensure round to nearest (division will truncate) */
	uint32_t fracpart = ((NSCSS_ABS(f) & 0x3ff) * 1000 + 500) / (1 << 10);
#अघोषित NSCSS_ABS

	ख_लिखो(stream, "%s%"PRIu32".%03"PRIu32, f < 0 ? "-" : "", uintpart, fracpart);
}

/**
 * Dump a numeric value to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 */
अटल व्योम dump_css_संख्या(खाता *stream, css_fixed val)
{
	अगर (INTTOFIX(FIXTOINT(val)) == val)
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
अटल व्योम dump_css_unit(खाता *stream, css_fixed val, css_unit unit)
{
	dump_css_संख्या(stream, val);

	चयन (unit) {
	हाल CSS_UNIT_PX:
		ख_लिखो(stream, "px");
		अवरोध;
	हाल CSS_UNIT_EX:
		ख_लिखो(stream, "ex");
		अवरोध;
	हाल CSS_UNIT_EM:
		ख_लिखो(stream, "em");
		अवरोध;
	हाल CSS_UNIT_IN:
		ख_लिखो(stream, "in");
		अवरोध;
	हाल CSS_UNIT_CM:
		ख_लिखो(stream, "cm");
		अवरोध;
	हाल CSS_UNIT_MM:
		ख_लिखो(stream, "mm");
		अवरोध;
	हाल CSS_UNIT_PT:
		ख_लिखो(stream, "pt");
		अवरोध;
	हाल CSS_UNIT_PC:
		ख_लिखो(stream, "pc");
		अवरोध;
	हाल CSS_UNIT_PCT:
		ख_लिखो(stream, "%%");
		अवरोध;
	हाल CSS_UNIT_DEG:
		ख_लिखो(stream, "deg");
		अवरोध;
	हाल CSS_UNIT_GRAD:
		ख_लिखो(stream, "grad");
		अवरोध;
	हाल CSS_UNIT_RAD:
		ख_लिखो(stream, "rad");
		अवरोध;
	हाल CSS_UNIT_MS:
		ख_लिखो(stream, "ms");
		अवरोध;
	हाल CSS_UNIT_S:
		ख_लिखो(stream, "s");
		अवरोध;
	हाल CSS_UNIT_HZ:
		ख_लिखो(stream, "Hz");
		अवरोध;
	हाल CSS_UNIT_KHZ:
		ख_लिखो(stream, "kHz");
		अवरोध;
	हाल CSS_UNIT_CH:
		ख_लिखो(stream, "ch");
		अवरोध;
	हाल CSS_UNIT_REM:
		ख_लिखो(stream, "rem");
		अवरोध;
	हाल CSS_UNIT_LH:
		ख_लिखो(stream, "lh");
		अवरोध;
	हाल CSS_UNIT_VH:
		ख_लिखो(stream, "vh");
		अवरोध;
	हाल CSS_UNIT_VW:
		ख_लिखो(stream, "vw");
		अवरोध;
	हाल CSS_UNIT_VI:
		ख_लिखो(stream, "vi");
		अवरोध;
	हाल CSS_UNIT_VB:
		ख_लिखो(stream, "vb");
		अवरोध;
	हाल CSS_UNIT_VMIN:
		ख_लिखो(stream, "vmin");
		अवरोध;
	हाल CSS_UNIT_VMAX:
		ख_लिखो(stream, "vmax");
		अवरोध;
	हाल CSS_UNIT_Q:
		ख_लिखो(stream, "q");
		अवरोध;
	हाल CSS_UNIT_CALC:
		ख_लिखो(stream, "calc()");
		अवरोध;
	}
}

/* exported interface documented in content/handlers/css/dump.h */
व्योम nscss_dump_computed_style(खाता *stream, स्थिर css_computed_style *style)
{
	uint8_t val;
	css_color color = 0;
	lwc_string *url = शून्य;
	css_fixed len1 = 0, len2 = 0;
	css_unit unit1 = CSS_UNIT_PX, unit2 = CSS_UNIT_PX;
	css_computed_clip_rect rect = { 0, 0, 0, 0, CSS_UNIT_PX, CSS_UNIT_PX,
					CSS_UNIT_PX, CSS_UNIT_PX, true, true,
					true, true };
	स्थिर css_computed_content_item *content = शून्य;
	स्थिर css_computed_गणक *गणक = शून्य;
	lwc_string **string_list = शून्य;
	int32_t zindex = 0;

	ख_लिखो(stream, "{ ");

	/* background-attachment */
	val = css_computed_background_attachment(style);
	चयन (val) {
	हाल CSS_BACKGROUND_ATTACHMENT_FIXED:
		ख_लिखो(stream, "background-attachment: fixed ");
		अवरोध;
	हाल CSS_BACKGROUND_ATTACHMENT_SCROLL:
		ख_लिखो(stream, "background-attachment: scroll ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* background-color */
	val = css_computed_background_color(style, &color);
	चयन (val) {
	हाल CSS_BACKGROUND_COLOR_COLOR:
		ख_लिखो(stream, "background-color: #%08"PRIx32" ", color);
		अवरोध;
	शेष:
		अवरोध;
	}

	/* background-image */
	val = css_computed_background_image(style, &url);
	अगर (val == CSS_BACKGROUND_IMAGE_IMAGE && url != शून्य) {
		ख_लिखो(stream, "background-image: url('%.*s') ",
				(पूर्णांक) lwc_string_लंबाई(url),
				lwc_string_तथ्य(url));
	} अन्यथा अगर (val == CSS_BACKGROUND_IMAGE_NONE) {
		ख_लिखो(stream, "background-image: none ");
	}

	/* background-position */
	val = css_computed_background_position(style, &len1, &unit1,
			&len2, &unit2);
	अगर (val == CSS_BACKGROUND_POSITION_SET) {
		ख_लिखो(stream, "background-position: ");
		dump_css_unit(stream, len1, unit1);
		ख_लिखो(stream, " ");
		dump_css_unit(stream, len2, unit2);
		ख_लिखो(stream, " ");
	}

	/* background-repeat */
	val = css_computed_background_repeat(style);
	चयन (val) {
	हाल CSS_BACKGROUND_REPEAT_REPEAT_X:
		ख_लिखो(stream, "background-repeat: repeat-x ");
		अवरोध;
	हाल CSS_BACKGROUND_REPEAT_REPEAT_Y:
		ख_लिखो(stream, "background-repeat: repeat-y ");
		अवरोध;
	हाल CSS_BACKGROUND_REPEAT_REPEAT:
		ख_लिखो(stream, "background-repeat: repeat ");
		अवरोध;
	हाल CSS_BACKGROUND_REPEAT_NO_REPEAT:
		ख_लिखो(stream, "background-repeat: no-repeat ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* border-collapse */
	val = css_computed_border_collapse(style);
	चयन (val) {
	हाल CSS_BORDER_COLLAPSE_SEPARATE:
		ख_लिखो(stream, "border-collapse: separate ");
		अवरोध;
	हाल CSS_BORDER_COLLAPSE_COLLAPSE:
		ख_लिखो(stream, "border-collapse: collapse ");
		अवरोध;
	शेष:

		अवरोध;
	}

	/* border-spacing */
	val = css_computed_border_spacing(style, &len1, &unit1, &len2, &unit2);
	अगर (val == CSS_BORDER_SPACING_SET) {
		ख_लिखो(stream, "border-spacing: ");
		dump_css_unit(stream, len1, unit1);
		ख_लिखो(stream, " ");
		dump_css_unit(stream, len2, unit2);
		ख_लिखो(stream, " ");
	}

	/* border-top-color */
	val = css_computed_border_शीर्ष_color(style, &color);
	चयन (val) {
	हाल CSS_BORDER_COLOR_COLOR:
		ख_लिखो(stream, "border-top-color: #%08"PRIx32" ", color);
		अवरोध;
	शेष:
		अवरोध;
	}

	/* border-right-color */
	val = css_computed_border_दायाँ_color(style, &color);
	चयन (val) {
	हाल CSS_BORDER_COLOR_COLOR:
		ख_लिखो(stream, "border-right-color: #%08"PRIx32" ", color);
		अवरोध;
	शेष:
		अवरोध;
	}

	/* border-bottom-color */
	val = css_computed_border_bottom_color(style, &color);
	चयन (val) {
	हाल CSS_BORDER_COLOR_COLOR:
		ख_लिखो(stream, "border-bottom-color: #%08"PRIx32" ", color);
		अवरोध;
	शेष:
		अवरोध;
	}

	/* border-left-color */
	val = css_computed_border_बायाँ_color(style, &color);
	चयन (val) {
	हाल CSS_BORDER_COLOR_COLOR:
		ख_लिखो(stream, "border-left-color: #%08"PRIx32" ", color);
		अवरोध;
	शेष:
		अवरोध;
	}

	/* border-top-style */
	val = css_computed_border_शीर्ष_style(style);
	चयन (val) {
	हाल CSS_BORDER_STYLE_NONE:
		ख_लिखो(stream, "border-top-style: none ");
		अवरोध;
	हाल CSS_BORDER_STYLE_HIDDEN:
		ख_लिखो(stream, "border-top-style: hidden ");
		अवरोध;
	हाल CSS_BORDER_STYLE_DOTTED:
		ख_लिखो(stream, "border-top-style: dotted ");
		अवरोध;
	हाल CSS_BORDER_STYLE_DASHED:
		ख_लिखो(stream, "border-top-style: dashed ");
		अवरोध;
	हाल CSS_BORDER_STYLE_SOLID:
		ख_लिखो(stream, "border-top-style: solid ");
		अवरोध;
	हाल CSS_BORDER_STYLE_DOUBLE:
		ख_लिखो(stream, "border-top-style: double ");
		अवरोध;
	हाल CSS_BORDER_STYLE_GROOVE:
		ख_लिखो(stream, "border-top-style: groove ");
		अवरोध;
	हाल CSS_BORDER_STYLE_RIDGE:
		ख_लिखो(stream, "border-top-style: ridge ");
		अवरोध;
	हाल CSS_BORDER_STYLE_INSET:
		ख_लिखो(stream, "border-top-style: inset ");
		अवरोध;
	हाल CSS_BORDER_STYLE_OUTSET:
		ख_लिखो(stream, "border-top-style: outset ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* border-right-style */
	val = css_computed_border_दायाँ_style(style);
	चयन (val) {
	हाल CSS_BORDER_STYLE_NONE:
		ख_लिखो(stream, "border-right-style: none ");
		अवरोध;
	हाल CSS_BORDER_STYLE_HIDDEN:
		ख_लिखो(stream, "border-right-style: hidden ");
		अवरोध;
	हाल CSS_BORDER_STYLE_DOTTED:
		ख_लिखो(stream, "border-right-style: dotted ");
		अवरोध;
	हाल CSS_BORDER_STYLE_DASHED:
		ख_लिखो(stream, "border-right-style: dashed ");
		अवरोध;
	हाल CSS_BORDER_STYLE_SOLID:
		ख_लिखो(stream, "border-right-style: solid ");
		अवरोध;
	हाल CSS_BORDER_STYLE_DOUBLE:
		ख_लिखो(stream, "border-right-style: double ");
		अवरोध;
	हाल CSS_BORDER_STYLE_GROOVE:
		ख_लिखो(stream, "border-right-style: groove ");
		अवरोध;
	हाल CSS_BORDER_STYLE_RIDGE:
		ख_लिखो(stream, "border-right-style: ridge ");
		अवरोध;
	हाल CSS_BORDER_STYLE_INSET:
		ख_लिखो(stream, "border-right-style: inset ");
		अवरोध;
	हाल CSS_BORDER_STYLE_OUTSET:
		ख_लिखो(stream, "border-right-style: outset ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* border-bottom-style */
	val = css_computed_border_bottom_style(style);
	चयन (val) {
	हाल CSS_BORDER_STYLE_NONE:
		ख_लिखो(stream, "border-bottom-style: none ");
		अवरोध;
	हाल CSS_BORDER_STYLE_HIDDEN:
		ख_लिखो(stream, "border-bottom-style: hidden ");
		अवरोध;
	हाल CSS_BORDER_STYLE_DOTTED:
		ख_लिखो(stream, "border-bottom-style: dotted ");
		अवरोध;
	हाल CSS_BORDER_STYLE_DASHED:
		ख_लिखो(stream, "border-bottom-style: dashed ");
		अवरोध;
	हाल CSS_BORDER_STYLE_SOLID:
		ख_लिखो(stream, "border-bottom-style: solid ");
		अवरोध;
	हाल CSS_BORDER_STYLE_DOUBLE:
		ख_लिखो(stream, "border-bottom-style: double ");
		अवरोध;
	हाल CSS_BORDER_STYLE_GROOVE:
		ख_लिखो(stream, "border-bottom-style: groove ");
		अवरोध;
	हाल CSS_BORDER_STYLE_RIDGE:
		ख_लिखो(stream, "border-bottom-style: ridge ");
		अवरोध;
	हाल CSS_BORDER_STYLE_INSET:
		ख_लिखो(stream, "border-bottom-style: inset ");
		अवरोध;
	हाल CSS_BORDER_STYLE_OUTSET:
		ख_लिखो(stream, "border-bottom-style: outset ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* border-left-style */
	val = css_computed_border_बायाँ_style(style);
	चयन (val) {
	हाल CSS_BORDER_STYLE_NONE:
		ख_लिखो(stream, "border-left-style: none ");
		अवरोध;
	हाल CSS_BORDER_STYLE_HIDDEN:
		ख_लिखो(stream, "border-left-style: hidden ");
		अवरोध;
	हाल CSS_BORDER_STYLE_DOTTED:
		ख_लिखो(stream, "border-left-style: dotted ");
		अवरोध;
	हाल CSS_BORDER_STYLE_DASHED:
		ख_लिखो(stream, "border-left-style: dashed ");
		अवरोध;
	हाल CSS_BORDER_STYLE_SOLID:
		ख_लिखो(stream, "border-left-style: solid ");
		अवरोध;
	हाल CSS_BORDER_STYLE_DOUBLE:
		ख_लिखो(stream, "border-left-style: double ");
		अवरोध;
	हाल CSS_BORDER_STYLE_GROOVE:
		ख_लिखो(stream, "border-left-style: groove ");
		अवरोध;
	हाल CSS_BORDER_STYLE_RIDGE:
		ख_लिखो(stream, "border-left-style: ridge ");
		अवरोध;
	हाल CSS_BORDER_STYLE_INSET:
		ख_लिखो(stream, "border-left-style: inset ");
		अवरोध;
	हाल CSS_BORDER_STYLE_OUTSET:
		ख_लिखो(stream, "border-left-style: outset ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* border-top-width */
	val = css_computed_border_शीर्ष_width(style, &len1, &unit1);
	चयन (val) {
	हाल CSS_BORDER_WIDTH_THIN:
		ख_लिखो(stream, "border-top-width: thin ");
		अवरोध;
	हाल CSS_BORDER_WIDTH_MEDIUM:
		ख_लिखो(stream, "border-top-width: medium ");
		अवरोध;
	हाल CSS_BORDER_WIDTH_THICK:
		ख_लिखो(stream, "border-top-width: thick ");
		अवरोध;
	हाल CSS_BORDER_WIDTH_WIDTH:
		ख_लिखो(stream, "border-top-width: ");
		dump_css_unit(stream, len1, unit1);
		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* border-right-width */
	val = css_computed_border_दायाँ_width(style, &len1, &unit1);
	चयन (val) {
	हाल CSS_BORDER_WIDTH_THIN:
		ख_लिखो(stream, "border-right-width: thin ");
		अवरोध;
	हाल CSS_BORDER_WIDTH_MEDIUM:
		ख_लिखो(stream, "border-right-width: medium ");
		अवरोध;
	हाल CSS_BORDER_WIDTH_THICK:
		ख_लिखो(stream, "border-right-width: thick ");
		अवरोध;
	हाल CSS_BORDER_WIDTH_WIDTH:
		ख_लिखो(stream, "border-right-width: ");
		dump_css_unit(stream, len1, unit1);
		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* border-bottom-width */
	val = css_computed_border_bottom_width(style, &len1, &unit1);
	चयन (val) {
	हाल CSS_BORDER_WIDTH_THIN:
		ख_लिखो(stream, "border-bottom-width: thin ");
		अवरोध;
	हाल CSS_BORDER_WIDTH_MEDIUM:
		ख_लिखो(stream, "border-bottom-width: medium ");
		अवरोध;
	हाल CSS_BORDER_WIDTH_THICK:
		ख_लिखो(stream, "border-bottom-width: thick ");
		अवरोध;
	हाल CSS_BORDER_WIDTH_WIDTH:
		ख_लिखो(stream, "border-bottom-width: ");
		dump_css_unit(stream, len1, unit1);
		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* border-left-width */
	val = css_computed_border_बायाँ_width(style, &len1, &unit1);
	चयन (val) {
	हाल CSS_BORDER_WIDTH_THIN:
		ख_लिखो(stream, "border-left-width: thin ");
		अवरोध;
	हाल CSS_BORDER_WIDTH_MEDIUM:
		ख_लिखो(stream, "border-left-width: medium ");
		अवरोध;
	हाल CSS_BORDER_WIDTH_THICK:
		ख_लिखो(stream, "border-left-width: thick ");
		अवरोध;
	हाल CSS_BORDER_WIDTH_WIDTH:
		ख_लिखो(stream, "border-left-width: ");
		dump_css_unit(stream, len1, unit1);
		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* bottom */
	val = css_computed_bottom(style, &len1, &unit1);
	चयन (val) {
	हाल CSS_BOTTOM_AUTO:
		ख_लिखो(stream, "bottom: auto ");
		अवरोध;
	हाल CSS_BOTTOM_SET:
		ख_लिखो(stream, "bottom: ");
		dump_css_unit(stream, len1, unit1);
		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* caption-side */
	val = css_computed_caption_side(style);
	चयन (val) {
	हाल CSS_CAPTION_SIDE_शीर्ष:
		ख_लिखो(stream, "caption_side: top ");
		अवरोध;
	हाल CSS_CAPTION_SIDE_BOTTOM:
		ख_लिखो(stream, "caption_side: bottom ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* clear */
	val = css_computed_clear(style);
	चयन (val) {
	हाल CSS_CLEAR_NONE:
		ख_लिखो(stream, "clear: none ");
		अवरोध;
	हाल CSS_CLEAR_बायाँ:
		ख_लिखो(stream, "clear: left ");
		अवरोध;
	हाल CSS_CLEAR_दायाँ:
		ख_लिखो(stream, "clear: right ");
		अवरोध;
	हाल CSS_CLEAR_BOTH:
		ख_लिखो(stream, "clear: both ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* clip */
	val = css_computed_clip(style, &rect);
	चयन (val) {
	हाल CSS_CLIP_AUTO:
		ख_लिखो(stream, "clip: auto ");
		अवरोध;
	हाल CSS_CLIP_RECT:
		ख_लिखो(stream, "clip: rect( ");

		अगर (rect.शीर्ष_auto)
			ख_लिखो(stream, "auto");
		अन्यथा
			dump_css_unit(stream, rect.शीर्ष, rect.tunit);
		ख_लिखो(stream, ", ");

		अगर (rect.दायाँ_auto)
			ख_लिखो(stream, "auto");
		अन्यथा
			dump_css_unit(stream, rect.दायाँ, rect.runit);
		ख_लिखो(stream, ", ");

		अगर (rect.bottom_auto)
			ख_लिखो(stream, "auto");
		अन्यथा
			dump_css_unit(stream, rect.bottom, rect.bunit);
		ख_लिखो(stream, ", ");

		अगर (rect.बायाँ_auto)
			ख_लिखो(stream, "auto");
		अन्यथा
			dump_css_unit(stream, rect.बायाँ, rect.lunit);
		ख_लिखो(stream, ") ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* color */
	val = css_computed_color(style, &color);
	अगर (val == CSS_COLOR_COLOR) {
		ख_लिखो(stream, "color: #%08"PRIx32" ", color);
	}

	/* content */
	val = css_computed_content(style, &content);
	चयन (val) {
	हाल CSS_CONTENT_NONE:
		ख_लिखो(stream, "content: none ");
		अवरोध;
	हाल CSS_CONTENT_NORMAL:
		ख_लिखो(stream, "content: normal ");
		अवरोध;
	हाल CSS_CONTENT_SET:
		ख_लिखो(stream, "content:");

		जबतक (content->type != CSS_COMPUTED_CONTENT_NONE) {
			ख_लिखो(stream, " ");

			चयन (content->type) {
			हाल CSS_COMPUTED_CONTENT_STRING:
				ख_लिखो(stream,	"\"%.*s\"",
						(पूर्णांक) lwc_string_लंबाई(
						content->तथ्य.string),
						lwc_string_तथ्य(
						content->तथ्य.string));
				अवरोध;
			हाल CSS_COMPUTED_CONTENT_URI:
				ख_लिखो(stream,	"uri(\"%.*s\")",
						(पूर्णांक) lwc_string_लंबाई(
						content->तथ्य.uri),
						lwc_string_तथ्य(
						content->तथ्य.uri));
				अवरोध;
			हाल CSS_COMPUTED_CONTENT_गणक:
				ख_लिखो(stream, "counter(%.*s)",
						(पूर्णांक) lwc_string_लंबाई(
						content->तथ्य.गणक.name),
						lwc_string_तथ्य(
						content->तथ्य.गणक.name));
				अवरोध;
			हाल CSS_COMPUTED_CONTENT_COUNTERS:
				ख_लिखो(stream, "counters(%.*s, \"%.*s\")",
						(पूर्णांक) lwc_string_लंबाई(
						content->तथ्य.counters.name),
						lwc_string_तथ्य(
						content->तथ्य.counters.name),
						(पूर्णांक) lwc_string_लंबाई(
						content->तथ्य.counters.sep),
						lwc_string_तथ्य(
						content->तथ्य.counters.sep));
				अवरोध;
			हाल CSS_COMPUTED_CONTENT_ATTR:
				ख_लिखो(stream, "attr(%.*s)",
						(पूर्णांक) lwc_string_लंबाई(
						content->तथ्य.attr),
						lwc_string_तथ्य(
						content->तथ्य.attr));
				अवरोध;
			हाल CSS_COMPUTED_CONTENT_OPEN_QUOTE:
				ख_लिखो(stream, "open-quote");
				अवरोध;
			हाल CSS_COMPUTED_CONTENT_CLOSE_QUOTE:
				ख_लिखो(stream, "close-quote");
				अवरोध;
			हाल CSS_COMPUTED_CONTENT_NO_OPEN_QUOTE:
				ख_लिखो(stream, "no-open-quote");
				अवरोध;
			हाल CSS_COMPUTED_CONTENT_NO_CLOSE_QUOTE:
				ख_लिखो(stream, "no-close-quote");
				अवरोध;
			}

			content++;
		}

		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* counter-increment */
	val = css_computed_गणक_increment(style, &गणक);
	अगर ((val == CSS_गणक_INCREMENT_NONE) || (गणक == शून्य)) {
		ख_लिखो(stream, "counter-increment: none ");
	} अन्यथा {
		ख_लिखो(stream, "counter-increment:");

		जबतक (गणक->name != शून्य) {
			ख_लिखो(stream, " %.*s ",
					(पूर्णांक) lwc_string_लंबाई(गणक->name),
					lwc_string_तथ्य(गणक->name));

			dump_css_fixed(stream, गणक->मूल्य);

			गणक++;
		}

		ख_लिखो(stream, " ");
	}

	/* counter-reset */
	val = css_computed_गणक_reset(style, &गणक);
	अगर ((val == CSS_गणक_RESET_NONE) || (गणक == शून्य)) {
		ख_लिखो(stream, "counter-reset: none ");
	} अन्यथा {
		ख_लिखो(stream, "counter-reset:");

		जबतक (गणक->name != शून्य) {
			ख_लिखो(stream, " %.*s ",
					(पूर्णांक) lwc_string_लंबाई(गणक->name),
					lwc_string_तथ्य(गणक->name));

			dump_css_fixed(stream, गणक->मूल्य);

			गणक++;
		}

		ख_लिखो(stream, " ");
	}

	/* cursor */
	val = css_computed_cursor(style, &string_list);
	ख_लिखो(stream, "cursor:");

	अगर (string_list != शून्य) {
		जबतक (*string_list != शून्य) {
			ख_लिखो(stream, " url\"%.*s\")",
					(पूर्णांक) lwc_string_लंबाई(*string_list),
					lwc_string_तथ्य(*string_list));

			string_list++;
		}
	}
	चयन (val) {
	हाल CSS_CURSOR_AUTO:
		ख_लिखो(stream, " auto ");
		अवरोध;
	हाल CSS_CURSOR_CROSSHAIR:
		ख_लिखो(stream, " crosshair ");
		अवरोध;
	हाल CSS_CURSOR_DEFAULT:
		ख_लिखो(stream, " default ");
		अवरोध;
	हाल CSS_CURSOR_POINTER:
		ख_लिखो(stream, " pointer ");
		अवरोध;
	हाल CSS_CURSOR_MOVE:
		ख_लिखो(stream, " move ");
		अवरोध;
	हाल CSS_CURSOR_E_RESIZE:
		ख_लिखो(stream, " e-resize ");
		अवरोध;
	हाल CSS_CURSOR_NE_RESIZE:
		ख_लिखो(stream, " ne-resize ");
		अवरोध;
	हाल CSS_CURSOR_NW_RESIZE:
		ख_लिखो(stream, " nw-resize ");
		अवरोध;
	हाल CSS_CURSOR_N_RESIZE:
		ख_लिखो(stream, " n-resize ");
		अवरोध;
	हाल CSS_CURSOR_SE_RESIZE:
		ख_लिखो(stream, " se-resize ");
		अवरोध;
	हाल CSS_CURSOR_SW_RESIZE:
		ख_लिखो(stream, " sw-resize ");
		अवरोध;
	हाल CSS_CURSOR_S_RESIZE:
		ख_लिखो(stream, " s-resize ");
		अवरोध;
	हाल CSS_CURSOR_W_RESIZE:
		ख_लिखो(stream, " w-resize ");
		अवरोध;
	हाल CSS_CURSOR_TEXT:
		ख_लिखो(stream, " text ");
		अवरोध;
	हाल CSS_CURSOR_WAIT:
		ख_लिखो(stream, " wait ");
		अवरोध;
	हाल CSS_CURSOR_HELP:
		ख_लिखो(stream, " help ");
		अवरोध;
	हाल CSS_CURSOR_PROGRESS:
		ख_लिखो(stream, " progress ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* direction */
	val = css_computed_direction(style);
	चयन (val) {
	हाल CSS_DIRECTION_LTR:
		ख_लिखो(stream, "direction: ltr ");
		अवरोध;
	हाल CSS_DIRECTION_RTL:
		ख_लिखो(stream, "direction: rtl ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* display */
	val = ns_computed_display_static(style);
	चयन (val) {
	हाल CSS_DISPLAY_INLINE:
		ख_लिखो(stream, "display: inline ");
		अवरोध;
	हाल CSS_DISPLAY_BLOCK:
		ख_लिखो(stream, "display: block ");
		अवरोध;
	हाल CSS_DISPLAY_LIST_ITEM:
		ख_लिखो(stream, "display: list-item ");
		अवरोध;
	हाल CSS_DISPLAY_RUN_IN:
		ख_लिखो(stream, "display: run-in ");
		अवरोध;
	हाल CSS_DISPLAY_INLINE_BLOCK:
		ख_लिखो(stream, "display: inline-block ");
		अवरोध;
	हाल CSS_DISPLAY_TABLE:
		ख_लिखो(stream, "display: table ");
		अवरोध;
	हाल CSS_DISPLAY_INLINE_TABLE:
		ख_लिखो(stream, "display: inline-table ");
		अवरोध;
	हाल CSS_DISPLAY_TABLE_ROW_GROUP:
		ख_लिखो(stream, "display: table-row-group ");
		अवरोध;
	हाल CSS_DISPLAY_TABLE_HEADER_GROUP:
		ख_लिखो(stream, "display: table-header-group ");
		अवरोध;
	हाल CSS_DISPLAY_TABLE_FOOTER_GROUP:
		ख_लिखो(stream, "display: table-footer-group ");
		अवरोध;
	हाल CSS_DISPLAY_TABLE_ROW:
		ख_लिखो(stream, "display: table-row ");
		अवरोध;
	हाल CSS_DISPLAY_TABLE_COLUMN_GROUP:
		ख_लिखो(stream, "display: table-column-group ");
		अवरोध;
	हाल CSS_DISPLAY_TABLE_COLUMN:
		ख_लिखो(stream, "display: table-column ");
		अवरोध;
	हाल CSS_DISPLAY_TABLE_CELL:
		ख_लिखो(stream, "display: table-cell ");
		अवरोध;
	हाल CSS_DISPLAY_TABLE_CAPTION:
		ख_लिखो(stream, "display: table-caption ");
		अवरोध;
	हाल CSS_DISPLAY_NONE:
		ख_लिखो(stream, "display: none ");
		अवरोध;
	हाल CSS_DISPLAY_FLEX:
		ख_लिखो(stream, "display: flex ");
		अवरोध;
	हाल CSS_DISPLAY_INLINE_FLEX:
		ख_लिखो(stream, "display: inline-flex ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* empty-cells */
	val = css_computed_empty_cells(style);
	चयन (val) {
	हाल CSS_EMPTY_CELLS_SHOW:
		ख_लिखो(stream, "empty-cells: show ");
		अवरोध;
	हाल CSS_EMPTY_CELLS_HIDE:
		ख_लिखो(stream, "empty-cells: hide ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* float */
	val = css_computed_float(style);
	चयन (val) {
	हाल CSS_FLOAT_बायाँ:
		ख_लिखो(stream, "float: left ");
		अवरोध;
	हाल CSS_FLOAT_दायाँ:
		ख_लिखो(stream, "float: right ");
		अवरोध;
	हाल CSS_FLOAT_NONE:
		ख_लिखो(stream, "float: none ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* font-family */
	val = css_computed_font_family(style, &string_list);
	अगर (val != CSS_FONT_FAMILY_INHERIT) {
		ख_लिखो(stream, "font-family:");

		अगर (string_list != शून्य) {
			जबतक (*string_list != शून्य) {
				ख_लिखो(stream, " \"%.*s\"",
					(पूर्णांक) lwc_string_लंबाई(*string_list),
					lwc_string_तथ्य(*string_list));

				string_list++;
			}
		}
		चयन (val) {
		हाल CSS_FONT_FAMILY_SERIF:
			ख_लिखो(stream, " serif ");
			अवरोध;
		हाल CSS_FONT_FAMILY_SANS_SERIF:
			ख_लिखो(stream, " sans-serif ");
			अवरोध;
		हाल CSS_FONT_FAMILY_CURSIVE:
			ख_लिखो(stream, " cursive ");
			अवरोध;
		हाल CSS_FONT_FAMILY_FANTASY:
			ख_लिखो(stream, " fantasy ");
			अवरोध;
		हाल CSS_FONT_FAMILY_MONOSPACE:
			ख_लिखो(stream, " monospace ");
			अवरोध;
		}
	}

	/* font-size */
	val = css_computed_font_आकार(style, &len1, &unit1);
	चयन (val) {
	हाल CSS_FONT_आकार_XX_SMALL:
		ख_लिखो(stream, "font-size: xx-small ");
		अवरोध;
	हाल CSS_FONT_आकार_X_SMALL:
		ख_लिखो(stream, "font-size: x-small ");
		अवरोध;
	हाल CSS_FONT_आकार_SMALL:
		ख_लिखो(stream, "font-size: small ");
		अवरोध;
	हाल CSS_FONT_आकार_MEDIUM:
		ख_लिखो(stream, "font-size: medium ");
		अवरोध;
	हाल CSS_FONT_आकार_LARGE:
		ख_लिखो(stream, "font-size: large ");
		अवरोध;
	हाल CSS_FONT_आकार_X_LARGE:
		ख_लिखो(stream, "font-size: x-large ");
		अवरोध;
	हाल CSS_FONT_आकार_XX_LARGE:
		ख_लिखो(stream, "font-size: xx-large ");
		अवरोध;
	हाल CSS_FONT_आकार_LARGER:
		ख_लिखो(stream, "font-size: larger ");
		अवरोध;
	हाल CSS_FONT_आकार_SMALLER:
		ख_लिखो(stream, "font-size: smaller ");
		अवरोध;
	हाल CSS_FONT_आकार_DIMENSION:
		ख_लिखो(stream, "font-size: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* font-style */
	val = css_computed_font_style(style);
	चयन (val) {
	हाल CSS_FONT_STYLE_NORMAL:
		ख_लिखो(stream, "font-style: normal ");
		अवरोध;
	हाल CSS_FONT_STYLE_ITALIC:
		ख_लिखो(stream, "font-style: italic ");
		अवरोध;
	हाल CSS_FONT_STYLE_OBLIQUE:
		ख_लिखो(stream, "font-style: oblique ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* font-variant */
	val = css_computed_font_variant(style);
	चयन (val) {
	हाल CSS_FONT_VARIANT_NORMAL:
		ख_लिखो(stream, "font-variant: normal ");
		अवरोध;
	हाल CSS_FONT_VARIANT_SMALL_CAPS:
		ख_लिखो(stream, "font-variant: small-caps ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* font-weight */
	val = css_computed_font_weight(style);
	चयन (val) {
	हाल CSS_FONT_WEIGHT_NORMAL:
		ख_लिखो(stream, "font-weight: normal ");
		अवरोध;
	हाल CSS_FONT_WEIGHT_BOLD:
		ख_लिखो(stream, "font-weight: bold ");
		अवरोध;
	हाल CSS_FONT_WEIGHT_BOLDER:
		ख_लिखो(stream, "font-weight: bolder ");
		अवरोध;
	हाल CSS_FONT_WEIGHT_LIGHTER:
		ख_लिखो(stream, "font-weight: lighter ");
		अवरोध;
	हाल CSS_FONT_WEIGHT_100:
		ख_लिखो(stream, "font-weight: 100 ");
		अवरोध;
	हाल CSS_FONT_WEIGHT_200:
		ख_लिखो(stream, "font-weight: 200 ");
		अवरोध;
	हाल CSS_FONT_WEIGHT_300:
		ख_लिखो(stream, "font-weight: 300 ");
		अवरोध;
	हाल CSS_FONT_WEIGHT_400:
		ख_लिखो(stream, "font-weight: 400 ");
		अवरोध;
	हाल CSS_FONT_WEIGHT_500:
		ख_लिखो(stream, "font-weight: 500 ");
		अवरोध;
	हाल CSS_FONT_WEIGHT_600:
		ख_लिखो(stream, "font-weight: 600 ");
		अवरोध;
	हाल CSS_FONT_WEIGHT_700:
		ख_लिखो(stream, "font-weight: 700 ");
		अवरोध;
	हाल CSS_FONT_WEIGHT_800:
		ख_लिखो(stream, "font-weight: 800 ");
		अवरोध;
	हाल CSS_FONT_WEIGHT_900:
		ख_लिखो(stream, "font-weight: 900 ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* height */
	val = css_computed_height(style, &len1, &unit1);
	चयन (val) {
	हाल CSS_HEIGHT_AUTO:
		ख_लिखो(stream, "height: auto ");
		अवरोध;
	हाल CSS_HEIGHT_SET:
		ख_लिखो(stream, "height: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/*बायाँ*/
	val = css_computed_बायाँ(style, &len1, &unit1);
	चयन (val) {
	हाल CSS_बायाँ_AUTO:
		ख_लिखो(stream, "left: auto ");
		अवरोध;
	हाल CSS_बायाँ_SET:
		ख_लिखो(stream, "left: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* letter-spacing */
	val = css_computed_letter_spacing(style, &len1, &unit1);
	चयन (val) {
	हाल CSS_LETTER_SPACING_NORMAL:
		ख_लिखो(stream, "letter-spacing: normal ");
		अवरोध;
	हाल CSS_LETTER_SPACING_SET:
		ख_लिखो(stream, "letter-spacing: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* line-height */
	val = css_computed_पंक्ति_height(style, &len1, &unit1);
	चयन (val) {
	हाल CSS_पंक्ति_HEIGHT_NORMAL:
		ख_लिखो(stream, "line-height: normal ");
		अवरोध;
	हाल CSS_पंक्ति_HEIGHT_संख्या:
		ख_लिखो(stream, "line-height: ");

		dump_css_fixed(stream, len1);

		ख_लिखो(stream, " ");
		अवरोध;
	हाल CSS_पंक्ति_HEIGHT_DIMENSION:
		ख_लिखो(stream, "line-height: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* list-style-image */
	val = css_computed_list_style_image(style, &url);
	अगर (url != शून्य) {
		ख_लिखो(stream, "list-style-image: url('%.*s') ",
				(पूर्णांक) lwc_string_लंबाई(url),
				lwc_string_तथ्य(url));
	} अन्यथा अगर (val == CSS_LIST_STYLE_IMAGE_NONE) {
		ख_लिखो(stream, "list-style-image: none ");
	}

	/* list-style-position */
	val = css_computed_list_style_position(style);
	चयन (val) {
	हाल CSS_LIST_STYLE_POSITION_INSIDE:
		ख_लिखो(stream, "list-style-position: inside ");
		अवरोध;
	हाल CSS_LIST_STYLE_POSITION_OUTSIDE:
		ख_लिखो(stream, "list-style-position: outside ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* list-style-type */
	val = css_computed_list_style_type(style);
	चयन (val) {
	हाल CSS_LIST_STYLE_TYPE_DISC:
		ख_लिखो(stream, "list-style-type: disc ");
		अवरोध;
	हाल CSS_LIST_STYLE_TYPE_CIRCLE:
		ख_लिखो(stream, "list-style-type: circle ");
		अवरोध;
	हाल CSS_LIST_STYLE_TYPE_SQUARE:
		ख_लिखो(stream, "list-style-type: square ");
		अवरोध;
	हाल CSS_LIST_STYLE_TYPE_DECIMAL:
		ख_लिखो(stream, "list-style-type: decimal ");
		अवरोध;
	हाल CSS_LIST_STYLE_TYPE_DECIMAL_LEADING_ZERO:
		ख_लिखो(stream, "list-style-type: decimal-leading-zero ");
		अवरोध;
	हाल CSS_LIST_STYLE_TYPE_LOWER_ROMAN:
		ख_लिखो(stream, "list-style-type: lower-roman ");
		अवरोध;
	हाल CSS_LIST_STYLE_TYPE_UPPER_ROMAN:
		ख_लिखो(stream, "list-style-type: upper-roman ");
		अवरोध;
	हाल CSS_LIST_STYLE_TYPE_LOWER_GREEK:
		ख_लिखो(stream, "list-style-type: lower-greek ");
		अवरोध;
	हाल CSS_LIST_STYLE_TYPE_LOWER_LATIN:
		ख_लिखो(stream, "list-style-type: lower-latin ");
		अवरोध;
	हाल CSS_LIST_STYLE_TYPE_UPPER_LATIN:
		ख_लिखो(stream, "list-style-type: upper-latin ");
		अवरोध;
	हाल CSS_LIST_STYLE_TYPE_ARMENIAN:
		ख_लिखो(stream, "list-style-type: armenian ");
		अवरोध;
	हाल CSS_LIST_STYLE_TYPE_GEORGIAN:
		ख_लिखो(stream, "list-style-type: georgian ");
		अवरोध;
	हाल CSS_LIST_STYLE_TYPE_LOWER_ALPHA:
		ख_लिखो(stream, "list-style-type: lower-alpha ");
		अवरोध;
	हाल CSS_LIST_STYLE_TYPE_UPPER_ALPHA:
		ख_लिखो(stream, "list-style-type: upper-alpha ");
		अवरोध;
	हाल CSS_LIST_STYLE_TYPE_NONE:
		ख_लिखो(stream, "list-style-type: none ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* margin-top */
	val = css_computed_margin_शीर्ष(style, &len1, &unit1);
	चयन (val) {
	हाल CSS_MARGIN_AUTO:
		ख_लिखो(stream, "margin-top: auto ");
		अवरोध;
	हाल CSS_MARGIN_SET:
		ख_लिखो(stream, "margin-top: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* margin-right */
	val = css_computed_margin_दायाँ(style, &len1, &unit1);
	चयन (val) {
	हाल CSS_MARGIN_AUTO:
		ख_लिखो(stream, "margin-right: auto ");
		अवरोध;
	हाल CSS_MARGIN_SET:
		ख_लिखो(stream, "margin-right: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* margin-bottom */
	val = css_computed_margin_bottom(style, &len1, &unit1);
	चयन (val) {
	हाल CSS_MARGIN_AUTO:
		ख_लिखो(stream, "margin-bottom: auto ");
		अवरोध;
	हाल CSS_MARGIN_SET:
		ख_लिखो(stream, "margin-bottom: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* margin-left */
	val = css_computed_margin_बायाँ(style, &len1, &unit1);
	चयन (val) {
	हाल CSS_MARGIN_AUTO:
		ख_लिखो(stream, "margin-left: auto ");
		अवरोध;
	हाल CSS_MARGIN_SET:
		ख_लिखो(stream, "margin-left: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* max-height */
	val = css_computed_max_height(style, &len1, &unit1);
	चयन (val) {
	हाल CSS_MAX_HEIGHT_NONE:
		ख_लिखो(stream, "max-height: none ");
		अवरोध;
	हाल CSS_MAX_HEIGHT_SET:
		ख_लिखो(stream, "max-height: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* max-width */
	val = css_computed_max_width(style, &len1, &unit1);
	चयन (val) {
	हाल CSS_MAX_WIDTH_NONE:
		ख_लिखो(stream, "max-width: none ");
		अवरोध;
	हाल CSS_MAX_WIDTH_SET:
		ख_लिखो(stream, "max-width: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* min-height */
	val = ns_computed_min_height(style, &len1, &unit1);
	चयन (val) {
	हाल CSS_MIN_HEIGHT_SET:
		ख_लिखो(stream, "min-height: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* min-width */
	val = ns_computed_min_width(style, &len1, &unit1);
	चयन (val) {
	हाल CSS_MIN_WIDTH_SET:
		ख_लिखो(stream, "min-width: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* opacity */
	val = css_computed_opacity(style, &len1);
	चयन (val) {
	हाल CSS_OPACITY_SET:
		ख_लिखो(stream, "opacity: ");

		dump_css_fixed(stream, len1);

		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* outline-color */
	val = css_computed_outline_color(style, &color);
	चयन (val) {
	हाल CSS_OUTLINE_COLOR_INVERT:
		ख_लिखो(stream, "outline-color: invert ");
		अवरोध;
	हाल CSS_OUTLINE_COLOR_COLOR:
		ख_लिखो(stream, "outline-color: #%08"PRIx32" ", color);
		अवरोध;
	शेष:
		अवरोध;
	}

	/* outline-style */
	val = css_computed_outline_style(style);
	चयन (val) {
	हाल CSS_OUTLINE_STYLE_NONE:
		ख_लिखो(stream, "outline-style: none ");
		अवरोध;
	हाल CSS_OUTLINE_STYLE_DOTTED:
		ख_लिखो(stream, "outline-style: dotted ");
		अवरोध;
	हाल CSS_OUTLINE_STYLE_DASHED:
		ख_लिखो(stream, "outline-style: dashed ");
		अवरोध;
	हाल CSS_OUTLINE_STYLE_SOLID:
		ख_लिखो(stream, "outline-style: solid ");
		अवरोध;
	हाल CSS_OUTLINE_STYLE_DOUBLE:
		ख_लिखो(stream, "outline-style: double ");
		अवरोध;
	हाल CSS_OUTLINE_STYLE_GROOVE:
		ख_लिखो(stream, "outline-style: groove ");
		अवरोध;
	हाल CSS_OUTLINE_STYLE_RIDGE:
		ख_लिखो(stream, "outline-style: ridge ");
		अवरोध;
	हाल CSS_OUTLINE_STYLE_INSET:
		ख_लिखो(stream, "outline-style: inset ");
		अवरोध;
	हाल CSS_OUTLINE_STYLE_OUTSET:
		ख_लिखो(stream, "outline-style: outset ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* outline-width */
	val = css_computed_outline_width(style, &len1, &unit1);
	चयन (val) {
	हाल CSS_OUTLINE_WIDTH_THIN:
		ख_लिखो(stream, "outline-width: thin ");
		अवरोध;
	हाल CSS_OUTLINE_WIDTH_MEDIUM:
		ख_लिखो(stream, "outline-width: medium ");
		अवरोध;
	हाल CSS_OUTLINE_WIDTH_THICK:
		ख_लिखो(stream, "outline-width: thick ");
		अवरोध;
	हाल CSS_OUTLINE_WIDTH_WIDTH:
		ख_लिखो(stream, "outline-width: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* overflow */
	val = css_computed_overflow_x(style);
	चयन (val) {
	हाल CSS_OVERFLOW_VISIBLE:
		ख_लिखो(stream, "overflow-x: visible ");
		अवरोध;
	हाल CSS_OVERFLOW_HIDDEN:
		ख_लिखो(stream, "overflow-x: hidden ");
		अवरोध;
	हाल CSS_OVERFLOW_SCROLL:
		ख_लिखो(stream, "overflow-x: scroll ");
		अवरोध;
	हाल CSS_OVERFLOW_AUTO:
		ख_लिखो(stream, "overflow-x auto ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* overflow */
	val = css_computed_overflow_y(style);
	चयन (val) {
	हाल CSS_OVERFLOW_VISIBLE:
		ख_लिखो(stream, "overflow-y: visible ");
		अवरोध;
	हाल CSS_OVERFLOW_HIDDEN:
		ख_लिखो(stream, "overflow-y: hidden ");
		अवरोध;
	हाल CSS_OVERFLOW_SCROLL:
		ख_लिखो(stream, "overflow-y: scroll ");
		अवरोध;
	हाल CSS_OVERFLOW_AUTO:
		ख_लिखो(stream, "overflow-y: auto ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* padding-top */
	val = css_computed_padding_शीर्ष(style, &len1, &unit1);
	चयन (val) {
	हाल CSS_PADDING_SET:
		ख_लिखो(stream, "padding-top: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* padding-right */
	val = css_computed_padding_दायाँ(style, &len1, &unit1);
	चयन (val) {
	हाल CSS_PADDING_SET:
		ख_लिखो(stream, "padding-right: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* padding-bottom */
	val = css_computed_padding_bottom(style, &len1, &unit1);
	चयन (val) {
	हाल CSS_PADDING_SET:
		ख_लिखो(stream, "padding-bottom: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* padding-left */
	val = css_computed_padding_बायाँ(style, &len1, &unit1);
	चयन (val) {
	हाल CSS_PADDING_SET:
		ख_लिखो(stream, "padding-left: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* position */
	val = css_computed_position(style);
	चयन (val) {
	हाल CSS_POSITION_STATIC:
		ख_लिखो(stream, "position: static ");
		अवरोध;
	हाल CSS_POSITION_RELATIVE:
		ख_लिखो(stream, "position: relative ");
		अवरोध;
	हाल CSS_POSITION_ABSOLUTE:
		ख_लिखो(stream, "position: absolute ");
		अवरोध;
	हाल CSS_POSITION_FIXED:
		ख_लिखो(stream, "position: fixed ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* quotes */
	val = css_computed_quotes(style, &string_list);
	अगर (val == CSS_QUOTES_STRING && string_list != शून्य) {
		ख_लिखो(stream, "quotes:");

		जबतक (*string_list != शून्य) {
			ख_लिखो(stream, " \"%.*s\"",
				(पूर्णांक) lwc_string_लंबाई(*string_list),
				lwc_string_तथ्य(*string_list));

			string_list++;
		}

		ख_लिखो(stream, " ");
	} अन्यथा {
		चयन (val) {
		हाल CSS_QUOTES_NONE:
			ख_लिखो(stream, "quotes: none ");
			अवरोध;
		शेष:
			अवरोध;
		}
	}

	/*दायाँ*/
	val = css_computed_दायाँ(style, &len1, &unit1);
	चयन (val) {
	हाल CSS_दायाँ_AUTO:
		ख_लिखो(stream, "right: auto ");
		अवरोध;
	हाल CSS_दायाँ_SET:
		ख_लिखो(stream, "right: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* table-layout */
	val = css_computed_table_layout(style);
	चयन (val) {
	हाल CSS_TABLE_LAYOUT_AUTO:
		ख_लिखो(stream, "table-layout: auto ");
		अवरोध;
	हाल CSS_TABLE_LAYOUT_FIXED:
		ख_लिखो(stream, "table-layout: fixed ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* text-align */
	val = css_computed_text_align(style);
	चयन (val) {
	हाल CSS_TEXT_ALIGN_बायाँ:
		ख_लिखो(stream, "text-align: left ");
		अवरोध;
	हाल CSS_TEXT_ALIGN_दायाँ:
		ख_लिखो(stream, "text-align: right ");
		अवरोध;
	हाल CSS_TEXT_ALIGN_CENTER:
		ख_लिखो(stream, "text-align: center ");
		अवरोध;
	हाल CSS_TEXT_ALIGN_JUSTIFY:
		ख_लिखो(stream, "text-align: justify ");
		अवरोध;
	हाल CSS_TEXT_ALIGN_DEFAULT:
		ख_लिखो(stream, "text-align: default ");
		अवरोध;
	हाल CSS_TEXT_ALIGN_LIBCSS_बायाँ:
		ख_लिखो(stream, "text-align: -libcss-left ");
		अवरोध;
	हाल CSS_TEXT_ALIGN_LIBCSS_CENTER:
		ख_लिखो(stream, "text-align: -libcss-center ");
		अवरोध;
	हाल CSS_TEXT_ALIGN_LIBCSS_दायाँ:
		ख_लिखो(stream, "text-align: -libcss-right ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* text-decoration */
	val = css_computed_text_decoration(style);
	अगर (val == CSS_TEXT_DECORATION_NONE) {
		ख_लिखो(stream, "text-decoration: none ");
	} अन्यथा {
		ख_लिखो(stream, "text-decoration:");

		अगर (val & CSS_TEXT_DECORATION_BLINK) {
			ख_लिखो(stream, " blink");
		}
		अगर (val & CSS_TEXT_DECORATION_पंक्ति_THROUGH) {
			ख_लिखो(stream, " line-through");
		}
		अगर (val & CSS_TEXT_DECORATION_OVERLINE) {
			ख_लिखो(stream, " overline");
		}
		अगर (val & CSS_TEXT_DECORATION_UNDERLINE) {
			ख_लिखो(stream, " underline");
		}

		ख_लिखो(stream, " ");
	}

	/* text-indent */
	val = css_computed_text_indent(style, &len1, &unit1);
	चयन (val) {
	हाल CSS_TEXT_INDENT_SET:
		ख_लिखो(stream, "text-indent: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* text-transform */
	val = css_computed_text_transform(style);
	चयन (val) {
	हाल CSS_TEXT_TRANSFORM_CAPITALIZE:
		ख_लिखो(stream, "text-transform: capitalize ");
		अवरोध;
	हाल CSS_TEXT_TRANSFORM_UPPERCASE:
		ख_लिखो(stream, "text-transform: uppercase ");
		अवरोध;
	हाल CSS_TEXT_TRANSFORM_LOWERCASE:
		ख_लिखो(stream, "text-transform: lowercase ");
		अवरोध;
	हाल CSS_TEXT_TRANSFORM_NONE:
		ख_लिखो(stream, "text-transform: none ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/*शीर्ष*/
	val = css_computed_शीर्ष(style, &len1, &unit1);
	चयन (val) {
	हाल CSS_शीर्ष_AUTO:
		ख_लिखो(stream, "top: auto ");
		अवरोध;
	हाल CSS_शीर्ष_SET:
		ख_लिखो(stream, "top: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* unicode-bidi */
	val = css_computed_unicode_bidi(style);
	चयन (val) {
	हाल CSS_UNICODE_BIDI_NORMAL:
		ख_लिखो(stream, "unicode-bidi: normal ");
		अवरोध;
	हाल CSS_UNICODE_BIDI_EMBED:
		ख_लिखो(stream, "unicode-bidi: embed ");
		अवरोध;
	हाल CSS_UNICODE_BIDI_BIDI_OVERRIDE:
		ख_लिखो(stream, "unicode-bidi: bidi-override ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* vertical-align */
	val = css_computed_vertical_align(style, &len1, &unit1);
	चयन (val) {
	हाल CSS_VERTICAL_ALIGN_BASELINE:
		ख_लिखो(stream, "vertical-align: baseline ");
		अवरोध;
	हाल CSS_VERTICAL_ALIGN_SUB:
		ख_लिखो(stream, "vertical-align: sub ");
		अवरोध;
	हाल CSS_VERTICAL_ALIGN_SUPER:
		ख_लिखो(stream, "vertical-align: super ");
		अवरोध;
	हाल CSS_VERTICAL_ALIGN_शीर्ष:
		ख_लिखो(stream, "vertical-align: top ");
		अवरोध;
	हाल CSS_VERTICAL_ALIGN_TEXT_शीर्ष:
		ख_लिखो(stream, "vertical-align: text-top ");
		अवरोध;
	हाल CSS_VERTICAL_ALIGN_मध्य:
		ख_लिखो(stream, "vertical-align: middle ");
		अवरोध;
	हाल CSS_VERTICAL_ALIGN_BOTTOM:
		ख_लिखो(stream, "vertical-align: bottom ");
		अवरोध;
	हाल CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
		ख_लिखो(stream, "vertical-align: text-bottom ");
		अवरोध;
	हाल CSS_VERTICAL_ALIGN_SET:
		ख_लिखो(stream, "vertical-align: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* visibility */
	val = css_computed_visibility(style);
	चयन (val) {
	हाल CSS_VISIBILITY_VISIBLE:
		ख_लिखो(stream, "visibility: visible ");
		अवरोध;
	हाल CSS_VISIBILITY_HIDDEN:
		ख_लिखो(stream, "visibility: hidden ");
		अवरोध;
	हाल CSS_VISIBILITY_COLLAPSE:
		ख_लिखो(stream, "visibility: collapse ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* white-space */
	val = css_computed_white_space(style);
	चयन (val) {
	हाल CSS_WHITE_SPACE_NORMAL:
		ख_लिखो(stream, "white-space: normal ");
		अवरोध;
	हाल CSS_WHITE_SPACE_PRE:
		ख_लिखो(stream, "white-space: pre ");
		अवरोध;
	हाल CSS_WHITE_SPACE_NOWRAP:
		ख_लिखो(stream, "white-space: nowrap ");
		अवरोध;
	हाल CSS_WHITE_SPACE_PRE_WRAP:
		ख_लिखो(stream, "white-space: pre-wrap ");
		अवरोध;
	हाल CSS_WHITE_SPACE_PRE_पंक्ति:
		ख_लिखो(stream, "white-space: pre-line ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* width */
	val = css_computed_width(style, &len1, &unit1);
	चयन (val) {
	हाल CSS_WIDTH_AUTO:
		ख_लिखो(stream, "width: auto ");
		अवरोध;
	हाल CSS_WIDTH_SET:
		ख_लिखो(stream, "width: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* word-spacing */
	val = css_computed_word_spacing(style, &len1, &unit1);
	चयन (val) {
	हाल CSS_WORD_SPACING_NORMAL:
		ख_लिखो(stream, "word-spacing: normal ");
		अवरोध;
	हाल CSS_WORD_SPACING_SET:
		ख_लिखो(stream, "word-spacing: ");

		dump_css_unit(stream, len1, unit1);

		ख_लिखो(stream, " ");
		अवरोध;
	शेष:
		अवरोध;
	}

	/* z-index */
	val = css_computed_z_सूचकांक(style, &zindex);
	चयन (val) {
	हाल CSS_Z_सूचकांक_AUTO:
		ख_लिखो(stream, "z-index: auto ");
		अवरोध;
	हाल CSS_Z_सूचकांक_SET:
		ख_लिखो(stream, "z-index: %"PRId32" ", zindex);
		अवरोध;
	शेष:
		अवरोध;
	}

	ख_लिखो(stream, "}");
}
