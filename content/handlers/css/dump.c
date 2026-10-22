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
स्थिर शून्य dump_css_fixed(FILE *प्रवाहः, css_fixed f)
{
#परिभाषय NSCSS_ABS(x) (uint32_t)((x) < 0 ? -(x) : (x))
	uint32_t uintpart = FIXTOINT(NSCSS_ABS(f));
	/* + 500 to ensure round to nearest (division will truncate) */
	uint32_t fracpart = ((NSCSS_ABS(f) & 0x3ff) * 1000 + 500) / (1 << 10);
#अपरिभाषय NSCSS_ABS

	fprintf(प्रवाहः, "%s%"PRIu32".%03"PRIu32, f < 0 ? "-" : "", uintpart, fracpart);
}

/**
 * Dump a numeric value to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 */
स्थिर शून्य dump_css_सङ्ख्या(FILE *प्रवाहः, css_fixed val)
{
	यदि (INTTOFIX(FIXTOINT(val)) == val)
		fprintf(प्रवाहः, "%"PRId32, FIXTOINT(val));
	अन्यथा
		dump_css_fixed(प्रवाहः, val);
}

/**
 * Dump a dimension to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 * \param unit    Unit to write
 */
स्थिर शून्य dump_css_unit(FILE *प्रवाहः, css_fixed val, css_unit unit)
{
	dump_css_सङ्ख्या(प्रवाहः, val);

	विकल्प (unit) {
	पक्ष CSS_UNIT_PX:
		fprintf(प्रवाहः, "px");
		भङ्ग;
	पक्ष CSS_UNIT_EX:
		fprintf(प्रवाहः, "ex");
		भङ्ग;
	पक्ष CSS_UNIT_EM:
		fprintf(प्रवाहः, "em");
		भङ्ग;
	पक्ष CSS_UNIT_अन्तर्:
		fprintf(प्रवाहः, "in");
		भङ्ग;
	पक्ष CSS_UNIT_CM:
		fprintf(प्रवाहः, "cm");
		भङ्ग;
	पक्ष CSS_UNIT_MM:
		fprintf(प्रवाहः, "mm");
		भङ्ग;
	पक्ष CSS_UNIT_PT:
		fprintf(प्रवाहः, "pt");
		भङ्ग;
	पक्ष CSS_UNIT_PC:
		fprintf(प्रवाहः, "pc");
		भङ्ग;
	पक्ष CSS_UNIT_PCT:
		fprintf(प्रवाहः, "%%");
		भङ्ग;
	पक्ष CSS_UNIT_DEG:
		fprintf(प्रवाहः, "deg");
		भङ्ग;
	पक्ष CSS_UNIT_GRAD:
		fprintf(प्रवाहः, "grad");
		भङ्ग;
	पक्ष CSS_UNIT_RAD:
		fprintf(प्रवाहः, "rad");
		भङ्ग;
	पक्ष CSS_UNIT_MS:
		fprintf(प्रवाहः, "ms");
		भङ्ग;
	पक्ष CSS_UNIT_S:
		fprintf(प्रवाहः, "s");
		भङ्ग;
	पक्ष CSS_UNIT_HZ:
		fprintf(प्रवाहः, "Hz");
		भङ्ग;
	पक्ष CSS_UNIT_KHZ:
		fprintf(प्रवाहः, "kHz");
		भङ्ग;
	पक्ष CSS_UNIT_CH:
		fprintf(प्रवाहः, "ch");
		भङ्ग;
	पक्ष CSS_UNIT_REM:
		fprintf(प्रवाहः, "rem");
		भङ्ग;
	पक्ष CSS_UNIT_LH:
		fprintf(प्रवाहः, "lh");
		भङ्ग;
	पक्ष CSS_UNIT_VH:
		fprintf(प्रवाहः, "vh");
		भङ्ग;
	पक्ष CSS_UNIT_VW:
		fprintf(प्रवाहः, "vw");
		भङ्ग;
	पक्ष CSS_UNIT_VI:
		fprintf(प्रवाहः, "vi");
		भङ्ग;
	पक्ष CSS_UNIT_VB:
		fprintf(प्रवाहः, "vb");
		भङ्ग;
	पक्ष CSS_UNIT_VMIN:
		fprintf(प्रवाहः, "vmin");
		भङ्ग;
	पक्ष CSS_UNIT_VMAX:
		fprintf(प्रवाहः, "vmax");
		भङ्ग;
	पक्ष CSS_UNIT_Q:
		fprintf(प्रवाहः, "q");
		भङ्ग;
	पक्ष CSS_UNIT_CALC:
		fprintf(प्रवाहः, "calc()");
		भङ्ग;
	}
}

/* exported interface documented in content/handlers/css/dump.h */
शून्य nscss_dump_computed_style(FILE *प्रवाहः, ध्रुव css_computed_style *style)
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
	ध्रुव css_computed_गणक *गणक = रिक्त;
	lwc_string **string_list = रिक्त;
	int32_t zindex = 0;

	fprintf(प्रवाहः, "{ ");

	/* background-attachment */
	val = css_computed_background_attachment(style);
	विकल्प (val) {
	पक्ष CSS_BACKGROUND_ATTACHMENT_FIXED:
		fprintf(प्रवाहः, "background-attachment: fixed ");
		भङ्ग;
	पक्ष CSS_BACKGROUND_ATTACHMENT_SCROLL:
		fprintf(प्रवाहः, "background-attachment: scroll ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* background-color */
	val = css_computed_background_color(style, &color);
	विकल्प (val) {
	पक्ष CSS_BACKGROUND_COLOR_COLOR:
		fprintf(प्रवाहः, "background-color: #%08"PRIx32" ", color);
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* background-image */
	val = css_computed_background_image(style, &url);
	यदि (val == CSS_BACKGROUND_IMAGE_IMAGE && url != रिक्त) {
		fprintf(प्रवाहः, "background-image: url('%.*s') ",
				(पूर्णाङ्क) lwc_string_दीर्घता(url),
				lwc_string_दत्त(url));
	} अन्यथा यदि (val == CSS_BACKGROUND_IMAGE_NONE) {
		fprintf(प्रवाहः, "background-image: none ");
	}

	/* background-position */
	val = css_computed_background_position(style, &len1, &unit1,
			&len2, &unit2);
	यदि (val == CSS_BACKGROUND_POSITION_समुच्चयः) {
		fprintf(प्रवाहः, "background-position: ");
		dump_css_unit(प्रवाहः, len1, unit1);
		fprintf(प्रवाहः, " ");
		dump_css_unit(प्रवाहः, len2, unit2);
		fprintf(प्रवाहः, " ");
	}

	/* background-repeat */
	val = css_computed_background_repeat(style);
	विकल्प (val) {
	पक्ष CSS_BACKGROUND_REPEAT_REPEAT_X:
		fprintf(प्रवाहः, "background-repeat: repeat-x ");
		भङ्ग;
	पक्ष CSS_BACKGROUND_REPEAT_REPEAT_Y:
		fprintf(प्रवाहः, "background-repeat: repeat-y ");
		भङ्ग;
	पक्ष CSS_BACKGROUND_REPEAT_REPEAT:
		fprintf(प्रवाहः, "background-repeat: repeat ");
		भङ्ग;
	पक्ष CSS_BACKGROUND_REPEAT_NO_REPEAT:
		fprintf(प्रवाहः, "background-repeat: no-repeat ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* border-collapse */
	val = css_computed_border_collapse(style);
	विकल्प (val) {
	पक्ष CSS_BORDER_COLLAPSE_SEPARATE:
		fprintf(प्रवाहः, "border-collapse: separate ");
		भङ्ग;
	पक्ष CSS_BORDER_COLLAPSE_COLLAPSE:
		fprintf(प्रवाहः, "border-collapse: collapse ");
		भङ्ग;
	स्वभाव:

		भङ्ग;
	}

	/* border-spacing */
	val = css_computed_border_spacing(style, &len1, &unit1, &len2, &unit2);
	यदि (val == CSS_BORDER_SPACING_समुच्चयः) {
		fprintf(प्रवाहः, "border-spacing: ");
		dump_css_unit(प्रवाहः, len1, unit1);
		fprintf(प्रवाहः, " ");
		dump_css_unit(प्रवाहः, len2, unit2);
		fprintf(प्रवाहः, " ");
	}

	/* border-top-color */
	val = css_computed_border_शिखरम्_color(style, &color);
	विकल्प (val) {
	पक्ष CSS_BORDER_COLOR_COLOR:
		fprintf(प्रवाहः, "border-top-color: #%08"PRIx32" ", color);
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* border-right-color */
	val = css_computed_border_दक्षिणः_color(style, &color);
	विकल्प (val) {
	पक्ष CSS_BORDER_COLOR_COLOR:
		fprintf(प्रवाहः, "border-right-color: #%08"PRIx32" ", color);
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* border-bottom-color */
	val = css_computed_border_bottom_color(style, &color);
	विकल्प (val) {
	पक्ष CSS_BORDER_COLOR_COLOR:
		fprintf(प्रवाहः, "border-bottom-color: #%08"PRIx32" ", color);
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* border-left-color */
	val = css_computed_border_वामः_color(style, &color);
	विकल्प (val) {
	पक्ष CSS_BORDER_COLOR_COLOR:
		fprintf(प्रवाहः, "border-left-color: #%08"PRIx32" ", color);
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* border-top-style */
	val = css_computed_border_शिखरम्_style(style);
	विकल्प (val) {
	पक्ष CSS_BORDER_STYLE_NONE:
		fprintf(प्रवाहः, "border-top-style: none ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_HIDDEN:
		fprintf(प्रवाहः, "border-top-style: hidden ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_DOTTED:
		fprintf(प्रवाहः, "border-top-style: dotted ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_DASHED:
		fprintf(प्रवाहः, "border-top-style: dashed ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_SOLID:
		fprintf(प्रवाहः, "border-top-style: solid ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_DOUBLE:
		fprintf(प्रवाहः, "border-top-style: double ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_GROOVE:
		fprintf(प्रवाहः, "border-top-style: groove ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_RIDGE:
		fprintf(प्रवाहः, "border-top-style: ridge ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_INSET:
		fprintf(प्रवाहः, "border-top-style: inset ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_OUTSET:
		fprintf(प्रवाहः, "border-top-style: outset ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* border-right-style */
	val = css_computed_border_दक्षिणः_style(style);
	विकल्प (val) {
	पक्ष CSS_BORDER_STYLE_NONE:
		fprintf(प्रवाहः, "border-right-style: none ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_HIDDEN:
		fprintf(प्रवाहः, "border-right-style: hidden ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_DOTTED:
		fprintf(प्रवाहः, "border-right-style: dotted ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_DASHED:
		fprintf(प्रवाहः, "border-right-style: dashed ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_SOLID:
		fprintf(प्रवाहः, "border-right-style: solid ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_DOUBLE:
		fprintf(प्रवाहः, "border-right-style: double ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_GROOVE:
		fprintf(प्रवाहः, "border-right-style: groove ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_RIDGE:
		fprintf(प्रवाहः, "border-right-style: ridge ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_INSET:
		fprintf(प्रवाहः, "border-right-style: inset ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_OUTSET:
		fprintf(प्रवाहः, "border-right-style: outset ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* border-bottom-style */
	val = css_computed_border_bottom_style(style);
	विकल्प (val) {
	पक्ष CSS_BORDER_STYLE_NONE:
		fprintf(प्रवाहः, "border-bottom-style: none ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_HIDDEN:
		fprintf(प्रवाहः, "border-bottom-style: hidden ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_DOTTED:
		fprintf(प्रवाहः, "border-bottom-style: dotted ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_DASHED:
		fprintf(प्रवाहः, "border-bottom-style: dashed ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_SOLID:
		fprintf(प्रवाहः, "border-bottom-style: solid ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_DOUBLE:
		fprintf(प्रवाहः, "border-bottom-style: double ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_GROOVE:
		fprintf(प्रवाहः, "border-bottom-style: groove ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_RIDGE:
		fprintf(प्रवाहः, "border-bottom-style: ridge ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_INSET:
		fprintf(प्रवाहः, "border-bottom-style: inset ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_OUTSET:
		fprintf(प्रवाहः, "border-bottom-style: outset ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* border-left-style */
	val = css_computed_border_वामः_style(style);
	विकल्प (val) {
	पक्ष CSS_BORDER_STYLE_NONE:
		fprintf(प्रवाहः, "border-left-style: none ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_HIDDEN:
		fprintf(प्रवाहः, "border-left-style: hidden ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_DOTTED:
		fprintf(प्रवाहः, "border-left-style: dotted ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_DASHED:
		fprintf(प्रवाहः, "border-left-style: dashed ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_SOLID:
		fprintf(प्रवाहः, "border-left-style: solid ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_DOUBLE:
		fprintf(प्रवाहः, "border-left-style: double ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_GROOVE:
		fprintf(प्रवाहः, "border-left-style: groove ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_RIDGE:
		fprintf(प्रवाहः, "border-left-style: ridge ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_INSET:
		fprintf(प्रवाहः, "border-left-style: inset ");
		भङ्ग;
	पक्ष CSS_BORDER_STYLE_OUTSET:
		fprintf(प्रवाहः, "border-left-style: outset ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* border-top-width */
	val = css_computed_border_शिखरम्_विस्तारः(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_BORDER_विस्तारः_THIN:
		fprintf(प्रवाहः, "border-top-width: thin ");
		भङ्ग;
	पक्ष CSS_BORDER_विस्तारः_MEDIUM:
		fprintf(प्रवाहः, "border-top-width: medium ");
		भङ्ग;
	पक्ष CSS_BORDER_विस्तारः_THICK:
		fprintf(प्रवाहः, "border-top-width: thick ");
		भङ्ग;
	पक्ष CSS_BORDER_विस्तारः_विस्तारः:
		fprintf(प्रवाहः, "border-top-width: ");
		dump_css_unit(प्रवाहः, len1, unit1);
		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* border-right-width */
	val = css_computed_border_दक्षिणः_विस्तारः(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_BORDER_विस्तारः_THIN:
		fprintf(प्रवाहः, "border-right-width: thin ");
		भङ्ग;
	पक्ष CSS_BORDER_विस्तारः_MEDIUM:
		fprintf(प्रवाहः, "border-right-width: medium ");
		भङ्ग;
	पक्ष CSS_BORDER_विस्तारः_THICK:
		fprintf(प्रवाहः, "border-right-width: thick ");
		भङ्ग;
	पक्ष CSS_BORDER_विस्तारः_विस्तारः:
		fprintf(प्रवाहः, "border-right-width: ");
		dump_css_unit(प्रवाहः, len1, unit1);
		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* border-bottom-width */
	val = css_computed_border_bottom_विस्तारः(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_BORDER_विस्तारः_THIN:
		fprintf(प्रवाहः, "border-bottom-width: thin ");
		भङ्ग;
	पक्ष CSS_BORDER_विस्तारः_MEDIUM:
		fprintf(प्रवाहः, "border-bottom-width: medium ");
		भङ्ग;
	पक्ष CSS_BORDER_विस्तारः_THICK:
		fprintf(प्रवाहः, "border-bottom-width: thick ");
		भङ्ग;
	पक्ष CSS_BORDER_विस्तारः_विस्तारः:
		fprintf(प्रवाहः, "border-bottom-width: ");
		dump_css_unit(प्रवाहः, len1, unit1);
		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* border-left-width */
	val = css_computed_border_वामः_विस्तारः(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_BORDER_विस्तारः_THIN:
		fprintf(प्रवाहः, "border-left-width: thin ");
		भङ्ग;
	पक्ष CSS_BORDER_विस्तारः_MEDIUM:
		fprintf(प्रवाहः, "border-left-width: medium ");
		भङ्ग;
	पक्ष CSS_BORDER_विस्तारः_THICK:
		fprintf(प्रवाहः, "border-left-width: thick ");
		भङ्ग;
	पक्ष CSS_BORDER_विस्तारः_विस्तारः:
		fprintf(प्रवाहः, "border-left-width: ");
		dump_css_unit(प्रवाहः, len1, unit1);
		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* bottom */
	val = css_computed_bottom(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_BOTTOM_AUTO:
		fprintf(प्रवाहः, "bottom: auto ");
		भङ्ग;
	पक्ष CSS_BOTTOM_समुच्चयः:
		fprintf(प्रवाहः, "bottom: ");
		dump_css_unit(प्रवाहः, len1, unit1);
		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* caption-side */
	val = css_computed_caption_side(style);
	विकल्प (val) {
	पक्ष CSS_CAPTION_SIDE_शिखरम्:
		fprintf(प्रवाहः, "caption_side: top ");
		भङ्ग;
	पक्ष CSS_CAPTION_SIDE_BOTTOM:
		fprintf(प्रवाहः, "caption_side: bottom ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* clear */
	val = css_computed_clear(style);
	विकल्प (val) {
	पक्ष CSS_CLEAR_NONE:
		fprintf(प्रवाहः, "clear: none ");
		भङ्ग;
	पक्ष CSS_CLEAR_वामः:
		fprintf(प्रवाहः, "clear: left ");
		भङ्ग;
	पक्ष CSS_CLEAR_दक्षिणः:
		fprintf(प्रवाहः, "clear: right ");
		भङ्ग;
	पक्ष CSS_CLEAR_BOTH:
		fprintf(प्रवाहः, "clear: both ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* clip */
	val = css_computed_clip(style, &rect);
	विकल्प (val) {
	पक्ष CSS_CLIP_AUTO:
		fprintf(प्रवाहः, "clip: auto ");
		भङ्ग;
	पक्ष CSS_CLIP_RECT:
		fprintf(प्रवाहः, "clip: rect( ");

		यदि (rect.शिखरम्_auto)
			fprintf(प्रवाहः, "auto");
		अन्यथा
			dump_css_unit(प्रवाहः, rect.शिखरम्, rect.tunit);
		fprintf(प्रवाहः, ", ");

		यदि (rect.दक्षिणः_auto)
			fprintf(प्रवाहः, "auto");
		अन्यथा
			dump_css_unit(प्रवाहः, rect.दक्षिणः, rect.runit);
		fprintf(प्रवाहः, ", ");

		यदि (rect.bottom_auto)
			fprintf(प्रवाहः, "auto");
		अन्यथा
			dump_css_unit(प्रवाहः, rect.bottom, rect.bunit);
		fprintf(प्रवाहः, ", ");

		यदि (rect.वामः_auto)
			fprintf(प्रवाहः, "auto");
		अन्यथा
			dump_css_unit(प्रवाहः, rect.वामः, rect.lunit);
		fprintf(प्रवाहः, ") ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* color */
	val = css_computed_color(style, &color);
	यदि (val == CSS_COLOR_COLOR) {
		fprintf(प्रवाहः, "color: #%08"PRIx32" ", color);
	}

	/* content */
	val = css_computed_content(style, &content);
	विकल्प (val) {
	पक्ष CSS_CONTENT_NONE:
		fprintf(प्रवाहः, "content: none ");
		भङ्ग;
	पक्ष CSS_CONTENT_प्रसामान्यम्:
		fprintf(प्रवाहः, "content: normal ");
		भङ्ग;
	पक्ष CSS_CONTENT_समुच्चयः:
		fprintf(प्रवाहः, "content:");

		यावत् (content->type != CSS_COMPUTED_CONTENT_NONE) {
			fprintf(प्रवाहः, " ");

			विकल्प (content->type) {
			पक्ष CSS_COMPUTED_CONTENT_STRING:
				fprintf(प्रवाहः,	"\"%.*s\"",
						(पूर्णाङ्क) lwc_string_दीर्घता(
						content->दत्त.string),
						lwc_string_दत्त(
						content->दत्त.string));
				भङ्ग;
			पक्ष CSS_COMPUTED_CONTENT_URI:
				fprintf(प्रवाहः,	"uri(\"%.*s\")",
						(पूर्णाङ्क) lwc_string_दीर्घता(
						content->दत्त.uri),
						lwc_string_दत्त(
						content->दत्त.uri));
				भङ्ग;
			पक्ष CSS_COMPUTED_CONTENT_गणक:
				fprintf(प्रवाहः, "counter(%.*s)",
						(पूर्णाङ्क) lwc_string_दीर्घता(
						content->दत्त.गणक.name),
						lwc_string_दत्त(
						content->दत्त.गणक.name));
				भङ्ग;
			पक्ष CSS_COMPUTED_CONTENT_COUNTERS:
				fprintf(प्रवाहः, "counters(%.*s, \"%.*s\")",
						(पूर्णाङ्क) lwc_string_दीर्घता(
						content->दत्त.counters.name),
						lwc_string_दत्त(
						content->दत्त.counters.name),
						(पूर्णाङ्क) lwc_string_दीर्घता(
						content->दत्त.counters.sep),
						lwc_string_दत्त(
						content->दत्त.counters.sep));
				भङ्ग;
			पक्ष CSS_COMPUTED_CONTENT_ATTR:
				fprintf(प्रवाहः, "attr(%.*s)",
						(पूर्णाङ्क) lwc_string_दीर्घता(
						content->दत्त.attr),
						lwc_string_दत्त(
						content->दत्त.attr));
				भङ्ग;
			पक्ष CSS_COMPUTED_CONTENT_OPEN_QUOTE:
				fprintf(प्रवाहः, "open-quote");
				भङ्ग;
			पक्ष CSS_COMPUTED_CONTENT_CLOSE_QUOTE:
				fprintf(प्रवाहः, "close-quote");
				भङ्ग;
			पक्ष CSS_COMPUTED_CONTENT_NO_OPEN_QUOTE:
				fprintf(प्रवाहः, "no-open-quote");
				भङ्ग;
			पक्ष CSS_COMPUTED_CONTENT_NO_CLOSE_QUOTE:
				fprintf(प्रवाहः, "no-close-quote");
				भङ्ग;
			}

			content++;
		}

		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* counter-increment */
	val = css_computed_गणक_increment(style, &गणक);
	यदि ((val == CSS_गणक_INCREMENT_NONE) || (गणक == रिक्त)) {
		fprintf(प्रवाहः, "counter-increment: none ");
	} अन्यथा {
		fprintf(प्रवाहः, "counter-increment:");

		यावत् (गणक->name != रिक्त) {
			fprintf(प्रवाहः, " %.*s ",
					(पूर्णाङ्क) lwc_string_दीर्घता(गणक->name),
					lwc_string_दत्त(गणक->name));

			dump_css_fixed(प्रवाहः, गणक->मानम्);

			गणक++;
		}

		fprintf(प्रवाहः, " ");
	}

	/* counter-reset */
	val = css_computed_गणक_reset(style, &गणक);
	यदि ((val == CSS_गणक_RESET_NONE) || (गणक == रिक्त)) {
		fprintf(प्रवाहः, "counter-reset: none ");
	} अन्यथा {
		fprintf(प्रवाहः, "counter-reset:");

		यावत् (गणक->name != रिक्त) {
			fprintf(प्रवाहः, " %.*s ",
					(पूर्णाङ्क) lwc_string_दीर्घता(गणक->name),
					lwc_string_दत्त(गणक->name));

			dump_css_fixed(प्रवाहः, गणक->मानम्);

			गणक++;
		}

		fprintf(प्रवाहः, " ");
	}

	/* cursor */
	val = css_computed_cursor(style, &string_list);
	fprintf(प्रवाहः, "cursor:");

	यदि (string_list != रिक्त) {
		यावत् (*string_list != रिक्त) {
			fprintf(प्रवाहः, " url\"%.*s\")",
					(पूर्णाङ्क) lwc_string_दीर्घता(*string_list),
					lwc_string_दत्त(*string_list));

			string_list++;
		}
	}
	विकल्प (val) {
	पक्ष CSS_CURSOR_AUTO:
		fprintf(प्रवाहः, " auto ");
		भङ्ग;
	पक्ष CSS_CURSOR_CROSSHAIR:
		fprintf(प्रवाहः, " crosshair ");
		भङ्ग;
	पक्ष CSS_CURSOR_DEFAULT:
		fprintf(प्रवाहः, " default ");
		भङ्ग;
	पक्ष CSS_CURSOR_POINTER:
		fprintf(प्रवाहः, " pointer ");
		भङ्ग;
	पक्ष CSS_CURSOR_MOVE:
		fprintf(प्रवाहः, " move ");
		भङ्ग;
	पक्ष CSS_CURSOR_E_RESIZE:
		fprintf(प्रवाहः, " e-resize ");
		भङ्ग;
	पक्ष CSS_CURSOR_NE_RESIZE:
		fprintf(प्रवाहः, " ne-resize ");
		भङ्ग;
	पक्ष CSS_CURSOR_NW_RESIZE:
		fprintf(प्रवाहः, " nw-resize ");
		भङ्ग;
	पक्ष CSS_CURSOR_N_RESIZE:
		fprintf(प्रवाहः, " n-resize ");
		भङ्ग;
	पक्ष CSS_CURSOR_SE_RESIZE:
		fprintf(प्रवाहः, " se-resize ");
		भङ्ग;
	पक्ष CSS_CURSOR_SW_RESIZE:
		fprintf(प्रवाहः, " sw-resize ");
		भङ्ग;
	पक्ष CSS_CURSOR_S_RESIZE:
		fprintf(प्रवाहः, " s-resize ");
		भङ्ग;
	पक्ष CSS_CURSOR_W_RESIZE:
		fprintf(प्रवाहः, " w-resize ");
		भङ्ग;
	पक्ष CSS_CURSOR_TEXT:
		fprintf(प्रवाहः, " text ");
		भङ्ग;
	पक्ष CSS_CURSOR_WAIT:
		fprintf(प्रवाहः, " wait ");
		भङ्ग;
	पक्ष CSS_CURSOR_HELP:
		fprintf(प्रवाहः, " help ");
		भङ्ग;
	पक्ष CSS_CURSOR_प्रगतिः:
		fprintf(प्रवाहः, " progress ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* direction */
	val = css_computed_direction(style);
	विकल्प (val) {
	पक्ष CSS_DIRECTION_LTR:
		fprintf(प्रवाहः, "direction: ltr ");
		भङ्ग;
	पक्ष CSS_DIRECTION_RTL:
		fprintf(प्रवाहः, "direction: rtl ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* display */
	val = ns_computed_display_static(style);
	विकल्प (val) {
	पक्ष CSS_DISPLAY_INLINE:
		fprintf(प्रवाहः, "display: inline ");
		भङ्ग;
	पक्ष CSS_DISPLAY_खण्डम्:
		fprintf(प्रवाहः, "display: block ");
		भङ्ग;
	पक्ष CSS_DISPLAY_LIST_ITEM:
		fprintf(प्रवाहः, "display: list-item ");
		भङ्ग;
	पक्ष CSS_DISPLAY_सततम्_अन्तर्:
		fprintf(प्रवाहः, "display: run-in ");
		भङ्ग;
	पक्ष CSS_DISPLAY_INLINE_खण्डम्:
		fprintf(प्रवाहः, "display: inline-block ");
		भङ्ग;
	पक्ष CSS_DISPLAY_सारणी:
		fprintf(प्रवाहः, "display: table ");
		भङ्ग;
	पक्ष CSS_DISPLAY_INLINE_सारणी:
		fprintf(प्रवाहः, "display: inline-table ");
		भङ्ग;
	पक्ष CSS_DISPLAY_सारणी_ROW_गणः:
		fprintf(प्रवाहः, "display: table-row-group ");
		भङ्ग;
	पक्ष CSS_DISPLAY_सारणी_शीर्षकम्_गणः:
		fprintf(प्रवाहः, "display: table-header-group ");
		भङ्ग;
	पक्ष CSS_DISPLAY_सारणी_FOOTER_गणः:
		fprintf(प्रवाहः, "display: table-footer-group ");
		भङ्ग;
	पक्ष CSS_DISPLAY_सारणी_ROW:
		fprintf(प्रवाहः, "display: table-row ");
		भङ्ग;
	पक्ष CSS_DISPLAY_सारणी_COLUMN_गणः:
		fprintf(प्रवाहः, "display: table-column-group ");
		भङ्ग;
	पक्ष CSS_DISPLAY_सारणी_COLUMN:
		fprintf(प्रवाहः, "display: table-column ");
		भङ्ग;
	पक्ष CSS_DISPLAY_सारणी_CELL:
		fprintf(प्रवाहः, "display: table-cell ");
		भङ्ग;
	पक्ष CSS_DISPLAY_सारणी_CAPTION:
		fprintf(प्रवाहः, "display: table-caption ");
		भङ्ग;
	पक्ष CSS_DISPLAY_NONE:
		fprintf(प्रवाहः, "display: none ");
		भङ्ग;
	पक्ष CSS_DISPLAY_FLEX:
		fprintf(प्रवाहः, "display: flex ");
		भङ्ग;
	पक्ष CSS_DISPLAY_INLINE_FLEX:
		fprintf(प्रवाहः, "display: inline-flex ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* empty-cells */
	val = css_computed_रिक्तानि_cells(style);
	विकल्प (val) {
	पक्ष CSS_रिक्तानि_CELLS_SHOW:
		fprintf(प्रवाहः, "empty-cells: show ");
		भङ्ग;
	पक्ष CSS_रिक्तानि_CELLS_HIDE:
		fprintf(प्रवाहः, "empty-cells: hide ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* float */
	val = css_computed_float(style);
	विकल्प (val) {
	पक्ष CSS_FLOAT_वामः:
		fprintf(प्रवाहः, "float: left ");
		भङ्ग;
	पक्ष CSS_FLOAT_दक्षिणः:
		fprintf(प्रवाहः, "float: right ");
		भङ्ग;
	पक्ष CSS_FLOAT_NONE:
		fprintf(प्रवाहः, "float: none ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* font-family */
	val = css_computed_font_family(style, &string_list);
	यदि (val != CSS_FONT_FAMILY_INHERIT) {
		fprintf(प्रवाहः, "font-family:");

		यदि (string_list != रिक्त) {
			यावत् (*string_list != रिक्त) {
				fprintf(प्रवाहः, " \"%.*s\"",
					(पूर्णाङ्क) lwc_string_दीर्घता(*string_list),
					lwc_string_दत्त(*string_list));

				string_list++;
			}
		}
		विकल्प (val) {
		पक्ष CSS_FONT_FAMILY_SERIF:
			fprintf(प्रवाहः, " serif ");
			भङ्ग;
		पक्ष CSS_FONT_FAMILY_SANS_SERIF:
			fprintf(प्रवाहः, " sans-serif ");
			भङ्ग;
		पक्ष CSS_FONT_FAMILY_CURSIVE:
			fprintf(प्रवाहः, " cursive ");
			भङ्ग;
		पक्ष CSS_FONT_FAMILY_FANTASY:
			fprintf(प्रवाहः, " fantasy ");
			भङ्ग;
		पक्ष CSS_FONT_FAMILY_MONOSPACE:
			fprintf(प्रवाहः, " monospace ");
			भङ्ग;
		}
	}

	/* font-size */
	val = css_computed_font_आकार(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_FONT_आकार_XX_SMALL:
		fprintf(प्रवाहः, "font-size: xx-small ");
		भङ्ग;
	पक्ष CSS_FONT_आकार_X_SMALL:
		fprintf(प्रवाहः, "font-size: x-small ");
		भङ्ग;
	पक्ष CSS_FONT_आकार_SMALL:
		fprintf(प्रवाहः, "font-size: small ");
		भङ्ग;
	पक्ष CSS_FONT_आकार_MEDIUM:
		fprintf(प्रवाहः, "font-size: medium ");
		भङ्ग;
	पक्ष CSS_FONT_आकार_LARGE:
		fprintf(प्रवाहः, "font-size: large ");
		भङ्ग;
	पक्ष CSS_FONT_आकार_X_LARGE:
		fprintf(प्रवाहः, "font-size: x-large ");
		भङ्ग;
	पक्ष CSS_FONT_आकार_XX_LARGE:
		fprintf(प्रवाहः, "font-size: xx-large ");
		भङ्ग;
	पक्ष CSS_FONT_आकार_LARGER:
		fprintf(प्रवाहः, "font-size: larger ");
		भङ्ग;
	पक्ष CSS_FONT_आकार_SMALLER:
		fprintf(प्रवाहः, "font-size: smaller ");
		भङ्ग;
	पक्ष CSS_FONT_आकार_DIMENSION:
		fprintf(प्रवाहः, "font-size: ");

		dump_css_unit(प्रवाहः, len1, unit1);

		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* font-style */
	val = css_computed_font_style(style);
	विकल्प (val) {
	पक्ष CSS_FONT_STYLE_प्रसामान्यम्:
		fprintf(प्रवाहः, "font-style: normal ");
		भङ्ग;
	पक्ष CSS_FONT_STYLE_ITALIC:
		fprintf(प्रवाहः, "font-style: italic ");
		भङ्ग;
	पक्ष CSS_FONT_STYLE_OBLIQUE:
		fprintf(प्रवाहः, "font-style: oblique ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* font-variant */
	val = css_computed_font_variant(style);
	विकल्प (val) {
	पक्ष CSS_FONT_VARIANT_प्रसामान्यम्:
		fprintf(प्रवाहः, "font-variant: normal ");
		भङ्ग;
	पक्ष CSS_FONT_VARIANT_SMALL_CAPS:
		fprintf(प्रवाहः, "font-variant: small-caps ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* font-weight */
	val = css_computed_font_weight(style);
	विकल्प (val) {
	पक्ष CSS_FONT_WEIGHT_प्रसामान्यम्:
		fprintf(प्रवाहः, "font-weight: normal ");
		भङ्ग;
	पक्ष CSS_FONT_WEIGHT_BOLD:
		fprintf(प्रवाहः, "font-weight: bold ");
		भङ्ग;
	पक्ष CSS_FONT_WEIGHT_BOLDER:
		fprintf(प्रवाहः, "font-weight: bolder ");
		भङ्ग;
	पक्ष CSS_FONT_WEIGHT_LIGHTER:
		fprintf(प्रवाहः, "font-weight: lighter ");
		भङ्ग;
	पक्ष CSS_FONT_WEIGHT_100:
		fprintf(प्रवाहः, "font-weight: 100 ");
		भङ्ग;
	पक्ष CSS_FONT_WEIGHT_200:
		fprintf(प्रवाहः, "font-weight: 200 ");
		भङ्ग;
	पक्ष CSS_FONT_WEIGHT_300:
		fprintf(प्रवाहः, "font-weight: 300 ");
		भङ्ग;
	पक्ष CSS_FONT_WEIGHT_400:
		fprintf(प्रवाहः, "font-weight: 400 ");
		भङ्ग;
	पक्ष CSS_FONT_WEIGHT_500:
		fprintf(प्रवाहः, "font-weight: 500 ");
		भङ्ग;
	पक्ष CSS_FONT_WEIGHT_600:
		fprintf(प्रवाहः, "font-weight: 600 ");
		भङ्ग;
	पक्ष CSS_FONT_WEIGHT_700:
		fprintf(प्रवाहः, "font-weight: 700 ");
		भङ्ग;
	पक्ष CSS_FONT_WEIGHT_800:
		fprintf(प्रवाहः, "font-weight: 800 ");
		भङ्ग;
	पक्ष CSS_FONT_WEIGHT_900:
		fprintf(प्रवाहः, "font-weight: 900 ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* height */
	val = css_computed_height(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_HEIGHT_AUTO:
		fprintf(प्रवाहः, "height: auto ");
		भङ्ग;
	पक्ष CSS_HEIGHT_समुच्चयः:
		fprintf(प्रवाहः, "height: ");

		dump_css_unit(प्रवाहः, len1, unit1);

		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/*वामः*/
	val = css_computed_वामः(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_वामः_AUTO:
		fprintf(प्रवाहः, "left: auto ");
		भङ्ग;
	पक्ष CSS_वामः_समुच्चयः:
		fprintf(प्रवाहः, "left: ");

		dump_css_unit(प्रवाहः, len1, unit1);

		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* letter-spacing */
	val = css_computed_letter_spacing(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_LETTER_SPACING_प्रसामान्यम्:
		fprintf(प्रवाहः, "letter-spacing: normal ");
		भङ्ग;
	पक्ष CSS_LETTER_SPACING_समुच्चयः:
		fprintf(प्रवाहः, "letter-spacing: ");

		dump_css_unit(प्रवाहः, len1, unit1);

		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* line-height */
	val = css_computed_पङ्क्तिः_height(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_पङ्क्तिः_HEIGHT_प्रसामान्यम्:
		fprintf(प्रवाहः, "line-height: normal ");
		भङ्ग;
	पक्ष CSS_पङ्क्तिः_HEIGHT_सङ्ख्या:
		fprintf(प्रवाहः, "line-height: ");

		dump_css_fixed(प्रवाहः, len1);

		fprintf(प्रवाहः, " ");
		भङ्ग;
	पक्ष CSS_पङ्क्तिः_HEIGHT_DIMENSION:
		fprintf(प्रवाहः, "line-height: ");

		dump_css_unit(प्रवाहः, len1, unit1);

		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* list-style-image */
	val = css_computed_list_style_image(style, &url);
	यदि (url != रिक्त) {
		fprintf(प्रवाहः, "list-style-image: url('%.*s') ",
				(पूर्णाङ्क) lwc_string_दीर्घता(url),
				lwc_string_दत्त(url));
	} अन्यथा यदि (val == CSS_LIST_STYLE_IMAGE_NONE) {
		fprintf(प्रवाहः, "list-style-image: none ");
	}

	/* list-style-position */
	val = css_computed_list_style_position(style);
	विकल्प (val) {
	पक्ष CSS_LIST_STYLE_POSITION_INSIDE:
		fprintf(प्रवाहः, "list-style-position: inside ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_POSITION_OUTSIDE:
		fprintf(प्रवाहः, "list-style-position: outside ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* list-style-type */
	val = css_computed_list_style_type(style);
	विकल्प (val) {
	पक्ष CSS_LIST_STYLE_TYPE_DISC:
		fprintf(प्रवाहः, "list-style-type: disc ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_TYPE_CIRCLE:
		fprintf(प्रवाहः, "list-style-type: circle ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_TYPE_वर्गफलम्:
		fprintf(प्रवाहः, "list-style-type: square ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_TYPE_DECIMAL:
		fprintf(प्रवाहः, "list-style-type: decimal ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_TYPE_DECIMAL_LEADING_ZERO:
		fprintf(प्रवाहः, "list-style-type: decimal-leading-zero ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_TYPE_अधस्तनम्_ROMAN:
		fprintf(प्रवाहः, "list-style-type: lower-roman ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_TYPE_ऊर्ध्वस्थम्_ROMAN:
		fprintf(प्रवाहः, "list-style-type: upper-roman ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_TYPE_अधस्तनम्_GREEK:
		fprintf(प्रवाहः, "list-style-type: lower-greek ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_TYPE_अधस्तनम्_LATIN:
		fprintf(प्रवाहः, "list-style-type: lower-latin ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_TYPE_ऊर्ध्वस्थम्_LATIN:
		fprintf(प्रवाहः, "list-style-type: upper-latin ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_TYPE_ARMENIAN:
		fprintf(प्रवाहः, "list-style-type: armenian ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_TYPE_GEORGIAN:
		fprintf(प्रवाहः, "list-style-type: georgian ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_TYPE_अधस्तनम्_ALPHA:
		fprintf(प्रवाहः, "list-style-type: lower-alpha ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_TYPE_ऊर्ध्वस्थम्_ALPHA:
		fprintf(प्रवाहः, "list-style-type: upper-alpha ");
		भङ्ग;
	पक्ष CSS_LIST_STYLE_TYPE_NONE:
		fprintf(प्रवाहः, "list-style-type: none ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* margin-top */
	val = css_computed_margin_शिखरम्(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_MARGIN_AUTO:
		fprintf(प्रवाहः, "margin-top: auto ");
		भङ्ग;
	पक्ष CSS_MARGIN_समुच्चयः:
		fprintf(प्रवाहः, "margin-top: ");

		dump_css_unit(प्रवाहः, len1, unit1);

		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* margin-right */
	val = css_computed_margin_दक्षिणः(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_MARGIN_AUTO:
		fprintf(प्रवाहः, "margin-right: auto ");
		भङ्ग;
	पक्ष CSS_MARGIN_समुच्चयः:
		fprintf(प्रवाहः, "margin-right: ");

		dump_css_unit(प्रवाहः, len1, unit1);

		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* margin-bottom */
	val = css_computed_margin_bottom(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_MARGIN_AUTO:
		fprintf(प्रवाहः, "margin-bottom: auto ");
		भङ्ग;
	पक्ष CSS_MARGIN_समुच्चयः:
		fprintf(प्रवाहः, "margin-bottom: ");

		dump_css_unit(प्रवाहः, len1, unit1);

		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* margin-left */
	val = css_computed_margin_वामः(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_MARGIN_AUTO:
		fprintf(प्रवाहः, "margin-left: auto ");
		भङ्ग;
	पक्ष CSS_MARGIN_समुच्चयः:
		fprintf(प्रवाहः, "margin-left: ");

		dump_css_unit(प्रवाहः, len1, unit1);

		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* max-height */
	val = css_computed_max_height(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_MAX_HEIGHT_NONE:
		fprintf(प्रवाहः, "max-height: none ");
		भङ्ग;
	पक्ष CSS_MAX_HEIGHT_समुच्चयः:
		fprintf(प्रवाहः, "max-height: ");

		dump_css_unit(प्रवाहः, len1, unit1);

		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* max-width */
	val = css_computed_max_विस्तारः(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_MAX_विस्तारः_NONE:
		fprintf(प्रवाहः, "max-width: none ");
		भङ्ग;
	पक्ष CSS_MAX_विस्तारः_समुच्चयः:
		fprintf(प्रवाहः, "max-width: ");

		dump_css_unit(प्रवाहः, len1, unit1);

		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* min-height */
	val = ns_computed_min_height(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_MIN_HEIGHT_समुच्चयः:
		fprintf(प्रवाहः, "min-height: ");

		dump_css_unit(प्रवाहः, len1, unit1);

		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* min-width */
	val = ns_computed_min_विस्तारः(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_MIN_विस्तारः_समुच्चयः:
		fprintf(प्रवाहः, "min-width: ");

		dump_css_unit(प्रवाहः, len1, unit1);

		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* opacity */
	val = css_computed_opacity(style, &len1);
	विकल्प (val) {
	पक्ष CSS_OPACITY_समुच्चयः:
		fprintf(प्रवाहः, "opacity: ");

		dump_css_fixed(प्रवाहः, len1);

		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* outline-color */
	val = css_computed_outline_color(style, &color);
	विकल्प (val) {
	पक्ष CSS_OUTLINE_COLOR_INVERT:
		fprintf(प्रवाहः, "outline-color: invert ");
		भङ्ग;
	पक्ष CSS_OUTLINE_COLOR_COLOR:
		fprintf(प्रवाहः, "outline-color: #%08"PRIx32" ", color);
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* outline-style */
	val = css_computed_outline_style(style);
	विकल्प (val) {
	पक्ष CSS_OUTLINE_STYLE_NONE:
		fprintf(प्रवाहः, "outline-style: none ");
		भङ्ग;
	पक्ष CSS_OUTLINE_STYLE_DOTTED:
		fprintf(प्रवाहः, "outline-style: dotted ");
		भङ्ग;
	पक्ष CSS_OUTLINE_STYLE_DASHED:
		fprintf(प्रवाहः, "outline-style: dashed ");
		भङ्ग;
	पक्ष CSS_OUTLINE_STYLE_SOLID:
		fprintf(प्रवाहः, "outline-style: solid ");
		भङ्ग;
	पक्ष CSS_OUTLINE_STYLE_DOUBLE:
		fprintf(प्रवाहः, "outline-style: double ");
		भङ्ग;
	पक्ष CSS_OUTLINE_STYLE_GROOVE:
		fprintf(प्रवाहः, "outline-style: groove ");
		भङ्ग;
	पक्ष CSS_OUTLINE_STYLE_RIDGE:
		fprintf(प्रवाहः, "outline-style: ridge ");
		भङ्ग;
	पक्ष CSS_OUTLINE_STYLE_INSET:
		fprintf(प्रवाहः, "outline-style: inset ");
		भङ्ग;
	पक्ष CSS_OUTLINE_STYLE_OUTSET:
		fprintf(प्रवाहः, "outline-style: outset ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* outline-width */
	val = css_computed_outline_विस्तारः(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_OUTLINE_विस्तारः_THIN:
		fprintf(प्रवाहः, "outline-width: thin ");
		भङ्ग;
	पक्ष CSS_OUTLINE_विस्तारः_MEDIUM:
		fprintf(प्रवाहः, "outline-width: medium ");
		भङ्ग;
	पक्ष CSS_OUTLINE_विस्तारः_THICK:
		fprintf(प्रवाहः, "outline-width: thick ");
		भङ्ग;
	पक्ष CSS_OUTLINE_विस्तारः_विस्तारः:
		fprintf(प्रवाहः, "outline-width: ");

		dump_css_unit(प्रवाहः, len1, unit1);

		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/*अतिप्रवाहः*/
	val = css_computed_अतिप्रवाहः_x(style);
	विकल्प (val) {
	पक्ष CSS_अतिप्रवाहः_VISIBLE:
		fprintf(प्रवाहः, "overflow-x: visible ");
		भङ्ग;
	पक्ष CSS_अतिप्रवाहः_HIDDEN:
		fprintf(प्रवाहः, "overflow-x: hidden ");
		भङ्ग;
	पक्ष CSS_अतिप्रवाहः_SCROLL:
		fprintf(प्रवाहः, "overflow-x: scroll ");
		भङ्ग;
	पक्ष CSS_अतिप्रवाहः_AUTO:
		fprintf(प्रवाहः, "overflow-x auto ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/*अतिप्रवाहः*/
	val = css_computed_अतिप्रवाहः_y(style);
	विकल्प (val) {
	पक्ष CSS_अतिप्रवाहः_VISIBLE:
		fprintf(प्रवाहः, "overflow-y: visible ");
		भङ्ग;
	पक्ष CSS_अतिप्रवाहः_HIDDEN:
		fprintf(प्रवाहः, "overflow-y: hidden ");
		भङ्ग;
	पक्ष CSS_अतिप्रवाहः_SCROLL:
		fprintf(प्रवाहः, "overflow-y: scroll ");
		भङ्ग;
	पक्ष CSS_अतिप्रवाहः_AUTO:
		fprintf(प्रवाहः, "overflow-y: auto ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* padding-top */
	val = css_computed_padding_शिखरम्(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_PADDING_समुच्चयः:
		fprintf(प्रवाहः, "padding-top: ");

		dump_css_unit(प्रवाहः, len1, unit1);

		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* padding-right */
	val = css_computed_padding_दक्षिणः(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_PADDING_समुच्चयः:
		fprintf(प्रवाहः, "padding-right: ");

		dump_css_unit(प्रवाहः, len1, unit1);

		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* padding-bottom */
	val = css_computed_padding_bottom(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_PADDING_समुच्चयः:
		fprintf(प्रवाहः, "padding-bottom: ");

		dump_css_unit(प्रवाहः, len1, unit1);

		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* padding-left */
	val = css_computed_padding_वामः(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_PADDING_समुच्चयः:
		fprintf(प्रवाहः, "padding-left: ");

		dump_css_unit(प्रवाहः, len1, unit1);

		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* position */
	val = css_computed_position(style);
	विकल्प (val) {
	पक्ष CSS_POSITION_STATIC:
		fprintf(प्रवाहः, "position: static ");
		भङ्ग;
	पक्ष CSS_POSITION_RELATIVE:
		fprintf(प्रवाहः, "position: relative ");
		भङ्ग;
	पक्ष CSS_POSITION_ABSOLUTE:
		fprintf(प्रवाहः, "position: absolute ");
		भङ्ग;
	पक्ष CSS_POSITION_FIXED:
		fprintf(प्रवाहः, "position: fixed ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* quotes */
	val = css_computed_quotes(style, &string_list);
	यदि (val == CSS_QUOTES_STRING && string_list != रिक्त) {
		fprintf(प्रवाहः, "quotes:");

		यावत् (*string_list != रिक्त) {
			fprintf(प्रवाहः, " \"%.*s\"",
				(पूर्णाङ्क) lwc_string_दीर्घता(*string_list),
				lwc_string_दत्त(*string_list));

			string_list++;
		}

		fprintf(प्रवाहः, " ");
	} अन्यथा {
		विकल्प (val) {
		पक्ष CSS_QUOTES_NONE:
			fprintf(प्रवाहः, "quotes: none ");
			भङ्ग;
		स्वभाव:
			भङ्ग;
		}
	}

	/*दक्षिणः*/
	val = css_computed_दक्षिणः(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_दक्षिणः_AUTO:
		fprintf(प्रवाहः, "right: auto ");
		भङ्ग;
	पक्ष CSS_दक्षिणः_समुच्चयः:
		fprintf(प्रवाहः, "right: ");

		dump_css_unit(प्रवाहः, len1, unit1);

		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* table-layout */
	val = css_computed_सारणी_layout(style);
	विकल्प (val) {
	पक्ष CSS_सारणी_LAYOUT_AUTO:
		fprintf(प्रवाहः, "table-layout: auto ");
		भङ्ग;
	पक्ष CSS_सारणी_LAYOUT_FIXED:
		fprintf(प्रवाहः, "table-layout: fixed ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* text-align */
	val = css_computed_text_align(style);
	विकल्प (val) {
	पक्ष CSS_TEXT_ALIGN_वामः:
		fprintf(प्रवाहः, "text-align: left ");
		भङ्ग;
	पक्ष CSS_TEXT_ALIGN_दक्षिणः:
		fprintf(प्रवाहः, "text-align: right ");
		भङ्ग;
	पक्ष CSS_TEXT_ALIGN_CENTER:
		fprintf(प्रवाहः, "text-align: center ");
		भङ्ग;
	पक्ष CSS_TEXT_ALIGN_JUSTIFY:
		fprintf(प्रवाहः, "text-align: justify ");
		भङ्ग;
	पक्ष CSS_TEXT_ALIGN_DEFAULT:
		fprintf(प्रवाहः, "text-align: default ");
		भङ्ग;
	पक्ष CSS_TEXT_ALIGN_LIBCSS_वामः:
		fprintf(प्रवाहः, "text-align: -libcss-left ");
		भङ्ग;
	पक्ष CSS_TEXT_ALIGN_LIBCSS_CENTER:
		fprintf(प्रवाहः, "text-align: -libcss-center ");
		भङ्ग;
	पक्ष CSS_TEXT_ALIGN_LIBCSS_दक्षिणः:
		fprintf(प्रवाहः, "text-align: -libcss-right ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* text-decoration */
	val = css_computed_text_decoration(style);
	यदि (val == CSS_TEXT_DECORATION_NONE) {
		fprintf(प्रवाहः, "text-decoration: none ");
	} अन्यथा {
		fprintf(प्रवाहः, "text-decoration:");

		यदि (val & CSS_TEXT_DECORATION_BLINK) {
			fprintf(प्रवाहः, " blink");
		}
		यदि (val & CSS_TEXT_DECORATION_पङ्क्तिः_THROUGH) {
			fprintf(प्रवाहः, " line-through");
		}
		यदि (val & CSS_TEXT_DECORATION_OVERLINE) {
			fprintf(प्रवाहः, " overline");
		}
		यदि (val & CSS_TEXT_DECORATION_UNDERLINE) {
			fprintf(प्रवाहः, " underline");
		}

		fprintf(प्रवाहः, " ");
	}

	/* text-indent */
	val = css_computed_text_indent(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_TEXT_INDENT_समुच्चयः:
		fprintf(प्रवाहः, "text-indent: ");

		dump_css_unit(प्रवाहः, len1, unit1);

		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* text-transform */
	val = css_computed_text_transform(style);
	विकल्प (val) {
	पक्ष CSS_TEXT_TRANSFORM_CAPITALIZE:
		fprintf(प्रवाहः, "text-transform: capitalize ");
		भङ्ग;
	पक्ष CSS_TEXT_TRANSFORM_UPPERCASE:
		fprintf(प्रवाहः, "text-transform: uppercase ");
		भङ्ग;
	पक्ष CSS_TEXT_TRANSFORM_LOWERCASE:
		fprintf(प्रवाहः, "text-transform: lowercase ");
		भङ्ग;
	पक्ष CSS_TEXT_TRANSFORM_NONE:
		fprintf(प्रवाहः, "text-transform: none ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/*शिखरम्*/
	val = css_computed_शिखरम्(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_शिखरम्_AUTO:
		fprintf(प्रवाहः, "top: auto ");
		भङ्ग;
	पक्ष CSS_शिखरम्_समुच्चयः:
		fprintf(प्रवाहः, "top: ");

		dump_css_unit(प्रवाहः, len1, unit1);

		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* unicode-bidi */
	val = css_computed_unicode_bidi(style);
	विकल्प (val) {
	पक्ष CSS_UNICODE_BIDI_प्रसामान्यम्:
		fprintf(प्रवाहः, "unicode-bidi: normal ");
		भङ्ग;
	पक्ष CSS_UNICODE_BIDI_EMBED:
		fprintf(प्रवाहः, "unicode-bidi: embed ");
		भङ्ग;
	पक्ष CSS_UNICODE_BIDI_BIDI_OVERRIDE:
		fprintf(प्रवाहः, "unicode-bidi: bidi-override ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* vertical-align */
	val = css_computed_vertical_align(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_VERTICAL_ALIGN_BASELINE:
		fprintf(प्रवाहः, "vertical-align: baseline ");
		भङ्ग;
	पक्ष CSS_VERTICAL_ALIGN_SUB:
		fprintf(प्रवाहः, "vertical-align: sub ");
		भङ्ग;
	पक्ष CSS_VERTICAL_ALIGN_SUPER:
		fprintf(प्रवाहः, "vertical-align: super ");
		भङ्ग;
	पक्ष CSS_VERTICAL_ALIGN_शिखरम्:
		fprintf(प्रवाहः, "vertical-align: top ");
		भङ्ग;
	पक्ष CSS_VERTICAL_ALIGN_TEXT_शिखरम्:
		fprintf(प्रवाहः, "vertical-align: text-top ");
		भङ्ग;
	पक्ष CSS_VERTICAL_ALIGN_मध्यः:
		fprintf(प्रवाहः, "vertical-align: middle ");
		भङ्ग;
	पक्ष CSS_VERTICAL_ALIGN_BOTTOM:
		fprintf(प्रवाहः, "vertical-align: bottom ");
		भङ्ग;
	पक्ष CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
		fprintf(प्रवाहः, "vertical-align: text-bottom ");
		भङ्ग;
	पक्ष CSS_VERTICAL_ALIGN_समुच्चयः:
		fprintf(प्रवाहः, "vertical-align: ");

		dump_css_unit(प्रवाहः, len1, unit1);

		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* visibility */
	val = css_computed_visibility(style);
	विकल्प (val) {
	पक्ष CSS_VISIBILITY_VISIBLE:
		fprintf(प्रवाहः, "visibility: visible ");
		भङ्ग;
	पक्ष CSS_VISIBILITY_HIDDEN:
		fprintf(प्रवाहः, "visibility: hidden ");
		भङ्ग;
	पक्ष CSS_VISIBILITY_COLLAPSE:
		fprintf(प्रवाहः, "visibility: collapse ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* white-space */
	val = css_computed_white_space(style);
	विकल्प (val) {
	पक्ष CSS_WHITE_SPACE_प्रसामान्यम्:
		fprintf(प्रवाहः, "white-space: normal ");
		भङ्ग;
	पक्ष CSS_WHITE_SPACE_PRE:
		fprintf(प्रवाहः, "white-space: pre ");
		भङ्ग;
	पक्ष CSS_WHITE_SPACE_NOWRAP:
		fprintf(प्रवाहः, "white-space: nowrap ");
		भङ्ग;
	पक्ष CSS_WHITE_SPACE_PRE_WRAP:
		fprintf(प्रवाहः, "white-space: pre-wrap ");
		भङ्ग;
	पक्ष CSS_WHITE_SPACE_PRE_पङ्क्तिः:
		fprintf(प्रवाहः, "white-space: pre-line ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/*विस्तारः*/
	val = css_computed_विस्तारः(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_विस्तारः_AUTO:
		fprintf(प्रवाहः, "width: auto ");
		भङ्ग;
	पक्ष CSS_विस्तारः_समुच्चयः:
		fprintf(प्रवाहः, "width: ");

		dump_css_unit(प्रवाहः, len1, unit1);

		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* word-spacing */
	val = css_computed_word_spacing(style, &len1, &unit1);
	विकल्प (val) {
	पक्ष CSS_WORD_SPACING_प्रसामान्यम्:
		fprintf(प्रवाहः, "word-spacing: normal ");
		भङ्ग;
	पक्ष CSS_WORD_SPACING_समुच्चयः:
		fprintf(प्रवाहः, "word-spacing: ");

		dump_css_unit(प्रवाहः, len1, unit1);

		fprintf(प्रवाहः, " ");
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* z-index */
	val = css_computed_z_सूचकांक(style, &zindex);
	विकल्प (val) {
	पक्ष CSS_Z_सूचकांक_AUTO:
		fprintf(प्रवाहः, "z-index: auto ");
		भङ्ग;
	पक्ष CSS_Z_सूचकांक_समुच्चयः:
		fprintf(प्रवाहः, "z-index: %"PRId32" ", zindex);
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	fprintf(प्रवाहः, "}");
}
