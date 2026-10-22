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

#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <libcss/libcss.h>

#અન્તર્ભાવ "netsurf/inttypes.h"

#અન્તર્ભાવ "css/dump.h"
#અન્તર્ભાવ "css/utils.h"

/**
 * Dump a fixed point value to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param f       Value to write
 */
સ્થિર શૂન્ય dump_css_fixed(FILE *stream, css_fixed f)
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
સ્થિર શૂન્ય dump_css_સંખ્યા(FILE *stream, css_fixed val)
{
	જો (INTTOFIX(FIXTOINT(val)) == val)
		fprintf(stream, "%"PRId32, FIXTOINT(val));
	નહીંતર
		dump_css_fixed(stream, val);
}

/**
 * Dump a dimension to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 * \param unit    Unit to write
 */
સ્થિર શૂન્ય dump_css_unit(FILE *stream, css_fixed val, css_unit unit)
{
	dump_css_સંખ્યા(stream, val);

	પસંદગી (unit) {
	સ્થિતિ CSS_UNIT_PX:
		fprintf(stream, "px");
		તોડો;
	સ્થિતિ CSS_UNIT_EX:
		fprintf(stream, "ex");
		તોડો;
	સ્થિતિ CSS_UNIT_EM:
		fprintf(stream, "em");
		તોડો;
	સ્થિતિ CSS_UNIT_IN:
		fprintf(stream, "in");
		તોડો;
	સ્થિતિ CSS_UNIT_CM:
		fprintf(stream, "cm");
		તોડો;
	સ્થિતિ CSS_UNIT_MM:
		fprintf(stream, "mm");
		તોડો;
	સ્થિતિ CSS_UNIT_PT:
		fprintf(stream, "pt");
		તોડો;
	સ્થિતિ CSS_UNIT_PC:
		fprintf(stream, "pc");
		તોડો;
	સ્થિતિ CSS_UNIT_PCT:
		fprintf(stream, "%%");
		તોડો;
	સ્થિતિ CSS_UNIT_DEG:
		fprintf(stream, "deg");
		તોડો;
	સ્થિતિ CSS_UNIT_GRAD:
		fprintf(stream, "grad");
		તોડો;
	સ્થિતિ CSS_UNIT_RAD:
		fprintf(stream, "rad");
		તોડો;
	સ્થિતિ CSS_UNIT_MS:
		fprintf(stream, "ms");
		તોડો;
	સ્થિતિ CSS_UNIT_S:
		fprintf(stream, "s");
		તોડો;
	સ્થિતિ CSS_UNIT_HZ:
		fprintf(stream, "Hz");
		તોડો;
	સ્થિતિ CSS_UNIT_KHZ:
		fprintf(stream, "kHz");
		તોડો;
	સ્થિતિ CSS_UNIT_CH:
		fprintf(stream, "ch");
		તોડો;
	સ્થિતિ CSS_UNIT_REM:
		fprintf(stream, "rem");
		તોડો;
	સ્થિતિ CSS_UNIT_LH:
		fprintf(stream, "lh");
		તોડો;
	સ્થિતિ CSS_UNIT_VH:
		fprintf(stream, "vh");
		તોડો;
	સ્થિતિ CSS_UNIT_VW:
		fprintf(stream, "vw");
		તોડો;
	સ્થિતિ CSS_UNIT_VI:
		fprintf(stream, "vi");
		તોડો;
	સ્થિતિ CSS_UNIT_VB:
		fprintf(stream, "vb");
		તોડો;
	સ્થિતિ CSS_UNIT_VMIN:
		fprintf(stream, "vmin");
		તોડો;
	સ્થિતિ CSS_UNIT_VMAX:
		fprintf(stream, "vmax");
		તોડો;
	સ્થિતિ CSS_UNIT_Q:
		fprintf(stream, "q");
		તોડો;
	સ્થિતિ CSS_UNIT_CALC:
		fprintf(stream, "calc()");
		તોડો;
	}
}

/* exported interface documented in content/handlers/css/dump.h */
શૂન્ય nscss_dump_computed_style(FILE *stream, અચલ css_computed_style *style)
{
	uint8_t val;
	css_color color = 0;
	lwc_string *url = ખાલી;
	css_fixed len1 = 0, len2 = 0;
	css_unit unit1 = CSS_UNIT_PX, unit2 = CSS_UNIT_PX;
	css_computed_clip_rect rect = { 0, 0, 0, 0, CSS_UNIT_PX, CSS_UNIT_PX,
					CSS_UNIT_PX, CSS_UNIT_PX, true, true,
					true, true };
	અચલ css_computed_content_item *content = ખાલી;
	અચલ css_computed_counter *counter = ખાલી;
	lwc_string **string_list = ખાલી;
	int32_t zindex = 0;

	fprintf(stream, "{ ");

	/* background-attachment */
	val = css_computed_background_attachment(style);
	પસંદગી (val) {
	સ્થિતિ CSS_BACKGROUND_ATTACHMENT_FIXED:
		fprintf(stream, "background-attachment: fixed ");
		તોડો;
	સ્થિતિ CSS_BACKGROUND_ATTACHMENT_SCROLL:
		fprintf(stream, "background-attachment: scroll ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* background-color */
	val = css_computed_background_color(style, &color);
	પસંદગી (val) {
	સ્થિતિ CSS_BACKGROUND_COLOR_COLOR:
		fprintf(stream, "background-color: #%08"PRIx32" ", color);
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* background-image */
	val = css_computed_background_image(style, &url);
	જો (val == CSS_BACKGROUND_IMAGE_IMAGE && url != ખાલી) {
		fprintf(stream, "background-image: url('%.*s') ",
				(પૂર્ણાંક) lwc_string_length(url),
				lwc_string_data(url));
	} નહીંતર જો (val == CSS_BACKGROUND_IMAGE_NONE) {
		fprintf(stream, "background-image: none ");
	}

	/* background-position */
	val = css_computed_background_position(style, &len1, &unit1,
			&len2, &unit2);
	જો (val == CSS_BACKGROUND_POSITION_SET) {
		fprintf(stream, "background-position: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		dump_css_unit(stream, len2, unit2);
		fprintf(stream, " ");
	}

	/* background-repeat */
	val = css_computed_background_repeat(style);
	પસંદગી (val) {
	સ્થિતિ CSS_BACKGROUND_REPEAT_REPEAT_X:
		fprintf(stream, "background-repeat: repeat-x ");
		તોડો;
	સ્થિતિ CSS_BACKGROUND_REPEAT_REPEAT_Y:
		fprintf(stream, "background-repeat: repeat-y ");
		તોડો;
	સ્થિતિ CSS_BACKGROUND_REPEAT_REPEAT:
		fprintf(stream, "background-repeat: repeat ");
		તોડો;
	સ્થિતિ CSS_BACKGROUND_REPEAT_NO_REPEAT:
		fprintf(stream, "background-repeat: no-repeat ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* border-collapse */
	val = css_computed_border_collapse(style);
	પસંદગી (val) {
	સ્થિતિ CSS_BORDER_COLLAPSE_SEPARATE:
		fprintf(stream, "border-collapse: separate ");
		તોડો;
	સ્થિતિ CSS_BORDER_COLLAPSE_COLLAPSE:
		fprintf(stream, "border-collapse: collapse ");
		તોડો;
	મૂળભૂત:

		તોડો;
	}

	/* border-spacing */
	val = css_computed_border_spacing(style, &len1, &unit1, &len2, &unit2);
	જો (val == CSS_BORDER_SPACING_SET) {
		fprintf(stream, "border-spacing: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		dump_css_unit(stream, len2, unit2);
		fprintf(stream, " ");
	}

	/* border-top-color */
	val = css_computed_border_ટોચ_color(style, &color);
	પસંદગી (val) {
	સ્થિતિ CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-top-color: #%08"PRIx32" ", color);
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* border-right-color */
	val = css_computed_border_જમણું_color(style, &color);
	પસંદગી (val) {
	સ્થિતિ CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-right-color: #%08"PRIx32" ", color);
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* border-bottom-color */
	val = css_computed_border_bottom_color(style, &color);
	પસંદગી (val) {
	સ્થિતિ CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-bottom-color: #%08"PRIx32" ", color);
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* border-left-color */
	val = css_computed_border_ડાબું_color(style, &color);
	પસંદગી (val) {
	સ્થિતિ CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-left-color: #%08"PRIx32" ", color);
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* border-top-style */
	val = css_computed_border_ટોચ_style(style);
	પસંદગી (val) {
	સ્થિતિ CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-top-style: none ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-top-style: hidden ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-top-style: dotted ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-top-style: dashed ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-top-style: solid ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-top-style: double ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-top-style: groove ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-top-style: ridge ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-top-style: inset ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-top-style: outset ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* border-right-style */
	val = css_computed_border_જમણું_style(style);
	પસંદગી (val) {
	સ્થિતિ CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-right-style: none ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-right-style: hidden ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-right-style: dotted ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-right-style: dashed ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-right-style: solid ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-right-style: double ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-right-style: groove ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-right-style: ridge ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-right-style: inset ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-right-style: outset ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* border-bottom-style */
	val = css_computed_border_bottom_style(style);
	પસંદગી (val) {
	સ્થિતિ CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-bottom-style: none ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-bottom-style: hidden ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-bottom-style: dotted ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-bottom-style: dashed ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-bottom-style: solid ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-bottom-style: double ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-bottom-style: groove ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-bottom-style: ridge ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-bottom-style: inset ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-bottom-style: outset ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* border-left-style */
	val = css_computed_border_ડાબું_style(style);
	પસંદગી (val) {
	સ્થિતિ CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-left-style: none ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-left-style: hidden ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-left-style: dotted ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-left-style: dashed ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-left-style: solid ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-left-style: double ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-left-style: groove ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-left-style: ridge ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-left-style: inset ");
		તોડો;
	સ્થિતિ CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-left-style: outset ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* border-top-width */
	val = css_computed_border_ટોચ_width(style, &len1, &unit1);
	પસંદગી (val) {
	સ્થિતિ CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-top-width: thin ");
		તોડો;
	સ્થિતિ CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-top-width: medium ");
		તોડો;
	સ્થિતિ CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-top-width: thick ");
		તોડો;
	સ્થિતિ CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-top-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* border-right-width */
	val = css_computed_border_જમણું_width(style, &len1, &unit1);
	પસંદગી (val) {
	સ્થિતિ CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-right-width: thin ");
		તોડો;
	સ્થિતિ CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-right-width: medium ");
		તોડો;
	સ્થિતિ CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-right-width: thick ");
		તોડો;
	સ્થિતિ CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-right-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* border-bottom-width */
	val = css_computed_border_bottom_width(style, &len1, &unit1);
	પસંદગી (val) {
	સ્થિતિ CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-bottom-width: thin ");
		તોડો;
	સ્થિતિ CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-bottom-width: medium ");
		તોડો;
	સ્થિતિ CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-bottom-width: thick ");
		તોડો;
	સ્થિતિ CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-bottom-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* border-left-width */
	val = css_computed_border_ડાબું_width(style, &len1, &unit1);
	પસંદગી (val) {
	સ્થિતિ CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-left-width: thin ");
		તોડો;
	સ્થિતિ CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-left-width: medium ");
		તોડો;
	સ્થિતિ CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-left-width: thick ");
		તોડો;
	સ્થિતિ CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-left-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* bottom */
	val = css_computed_bottom(style, &len1, &unit1);
	પસંદગી (val) {
	સ્થિતિ CSS_BOTTOM_AUTO:
		fprintf(stream, "bottom: auto ");
		તોડો;
	સ્થિતિ CSS_BOTTOM_SET:
		fprintf(stream, "bottom: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* caption-side */
	val = css_computed_caption_side(style);
	પસંદગી (val) {
	સ્થિતિ CSS_CAPTION_SIDE_ટોચ:
		fprintf(stream, "caption_side: top ");
		તોડો;
	સ્થિતિ CSS_CAPTION_SIDE_BOTTOM:
		fprintf(stream, "caption_side: bottom ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* clear */
	val = css_computed_clear(style);
	પસંદગી (val) {
	સ્થિતિ CSS_CLEAR_NONE:
		fprintf(stream, "clear: none ");
		તોડો;
	સ્થિતિ CSS_CLEAR_ડાબું:
		fprintf(stream, "clear: left ");
		તોડો;
	સ્થિતિ CSS_CLEAR_જમણું:
		fprintf(stream, "clear: right ");
		તોડો;
	સ્થિતિ CSS_CLEAR_BOTH:
		fprintf(stream, "clear: both ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* clip */
	val = css_computed_clip(style, &rect);
	પસંદગી (val) {
	સ્થિતિ CSS_CLIP_AUTO:
		fprintf(stream, "clip: auto ");
		તોડો;
	સ્થિતિ CSS_CLIP_RECT:
		fprintf(stream, "clip: rect( ");

		જો (rect.ટોચ_auto)
			fprintf(stream, "auto");
		નહીંતર
			dump_css_unit(stream, rect.ટોચ, rect.tunit);
		fprintf(stream, ", ");

		જો (rect.જમણું_auto)
			fprintf(stream, "auto");
		નહીંતર
			dump_css_unit(stream, rect.જમણું, rect.runit);
		fprintf(stream, ", ");

		જો (rect.bottom_auto)
			fprintf(stream, "auto");
		નહીંતર
			dump_css_unit(stream, rect.bottom, rect.bunit);
		fprintf(stream, ", ");

		જો (rect.ડાબું_auto)
			fprintf(stream, "auto");
		નહીંતર
			dump_css_unit(stream, rect.ડાબું, rect.lunit);
		fprintf(stream, ") ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* color */
	val = css_computed_color(style, &color);
	જો (val == CSS_COLOR_COLOR) {
		fprintf(stream, "color: #%08"PRIx32" ", color);
	}

	/* content */
	val = css_computed_content(style, &content);
	પસંદગી (val) {
	સ્થિતિ CSS_CONTENT_NONE:
		fprintf(stream, "content: none ");
		તોડો;
	સ્થિતિ CSS_CONTENT_NORMAL:
		fprintf(stream, "content: normal ");
		તોડો;
	સ્થિતિ CSS_CONTENT_SET:
		fprintf(stream, "content:");

		જ્યારે (content->type != CSS_COMPUTED_CONTENT_NONE) {
			fprintf(stream, " ");

			પસંદગી (content->type) {
			સ્થિતિ CSS_COMPUTED_CONTENT_STRING:
				fprintf(stream,	"\"%.*s\"",
						(પૂર્ણાંક) lwc_string_length(
						content->data.string),
						lwc_string_data(
						content->data.string));
				તોડો;
			સ્થિતિ CSS_COMPUTED_CONTENT_URI:
				fprintf(stream,	"uri(\"%.*s\")",
						(પૂર્ણાંક) lwc_string_length(
						content->data.uri),
						lwc_string_data(
						content->data.uri));
				તોડો;
			સ્થિતિ CSS_COMPUTED_CONTENT_COUNTER:
				fprintf(stream, "counter(%.*s)",
						(પૂર્ણાંક) lwc_string_length(
						content->data.counter.name),
						lwc_string_data(
						content->data.counter.name));
				તોડો;
			સ્થિતિ CSS_COMPUTED_CONTENT_COUNTERS:
				fprintf(stream, "counters(%.*s, \"%.*s\")",
						(પૂર્ણાંક) lwc_string_length(
						content->data.counters.name),
						lwc_string_data(
						content->data.counters.name),
						(પૂર્ણાંક) lwc_string_length(
						content->data.counters.sep),
						lwc_string_data(
						content->data.counters.sep));
				તોડો;
			સ્થિતિ CSS_COMPUTED_CONTENT_ATTR:
				fprintf(stream, "attr(%.*s)",
						(પૂર્ણાંક) lwc_string_length(
						content->data.attr),
						lwc_string_data(
						content->data.attr));
				તોડો;
			સ્થિતિ CSS_COMPUTED_CONTENT_OPEN_QUOTE:
				fprintf(stream, "open-quote");
				તોડો;
			સ્થિતિ CSS_COMPUTED_CONTENT_CLOSE_QUOTE:
				fprintf(stream, "close-quote");
				તોડો;
			સ્થિતિ CSS_COMPUTED_CONTENT_NO_OPEN_QUOTE:
				fprintf(stream, "no-open-quote");
				તોડો;
			સ્થિતિ CSS_COMPUTED_CONTENT_NO_CLOSE_QUOTE:
				fprintf(stream, "no-close-quote");
				તોડો;
			}

			content++;
		}

		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* counter-increment */
	val = css_computed_counter_increment(style, &counter);
	જો ((val == CSS_COUNTER_INCREMENT_NONE) || (counter == ખાલી)) {
		fprintf(stream, "counter-increment: none ");
	} નહીંતર {
		fprintf(stream, "counter-increment:");

		જ્યારે (counter->name != ખાલી) {
			fprintf(stream, " %.*s ",
					(પૂર્ણાંક) lwc_string_length(counter->name),
					lwc_string_data(counter->name));

			dump_css_fixed(stream, counter->કિંમત);

			counter++;
		}

		fprintf(stream, " ");
	}

	/* counter-reset */
	val = css_computed_counter_reset(style, &counter);
	જો ((val == CSS_COUNTER_RESET_NONE) || (counter == ખાલી)) {
		fprintf(stream, "counter-reset: none ");
	} નહીંતર {
		fprintf(stream, "counter-reset:");

		જ્યારે (counter->name != ખાલી) {
			fprintf(stream, " %.*s ",
					(પૂર્ણાંક) lwc_string_length(counter->name),
					lwc_string_data(counter->name));

			dump_css_fixed(stream, counter->કિંમત);

			counter++;
		}

		fprintf(stream, " ");
	}

	/* cursor */
	val = css_computed_cursor(style, &string_list);
	fprintf(stream, "cursor:");

	જો (string_list != ખાલી) {
		જ્યારે (*string_list != ખાલી) {
			fprintf(stream, " url\"%.*s\")",
					(પૂર્ણાંક) lwc_string_length(*string_list),
					lwc_string_data(*string_list));

			string_list++;
		}
	}
	પસંદગી (val) {
	સ્થિતિ CSS_CURSOR_AUTO:
		fprintf(stream, " auto ");
		તોડો;
	સ્થિતિ CSS_CURSOR_CROSSHAIR:
		fprintf(stream, " crosshair ");
		તોડો;
	સ્થિતિ CSS_CURSOR_DEFAULT:
		fprintf(stream, " default ");
		તોડો;
	સ્થિતિ CSS_CURSOR_POINTER:
		fprintf(stream, " pointer ");
		તોડો;
	સ્થિતિ CSS_CURSOR_MOVE:
		fprintf(stream, " move ");
		તોડો;
	સ્થિતિ CSS_CURSOR_E_RESIZE:
		fprintf(stream, " e-resize ");
		તોડો;
	સ્થિતિ CSS_CURSOR_NE_RESIZE:
		fprintf(stream, " ne-resize ");
		તોડો;
	સ્થિતિ CSS_CURSOR_NW_RESIZE:
		fprintf(stream, " nw-resize ");
		તોડો;
	સ્થિતિ CSS_CURSOR_N_RESIZE:
		fprintf(stream, " n-resize ");
		તોડો;
	સ્થિતિ CSS_CURSOR_SE_RESIZE:
		fprintf(stream, " se-resize ");
		તોડો;
	સ્થિતિ CSS_CURSOR_SW_RESIZE:
		fprintf(stream, " sw-resize ");
		તોડો;
	સ્થિતિ CSS_CURSOR_S_RESIZE:
		fprintf(stream, " s-resize ");
		તોડો;
	સ્થિતિ CSS_CURSOR_W_RESIZE:
		fprintf(stream, " w-resize ");
		તોડો;
	સ્થિતિ CSS_CURSOR_TEXT:
		fprintf(stream, " text ");
		તોડો;
	સ્થિતિ CSS_CURSOR_WAIT:
		fprintf(stream, " wait ");
		તોડો;
	સ્થિતિ CSS_CURSOR_HELP:
		fprintf(stream, " help ");
		તોડો;
	સ્થિતિ CSS_CURSOR_PROGRESS:
		fprintf(stream, " progress ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* direction */
	val = css_computed_direction(style);
	પસંદગી (val) {
	સ્થિતિ CSS_DIRECTION_LTR:
		fprintf(stream, "direction: ltr ");
		તોડો;
	સ્થિતિ CSS_DIRECTION_RTL:
		fprintf(stream, "direction: rtl ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* display */
	val = ns_computed_display_static(style);
	પસંદગી (val) {
	સ્થિતિ CSS_DISPLAY_INLINE:
		fprintf(stream, "display: inline ");
		તોડો;
	સ્થિતિ CSS_DISPLAY_BLOCK:
		fprintf(stream, "display: block ");
		તોડો;
	સ્થિતિ CSS_DISPLAY_LIST_ITEM:
		fprintf(stream, "display: list-item ");
		તોડો;
	સ્થિતિ CSS_DISPLAY_RUN_IN:
		fprintf(stream, "display: run-in ");
		તોડો;
	સ્થિતિ CSS_DISPLAY_INLINE_BLOCK:
		fprintf(stream, "display: inline-block ");
		તોડો;
	સ્થિતિ CSS_DISPLAY_TABLE:
		fprintf(stream, "display: table ");
		તોડો;
	સ્થિતિ CSS_DISPLAY_INLINE_TABLE:
		fprintf(stream, "display: inline-table ");
		તોડો;
	સ્થિતિ CSS_DISPLAY_TABLE_ROW_GROUP:
		fprintf(stream, "display: table-row-group ");
		તોડો;
	સ્થિતિ CSS_DISPLAY_TABLE_HEADER_GROUP:
		fprintf(stream, "display: table-header-group ");
		તોડો;
	સ્થિતિ CSS_DISPLAY_TABLE_FOOTER_GROUP:
		fprintf(stream, "display: table-footer-group ");
		તોડો;
	સ્થિતિ CSS_DISPLAY_TABLE_ROW:
		fprintf(stream, "display: table-row ");
		તોડો;
	સ્થિતિ CSS_DISPLAY_TABLE_COLUMN_GROUP:
		fprintf(stream, "display: table-column-group ");
		તોડો;
	સ્થિતિ CSS_DISPLAY_TABLE_COLUMN:
		fprintf(stream, "display: table-column ");
		તોડો;
	સ્થિતિ CSS_DISPLAY_TABLE_CELL:
		fprintf(stream, "display: table-cell ");
		તોડો;
	સ્થિતિ CSS_DISPLAY_TABLE_CAPTION:
		fprintf(stream, "display: table-caption ");
		તોડો;
	સ્થિતિ CSS_DISPLAY_NONE:
		fprintf(stream, "display: none ");
		તોડો;
	સ્થિતિ CSS_DISPLAY_FLEX:
		fprintf(stream, "display: flex ");
		તોડો;
	સ્થિતિ CSS_DISPLAY_INLINE_FLEX:
		fprintf(stream, "display: inline-flex ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* empty-cells */
	val = css_computed_empty_cells(style);
	પસંદગી (val) {
	સ્થિતિ CSS_EMPTY_CELLS_SHOW:
		fprintf(stream, "empty-cells: show ");
		તોડો;
	સ્થિતિ CSS_EMPTY_CELLS_HIDE:
		fprintf(stream, "empty-cells: hide ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* float */
	val = css_computed_float(style);
	પસંદગી (val) {
	સ્થિતિ CSS_FLOAT_ડાબું:
		fprintf(stream, "float: left ");
		તોડો;
	સ્થિતિ CSS_FLOAT_જમણું:
		fprintf(stream, "float: right ");
		તોડો;
	સ્થિતિ CSS_FLOAT_NONE:
		fprintf(stream, "float: none ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* font-family */
	val = css_computed_font_family(style, &string_list);
	જો (val != CSS_FONT_FAMILY_INHERIT) {
		fprintf(stream, "font-family:");

		જો (string_list != ખાલી) {
			જ્યારે (*string_list != ખાલી) {
				fprintf(stream, " \"%.*s\"",
					(પૂર્ણાંક) lwc_string_length(*string_list),
					lwc_string_data(*string_list));

				string_list++;
			}
		}
		પસંદગી (val) {
		સ્થિતિ CSS_FONT_FAMILY_SERIF:
			fprintf(stream, " serif ");
			તોડો;
		સ્થિતિ CSS_FONT_FAMILY_SANS_SERIF:
			fprintf(stream, " sans-serif ");
			તોડો;
		સ્થિતિ CSS_FONT_FAMILY_CURSIVE:
			fprintf(stream, " cursive ");
			તોડો;
		સ્થિતિ CSS_FONT_FAMILY_FANTASY:
			fprintf(stream, " fantasy ");
			તોડો;
		સ્થિતિ CSS_FONT_FAMILY_MONOSPACE:
			fprintf(stream, " monospace ");
			તોડો;
		}
	}

	/* font-size */
	val = css_computed_font_size(style, &len1, &unit1);
	પસંદગી (val) {
	સ્થિતિ CSS_FONT_SIZE_XX_SMALL:
		fprintf(stream, "font-size: xx-small ");
		તોડો;
	સ્થિતિ CSS_FONT_SIZE_X_SMALL:
		fprintf(stream, "font-size: x-small ");
		તોડો;
	સ્થિતિ CSS_FONT_SIZE_SMALL:
		fprintf(stream, "font-size: small ");
		તોડો;
	સ્થિતિ CSS_FONT_SIZE_MEDIUM:
		fprintf(stream, "font-size: medium ");
		તોડો;
	સ્થિતિ CSS_FONT_SIZE_LARGE:
		fprintf(stream, "font-size: large ");
		તોડો;
	સ્થિતિ CSS_FONT_SIZE_X_LARGE:
		fprintf(stream, "font-size: x-large ");
		તોડો;
	સ્થિતિ CSS_FONT_SIZE_XX_LARGE:
		fprintf(stream, "font-size: xx-large ");
		તોડો;
	સ્થિતિ CSS_FONT_SIZE_LARGER:
		fprintf(stream, "font-size: larger ");
		તોડો;
	સ્થિતિ CSS_FONT_SIZE_SMALLER:
		fprintf(stream, "font-size: smaller ");
		તોડો;
	સ્થિતિ CSS_FONT_SIZE_DIMENSION:
		fprintf(stream, "font-size: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* font-style */
	val = css_computed_font_style(style);
	પસંદગી (val) {
	સ્થિતિ CSS_FONT_STYLE_NORMAL:
		fprintf(stream, "font-style: normal ");
		તોડો;
	સ્થિતિ CSS_FONT_STYLE_ITALIC:
		fprintf(stream, "font-style: italic ");
		તોડો;
	સ્થિતિ CSS_FONT_STYLE_OBLIQUE:
		fprintf(stream, "font-style: oblique ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* font-variant */
	val = css_computed_font_variant(style);
	પસંદગી (val) {
	સ્થિતિ CSS_FONT_VARIANT_NORMAL:
		fprintf(stream, "font-variant: normal ");
		તોડો;
	સ્થિતિ CSS_FONT_VARIANT_SMALL_CAPS:
		fprintf(stream, "font-variant: small-caps ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* font-weight */
	val = css_computed_font_weight(style);
	પસંદગી (val) {
	સ્થિતિ CSS_FONT_WEIGHT_NORMAL:
		fprintf(stream, "font-weight: normal ");
		તોડો;
	સ્થિતિ CSS_FONT_WEIGHT_BOLD:
		fprintf(stream, "font-weight: bold ");
		તોડો;
	સ્થિતિ CSS_FONT_WEIGHT_BOLDER:
		fprintf(stream, "font-weight: bolder ");
		તોડો;
	સ્થિતિ CSS_FONT_WEIGHT_LIGHTER:
		fprintf(stream, "font-weight: lighter ");
		તોડો;
	સ્થિતિ CSS_FONT_WEIGHT_100:
		fprintf(stream, "font-weight: 100 ");
		તોડો;
	સ્થિતિ CSS_FONT_WEIGHT_200:
		fprintf(stream, "font-weight: 200 ");
		તોડો;
	સ્થિતિ CSS_FONT_WEIGHT_300:
		fprintf(stream, "font-weight: 300 ");
		તોડો;
	સ્થિતિ CSS_FONT_WEIGHT_400:
		fprintf(stream, "font-weight: 400 ");
		તોડો;
	સ્થિતિ CSS_FONT_WEIGHT_500:
		fprintf(stream, "font-weight: 500 ");
		તોડો;
	સ્થિતિ CSS_FONT_WEIGHT_600:
		fprintf(stream, "font-weight: 600 ");
		તોડો;
	સ્થિતિ CSS_FONT_WEIGHT_700:
		fprintf(stream, "font-weight: 700 ");
		તોડો;
	સ્થિતિ CSS_FONT_WEIGHT_800:
		fprintf(stream, "font-weight: 800 ");
		તોડો;
	સ્થિતિ CSS_FONT_WEIGHT_900:
		fprintf(stream, "font-weight: 900 ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* height */
	val = css_computed_height(style, &len1, &unit1);
	પસંદગી (val) {
	સ્થિતિ CSS_HEIGHT_AUTO:
		fprintf(stream, "height: auto ");
		તોડો;
	સ્થિતિ CSS_HEIGHT_SET:
		fprintf(stream, "height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/*ડાબું*/
	val = css_computed_ડાબું(style, &len1, &unit1);
	પસંદગી (val) {
	સ્થિતિ CSS_ડાબું_AUTO:
		fprintf(stream, "left: auto ");
		તોડો;
	સ્થિતિ CSS_ડાબું_SET:
		fprintf(stream, "left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* letter-spacing */
	val = css_computed_letter_spacing(style, &len1, &unit1);
	પસંદગી (val) {
	સ્થિતિ CSS_LETTER_SPACING_NORMAL:
		fprintf(stream, "letter-spacing: normal ");
		તોડો;
	સ્થિતિ CSS_LETTER_SPACING_SET:
		fprintf(stream, "letter-spacing: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* line-height */
	val = css_computed_લીટી_height(style, &len1, &unit1);
	પસંદગી (val) {
	સ્થિતિ CSS_લીટી_HEIGHT_NORMAL:
		fprintf(stream, "line-height: normal ");
		તોડો;
	સ્થિતિ CSS_લીટી_HEIGHT_સંખ્યા:
		fprintf(stream, "line-height: ");

		dump_css_fixed(stream, len1);

		fprintf(stream, " ");
		તોડો;
	સ્થિતિ CSS_લીટી_HEIGHT_DIMENSION:
		fprintf(stream, "line-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* list-style-image */
	val = css_computed_list_style_image(style, &url);
	જો (url != ખાલી) {
		fprintf(stream, "list-style-image: url('%.*s') ",
				(પૂર્ણાંક) lwc_string_length(url),
				lwc_string_data(url));
	} નહીંતર જો (val == CSS_LIST_STYLE_IMAGE_NONE) {
		fprintf(stream, "list-style-image: none ");
	}

	/* list-style-position */
	val = css_computed_list_style_position(style);
	પસંદગી (val) {
	સ્થિતિ CSS_LIST_STYLE_POSITION_INSIDE:
		fprintf(stream, "list-style-position: inside ");
		તોડો;
	સ્થિતિ CSS_LIST_STYLE_POSITION_OUTSIDE:
		fprintf(stream, "list-style-position: outside ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* list-style-type */
	val = css_computed_list_style_type(style);
	પસંદગી (val) {
	સ્થિતિ CSS_LIST_STYLE_TYPE_DISC:
		fprintf(stream, "list-style-type: disc ");
		તોડો;
	સ્થિતિ CSS_LIST_STYLE_TYPE_CIRCLE:
		fprintf(stream, "list-style-type: circle ");
		તોડો;
	સ્થિતિ CSS_LIST_STYLE_TYPE_SQUARE:
		fprintf(stream, "list-style-type: square ");
		તોડો;
	સ્થિતિ CSS_LIST_STYLE_TYPE_DECIMAL:
		fprintf(stream, "list-style-type: decimal ");
		તોડો;
	સ્થિતિ CSS_LIST_STYLE_TYPE_DECIMAL_LEADING_ZERO:
		fprintf(stream, "list-style-type: decimal-leading-zero ");
		તોડો;
	સ્થિતિ CSS_LIST_STYLE_TYPE_LOWER_ROMAN:
		fprintf(stream, "list-style-type: lower-roman ");
		તોડો;
	સ્થિતિ CSS_LIST_STYLE_TYPE_UPPER_ROMAN:
		fprintf(stream, "list-style-type: upper-roman ");
		તોડો;
	સ્થિતિ CSS_LIST_STYLE_TYPE_LOWER_GREEK:
		fprintf(stream, "list-style-type: lower-greek ");
		તોડો;
	સ્થિતિ CSS_LIST_STYLE_TYPE_LOWER_LATIN:
		fprintf(stream, "list-style-type: lower-latin ");
		તોડો;
	સ્થિતિ CSS_LIST_STYLE_TYPE_UPPER_LATIN:
		fprintf(stream, "list-style-type: upper-latin ");
		તોડો;
	સ્થિતિ CSS_LIST_STYLE_TYPE_ARMENIAN:
		fprintf(stream, "list-style-type: armenian ");
		તોડો;
	સ્થિતિ CSS_LIST_STYLE_TYPE_GEORGIAN:
		fprintf(stream, "list-style-type: georgian ");
		તોડો;
	સ્થિતિ CSS_LIST_STYLE_TYPE_LOWER_ALPHA:
		fprintf(stream, "list-style-type: lower-alpha ");
		તોડો;
	સ્થિતિ CSS_LIST_STYLE_TYPE_UPPER_ALPHA:
		fprintf(stream, "list-style-type: upper-alpha ");
		તોડો;
	સ્થિતિ CSS_LIST_STYLE_TYPE_NONE:
		fprintf(stream, "list-style-type: none ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* margin-top */
	val = css_computed_margin_ટોચ(style, &len1, &unit1);
	પસંદગી (val) {
	સ્થિતિ CSS_MARGIN_AUTO:
		fprintf(stream, "margin-top: auto ");
		તોડો;
	સ્થિતિ CSS_MARGIN_SET:
		fprintf(stream, "margin-top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* margin-right */
	val = css_computed_margin_જમણું(style, &len1, &unit1);
	પસંદગી (val) {
	સ્થિતિ CSS_MARGIN_AUTO:
		fprintf(stream, "margin-right: auto ");
		તોડો;
	સ્થિતિ CSS_MARGIN_SET:
		fprintf(stream, "margin-right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* margin-bottom */
	val = css_computed_margin_bottom(style, &len1, &unit1);
	પસંદગી (val) {
	સ્થિતિ CSS_MARGIN_AUTO:
		fprintf(stream, "margin-bottom: auto ");
		તોડો;
	સ્થિતિ CSS_MARGIN_SET:
		fprintf(stream, "margin-bottom: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* margin-left */
	val = css_computed_margin_ડાબું(style, &len1, &unit1);
	પસંદગી (val) {
	સ્થિતિ CSS_MARGIN_AUTO:
		fprintf(stream, "margin-left: auto ");
		તોડો;
	સ્થિતિ CSS_MARGIN_SET:
		fprintf(stream, "margin-left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* max-height */
	val = css_computed_max_height(style, &len1, &unit1);
	પસંદગી (val) {
	સ્થિતિ CSS_MAX_HEIGHT_NONE:
		fprintf(stream, "max-height: none ");
		તોડો;
	સ્થિતિ CSS_MAX_HEIGHT_SET:
		fprintf(stream, "max-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* max-width */
	val = css_computed_max_width(style, &len1, &unit1);
	પસંદગી (val) {
	સ્થિતિ CSS_MAX_WIDTH_NONE:
		fprintf(stream, "max-width: none ");
		તોડો;
	સ્થિતિ CSS_MAX_WIDTH_SET:
		fprintf(stream, "max-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* min-height */
	val = ns_computed_min_height(style, &len1, &unit1);
	પસંદગી (val) {
	સ્થિતિ CSS_MIN_HEIGHT_SET:
		fprintf(stream, "min-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* min-width */
	val = ns_computed_min_width(style, &len1, &unit1);
	પસંદગી (val) {
	સ્થિતિ CSS_MIN_WIDTH_SET:
		fprintf(stream, "min-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* opacity */
	val = css_computed_opacity(style, &len1);
	પસંદગી (val) {
	સ્થિતિ CSS_OPACITY_SET:
		fprintf(stream, "opacity: ");

		dump_css_fixed(stream, len1);

		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* outline-color */
	val = css_computed_outline_color(style, &color);
	પસંદગી (val) {
	સ્થિતિ CSS_OUTLINE_COLOR_INVERT:
		fprintf(stream, "outline-color: invert ");
		તોડો;
	સ્થિતિ CSS_OUTLINE_COLOR_COLOR:
		fprintf(stream, "outline-color: #%08"PRIx32" ", color);
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* outline-style */
	val = css_computed_outline_style(style);
	પસંદગી (val) {
	સ્થિતિ CSS_OUTLINE_STYLE_NONE:
		fprintf(stream, "outline-style: none ");
		તોડો;
	સ્થિતિ CSS_OUTLINE_STYLE_DOTTED:
		fprintf(stream, "outline-style: dotted ");
		તોડો;
	સ્થિતિ CSS_OUTLINE_STYLE_DASHED:
		fprintf(stream, "outline-style: dashed ");
		તોડો;
	સ્થિતિ CSS_OUTLINE_STYLE_SOLID:
		fprintf(stream, "outline-style: solid ");
		તોડો;
	સ્થિતિ CSS_OUTLINE_STYLE_DOUBLE:
		fprintf(stream, "outline-style: double ");
		તોડો;
	સ્થિતિ CSS_OUTLINE_STYLE_GROOVE:
		fprintf(stream, "outline-style: groove ");
		તોડો;
	સ્થિતિ CSS_OUTLINE_STYLE_RIDGE:
		fprintf(stream, "outline-style: ridge ");
		તોડો;
	સ્થિતિ CSS_OUTLINE_STYLE_INSET:
		fprintf(stream, "outline-style: inset ");
		તોડો;
	સ્થિતિ CSS_OUTLINE_STYLE_OUTSET:
		fprintf(stream, "outline-style: outset ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* outline-width */
	val = css_computed_outline_width(style, &len1, &unit1);
	પસંદગી (val) {
	સ્થિતિ CSS_OUTLINE_WIDTH_THIN:
		fprintf(stream, "outline-width: thin ");
		તોડો;
	સ્થિતિ CSS_OUTLINE_WIDTH_MEDIUM:
		fprintf(stream, "outline-width: medium ");
		તોડો;
	સ્થિતિ CSS_OUTLINE_WIDTH_THICK:
		fprintf(stream, "outline-width: thick ");
		તોડો;
	સ્થિતિ CSS_OUTLINE_WIDTH_WIDTH:
		fprintf(stream, "outline-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* overflow */
	val = css_computed_overflow_x(style);
	પસંદગી (val) {
	સ્થિતિ CSS_OVERFLOW_VISIBLE:
		fprintf(stream, "overflow-x: visible ");
		તોડો;
	સ્થિતિ CSS_OVERFLOW_HIDDEN:
		fprintf(stream, "overflow-x: hidden ");
		તોડો;
	સ્થિતિ CSS_OVERFLOW_SCROLL:
		fprintf(stream, "overflow-x: scroll ");
		તોડો;
	સ્થિતિ CSS_OVERFLOW_AUTO:
		fprintf(stream, "overflow-x auto ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* overflow */
	val = css_computed_overflow_y(style);
	પસંદગી (val) {
	સ્થિતિ CSS_OVERFLOW_VISIBLE:
		fprintf(stream, "overflow-y: visible ");
		તોડો;
	સ્થિતિ CSS_OVERFLOW_HIDDEN:
		fprintf(stream, "overflow-y: hidden ");
		તોડો;
	સ્થિતિ CSS_OVERFLOW_SCROLL:
		fprintf(stream, "overflow-y: scroll ");
		તોડો;
	સ્થિતિ CSS_OVERFLOW_AUTO:
		fprintf(stream, "overflow-y: auto ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* padding-top */
	val = css_computed_padding_ટોચ(style, &len1, &unit1);
	પસંદગી (val) {
	સ્થિતિ CSS_PADDING_SET:
		fprintf(stream, "padding-top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* padding-right */
	val = css_computed_padding_જમણું(style, &len1, &unit1);
	પસંદગી (val) {
	સ્થિતિ CSS_PADDING_SET:
		fprintf(stream, "padding-right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* padding-bottom */
	val = css_computed_padding_bottom(style, &len1, &unit1);
	પસંદગી (val) {
	સ્થિતિ CSS_PADDING_SET:
		fprintf(stream, "padding-bottom: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* padding-left */
	val = css_computed_padding_ડાબું(style, &len1, &unit1);
	પસંદગી (val) {
	સ્થિતિ CSS_PADDING_SET:
		fprintf(stream, "padding-left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* position */
	val = css_computed_position(style);
	પસંદગી (val) {
	સ્થિતિ CSS_POSITION_STATIC:
		fprintf(stream, "position: static ");
		તોડો;
	સ્થિતિ CSS_POSITION_RELATIVE:
		fprintf(stream, "position: relative ");
		તોડો;
	સ્થિતિ CSS_POSITION_ABSOLUTE:
		fprintf(stream, "position: absolute ");
		તોડો;
	સ્થિતિ CSS_POSITION_FIXED:
		fprintf(stream, "position: fixed ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* quotes */
	val = css_computed_quotes(style, &string_list);
	જો (val == CSS_QUOTES_STRING && string_list != ખાલી) {
		fprintf(stream, "quotes:");

		જ્યારે (*string_list != ખાલી) {
			fprintf(stream, " \"%.*s\"",
				(પૂર્ણાંક) lwc_string_length(*string_list),
				lwc_string_data(*string_list));

			string_list++;
		}

		fprintf(stream, " ");
	} નહીંતર {
		પસંદગી (val) {
		સ્થિતિ CSS_QUOTES_NONE:
			fprintf(stream, "quotes: none ");
			તોડો;
		મૂળભૂત:
			તોડો;
		}
	}

	/*જમણું*/
	val = css_computed_જમણું(style, &len1, &unit1);
	પસંદગી (val) {
	સ્થિતિ CSS_જમણું_AUTO:
		fprintf(stream, "right: auto ");
		તોડો;
	સ્થિતિ CSS_જમણું_SET:
		fprintf(stream, "right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* table-layout */
	val = css_computed_table_layout(style);
	પસંદગી (val) {
	સ્થિતિ CSS_TABLE_LAYOUT_AUTO:
		fprintf(stream, "table-layout: auto ");
		તોડો;
	સ્થિતિ CSS_TABLE_LAYOUT_FIXED:
		fprintf(stream, "table-layout: fixed ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* text-align */
	val = css_computed_text_align(style);
	પસંદગી (val) {
	સ્થિતિ CSS_TEXT_ALIGN_ડાબું:
		fprintf(stream, "text-align: left ");
		તોડો;
	સ્થિતિ CSS_TEXT_ALIGN_જમણું:
		fprintf(stream, "text-align: right ");
		તોડો;
	સ્થિતિ CSS_TEXT_ALIGN_CENTER:
		fprintf(stream, "text-align: center ");
		તોડો;
	સ્થિતિ CSS_TEXT_ALIGN_JUSTIFY:
		fprintf(stream, "text-align: justify ");
		તોડો;
	સ્થિતિ CSS_TEXT_ALIGN_DEFAULT:
		fprintf(stream, "text-align: default ");
		તોડો;
	સ્થિતિ CSS_TEXT_ALIGN_LIBCSS_ડાબું:
		fprintf(stream, "text-align: -libcss-left ");
		તોડો;
	સ્થિતિ CSS_TEXT_ALIGN_LIBCSS_CENTER:
		fprintf(stream, "text-align: -libcss-center ");
		તોડો;
	સ્થિતિ CSS_TEXT_ALIGN_LIBCSS_જમણું:
		fprintf(stream, "text-align: -libcss-right ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* text-decoration */
	val = css_computed_text_decoration(style);
	જો (val == CSS_TEXT_DECORATION_NONE) {
		fprintf(stream, "text-decoration: none ");
	} નહીંતર {
		fprintf(stream, "text-decoration:");

		જો (val & CSS_TEXT_DECORATION_BLINK) {
			fprintf(stream, " blink");
		}
		જો (val & CSS_TEXT_DECORATION_લીટી_THROUGH) {
			fprintf(stream, " line-through");
		}
		જો (val & CSS_TEXT_DECORATION_OVERLINE) {
			fprintf(stream, " overline");
		}
		જો (val & CSS_TEXT_DECORATION_UNDERLINE) {
			fprintf(stream, " underline");
		}

		fprintf(stream, " ");
	}

	/* text-indent */
	val = css_computed_text_indent(style, &len1, &unit1);
	પસંદગી (val) {
	સ્થિતિ CSS_TEXT_INDENT_SET:
		fprintf(stream, "text-indent: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* text-transform */
	val = css_computed_text_transform(style);
	પસંદગી (val) {
	સ્થિતિ CSS_TEXT_TRANSFORM_CAPITALIZE:
		fprintf(stream, "text-transform: capitalize ");
		તોડો;
	સ્થિતિ CSS_TEXT_TRANSFORM_UPPERCASE:
		fprintf(stream, "text-transform: uppercase ");
		તોડો;
	સ્થિતિ CSS_TEXT_TRANSFORM_LOWERCASE:
		fprintf(stream, "text-transform: lowercase ");
		તોડો;
	સ્થિતિ CSS_TEXT_TRANSFORM_NONE:
		fprintf(stream, "text-transform: none ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/*ટોચ*/
	val = css_computed_ટોચ(style, &len1, &unit1);
	પસંદગી (val) {
	સ્થિતિ CSS_ટોચ_AUTO:
		fprintf(stream, "top: auto ");
		તોડો;
	સ્થિતિ CSS_ટોચ_SET:
		fprintf(stream, "top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* unicode-bidi */
	val = css_computed_unicode_bidi(style);
	પસંદગી (val) {
	સ્થિતિ CSS_UNICODE_BIDI_NORMAL:
		fprintf(stream, "unicode-bidi: normal ");
		તોડો;
	સ્થિતિ CSS_UNICODE_BIDI_EMBED:
		fprintf(stream, "unicode-bidi: embed ");
		તોડો;
	સ્થિતિ CSS_UNICODE_BIDI_BIDI_OVERRIDE:
		fprintf(stream, "unicode-bidi: bidi-override ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* vertical-align */
	val = css_computed_vertical_align(style, &len1, &unit1);
	પસંદગી (val) {
	સ્થિતિ CSS_VERTICAL_ALIGN_BASELINE:
		fprintf(stream, "vertical-align: baseline ");
		તોડો;
	સ્થિતિ CSS_VERTICAL_ALIGN_SUB:
		fprintf(stream, "vertical-align: sub ");
		તોડો;
	સ્થિતિ CSS_VERTICAL_ALIGN_SUPER:
		fprintf(stream, "vertical-align: super ");
		તોડો;
	સ્થિતિ CSS_VERTICAL_ALIGN_ટોચ:
		fprintf(stream, "vertical-align: top ");
		તોડો;
	સ્થિતિ CSS_VERTICAL_ALIGN_TEXT_ટોચ:
		fprintf(stream, "vertical-align: text-top ");
		તોડો;
	સ્થિતિ CSS_VERTICAL_ALIGN_મધ્ય:
		fprintf(stream, "vertical-align: middle ");
		તોડો;
	સ્થિતિ CSS_VERTICAL_ALIGN_BOTTOM:
		fprintf(stream, "vertical-align: bottom ");
		તોડો;
	સ્થિતિ CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
		fprintf(stream, "vertical-align: text-bottom ");
		તોડો;
	સ્થિતિ CSS_VERTICAL_ALIGN_SET:
		fprintf(stream, "vertical-align: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* visibility */
	val = css_computed_visibility(style);
	પસંદગી (val) {
	સ્થિતિ CSS_VISIBILITY_VISIBLE:
		fprintf(stream, "visibility: visible ");
		તોડો;
	સ્થિતિ CSS_VISIBILITY_HIDDEN:
		fprintf(stream, "visibility: hidden ");
		તોડો;
	સ્થિતિ CSS_VISIBILITY_COLLAPSE:
		fprintf(stream, "visibility: collapse ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* white-space */
	val = css_computed_white_space(style);
	પસંદગી (val) {
	સ્થિતિ CSS_WHITE_SPACE_NORMAL:
		fprintf(stream, "white-space: normal ");
		તોડો;
	સ્થિતિ CSS_WHITE_SPACE_PRE:
		fprintf(stream, "white-space: pre ");
		તોડો;
	સ્થિતિ CSS_WHITE_SPACE_NOWRAP:
		fprintf(stream, "white-space: nowrap ");
		તોડો;
	સ્થિતિ CSS_WHITE_SPACE_PRE_WRAP:
		fprintf(stream, "white-space: pre-wrap ");
		તોડો;
	સ્થિતિ CSS_WHITE_SPACE_PRE_લીટી:
		fprintf(stream, "white-space: pre-line ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* width */
	val = css_computed_width(style, &len1, &unit1);
	પસંદગી (val) {
	સ્થિતિ CSS_WIDTH_AUTO:
		fprintf(stream, "width: auto ");
		તોડો;
	સ્થિતિ CSS_WIDTH_SET:
		fprintf(stream, "width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* word-spacing */
	val = css_computed_word_spacing(style, &len1, &unit1);
	પસંદગી (val) {
	સ્થિતિ CSS_WORD_SPACING_NORMAL:
		fprintf(stream, "word-spacing: normal ");
		તોડો;
	સ્થિતિ CSS_WORD_SPACING_SET:
		fprintf(stream, "word-spacing: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* z-index */
	val = css_computed_z_index(style, &zindex);
	પસંદગી (val) {
	સ્થિતિ CSS_Z_INDEX_AUTO:
		fprintf(stream, "z-index: auto ");
		તોડો;
	સ્થિતિ CSS_Z_INDEX_SET:
		fprintf(stream, "z-index: %"PRId32" ", zindex);
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	fprintf(stream, "}");
}
