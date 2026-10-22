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

#అన్తర్భావ <stdio.h>
#అన్తర్భావ <libcss/libcss.h>

#అన్తర్భావ "netsurf/inttypes.h"

#అన్తర్భావ "css/dump.h"
#అన్తర్భావ "css/utils.h"

/**
 * Dump a fixed point value to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param f       Value to write
 */
స్థిర శూన్య dump_css_fixed(FILE *ప్రవాహం, css_fixed f)
{
#నిర్వచించు NSCSS_ABS(x) (uint32_t)((x) < 0 ? -(x) : (x))
	uint32_t uintpart = FIXTOINT(NSCSS_ABS(f));
	/* + 500 to ensure round to nearest (division will truncate) */
	uint32_t fracpart = ((NSCSS_ABS(f) & 0x3ff) * 1000 + 500) / (1 << 10);
#నిర్వచనం_రద్దు NSCSS_ABS

	fprintf(ప్రవాహం, "%s%"PRIu32".%03"PRIu32, f < 0 ? "-" : "", uintpart, fracpart);
}

/**
 * Dump a numeric value to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 */
స్థిర శూన్య dump_css_సంఖ్య(FILE *ప్రవాహం, css_fixed val)
{
	ఒకవేళ (INTTOFIX(FIXTOINT(val)) == val)
		fprintf(ప్రవాహం, "%"PRId32, FIXTOINT(val));
	లేకపోతే
		dump_css_fixed(ప్రవాహం, val);
}

/**
 * Dump a dimension to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 * \param unit    Unit to write
 */
స్థిర శూన్య dump_css_unit(FILE *ప్రవాహం, css_fixed val, css_unit unit)
{
	dump_css_సంఖ్య(ప్రవాహం, val);

	ఎంపిక (unit) {
	స్థితి CSS_UNIT_PX:
		fprintf(ప్రవాహం, "px");
		ఆపు;
	స్థితి CSS_UNIT_EX:
		fprintf(ప్రవాహం, "ex");
		ఆపు;
	స్థితి CSS_UNIT_EM:
		fprintf(ప్రవాహం, "em");
		ఆపు;
	స్థితి CSS_UNIT_లోపల:
		fprintf(ప్రవాహం, "in");
		ఆపు;
	స్థితి CSS_UNIT_CM:
		fprintf(ప్రవాహం, "cm");
		ఆపు;
	స్థితి CSS_UNIT_MM:
		fprintf(ప్రవాహం, "mm");
		ఆపు;
	స్థితి CSS_UNIT_PT:
		fprintf(ప్రవాహం, "pt");
		ఆపు;
	స్థితి CSS_UNIT_PC:
		fprintf(ప్రవాహం, "pc");
		ఆపు;
	స్థితి CSS_UNIT_PCT:
		fprintf(ప్రవాహం, "%%");
		ఆపు;
	స్థితి CSS_UNIT_DEG:
		fprintf(ప్రవాహం, "deg");
		ఆపు;
	స్థితి CSS_UNIT_GRAD:
		fprintf(ప్రవాహం, "grad");
		ఆపు;
	స్థితి CSS_UNIT_RAD:
		fprintf(ప్రవాహం, "rad");
		ఆపు;
	స్థితి CSS_UNIT_MS:
		fprintf(ప్రవాహం, "ms");
		ఆపు;
	స్థితి CSS_UNIT_S:
		fprintf(ప్రవాహం, "s");
		ఆపు;
	స్థితి CSS_UNIT_HZ:
		fprintf(ప్రవాహం, "Hz");
		ఆపు;
	స్థితి CSS_UNIT_KHZ:
		fprintf(ప్రవాహం, "kHz");
		ఆపు;
	స్థితి CSS_UNIT_CH:
		fprintf(ప్రవాహం, "ch");
		ఆపు;
	స్థితి CSS_UNIT_REM:
		fprintf(ప్రవాహం, "rem");
		ఆపు;
	స్థితి CSS_UNIT_LH:
		fprintf(ప్రవాహం, "lh");
		ఆపు;
	స్థితి CSS_UNIT_VH:
		fprintf(ప్రవాహం, "vh");
		ఆపు;
	స్థితి CSS_UNIT_VW:
		fprintf(ప్రవాహం, "vw");
		ఆపు;
	స్థితి CSS_UNIT_VI:
		fprintf(ప్రవాహం, "vi");
		ఆపు;
	స్థితి CSS_UNIT_VB:
		fprintf(ప్రవాహం, "vb");
		ఆపు;
	స్థితి CSS_UNIT_VMIN:
		fprintf(ప్రవాహం, "vmin");
		ఆపు;
	స్థితి CSS_UNIT_VMAX:
		fprintf(ప్రవాహం, "vmax");
		ఆపు;
	స్థితి CSS_UNIT_Q:
		fprintf(ప్రవాహం, "q");
		ఆపు;
	స్థితి CSS_UNIT_CALC:
		fprintf(ప్రవాహం, "calc()");
		ఆపు;
	}
}

/* exported interface documented in content/handlers/css/dump.h */
శూన్య nscss_dump_computed_style(FILE *ప్రవాహం, స్థిరరాశి css_computed_style *style)
{
	uint8_t val;
	css_color color = 0;
	lwc_string *url = ఖాళీ;
	css_fixed len1 = 0, len2 = 0;
	css_unit unit1 = CSS_UNIT_PX, unit2 = CSS_UNIT_PX;
	css_computed_clip_rect rect = { 0, 0, 0, 0, CSS_UNIT_PX, CSS_UNIT_PX,
					CSS_UNIT_PX, CSS_UNIT_PX, true, true,
					true, true };
	స్థిరరాశి css_computed_content_అంశం *content = ఖాళీ;
	స్థిరరాశి css_computed_counter *counter = ఖాళీ;
	lwc_string **string_list = ఖాళీ;
	int32_t zindex = 0;

	fprintf(ప్రవాహం, "{ ");

	/* background-attachment */
	val = css_computed_background_attachment(style);
	ఎంపిక (val) {
	స్థితి CSS_BACKGROUND_ATTACHMENT_FIXED:
		fprintf(ప్రవాహం, "background-attachment: fixed ");
		ఆపు;
	స్థితి CSS_BACKGROUND_ATTACHMENT_SCROLL:
		fprintf(ప్రవాహం, "background-attachment: scroll ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* background-color */
	val = css_computed_background_color(style, &color);
	ఎంపిక (val) {
	స్థితి CSS_BACKGROUND_COLOR_COLOR:
		fprintf(ప్రవాహం, "background-color: #%08"PRIx32" ", color);
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* background-image */
	val = css_computed_background_image(style, &url);
	ఒకవేళ (val == CSS_BACKGROUND_IMAGE_IMAGE && url != ఖాళీ) {
		fprintf(ప్రవాహం, "background-image: url('%.*s') ",
				(పూర్ణాంకం) lwc_string_నిడివి(url),
				lwc_string_దత్తాంశం(url));
	} లేకపోతే ఒకవేళ (val == CSS_BACKGROUND_IMAGE_NONE) {
		fprintf(ప్రవాహం, "background-image: none ");
	}

	/* background-position */
	val = css_computed_background_position(style, &len1, &unit1,
			&len2, &unit2);
	ఒకవేళ (val == CSS_BACKGROUND_POSITION_సమితి) {
		fprintf(ప్రవాహం, "background-position: ");
		dump_css_unit(ప్రవాహం, len1, unit1);
		fprintf(ప్రవాహం, " ");
		dump_css_unit(ప్రవాహం, len2, unit2);
		fprintf(ప్రవాహం, " ");
	}

	/* background-repeat */
	val = css_computed_background_repeat(style);
	ఎంపిక (val) {
	స్థితి CSS_BACKGROUND_REPEAT_REPEAT_X:
		fprintf(ప్రవాహం, "background-repeat: repeat-x ");
		ఆపు;
	స్థితి CSS_BACKGROUND_REPEAT_REPEAT_Y:
		fprintf(ప్రవాహం, "background-repeat: repeat-y ");
		ఆపు;
	స్థితి CSS_BACKGROUND_REPEAT_REPEAT:
		fprintf(ప్రవాహం, "background-repeat: repeat ");
		ఆపు;
	స్థితి CSS_BACKGROUND_REPEAT_NO_REPEAT:
		fprintf(ప్రవాహం, "background-repeat: no-repeat ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* border-collapse */
	val = css_computed_border_collapse(style);
	ఎంపిక (val) {
	స్థితి CSS_BORDER_COLLAPSE_SEPARATE:
		fprintf(ప్రవాహం, "border-collapse: separate ");
		ఆపు;
	స్థితి CSS_BORDER_COLLAPSE_COLLAPSE:
		fprintf(ప్రవాహం, "border-collapse: collapse ");
		ఆపు;
	అప్రమేయ:

		ఆపు;
	}

	/* border-spacing */
	val = css_computed_border_spacing(style, &len1, &unit1, &len2, &unit2);
	ఒకవేళ (val == CSS_BORDER_SPACING_సమితి) {
		fprintf(ప్రవాహం, "border-spacing: ");
		dump_css_unit(ప్రవాహం, len1, unit1);
		fprintf(ప్రవాహం, " ");
		dump_css_unit(ప్రవాహం, len2, unit2);
		fprintf(ప్రవాహం, " ");
	}

	/* border-top-color */
	val = css_computed_border_శిఖరం_color(style, &color);
	ఎంపిక (val) {
	స్థితి CSS_BORDER_COLOR_COLOR:
		fprintf(ప్రవాహం, "border-top-color: #%08"PRIx32" ", color);
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* border-right-color */
	val = css_computed_border_కుడి_color(style, &color);
	ఎంపిక (val) {
	స్థితి CSS_BORDER_COLOR_COLOR:
		fprintf(ప్రవాహం, "border-right-color: #%08"PRIx32" ", color);
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* border-bottom-color */
	val = css_computed_border_bottom_color(style, &color);
	ఎంపిక (val) {
	స్థితి CSS_BORDER_COLOR_COLOR:
		fprintf(ప్రవాహం, "border-bottom-color: #%08"PRIx32" ", color);
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* border-left-color */
	val = css_computed_border_ఎడమ_color(style, &color);
	ఎంపిక (val) {
	స్థితి CSS_BORDER_COLOR_COLOR:
		fprintf(ప్రవాహం, "border-left-color: #%08"PRIx32" ", color);
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* border-top-style */
	val = css_computed_border_శిఖరం_style(style);
	ఎంపిక (val) {
	స్థితి CSS_BORDER_STYLE_NONE:
		fprintf(ప్రవాహం, "border-top-style: none ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_HIDDEN:
		fprintf(ప్రవాహం, "border-top-style: hidden ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_DOTTED:
		fprintf(ప్రవాహం, "border-top-style: dotted ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_DASHED:
		fprintf(ప్రవాహం, "border-top-style: dashed ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_SOLID:
		fprintf(ప్రవాహం, "border-top-style: solid ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_DOUBLE:
		fprintf(ప్రవాహం, "border-top-style: double ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_GROOVE:
		fprintf(ప్రవాహం, "border-top-style: groove ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_RIDGE:
		fprintf(ప్రవాహం, "border-top-style: ridge ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_INSET:
		fprintf(ప్రవాహం, "border-top-style: inset ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_OUTSET:
		fprintf(ప్రవాహం, "border-top-style: outset ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* border-right-style */
	val = css_computed_border_కుడి_style(style);
	ఎంపిక (val) {
	స్థితి CSS_BORDER_STYLE_NONE:
		fprintf(ప్రవాహం, "border-right-style: none ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_HIDDEN:
		fprintf(ప్రవాహం, "border-right-style: hidden ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_DOTTED:
		fprintf(ప్రవాహం, "border-right-style: dotted ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_DASHED:
		fprintf(ప్రవాహం, "border-right-style: dashed ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_SOLID:
		fprintf(ప్రవాహం, "border-right-style: solid ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_DOUBLE:
		fprintf(ప్రవాహం, "border-right-style: double ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_GROOVE:
		fprintf(ప్రవాహం, "border-right-style: groove ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_RIDGE:
		fprintf(ప్రవాహం, "border-right-style: ridge ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_INSET:
		fprintf(ప్రవాహం, "border-right-style: inset ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_OUTSET:
		fprintf(ప్రవాహం, "border-right-style: outset ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* border-bottom-style */
	val = css_computed_border_bottom_style(style);
	ఎంపిక (val) {
	స్థితి CSS_BORDER_STYLE_NONE:
		fprintf(ప్రవాహం, "border-bottom-style: none ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_HIDDEN:
		fprintf(ప్రవాహం, "border-bottom-style: hidden ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_DOTTED:
		fprintf(ప్రవాహం, "border-bottom-style: dotted ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_DASHED:
		fprintf(ప్రవాహం, "border-bottom-style: dashed ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_SOLID:
		fprintf(ప్రవాహం, "border-bottom-style: solid ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_DOUBLE:
		fprintf(ప్రవాహం, "border-bottom-style: double ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_GROOVE:
		fprintf(ప్రవాహం, "border-bottom-style: groove ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_RIDGE:
		fprintf(ప్రవాహం, "border-bottom-style: ridge ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_INSET:
		fprintf(ప్రవాహం, "border-bottom-style: inset ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_OUTSET:
		fprintf(ప్రవాహం, "border-bottom-style: outset ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* border-left-style */
	val = css_computed_border_ఎడమ_style(style);
	ఎంపిక (val) {
	స్థితి CSS_BORDER_STYLE_NONE:
		fprintf(ప్రవాహం, "border-left-style: none ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_HIDDEN:
		fprintf(ప్రవాహం, "border-left-style: hidden ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_DOTTED:
		fprintf(ప్రవాహం, "border-left-style: dotted ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_DASHED:
		fprintf(ప్రవాహం, "border-left-style: dashed ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_SOLID:
		fprintf(ప్రవాహం, "border-left-style: solid ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_DOUBLE:
		fprintf(ప్రవాహం, "border-left-style: double ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_GROOVE:
		fprintf(ప్రవాహం, "border-left-style: groove ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_RIDGE:
		fprintf(ప్రవాహం, "border-left-style: ridge ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_INSET:
		fprintf(ప్రవాహం, "border-left-style: inset ");
		ఆపు;
	స్థితి CSS_BORDER_STYLE_OUTSET:
		fprintf(ప్రవాహం, "border-left-style: outset ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* border-top-width */
	val = css_computed_border_శిఖరం_వెడల్పు(style, &len1, &unit1);
	ఎంపిక (val) {
	స్థితి CSS_BORDER_వెడల్పు_THIN:
		fprintf(ప్రవాహం, "border-top-width: thin ");
		ఆపు;
	స్థితి CSS_BORDER_వెడల్పు_MEDIUM:
		fprintf(ప్రవాహం, "border-top-width: medium ");
		ఆపు;
	స్థితి CSS_BORDER_వెడల్పు_THICK:
		fprintf(ప్రవాహం, "border-top-width: thick ");
		ఆపు;
	స్థితి CSS_BORDER_వెడల్పు_వెడల్పు:
		fprintf(ప్రవాహం, "border-top-width: ");
		dump_css_unit(ప్రవాహం, len1, unit1);
		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* border-right-width */
	val = css_computed_border_కుడి_వెడల్పు(style, &len1, &unit1);
	ఎంపిక (val) {
	స్థితి CSS_BORDER_వెడల్పు_THIN:
		fprintf(ప్రవాహం, "border-right-width: thin ");
		ఆపు;
	స్థితి CSS_BORDER_వెడల్పు_MEDIUM:
		fprintf(ప్రవాహం, "border-right-width: medium ");
		ఆపు;
	స్థితి CSS_BORDER_వెడల్పు_THICK:
		fprintf(ప్రవాహం, "border-right-width: thick ");
		ఆపు;
	స్థితి CSS_BORDER_వెడల్పు_వెడల్పు:
		fprintf(ప్రవాహం, "border-right-width: ");
		dump_css_unit(ప్రవాహం, len1, unit1);
		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* border-bottom-width */
	val = css_computed_border_bottom_వెడల్పు(style, &len1, &unit1);
	ఎంపిక (val) {
	స్థితి CSS_BORDER_వెడల్పు_THIN:
		fprintf(ప్రవాహం, "border-bottom-width: thin ");
		ఆపు;
	స్థితి CSS_BORDER_వెడల్పు_MEDIUM:
		fprintf(ప్రవాహం, "border-bottom-width: medium ");
		ఆపు;
	స్థితి CSS_BORDER_వెడల్పు_THICK:
		fprintf(ప్రవాహం, "border-bottom-width: thick ");
		ఆపు;
	స్థితి CSS_BORDER_వెడల్పు_వెడల్పు:
		fprintf(ప్రవాహం, "border-bottom-width: ");
		dump_css_unit(ప్రవాహం, len1, unit1);
		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* border-left-width */
	val = css_computed_border_ఎడమ_వెడల్పు(style, &len1, &unit1);
	ఎంపిక (val) {
	స్థితి CSS_BORDER_వెడల్పు_THIN:
		fprintf(ప్రవాహం, "border-left-width: thin ");
		ఆపు;
	స్థితి CSS_BORDER_వెడల్పు_MEDIUM:
		fprintf(ప్రవాహం, "border-left-width: medium ");
		ఆపు;
	స్థితి CSS_BORDER_వెడల్పు_THICK:
		fprintf(ప్రవాహం, "border-left-width: thick ");
		ఆపు;
	స్థితి CSS_BORDER_వెడల్పు_వెడల్పు:
		fprintf(ప్రవాహం, "border-left-width: ");
		dump_css_unit(ప్రవాహం, len1, unit1);
		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* bottom */
	val = css_computed_bottom(style, &len1, &unit1);
	ఎంపిక (val) {
	స్థితి CSS_BOTTOM_AUTO:
		fprintf(ప్రవాహం, "bottom: auto ");
		ఆపు;
	స్థితి CSS_BOTTOM_సమితి:
		fprintf(ప్రవాహం, "bottom: ");
		dump_css_unit(ప్రవాహం, len1, unit1);
		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* caption-side */
	val = css_computed_caption_side(style);
	ఎంపిక (val) {
	స్థితి CSS_CAPTION_SIDE_శిఖరం:
		fprintf(ప్రవాహం, "caption_side: top ");
		ఆపు;
	స్థితి CSS_CAPTION_SIDE_BOTTOM:
		fprintf(ప్రవాహం, "caption_side: bottom ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* clear */
	val = css_computed_clear(style);
	ఎంపిక (val) {
	స్థితి CSS_CLEAR_NONE:
		fprintf(ప్రవాహం, "clear: none ");
		ఆపు;
	స్థితి CSS_CLEAR_ఎడమ:
		fprintf(ప్రవాహం, "clear: left ");
		ఆపు;
	స్థితి CSS_CLEAR_కుడి:
		fprintf(ప్రవాహం, "clear: right ");
		ఆపు;
	స్థితి CSS_CLEAR_BOTH:
		fprintf(ప్రవాహం, "clear: both ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* clip */
	val = css_computed_clip(style, &rect);
	ఎంపిక (val) {
	స్థితి CSS_CLIP_AUTO:
		fprintf(ప్రవాహం, "clip: auto ");
		ఆపు;
	స్థితి CSS_CLIP_RECT:
		fprintf(ప్రవాహం, "clip: rect( ");

		ఒకవేళ (rect.శిఖరం_auto)
			fprintf(ప్రవాహం, "auto");
		లేకపోతే
			dump_css_unit(ప్రవాహం, rect.శిఖరం, rect.tunit);
		fprintf(ప్రవాహం, ", ");

		ఒకవేళ (rect.కుడి_auto)
			fprintf(ప్రవాహం, "auto");
		లేకపోతే
			dump_css_unit(ప్రవాహం, rect.కుడి, rect.runit);
		fprintf(ప్రవాహం, ", ");

		ఒకవేళ (rect.bottom_auto)
			fprintf(ప్రవాహం, "auto");
		లేకపోతే
			dump_css_unit(ప్రవాహం, rect.bottom, rect.bunit);
		fprintf(ప్రవాహం, ", ");

		ఒకవేళ (rect.ఎడమ_auto)
			fprintf(ప్రవాహం, "auto");
		లేకపోతే
			dump_css_unit(ప్రవాహం, rect.ఎడమ, rect.lunit);
		fprintf(ప్రవాహం, ") ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* color */
	val = css_computed_color(style, &color);
	ఒకవేళ (val == CSS_COLOR_COLOR) {
		fprintf(ప్రవాహం, "color: #%08"PRIx32" ", color);
	}

	/* content */
	val = css_computed_content(style, &content);
	ఎంపిక (val) {
	స్థితి CSS_CONTENT_NONE:
		fprintf(ప్రవాహం, "content: none ");
		ఆపు;
	స్థితి CSS_CONTENT_సాధారణ:
		fprintf(ప్రవాహం, "content: normal ");
		ఆపు;
	స్థితి CSS_CONTENT_సమితి:
		fprintf(ప్రవాహం, "content:");

		అయితే (content->type != CSS_COMPUTED_CONTENT_NONE) {
			fprintf(ప్రవాహం, " ");

			ఎంపిక (content->type) {
			స్థితి CSS_COMPUTED_CONTENT_STRING:
				fprintf(ప్రవాహం,	"\"%.*s\"",
						(పూర్ణాంకం) lwc_string_నిడివి(
						content->దత్తాంశం.string),
						lwc_string_దత్తాంశం(
						content->దత్తాంశం.string));
				ఆపు;
			స్థితి CSS_COMPUTED_CONTENT_URI:
				fprintf(ప్రవాహం,	"uri(\"%.*s\")",
						(పూర్ణాంకం) lwc_string_నిడివి(
						content->దత్తాంశం.uri),
						lwc_string_దత్తాంశం(
						content->దత్తాంశం.uri));
				ఆపు;
			స్థితి CSS_COMPUTED_CONTENT_COUNTER:
				fprintf(ప్రవాహం, "counter(%.*s)",
						(పూర్ణాంకం) lwc_string_నిడివి(
						content->దత్తాంశం.counter.name),
						lwc_string_దత్తాంశం(
						content->దత్తాంశం.counter.name));
				ఆపు;
			స్థితి CSS_COMPUTED_CONTENT_COUNTERS:
				fprintf(ప్రవాహం, "counters(%.*s, \"%.*s\")",
						(పూర్ణాంకం) lwc_string_నిడివి(
						content->దత్తాంశం.counters.name),
						lwc_string_దత్తాంశం(
						content->దత్తాంశం.counters.name),
						(పూర్ణాంకం) lwc_string_నిడివి(
						content->దత్తాంశం.counters.sep),
						lwc_string_దత్తాంశం(
						content->దత్తాంశం.counters.sep));
				ఆపు;
			స్థితి CSS_COMPUTED_CONTENT_ATTR:
				fprintf(ప్రవాహం, "attr(%.*s)",
						(పూర్ణాంకం) lwc_string_నిడివి(
						content->దత్తాంశం.attr),
						lwc_string_దత్తాంశం(
						content->దత్తాంశం.attr));
				ఆపు;
			స్థితి CSS_COMPUTED_CONTENT_OPEN_QUOTE:
				fprintf(ప్రవాహం, "open-quote");
				ఆపు;
			స్థితి CSS_COMPUTED_CONTENT_CLOSE_QUOTE:
				fprintf(ప్రవాహం, "close-quote");
				ఆపు;
			స్థితి CSS_COMPUTED_CONTENT_NO_OPEN_QUOTE:
				fprintf(ప్రవాహం, "no-open-quote");
				ఆపు;
			స్థితి CSS_COMPUTED_CONTENT_NO_CLOSE_QUOTE:
				fprintf(ప్రవాహం, "no-close-quote");
				ఆపు;
			}

			content++;
		}

		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* counter-increment */
	val = css_computed_counter_increment(style, &counter);
	ఒకవేళ ((val == CSS_COUNTER_INCREMENT_NONE) || (counter == ఖాళీ)) {
		fprintf(ప్రవాహం, "counter-increment: none ");
	} లేకపోతే {
		fprintf(ప్రవాహం, "counter-increment:");

		అయితే (counter->name != ఖాళీ) {
			fprintf(ప్రవాహం, " %.*s ",
					(పూర్ణాంకం) lwc_string_నిడివి(counter->name),
					lwc_string_దత్తాంశం(counter->name));

			dump_css_fixed(ప్రవాహం, counter->విలువ);

			counter++;
		}

		fprintf(ప్రవాహం, " ");
	}

	/* counter-reset */
	val = css_computed_counter_reset(style, &counter);
	ఒకవేళ ((val == CSS_COUNTER_RESET_NONE) || (counter == ఖాళీ)) {
		fprintf(ప్రవాహం, "counter-reset: none ");
	} లేకపోతే {
		fprintf(ప్రవాహం, "counter-reset:");

		అయితే (counter->name != ఖాళీ) {
			fprintf(ప్రవాహం, " %.*s ",
					(పూర్ణాంకం) lwc_string_నిడివి(counter->name),
					lwc_string_దత్తాంశం(counter->name));

			dump_css_fixed(ప్రవాహం, counter->విలువ);

			counter++;
		}

		fprintf(ప్రవాహం, " ");
	}

	/* cursor */
	val = css_computed_cursor(style, &string_list);
	fprintf(ప్రవాహం, "cursor:");

	ఒకవేళ (string_list != ఖాళీ) {
		అయితే (*string_list != ఖాళీ) {
			fprintf(ప్రవాహం, " url\"%.*s\")",
					(పూర్ణాంకం) lwc_string_నిడివి(*string_list),
					lwc_string_దత్తాంశం(*string_list));

			string_list++;
		}
	}
	ఎంపిక (val) {
	స్థితి CSS_CURSOR_AUTO:
		fprintf(ప్రవాహం, " auto ");
		ఆపు;
	స్థితి CSS_CURSOR_CROSSHAIR:
		fprintf(ప్రవాహం, " crosshair ");
		ఆపు;
	స్థితి CSS_CURSOR_DEFAULT:
		fprintf(ప్రవాహం, " default ");
		ఆపు;
	స్థితి CSS_CURSOR_సూచిక:
		fprintf(ప్రవాహం, " pointer ");
		ఆపు;
	స్థితి CSS_CURSOR_MOVE:
		fprintf(ప్రవాహం, " move ");
		ఆపు;
	స్థితి CSS_CURSOR_E_RESIZE:
		fprintf(ప్రవాహం, " e-resize ");
		ఆపు;
	స్థితి CSS_CURSOR_NE_RESIZE:
		fprintf(ప్రవాహం, " ne-resize ");
		ఆపు;
	స్థితి CSS_CURSOR_NW_RESIZE:
		fprintf(ప్రవాహం, " nw-resize ");
		ఆపు;
	స్థితి CSS_CURSOR_N_RESIZE:
		fprintf(ప్రవాహం, " n-resize ");
		ఆపు;
	స్థితి CSS_CURSOR_SE_RESIZE:
		fprintf(ప్రవాహం, " se-resize ");
		ఆపు;
	స్థితి CSS_CURSOR_SW_RESIZE:
		fprintf(ప్రవాహం, " sw-resize ");
		ఆపు;
	స్థితి CSS_CURSOR_S_RESIZE:
		fprintf(ప్రవాహం, " s-resize ");
		ఆపు;
	స్థితి CSS_CURSOR_W_RESIZE:
		fprintf(ప్రవాహం, " w-resize ");
		ఆపు;
	స్థితి CSS_CURSOR_పాఠ్యం:
		fprintf(ప్రవాహం, " text ");
		ఆపు;
	స్థితి CSS_CURSOR_WAIT:
		fprintf(ప్రవాహం, " wait ");
		ఆపు;
	స్థితి CSS_CURSOR_HELP:
		fprintf(ప్రవాహం, " help ");
		ఆపు;
	స్థితి CSS_CURSOR_పురోగతి:
		fprintf(ప్రవాహం, " progress ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* direction */
	val = css_computed_direction(style);
	ఎంపిక (val) {
	స్థితి CSS_DIRECTION_LTR:
		fprintf(ప్రవాహం, "direction: ltr ");
		ఆపు;
	స్థితి CSS_DIRECTION_RTL:
		fprintf(ప్రవాహం, "direction: rtl ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* display */
	val = ns_computed_display_static(style);
	ఎంపిక (val) {
	స్థితి CSS_DISPLAY_INLINE:
		fprintf(ప్రవాహం, "display: inline ");
		ఆపు;
	స్థితి CSS_DISPLAY_ఖండం:
		fprintf(ప్రవాహం, "display: block ");
		ఆపు;
	స్థితి CSS_DISPLAY_LIST_అంశం:
		fprintf(ప్రవాహం, "display: list-item ");
		ఆపు;
	స్థితి CSS_DISPLAY_నిరంతరం_లోపల:
		fprintf(ప్రవాహం, "display: run-in ");
		ఆపు;
	స్థితి CSS_DISPLAY_INLINE_ఖండం:
		fprintf(ప్రవాహం, "display: inline-block ");
		ఆపు;
	స్థితి CSS_DISPLAY_పట్టిక:
		fprintf(ప్రవాహం, "display: table ");
		ఆపు;
	స్థితి CSS_DISPLAY_INLINE_పట్టిక:
		fprintf(ప్రవాహం, "display: inline-table ");
		ఆపు;
	స్థితి CSS_DISPLAY_పట్టిక_అడ్డువరుస_సమూహం:
		fprintf(ప్రవాహం, "display: table-row-group ");
		ఆపు;
	స్థితి CSS_DISPLAY_పట్టిక_శీర్షిక_సమూహం:
		fprintf(ప్రవాహం, "display: table-header-group ");
		ఆపు;
	స్థితి CSS_DISPLAY_పట్టిక_FOOTER_సమూహం:
		fprintf(ప్రవాహం, "display: table-footer-group ");
		ఆపు;
	స్థితి CSS_DISPLAY_పట్టిక_అడ్డువరుస:
		fprintf(ప్రవాహం, "display: table-row ");
		ఆపు;
	స్థితి CSS_DISPLAY_పట్టిక_స్తంభం_సమూహం:
		fprintf(ప్రవాహం, "display: table-column-group ");
		ఆపు;
	స్థితి CSS_DISPLAY_పట్టిక_స్తంభం:
		fprintf(ప్రవాహం, "display: table-column ");
		ఆపు;
	స్థితి CSS_DISPLAY_పట్టిక_CELL:
		fprintf(ప్రవాహం, "display: table-cell ");
		ఆపు;
	స్థితి CSS_DISPLAY_పట్టిక_CAPTION:
		fprintf(ప్రవాహం, "display: table-caption ");
		ఆపు;
	స్థితి CSS_DISPLAY_NONE:
		fprintf(ప్రవాహం, "display: none ");
		ఆపు;
	స్థితి CSS_DISPLAY_FLEX:
		fprintf(ప్రవాహం, "display: flex ");
		ఆపు;
	స్థితి CSS_DISPLAY_INLINE_FLEX:
		fprintf(ప్రవాహం, "display: inline-flex ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* empty-cells */
	val = css_computed_ఖాళీలు_cells(style);
	ఎంపిక (val) {
	స్థితి CSS_ఖాళీలు_CELLS_SHOW:
		fprintf(ప్రవాహం, "empty-cells: show ");
		ఆపు;
	స్థితి CSS_ఖాళీలు_CELLS_HIDE:
		fprintf(ప్రవాహం, "empty-cells: hide ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* float */
	val = css_computed_float(style);
	ఎంపిక (val) {
	స్థితి CSS_FLOAT_ఎడమ:
		fprintf(ప్రవాహం, "float: left ");
		ఆపు;
	స్థితి CSS_FLOAT_కుడి:
		fprintf(ప్రవాహం, "float: right ");
		ఆపు;
	స్థితి CSS_FLOAT_NONE:
		fprintf(ప్రవాహం, "float: none ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* font-family */
	val = css_computed_font_family(style, &string_list);
	ఒకవేళ (val != CSS_FONT_FAMILY_INHERIT) {
		fprintf(ప్రవాహం, "font-family:");

		ఒకవేళ (string_list != ఖాళీ) {
			అయితే (*string_list != ఖాళీ) {
				fprintf(ప్రవాహం, " \"%.*s\"",
					(పూర్ణాంకం) lwc_string_నిడివి(*string_list),
					lwc_string_దత్తాంశం(*string_list));

				string_list++;
			}
		}
		ఎంపిక (val) {
		స్థితి CSS_FONT_FAMILY_SERIF:
			fprintf(ప్రవాహం, " serif ");
			ఆపు;
		స్థితి CSS_FONT_FAMILY_SANS_SERIF:
			fprintf(ప్రవాహం, " sans-serif ");
			ఆపు;
		స్థితి CSS_FONT_FAMILY_CURSIVE:
			fprintf(ప్రవాహం, " cursive ");
			ఆపు;
		స్థితి CSS_FONT_FAMILY_FANTASY:
			fprintf(ప్రవాహం, " fantasy ");
			ఆపు;
		స్థితి CSS_FONT_FAMILY_MONOSPACE:
			fprintf(ప్రవాహం, " monospace ");
			ఆపు;
		}
	}

	/* font-size */
	val = css_computed_font_కొలత(style, &len1, &unit1);
	ఎంపిక (val) {
	స్థితి CSS_FONT_కొలత_XX_SMALL:
		fprintf(ప్రవాహం, "font-size: xx-small ");
		ఆపు;
	స్థితి CSS_FONT_కొలత_X_SMALL:
		fprintf(ప్రవాహం, "font-size: x-small ");
		ఆపు;
	స్థితి CSS_FONT_కొలత_SMALL:
		fprintf(ప్రవాహం, "font-size: small ");
		ఆపు;
	స్థితి CSS_FONT_కొలత_MEDIUM:
		fprintf(ప్రవాహం, "font-size: medium ");
		ఆపు;
	స్థితి CSS_FONT_కొలత_LARGE:
		fprintf(ప్రవాహం, "font-size: large ");
		ఆపు;
	స్థితి CSS_FONT_కొలత_X_LARGE:
		fprintf(ప్రవాహం, "font-size: x-large ");
		ఆపు;
	స్థితి CSS_FONT_కొలత_XX_LARGE:
		fprintf(ప్రవాహం, "font-size: xx-large ");
		ఆపు;
	స్థితి CSS_FONT_కొలత_LARGER:
		fprintf(ప్రవాహం, "font-size: larger ");
		ఆపు;
	స్థితి CSS_FONT_కొలత_SMALLER:
		fprintf(ప్రవాహం, "font-size: smaller ");
		ఆపు;
	స్థితి CSS_FONT_కొలత_DIMENSION:
		fprintf(ప్రవాహం, "font-size: ");

		dump_css_unit(ప్రవాహం, len1, unit1);

		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* font-style */
	val = css_computed_font_style(style);
	ఎంపిక (val) {
	స్థితి CSS_FONT_STYLE_సాధారణ:
		fprintf(ప్రవాహం, "font-style: normal ");
		ఆపు;
	స్థితి CSS_FONT_STYLE_ITALIC:
		fprintf(ప్రవాహం, "font-style: italic ");
		ఆపు;
	స్థితి CSS_FONT_STYLE_OBLIQUE:
		fprintf(ప్రవాహం, "font-style: oblique ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* font-variant */
	val = css_computed_font_variant(style);
	ఎంపిక (val) {
	స్థితి CSS_FONT_VARIANT_సాధారణ:
		fprintf(ప్రవాహం, "font-variant: normal ");
		ఆపు;
	స్థితి CSS_FONT_VARIANT_SMALL_CAPS:
		fprintf(ప్రవాహం, "font-variant: small-caps ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* font-weight */
	val = css_computed_font_బరువు(style);
	ఎంపిక (val) {
	స్థితి CSS_FONT_బరువు_సాధారణ:
		fprintf(ప్రవాహం, "font-weight: normal ");
		ఆపు;
	స్థితి CSS_FONT_బరువు_BOLD:
		fprintf(ప్రవాహం, "font-weight: bold ");
		ఆపు;
	స్థితి CSS_FONT_బరువు_BOLDER:
		fprintf(ప్రవాహం, "font-weight: bolder ");
		ఆపు;
	స్థితి CSS_FONT_బరువు_LIGHTER:
		fprintf(ప్రవాహం, "font-weight: lighter ");
		ఆపు;
	స్థితి CSS_FONT_బరువు_100:
		fprintf(ప్రవాహం, "font-weight: 100 ");
		ఆపు;
	స్థితి CSS_FONT_బరువు_200:
		fprintf(ప్రవాహం, "font-weight: 200 ");
		ఆపు;
	స్థితి CSS_FONT_బరువు_300:
		fprintf(ప్రవాహం, "font-weight: 300 ");
		ఆపు;
	స్థితి CSS_FONT_బరువు_400:
		fprintf(ప్రవాహం, "font-weight: 400 ");
		ఆపు;
	స్థితి CSS_FONT_బరువు_500:
		fprintf(ప్రవాహం, "font-weight: 500 ");
		ఆపు;
	స్థితి CSS_FONT_బరువు_600:
		fprintf(ప్రవాహం, "font-weight: 600 ");
		ఆపు;
	స్థితి CSS_FONT_బరువు_700:
		fprintf(ప్రవాహం, "font-weight: 700 ");
		ఆపు;
	స్థితి CSS_FONT_బరువు_800:
		fprintf(ప్రవాహం, "font-weight: 800 ");
		ఆపు;
	స్థితి CSS_FONT_బరువు_900:
		fprintf(ప్రవాహం, "font-weight: 900 ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/*ఎత్తు*/
	val = css_computed_ఎత్తు(style, &len1, &unit1);
	ఎంపిక (val) {
	స్థితి CSS_ఎత్తు_AUTO:
		fprintf(ప్రవాహం, "height: auto ");
		ఆపు;
	స్థితి CSS_ఎత్తు_సమితి:
		fprintf(ప్రవాహం, "height: ");

		dump_css_unit(ప్రవాహం, len1, unit1);

		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/*ఎడమ*/
	val = css_computed_ఎడమ(style, &len1, &unit1);
	ఎంపిక (val) {
	స్థితి CSS_ఎడమ_AUTO:
		fprintf(ప్రవాహం, "left: auto ");
		ఆపు;
	స్థితి CSS_ఎడమ_సమితి:
		fprintf(ప్రవాహం, "left: ");

		dump_css_unit(ప్రవాహం, len1, unit1);

		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* letter-spacing */
	val = css_computed_letter_spacing(style, &len1, &unit1);
	ఎంపిక (val) {
	స్థితి CSS_LETTER_SPACING_సాధారణ:
		fprintf(ప్రవాహం, "letter-spacing: normal ");
		ఆపు;
	స్థితి CSS_LETTER_SPACING_సమితి:
		fprintf(ప్రవాహం, "letter-spacing: ");

		dump_css_unit(ప్రవాహం, len1, unit1);

		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* line-height */
	val = css_computed_పంక్తి_ఎత్తు(style, &len1, &unit1);
	ఎంపిక (val) {
	స్థితి CSS_పంక్తి_ఎత్తు_సాధారణ:
		fprintf(ప్రవాహం, "line-height: normal ");
		ఆపు;
	స్థితి CSS_పంక్తి_ఎత్తు_సంఖ్య:
		fprintf(ప్రవాహం, "line-height: ");

		dump_css_fixed(ప్రవాహం, len1);

		fprintf(ప్రవాహం, " ");
		ఆపు;
	స్థితి CSS_పంక్తి_ఎత్తు_DIMENSION:
		fprintf(ప్రవాహం, "line-height: ");

		dump_css_unit(ప్రవాహం, len1, unit1);

		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* list-style-image */
	val = css_computed_list_style_image(style, &url);
	ఒకవేళ (url != ఖాళీ) {
		fprintf(ప్రవాహం, "list-style-image: url('%.*s') ",
				(పూర్ణాంకం) lwc_string_నిడివి(url),
				lwc_string_దత్తాంశం(url));
	} లేకపోతే ఒకవేళ (val == CSS_LIST_STYLE_IMAGE_NONE) {
		fprintf(ప్రవాహం, "list-style-image: none ");
	}

	/* list-style-position */
	val = css_computed_list_style_position(style);
	ఎంపిక (val) {
	స్థితి CSS_LIST_STYLE_POSITION_INSIDE:
		fprintf(ప్రవాహం, "list-style-position: inside ");
		ఆపు;
	స్థితి CSS_LIST_STYLE_POSITION_OUTSIDE:
		fprintf(ప్రవాహం, "list-style-position: outside ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* list-style-type */
	val = css_computed_list_style_type(style);
	ఎంపిక (val) {
	స్థితి CSS_LIST_STYLE_TYPE_DISC:
		fprintf(ప్రవాహం, "list-style-type: disc ");
		ఆపు;
	స్థితి CSS_LIST_STYLE_TYPE_CIRCLE:
		fprintf(ప్రవాహం, "list-style-type: circle ");
		ఆపు;
	స్థితి CSS_LIST_STYLE_TYPE_వర్గం:
		fprintf(ప్రవాహం, "list-style-type: square ");
		ఆపు;
	స్థితి CSS_LIST_STYLE_TYPE_DECIMAL:
		fprintf(ప్రవాహం, "list-style-type: decimal ");
		ఆపు;
	స్థితి CSS_LIST_STYLE_TYPE_DECIMAL_LEADING_ZERO:
		fprintf(ప్రవాహం, "list-style-type: decimal-leading-zero ");
		ఆపు;
	స్థితి CSS_LIST_STYLE_TYPE_క్రింది_ROMAN:
		fprintf(ప్రవాహం, "list-style-type: lower-roman ");
		ఆపు;
	స్థితి CSS_LIST_STYLE_TYPE_పై_ROMAN:
		fprintf(ప్రవాహం, "list-style-type: upper-roman ");
		ఆపు;
	స్థితి CSS_LIST_STYLE_TYPE_క్రింది_GREEK:
		fprintf(ప్రవాహం, "list-style-type: lower-greek ");
		ఆపు;
	స్థితి CSS_LIST_STYLE_TYPE_క్రింది_LATIN:
		fprintf(ప్రవాహం, "list-style-type: lower-latin ");
		ఆపు;
	స్థితి CSS_LIST_STYLE_TYPE_పై_LATIN:
		fprintf(ప్రవాహం, "list-style-type: upper-latin ");
		ఆపు;
	స్థితి CSS_LIST_STYLE_TYPE_ARMENIAN:
		fprintf(ప్రవాహం, "list-style-type: armenian ");
		ఆపు;
	స్థితి CSS_LIST_STYLE_TYPE_GEORGIAN:
		fprintf(ప్రవాహం, "list-style-type: georgian ");
		ఆపు;
	స్థితి CSS_LIST_STYLE_TYPE_క్రింది_ALPHA:
		fprintf(ప్రవాహం, "list-style-type: lower-alpha ");
		ఆపు;
	స్థితి CSS_LIST_STYLE_TYPE_పై_ALPHA:
		fprintf(ప్రవాహం, "list-style-type: upper-alpha ");
		ఆపు;
	స్థితి CSS_LIST_STYLE_TYPE_NONE:
		fprintf(ప్రవాహం, "list-style-type: none ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* margin-top */
	val = css_computed_margin_శిఖరం(style, &len1, &unit1);
	ఎంపిక (val) {
	స్థితి CSS_MARGIN_AUTO:
		fprintf(ప్రవాహం, "margin-top: auto ");
		ఆపు;
	స్థితి CSS_MARGIN_సమితి:
		fprintf(ప్రవాహం, "margin-top: ");

		dump_css_unit(ప్రవాహం, len1, unit1);

		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* margin-right */
	val = css_computed_margin_కుడి(style, &len1, &unit1);
	ఎంపిక (val) {
	స్థితి CSS_MARGIN_AUTO:
		fprintf(ప్రవాహం, "margin-right: auto ");
		ఆపు;
	స్థితి CSS_MARGIN_సమితి:
		fprintf(ప్రవాహం, "margin-right: ");

		dump_css_unit(ప్రవాహం, len1, unit1);

		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* margin-bottom */
	val = css_computed_margin_bottom(style, &len1, &unit1);
	ఎంపిక (val) {
	స్థితి CSS_MARGIN_AUTO:
		fprintf(ప్రవాహం, "margin-bottom: auto ");
		ఆపు;
	స్థితి CSS_MARGIN_సమితి:
		fprintf(ప్రవాహం, "margin-bottom: ");

		dump_css_unit(ప్రవాహం, len1, unit1);

		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* margin-left */
	val = css_computed_margin_ఎడమ(style, &len1, &unit1);
	ఎంపిక (val) {
	స్థితి CSS_MARGIN_AUTO:
		fprintf(ప్రవాహం, "margin-left: auto ");
		ఆపు;
	స్థితి CSS_MARGIN_సమితి:
		fprintf(ప్రవాహం, "margin-left: ");

		dump_css_unit(ప్రవాహం, len1, unit1);

		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* max-height */
	val = css_computed_max_ఎత్తు(style, &len1, &unit1);
	ఎంపిక (val) {
	స్థితి CSS_MAX_ఎత్తు_NONE:
		fprintf(ప్రవాహం, "max-height: none ");
		ఆపు;
	స్థితి CSS_MAX_ఎత్తు_సమితి:
		fprintf(ప్రవాహం, "max-height: ");

		dump_css_unit(ప్రవాహం, len1, unit1);

		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* max-width */
	val = css_computed_max_వెడల్పు(style, &len1, &unit1);
	ఎంపిక (val) {
	స్థితి CSS_MAX_వెడల్పు_NONE:
		fprintf(ప్రవాహం, "max-width: none ");
		ఆపు;
	స్థితి CSS_MAX_వెడల్పు_సమితి:
		fprintf(ప్రవాహం, "max-width: ");

		dump_css_unit(ప్రవాహం, len1, unit1);

		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* min-height */
	val = ns_computed_min_ఎత్తు(style, &len1, &unit1);
	ఎంపిక (val) {
	స్థితి CSS_MIN_ఎత్తు_సమితి:
		fprintf(ప్రవాహం, "min-height: ");

		dump_css_unit(ప్రవాహం, len1, unit1);

		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* min-width */
	val = ns_computed_min_వెడల్పు(style, &len1, &unit1);
	ఎంపిక (val) {
	స్థితి CSS_MIN_వెడల్పు_సమితి:
		fprintf(ప్రవాహం, "min-width: ");

		dump_css_unit(ప్రవాహం, len1, unit1);

		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* opacity */
	val = css_computed_opacity(style, &len1);
	ఎంపిక (val) {
	స్థితి CSS_OPACITY_సమితి:
		fprintf(ప్రవాహం, "opacity: ");

		dump_css_fixed(ప్రవాహం, len1);

		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* outline-color */
	val = css_computed_outline_color(style, &color);
	ఎంపిక (val) {
	స్థితి CSS_OUTLINE_COLOR_INVERT:
		fprintf(ప్రవాహం, "outline-color: invert ");
		ఆపు;
	స్థితి CSS_OUTLINE_COLOR_COLOR:
		fprintf(ప్రవాహం, "outline-color: #%08"PRIx32" ", color);
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* outline-style */
	val = css_computed_outline_style(style);
	ఎంపిక (val) {
	స్థితి CSS_OUTLINE_STYLE_NONE:
		fprintf(ప్రవాహం, "outline-style: none ");
		ఆపు;
	స్థితి CSS_OUTLINE_STYLE_DOTTED:
		fprintf(ప్రవాహం, "outline-style: dotted ");
		ఆపు;
	స్థితి CSS_OUTLINE_STYLE_DASHED:
		fprintf(ప్రవాహం, "outline-style: dashed ");
		ఆపు;
	స్థితి CSS_OUTLINE_STYLE_SOLID:
		fprintf(ప్రవాహం, "outline-style: solid ");
		ఆపు;
	స్థితి CSS_OUTLINE_STYLE_DOUBLE:
		fprintf(ప్రవాహం, "outline-style: double ");
		ఆపు;
	స్థితి CSS_OUTLINE_STYLE_GROOVE:
		fprintf(ప్రవాహం, "outline-style: groove ");
		ఆపు;
	స్థితి CSS_OUTLINE_STYLE_RIDGE:
		fprintf(ప్రవాహం, "outline-style: ridge ");
		ఆపు;
	స్థితి CSS_OUTLINE_STYLE_INSET:
		fprintf(ప్రవాహం, "outline-style: inset ");
		ఆపు;
	స్థితి CSS_OUTLINE_STYLE_OUTSET:
		fprintf(ప్రవాహం, "outline-style: outset ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* outline-width */
	val = css_computed_outline_వెడల్పు(style, &len1, &unit1);
	ఎంపిక (val) {
	స్థితి CSS_OUTLINE_వెడల్పు_THIN:
		fprintf(ప్రవాహం, "outline-width: thin ");
		ఆపు;
	స్థితి CSS_OUTLINE_వెడల్పు_MEDIUM:
		fprintf(ప్రవాహం, "outline-width: medium ");
		ఆపు;
	స్థితి CSS_OUTLINE_వెడల్పు_THICK:
		fprintf(ప్రవాహం, "outline-width: thick ");
		ఆపు;
	స్థితి CSS_OUTLINE_వెడల్పు_వెడల్పు:
		fprintf(ప్రవాహం, "outline-width: ");

		dump_css_unit(ప్రవాహం, len1, unit1);

		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/*అధికప్రవాహం*/
	val = css_computed_అధికప్రవాహం_x(style);
	ఎంపిక (val) {
	స్థితి CSS_అధికప్రవాహం_VISIBLE:
		fprintf(ప్రవాహం, "overflow-x: visible ");
		ఆపు;
	స్థితి CSS_అధికప్రవాహం_HIDDEN:
		fprintf(ప్రవాహం, "overflow-x: hidden ");
		ఆపు;
	స్థితి CSS_అధికప్రవాహం_SCROLL:
		fprintf(ప్రవాహం, "overflow-x: scroll ");
		ఆపు;
	స్థితి CSS_అధికప్రవాహం_AUTO:
		fprintf(ప్రవాహం, "overflow-x auto ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/*అధికప్రవాహం*/
	val = css_computed_అధికప్రవాహం_y(style);
	ఎంపిక (val) {
	స్థితి CSS_అధికప్రవాహం_VISIBLE:
		fprintf(ప్రవాహం, "overflow-y: visible ");
		ఆపు;
	స్థితి CSS_అధికప్రవాహం_HIDDEN:
		fprintf(ప్రవాహం, "overflow-y: hidden ");
		ఆపు;
	స్థితి CSS_అధికప్రవాహం_SCROLL:
		fprintf(ప్రవాహం, "overflow-y: scroll ");
		ఆపు;
	స్థితి CSS_అధికప్రవాహం_AUTO:
		fprintf(ప్రవాహం, "overflow-y: auto ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* padding-top */
	val = css_computed_padding_శిఖరం(style, &len1, &unit1);
	ఎంపిక (val) {
	స్థితి CSS_PADDING_సమితి:
		fprintf(ప్రవాహం, "padding-top: ");

		dump_css_unit(ప్రవాహం, len1, unit1);

		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* padding-right */
	val = css_computed_padding_కుడి(style, &len1, &unit1);
	ఎంపిక (val) {
	స్థితి CSS_PADDING_సమితి:
		fprintf(ప్రవాహం, "padding-right: ");

		dump_css_unit(ప్రవాహం, len1, unit1);

		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* padding-bottom */
	val = css_computed_padding_bottom(style, &len1, &unit1);
	ఎంపిక (val) {
	స్థితి CSS_PADDING_సమితి:
		fprintf(ప్రవాహం, "padding-bottom: ");

		dump_css_unit(ప్రవాహం, len1, unit1);

		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* padding-left */
	val = css_computed_padding_ఎడమ(style, &len1, &unit1);
	ఎంపిక (val) {
	స్థితి CSS_PADDING_సమితి:
		fprintf(ప్రవాహం, "padding-left: ");

		dump_css_unit(ప్రవాహం, len1, unit1);

		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* position */
	val = css_computed_position(style);
	ఎంపిక (val) {
	స్థితి CSS_POSITION_STATIC:
		fprintf(ప్రవాహం, "position: static ");
		ఆపు;
	స్థితి CSS_POSITION_RELATIVE:
		fprintf(ప్రవాహం, "position: relative ");
		ఆపు;
	స్థితి CSS_POSITION_ABSOLUTE:
		fprintf(ప్రవాహం, "position: absolute ");
		ఆపు;
	స్థితి CSS_POSITION_FIXED:
		fprintf(ప్రవాహం, "position: fixed ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* quotes */
	val = css_computed_quotes(style, &string_list);
	ఒకవేళ (val == CSS_QUOTES_STRING && string_list != ఖాళీ) {
		fprintf(ప్రవాహం, "quotes:");

		అయితే (*string_list != ఖాళీ) {
			fprintf(ప్రవాహం, " \"%.*s\"",
				(పూర్ణాంకం) lwc_string_నిడివి(*string_list),
				lwc_string_దత్తాంశం(*string_list));

			string_list++;
		}

		fprintf(ప్రవాహం, " ");
	} లేకపోతే {
		ఎంపిక (val) {
		స్థితి CSS_QUOTES_NONE:
			fprintf(ప్రవాహం, "quotes: none ");
			ఆపు;
		అప్రమేయ:
			ఆపు;
		}
	}

	/*కుడి*/
	val = css_computed_కుడి(style, &len1, &unit1);
	ఎంపిక (val) {
	స్థితి CSS_కుడి_AUTO:
		fprintf(ప్రవాహం, "right: auto ");
		ఆపు;
	స్థితి CSS_కుడి_సమితి:
		fprintf(ప్రవాహం, "right: ");

		dump_css_unit(ప్రవాహం, len1, unit1);

		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* table-layout */
	val = css_computed_పట్టిక_layout(style);
	ఎంపిక (val) {
	స్థితి CSS_పట్టిక_LAYOUT_AUTO:
		fprintf(ప్రవాహం, "table-layout: auto ");
		ఆపు;
	స్థితి CSS_పట్టిక_LAYOUT_FIXED:
		fprintf(ప్రవాహం, "table-layout: fixed ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* text-align */
	val = css_computed_పాఠ్యం_align(style);
	ఎంపిక (val) {
	స్థితి CSS_పాఠ్యం_ALIGN_ఎడమ:
		fprintf(ప్రవాహం, "text-align: left ");
		ఆపు;
	స్థితి CSS_పాఠ్యం_ALIGN_కుడి:
		fprintf(ప్రవాహం, "text-align: right ");
		ఆపు;
	స్థితి CSS_పాఠ్యం_ALIGN_CENTER:
		fprintf(ప్రవాహం, "text-align: center ");
		ఆపు;
	స్థితి CSS_పాఠ్యం_ALIGN_JUSTIFY:
		fprintf(ప్రవాహం, "text-align: justify ");
		ఆపు;
	స్థితి CSS_పాఠ్యం_ALIGN_DEFAULT:
		fprintf(ప్రవాహం, "text-align: default ");
		ఆపు;
	స్థితి CSS_పాఠ్యం_ALIGN_LIBCSS_ఎడమ:
		fprintf(ప్రవాహం, "text-align: -libcss-left ");
		ఆపు;
	స్థితి CSS_పాఠ్యం_ALIGN_LIBCSS_CENTER:
		fprintf(ప్రవాహం, "text-align: -libcss-center ");
		ఆపు;
	స్థితి CSS_పాఠ్యం_ALIGN_LIBCSS_కుడి:
		fprintf(ప్రవాహం, "text-align: -libcss-right ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* text-decoration */
	val = css_computed_పాఠ్యం_decoration(style);
	ఒకవేళ (val == CSS_పాఠ్యం_DECORATION_NONE) {
		fprintf(ప్రవాహం, "text-decoration: none ");
	} లేకపోతే {
		fprintf(ప్రవాహం, "text-decoration:");

		ఒకవేళ (val & CSS_పాఠ్యం_DECORATION_BLINK) {
			fprintf(ప్రవాహం, " blink");
		}
		ఒకవేళ (val & CSS_పాఠ్యం_DECORATION_పంక్తి_THROUGH) {
			fprintf(ప్రవాహం, " line-through");
		}
		ఒకవేళ (val & CSS_పాఠ్యం_DECORATION_OVERLINE) {
			fprintf(ప్రవాహం, " overline");
		}
		ఒకవేళ (val & CSS_పాఠ్యం_DECORATION_UNDERLINE) {
			fprintf(ప్రవాహం, " underline");
		}

		fprintf(ప్రవాహం, " ");
	}

	/* text-indent */
	val = css_computed_పాఠ్యం_indent(style, &len1, &unit1);
	ఎంపిక (val) {
	స్థితి CSS_పాఠ్యం_INDENT_సమితి:
		fprintf(ప్రవాహం, "text-indent: ");

		dump_css_unit(ప్రవాహం, len1, unit1);

		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* text-transform */
	val = css_computed_పాఠ్యం_transform(style);
	ఎంపిక (val) {
	స్థితి CSS_పాఠ్యం_TRANSFORM_CAPITALIZE:
		fprintf(ప్రవాహం, "text-transform: capitalize ");
		ఆపు;
	స్థితి CSS_పాఠ్యం_TRANSFORM_UPPERCASE:
		fprintf(ప్రవాహం, "text-transform: uppercase ");
		ఆపు;
	స్థితి CSS_పాఠ్యం_TRANSFORM_LOWERCASE:
		fprintf(ప్రవాహం, "text-transform: lowercase ");
		ఆపు;
	స్థితి CSS_పాఠ్యం_TRANSFORM_NONE:
		fprintf(ప్రవాహం, "text-transform: none ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/*శిఖరం*/
	val = css_computed_శిఖరం(style, &len1, &unit1);
	ఎంపిక (val) {
	స్థితి CSS_శిఖరం_AUTO:
		fprintf(ప్రవాహం, "top: auto ");
		ఆపు;
	స్థితి CSS_శిఖరం_సమితి:
		fprintf(ప్రవాహం, "top: ");

		dump_css_unit(ప్రవాహం, len1, unit1);

		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* unicode-bidi */
	val = css_computed_unicode_bidi(style);
	ఎంపిక (val) {
	స్థితి CSS_UNICODE_BIDI_సాధారణ:
		fprintf(ప్రవాహం, "unicode-bidi: normal ");
		ఆపు;
	స్థితి CSS_UNICODE_BIDI_EMBED:
		fprintf(ప్రవాహం, "unicode-bidi: embed ");
		ఆపు;
	స్థితి CSS_UNICODE_BIDI_BIDI_OVERRIDE:
		fprintf(ప్రవాహం, "unicode-bidi: bidi-override ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* vertical-align */
	val = css_computed_vertical_align(style, &len1, &unit1);
	ఎంపిక (val) {
	స్థితి CSS_VERTICAL_ALIGN_BASELINE:
		fprintf(ప్రవాహం, "vertical-align: baseline ");
		ఆపు;
	స్థితి CSS_VERTICAL_ALIGN_SUB:
		fprintf(ప్రవాహం, "vertical-align: sub ");
		ఆపు;
	స్థితి CSS_VERTICAL_ALIGN_SUPER:
		fprintf(ప్రవాహం, "vertical-align: super ");
		ఆపు;
	స్థితి CSS_VERTICAL_ALIGN_శిఖరం:
		fprintf(ప్రవాహం, "vertical-align: top ");
		ఆపు;
	స్థితి CSS_VERTICAL_ALIGN_పాఠ్యం_శిఖరం:
		fprintf(ప్రవాహం, "vertical-align: text-top ");
		ఆపు;
	స్థితి CSS_VERTICAL_ALIGN_మధ్య:
		fprintf(ప్రవాహం, "vertical-align: middle ");
		ఆపు;
	స్థితి CSS_VERTICAL_ALIGN_BOTTOM:
		fprintf(ప్రవాహం, "vertical-align: bottom ");
		ఆపు;
	స్థితి CSS_VERTICAL_ALIGN_పాఠ్యం_BOTTOM:
		fprintf(ప్రవాహం, "vertical-align: text-bottom ");
		ఆపు;
	స్థితి CSS_VERTICAL_ALIGN_సమితి:
		fprintf(ప్రవాహం, "vertical-align: ");

		dump_css_unit(ప్రవాహం, len1, unit1);

		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* visibility */
	val = css_computed_visibility(style);
	ఎంపిక (val) {
	స్థితి CSS_VISIBILITY_VISIBLE:
		fprintf(ప్రవాహం, "visibility: visible ");
		ఆపు;
	స్థితి CSS_VISIBILITY_HIDDEN:
		fprintf(ప్రవాహం, "visibility: hidden ");
		ఆపు;
	స్థితి CSS_VISIBILITY_COLLAPSE:
		fprintf(ప్రవాహం, "visibility: collapse ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* white-space */
	val = css_computed_white_space(style);
	ఎంపిక (val) {
	స్థితి CSS_WHITE_SPACE_సాధారణ:
		fprintf(ప్రవాహం, "white-space: normal ");
		ఆపు;
	స్థితి CSS_WHITE_SPACE_PRE:
		fprintf(ప్రవాహం, "white-space: pre ");
		ఆపు;
	స్థితి CSS_WHITE_SPACE_NOWRAP:
		fprintf(ప్రవాహం, "white-space: nowrap ");
		ఆపు;
	స్థితి CSS_WHITE_SPACE_PRE_WRAP:
		fprintf(ప్రవాహం, "white-space: pre-wrap ");
		ఆపు;
	స్థితి CSS_WHITE_SPACE_PRE_పంక్తి:
		fprintf(ప్రవాహం, "white-space: pre-line ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/*వెడల్పు*/
	val = css_computed_వెడల్పు(style, &len1, &unit1);
	ఎంపిక (val) {
	స్థితి CSS_వెడల్పు_AUTO:
		fprintf(ప్రవాహం, "width: auto ");
		ఆపు;
	స్థితి CSS_వెడల్పు_సమితి:
		fprintf(ప్రవాహం, "width: ");

		dump_css_unit(ప్రవాహం, len1, unit1);

		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* word-spacing */
	val = css_computed_మాట_spacing(style, &len1, &unit1);
	ఎంపిక (val) {
	స్థితి CSS_మాట_SPACING_సాధారణ:
		fprintf(ప్రవాహం, "word-spacing: normal ");
		ఆపు;
	స్థితి CSS_మాట_SPACING_సమితి:
		fprintf(ప్రవాహం, "word-spacing: ");

		dump_css_unit(ప్రవాహం, len1, unit1);

		fprintf(ప్రవాహం, " ");
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* z-index */
	val = css_computed_z_సూచీ(style, &zindex);
	ఎంపిక (val) {
	స్థితి CSS_Z_సూచీ_AUTO:
		fprintf(ప్రవాహం, "z-index: auto ");
		ఆపు;
	స్థితి CSS_Z_సూచీ_సమితి:
		fprintf(ప్రవాహం, "z-index: %"PRId32" ", zindex);
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	fprintf(ప్రవాహం, "}");
}
