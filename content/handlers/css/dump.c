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

#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <libcss/libcss.h>

#அந்தர்பாவ "netsurf/inttypes.h"

#அந்தர்பாவ "css/dump.h"
#அந்தர்பாவ "css/utils.h"

/**
 * Dump a fixed point value to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param f       Value to write
 */
நிலையான வெறுமை dump_css_fixed(FILE *stream, css_fixed f)
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
நிலையான வெறுமை dump_css_எண்(FILE *stream, css_fixed val)
{
	எனில் (INTTOFIX(FIXTOINT(val)) == val)
		fprintf(stream, "%"PRId32, FIXTOINT(val));
	இல்லையெனில்
		dump_css_fixed(stream, val);
}

/**
 * Dump a dimension to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 * \param unit    Unit to write
 */
நிலையான வெறுமை dump_css_unit(FILE *stream, css_fixed val, css_unit unit)
{
	dump_css_எண்(stream, val);

	தேர்வு (unit) {
	நிலை CSS_UNIT_PX:
		fprintf(stream, "px");
		நிறுத்து;
	நிலை CSS_UNIT_EX:
		fprintf(stream, "ex");
		நிறுத்து;
	நிலை CSS_UNIT_EM:
		fprintf(stream, "em");
		நிறுத்து;
	நிலை CSS_UNIT_IN:
		fprintf(stream, "in");
		நிறுத்து;
	நிலை CSS_UNIT_CM:
		fprintf(stream, "cm");
		நிறுத்து;
	நிலை CSS_UNIT_MM:
		fprintf(stream, "mm");
		நிறுத்து;
	நிலை CSS_UNIT_PT:
		fprintf(stream, "pt");
		நிறுத்து;
	நிலை CSS_UNIT_PC:
		fprintf(stream, "pc");
		நிறுத்து;
	நிலை CSS_UNIT_PCT:
		fprintf(stream, "%%");
		நிறுத்து;
	நிலை CSS_UNIT_DEG:
		fprintf(stream, "deg");
		நிறுத்து;
	நிலை CSS_UNIT_GRAD:
		fprintf(stream, "grad");
		நிறுத்து;
	நிலை CSS_UNIT_RAD:
		fprintf(stream, "rad");
		நிறுத்து;
	நிலை CSS_UNIT_MS:
		fprintf(stream, "ms");
		நிறுத்து;
	நிலை CSS_UNIT_S:
		fprintf(stream, "s");
		நிறுத்து;
	நிலை CSS_UNIT_HZ:
		fprintf(stream, "Hz");
		நிறுத்து;
	நிலை CSS_UNIT_KHZ:
		fprintf(stream, "kHz");
		நிறுத்து;
	நிலை CSS_UNIT_CH:
		fprintf(stream, "ch");
		நிறுத்து;
	நிலை CSS_UNIT_REM:
		fprintf(stream, "rem");
		நிறுத்து;
	நிலை CSS_UNIT_LH:
		fprintf(stream, "lh");
		நிறுத்து;
	நிலை CSS_UNIT_VH:
		fprintf(stream, "vh");
		நிறுத்து;
	நிலை CSS_UNIT_VW:
		fprintf(stream, "vw");
		நிறுத்து;
	நிலை CSS_UNIT_VI:
		fprintf(stream, "vi");
		நிறுத்து;
	நிலை CSS_UNIT_VB:
		fprintf(stream, "vb");
		நிறுத்து;
	நிலை CSS_UNIT_VMIN:
		fprintf(stream, "vmin");
		நிறுத்து;
	நிலை CSS_UNIT_VMAX:
		fprintf(stream, "vmax");
		நிறுத்து;
	நிலை CSS_UNIT_Q:
		fprintf(stream, "q");
		நிறுத்து;
	நிலை CSS_UNIT_CALC:
		fprintf(stream, "calc()");
		நிறுத்து;
	}
}

/* exported interface documented in content/handlers/css/dump.h */
வெறுமை nscss_dump_computed_style(FILE *stream, மாறிலி css_computed_style *style)
{
	uint8_t val;
	css_color color = 0;
	lwc_string *url = வெற்று;
	css_fixed len1 = 0, len2 = 0;
	css_unit unit1 = CSS_UNIT_PX, unit2 = CSS_UNIT_PX;
	css_computed_clip_rect rect = { 0, 0, 0, 0, CSS_UNIT_PX, CSS_UNIT_PX,
					CSS_UNIT_PX, CSS_UNIT_PX, true, true,
					true, true };
	மாறிலி css_computed_content_item *content = வெற்று;
	மாறிலி css_computed_counter *counter = வெற்று;
	lwc_string **string_list = வெற்று;
	int32_t zindex = 0;

	fprintf(stream, "{ ");

	/* background-attachment */
	val = css_computed_background_attachment(style);
	தேர்வு (val) {
	நிலை CSS_BACKGROUND_ATTACHMENT_FIXED:
		fprintf(stream, "background-attachment: fixed ");
		நிறுத்து;
	நிலை CSS_BACKGROUND_ATTACHMENT_SCROLL:
		fprintf(stream, "background-attachment: scroll ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* background-color */
	val = css_computed_background_color(style, &color);
	தேர்வு (val) {
	நிலை CSS_BACKGROUND_COLOR_COLOR:
		fprintf(stream, "background-color: #%08"PRIx32" ", color);
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* background-image */
	val = css_computed_background_image(style, &url);
	எனில் (val == CSS_BACKGROUND_IMAGE_IMAGE && url != வெற்று) {
		fprintf(stream, "background-image: url('%.*s') ",
				(முழு) lwc_string_length(url),
				lwc_string_data(url));
	} இல்லையெனில் எனில் (val == CSS_BACKGROUND_IMAGE_NONE) {
		fprintf(stream, "background-image: none ");
	}

	/* background-position */
	val = css_computed_background_position(style, &len1, &unit1,
			&len2, &unit2);
	எனில் (val == CSS_BACKGROUND_POSITION_SET) {
		fprintf(stream, "background-position: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		dump_css_unit(stream, len2, unit2);
		fprintf(stream, " ");
	}

	/* background-repeat */
	val = css_computed_background_repeat(style);
	தேர்வு (val) {
	நிலை CSS_BACKGROUND_REPEAT_REPEAT_X:
		fprintf(stream, "background-repeat: repeat-x ");
		நிறுத்து;
	நிலை CSS_BACKGROUND_REPEAT_REPEAT_Y:
		fprintf(stream, "background-repeat: repeat-y ");
		நிறுத்து;
	நிலை CSS_BACKGROUND_REPEAT_REPEAT:
		fprintf(stream, "background-repeat: repeat ");
		நிறுத்து;
	நிலை CSS_BACKGROUND_REPEAT_NO_REPEAT:
		fprintf(stream, "background-repeat: no-repeat ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* border-collapse */
	val = css_computed_border_collapse(style);
	தேர்வு (val) {
	நிலை CSS_BORDER_COLLAPSE_SEPARATE:
		fprintf(stream, "border-collapse: separate ");
		நிறுத்து;
	நிலை CSS_BORDER_COLLAPSE_COLLAPSE:
		fprintf(stream, "border-collapse: collapse ");
		நிறுத்து;
	இயல்பு:

		நிறுத்து;
	}

	/* border-spacing */
	val = css_computed_border_spacing(style, &len1, &unit1, &len2, &unit2);
	எனில் (val == CSS_BORDER_SPACING_SET) {
		fprintf(stream, "border-spacing: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		dump_css_unit(stream, len2, unit2);
		fprintf(stream, " ");
	}

	/* border-top-color */
	val = css_computed_border_உச்சி_color(style, &color);
	தேர்வு (val) {
	நிலை CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-top-color: #%08"PRIx32" ", color);
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* border-right-color */
	val = css_computed_border_வலது_color(style, &color);
	தேர்வு (val) {
	நிலை CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-right-color: #%08"PRIx32" ", color);
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* border-bottom-color */
	val = css_computed_border_bottom_color(style, &color);
	தேர்வு (val) {
	நிலை CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-bottom-color: #%08"PRIx32" ", color);
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* border-left-color */
	val = css_computed_border_இடது_color(style, &color);
	தேர்வு (val) {
	நிலை CSS_BORDER_COLOR_COLOR:
		fprintf(stream, "border-left-color: #%08"PRIx32" ", color);
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* border-top-style */
	val = css_computed_border_உச்சி_style(style);
	தேர்வு (val) {
	நிலை CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-top-style: none ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-top-style: hidden ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-top-style: dotted ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-top-style: dashed ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-top-style: solid ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-top-style: double ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-top-style: groove ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-top-style: ridge ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-top-style: inset ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-top-style: outset ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* border-right-style */
	val = css_computed_border_வலது_style(style);
	தேர்வு (val) {
	நிலை CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-right-style: none ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-right-style: hidden ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-right-style: dotted ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-right-style: dashed ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-right-style: solid ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-right-style: double ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-right-style: groove ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-right-style: ridge ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-right-style: inset ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-right-style: outset ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* border-bottom-style */
	val = css_computed_border_bottom_style(style);
	தேர்வு (val) {
	நிலை CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-bottom-style: none ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-bottom-style: hidden ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-bottom-style: dotted ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-bottom-style: dashed ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-bottom-style: solid ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-bottom-style: double ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-bottom-style: groove ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-bottom-style: ridge ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-bottom-style: inset ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-bottom-style: outset ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* border-left-style */
	val = css_computed_border_இடது_style(style);
	தேர்வு (val) {
	நிலை CSS_BORDER_STYLE_NONE:
		fprintf(stream, "border-left-style: none ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_HIDDEN:
		fprintf(stream, "border-left-style: hidden ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_DOTTED:
		fprintf(stream, "border-left-style: dotted ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_DASHED:
		fprintf(stream, "border-left-style: dashed ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_SOLID:
		fprintf(stream, "border-left-style: solid ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_DOUBLE:
		fprintf(stream, "border-left-style: double ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_GROOVE:
		fprintf(stream, "border-left-style: groove ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_RIDGE:
		fprintf(stream, "border-left-style: ridge ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_INSET:
		fprintf(stream, "border-left-style: inset ");
		நிறுத்து;
	நிலை CSS_BORDER_STYLE_OUTSET:
		fprintf(stream, "border-left-style: outset ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* border-top-width */
	val = css_computed_border_உச்சி_width(style, &len1, &unit1);
	தேர்வு (val) {
	நிலை CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-top-width: thin ");
		நிறுத்து;
	நிலை CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-top-width: medium ");
		நிறுத்து;
	நிலை CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-top-width: thick ");
		நிறுத்து;
	நிலை CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-top-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* border-right-width */
	val = css_computed_border_வலது_width(style, &len1, &unit1);
	தேர்வு (val) {
	நிலை CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-right-width: thin ");
		நிறுத்து;
	நிலை CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-right-width: medium ");
		நிறுத்து;
	நிலை CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-right-width: thick ");
		நிறுத்து;
	நிலை CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-right-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* border-bottom-width */
	val = css_computed_border_bottom_width(style, &len1, &unit1);
	தேர்வு (val) {
	நிலை CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-bottom-width: thin ");
		நிறுத்து;
	நிலை CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-bottom-width: medium ");
		நிறுத்து;
	நிலை CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-bottom-width: thick ");
		நிறுத்து;
	நிலை CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-bottom-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* border-left-width */
	val = css_computed_border_இடது_width(style, &len1, &unit1);
	தேர்வு (val) {
	நிலை CSS_BORDER_WIDTH_THIN:
		fprintf(stream, "border-left-width: thin ");
		நிறுத்து;
	நிலை CSS_BORDER_WIDTH_MEDIUM:
		fprintf(stream, "border-left-width: medium ");
		நிறுத்து;
	நிலை CSS_BORDER_WIDTH_THICK:
		fprintf(stream, "border-left-width: thick ");
		நிறுத்து;
	நிலை CSS_BORDER_WIDTH_WIDTH:
		fprintf(stream, "border-left-width: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* bottom */
	val = css_computed_bottom(style, &len1, &unit1);
	தேர்வு (val) {
	நிலை CSS_BOTTOM_AUTO:
		fprintf(stream, "bottom: auto ");
		நிறுத்து;
	நிலை CSS_BOTTOM_SET:
		fprintf(stream, "bottom: ");
		dump_css_unit(stream, len1, unit1);
		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* caption-side */
	val = css_computed_caption_side(style);
	தேர்வு (val) {
	நிலை CSS_CAPTION_SIDE_உச்சி:
		fprintf(stream, "caption_side: top ");
		நிறுத்து;
	நிலை CSS_CAPTION_SIDE_BOTTOM:
		fprintf(stream, "caption_side: bottom ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* clear */
	val = css_computed_clear(style);
	தேர்வு (val) {
	நிலை CSS_CLEAR_NONE:
		fprintf(stream, "clear: none ");
		நிறுத்து;
	நிலை CSS_CLEAR_இடது:
		fprintf(stream, "clear: left ");
		நிறுத்து;
	நிலை CSS_CLEAR_வலது:
		fprintf(stream, "clear: right ");
		நிறுத்து;
	நிலை CSS_CLEAR_BOTH:
		fprintf(stream, "clear: both ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* clip */
	val = css_computed_clip(style, &rect);
	தேர்வு (val) {
	நிலை CSS_CLIP_AUTO:
		fprintf(stream, "clip: auto ");
		நிறுத்து;
	நிலை CSS_CLIP_RECT:
		fprintf(stream, "clip: rect( ");

		எனில் (rect.உச்சி_auto)
			fprintf(stream, "auto");
		இல்லையெனில்
			dump_css_unit(stream, rect.உச்சி, rect.tunit);
		fprintf(stream, ", ");

		எனில் (rect.வலது_auto)
			fprintf(stream, "auto");
		இல்லையெனில்
			dump_css_unit(stream, rect.வலது, rect.runit);
		fprintf(stream, ", ");

		எனில் (rect.bottom_auto)
			fprintf(stream, "auto");
		இல்லையெனில்
			dump_css_unit(stream, rect.bottom, rect.bunit);
		fprintf(stream, ", ");

		எனில் (rect.இடது_auto)
			fprintf(stream, "auto");
		இல்லையெனில்
			dump_css_unit(stream, rect.இடது, rect.lunit);
		fprintf(stream, ") ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* color */
	val = css_computed_color(style, &color);
	எனில் (val == CSS_COLOR_COLOR) {
		fprintf(stream, "color: #%08"PRIx32" ", color);
	}

	/* content */
	val = css_computed_content(style, &content);
	தேர்வு (val) {
	நிலை CSS_CONTENT_NONE:
		fprintf(stream, "content: none ");
		நிறுத்து;
	நிலை CSS_CONTENT_NORMAL:
		fprintf(stream, "content: normal ");
		நிறுத்து;
	நிலை CSS_CONTENT_SET:
		fprintf(stream, "content:");

		போது (content->type != CSS_COMPUTED_CONTENT_NONE) {
			fprintf(stream, " ");

			தேர்வு (content->type) {
			நிலை CSS_COMPUTED_CONTENT_STRING:
				fprintf(stream,	"\"%.*s\"",
						(முழு) lwc_string_length(
						content->data.string),
						lwc_string_data(
						content->data.string));
				நிறுத்து;
			நிலை CSS_COMPUTED_CONTENT_URI:
				fprintf(stream,	"uri(\"%.*s\")",
						(முழு) lwc_string_length(
						content->data.uri),
						lwc_string_data(
						content->data.uri));
				நிறுத்து;
			நிலை CSS_COMPUTED_CONTENT_COUNTER:
				fprintf(stream, "counter(%.*s)",
						(முழு) lwc_string_length(
						content->data.counter.name),
						lwc_string_data(
						content->data.counter.name));
				நிறுத்து;
			நிலை CSS_COMPUTED_CONTENT_COUNTERS:
				fprintf(stream, "counters(%.*s, \"%.*s\")",
						(முழு) lwc_string_length(
						content->data.counters.name),
						lwc_string_data(
						content->data.counters.name),
						(முழு) lwc_string_length(
						content->data.counters.sep),
						lwc_string_data(
						content->data.counters.sep));
				நிறுத்து;
			நிலை CSS_COMPUTED_CONTENT_ATTR:
				fprintf(stream, "attr(%.*s)",
						(முழு) lwc_string_length(
						content->data.attr),
						lwc_string_data(
						content->data.attr));
				நிறுத்து;
			நிலை CSS_COMPUTED_CONTENT_OPEN_QUOTE:
				fprintf(stream, "open-quote");
				நிறுத்து;
			நிலை CSS_COMPUTED_CONTENT_CLOSE_QUOTE:
				fprintf(stream, "close-quote");
				நிறுத்து;
			நிலை CSS_COMPUTED_CONTENT_NO_OPEN_QUOTE:
				fprintf(stream, "no-open-quote");
				நிறுத்து;
			நிலை CSS_COMPUTED_CONTENT_NO_CLOSE_QUOTE:
				fprintf(stream, "no-close-quote");
				நிறுத்து;
			}

			content++;
		}

		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* counter-increment */
	val = css_computed_counter_increment(style, &counter);
	எனில் ((val == CSS_COUNTER_INCREMENT_NONE) || (counter == வெற்று)) {
		fprintf(stream, "counter-increment: none ");
	} இல்லையெனில் {
		fprintf(stream, "counter-increment:");

		போது (counter->name != வெற்று) {
			fprintf(stream, " %.*s ",
					(முழு) lwc_string_length(counter->name),
					lwc_string_data(counter->name));

			dump_css_fixed(stream, counter->மதிப்பு);

			counter++;
		}

		fprintf(stream, " ");
	}

	/* counter-reset */
	val = css_computed_counter_reset(style, &counter);
	எனில் ((val == CSS_COUNTER_RESET_NONE) || (counter == வெற்று)) {
		fprintf(stream, "counter-reset: none ");
	} இல்லையெனில் {
		fprintf(stream, "counter-reset:");

		போது (counter->name != வெற்று) {
			fprintf(stream, " %.*s ",
					(முழு) lwc_string_length(counter->name),
					lwc_string_data(counter->name));

			dump_css_fixed(stream, counter->மதிப்பு);

			counter++;
		}

		fprintf(stream, " ");
	}

	/* cursor */
	val = css_computed_cursor(style, &string_list);
	fprintf(stream, "cursor:");

	எனில் (string_list != வெற்று) {
		போது (*string_list != வெற்று) {
			fprintf(stream, " url\"%.*s\")",
					(முழு) lwc_string_length(*string_list),
					lwc_string_data(*string_list));

			string_list++;
		}
	}
	தேர்வு (val) {
	நிலை CSS_CURSOR_AUTO:
		fprintf(stream, " auto ");
		நிறுத்து;
	நிலை CSS_CURSOR_CROSSHAIR:
		fprintf(stream, " crosshair ");
		நிறுத்து;
	நிலை CSS_CURSOR_DEFAULT:
		fprintf(stream, " default ");
		நிறுத்து;
	நிலை CSS_CURSOR_POINTER:
		fprintf(stream, " pointer ");
		நிறுத்து;
	நிலை CSS_CURSOR_MOVE:
		fprintf(stream, " move ");
		நிறுத்து;
	நிலை CSS_CURSOR_E_RESIZE:
		fprintf(stream, " e-resize ");
		நிறுத்து;
	நிலை CSS_CURSOR_NE_RESIZE:
		fprintf(stream, " ne-resize ");
		நிறுத்து;
	நிலை CSS_CURSOR_NW_RESIZE:
		fprintf(stream, " nw-resize ");
		நிறுத்து;
	நிலை CSS_CURSOR_N_RESIZE:
		fprintf(stream, " n-resize ");
		நிறுத்து;
	நிலை CSS_CURSOR_SE_RESIZE:
		fprintf(stream, " se-resize ");
		நிறுத்து;
	நிலை CSS_CURSOR_SW_RESIZE:
		fprintf(stream, " sw-resize ");
		நிறுத்து;
	நிலை CSS_CURSOR_S_RESIZE:
		fprintf(stream, " s-resize ");
		நிறுத்து;
	நிலை CSS_CURSOR_W_RESIZE:
		fprintf(stream, " w-resize ");
		நிறுத்து;
	நிலை CSS_CURSOR_TEXT:
		fprintf(stream, " text ");
		நிறுத்து;
	நிலை CSS_CURSOR_WAIT:
		fprintf(stream, " wait ");
		நிறுத்து;
	நிலை CSS_CURSOR_HELP:
		fprintf(stream, " help ");
		நிறுத்து;
	நிலை CSS_CURSOR_PROGRESS:
		fprintf(stream, " progress ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* direction */
	val = css_computed_direction(style);
	தேர்வு (val) {
	நிலை CSS_DIRECTION_LTR:
		fprintf(stream, "direction: ltr ");
		நிறுத்து;
	நிலை CSS_DIRECTION_RTL:
		fprintf(stream, "direction: rtl ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* display */
	val = ns_computed_display_static(style);
	தேர்வு (val) {
	நிலை CSS_DISPLAY_INLINE:
		fprintf(stream, "display: inline ");
		நிறுத்து;
	நிலை CSS_DISPLAY_BLOCK:
		fprintf(stream, "display: block ");
		நிறுத்து;
	நிலை CSS_DISPLAY_LIST_ITEM:
		fprintf(stream, "display: list-item ");
		நிறுத்து;
	நிலை CSS_DISPLAY_RUN_IN:
		fprintf(stream, "display: run-in ");
		நிறுத்து;
	நிலை CSS_DISPLAY_INLINE_BLOCK:
		fprintf(stream, "display: inline-block ");
		நிறுத்து;
	நிலை CSS_DISPLAY_TABLE:
		fprintf(stream, "display: table ");
		நிறுத்து;
	நிலை CSS_DISPLAY_INLINE_TABLE:
		fprintf(stream, "display: inline-table ");
		நிறுத்து;
	நிலை CSS_DISPLAY_TABLE_ROW_GROUP:
		fprintf(stream, "display: table-row-group ");
		நிறுத்து;
	நிலை CSS_DISPLAY_TABLE_HEADER_GROUP:
		fprintf(stream, "display: table-header-group ");
		நிறுத்து;
	நிலை CSS_DISPLAY_TABLE_FOOTER_GROUP:
		fprintf(stream, "display: table-footer-group ");
		நிறுத்து;
	நிலை CSS_DISPLAY_TABLE_ROW:
		fprintf(stream, "display: table-row ");
		நிறுத்து;
	நிலை CSS_DISPLAY_TABLE_COLUMN_GROUP:
		fprintf(stream, "display: table-column-group ");
		நிறுத்து;
	நிலை CSS_DISPLAY_TABLE_COLUMN:
		fprintf(stream, "display: table-column ");
		நிறுத்து;
	நிலை CSS_DISPLAY_TABLE_CELL:
		fprintf(stream, "display: table-cell ");
		நிறுத்து;
	நிலை CSS_DISPLAY_TABLE_CAPTION:
		fprintf(stream, "display: table-caption ");
		நிறுத்து;
	நிலை CSS_DISPLAY_NONE:
		fprintf(stream, "display: none ");
		நிறுத்து;
	நிலை CSS_DISPLAY_FLEX:
		fprintf(stream, "display: flex ");
		நிறுத்து;
	நிலை CSS_DISPLAY_INLINE_FLEX:
		fprintf(stream, "display: inline-flex ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* empty-cells */
	val = css_computed_empty_cells(style);
	தேர்வு (val) {
	நிலை CSS_EMPTY_CELLS_SHOW:
		fprintf(stream, "empty-cells: show ");
		நிறுத்து;
	நிலை CSS_EMPTY_CELLS_HIDE:
		fprintf(stream, "empty-cells: hide ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* float */
	val = css_computed_float(style);
	தேர்வு (val) {
	நிலை CSS_FLOAT_இடது:
		fprintf(stream, "float: left ");
		நிறுத்து;
	நிலை CSS_FLOAT_வலது:
		fprintf(stream, "float: right ");
		நிறுத்து;
	நிலை CSS_FLOAT_NONE:
		fprintf(stream, "float: none ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* font-family */
	val = css_computed_font_family(style, &string_list);
	எனில் (val != CSS_FONT_FAMILY_INHERIT) {
		fprintf(stream, "font-family:");

		எனில் (string_list != வெற்று) {
			போது (*string_list != வெற்று) {
				fprintf(stream, " \"%.*s\"",
					(முழு) lwc_string_length(*string_list),
					lwc_string_data(*string_list));

				string_list++;
			}
		}
		தேர்வு (val) {
		நிலை CSS_FONT_FAMILY_SERIF:
			fprintf(stream, " serif ");
			நிறுத்து;
		நிலை CSS_FONT_FAMILY_SANS_SERIF:
			fprintf(stream, " sans-serif ");
			நிறுத்து;
		நிலை CSS_FONT_FAMILY_CURSIVE:
			fprintf(stream, " cursive ");
			நிறுத்து;
		நிலை CSS_FONT_FAMILY_FANTASY:
			fprintf(stream, " fantasy ");
			நிறுத்து;
		நிலை CSS_FONT_FAMILY_MONOSPACE:
			fprintf(stream, " monospace ");
			நிறுத்து;
		}
	}

	/* font-size */
	val = css_computed_font_size(style, &len1, &unit1);
	தேர்வு (val) {
	நிலை CSS_FONT_SIZE_XX_SMALL:
		fprintf(stream, "font-size: xx-small ");
		நிறுத்து;
	நிலை CSS_FONT_SIZE_X_SMALL:
		fprintf(stream, "font-size: x-small ");
		நிறுத்து;
	நிலை CSS_FONT_SIZE_SMALL:
		fprintf(stream, "font-size: small ");
		நிறுத்து;
	நிலை CSS_FONT_SIZE_MEDIUM:
		fprintf(stream, "font-size: medium ");
		நிறுத்து;
	நிலை CSS_FONT_SIZE_LARGE:
		fprintf(stream, "font-size: large ");
		நிறுத்து;
	நிலை CSS_FONT_SIZE_X_LARGE:
		fprintf(stream, "font-size: x-large ");
		நிறுத்து;
	நிலை CSS_FONT_SIZE_XX_LARGE:
		fprintf(stream, "font-size: xx-large ");
		நிறுத்து;
	நிலை CSS_FONT_SIZE_LARGER:
		fprintf(stream, "font-size: larger ");
		நிறுத்து;
	நிலை CSS_FONT_SIZE_SMALLER:
		fprintf(stream, "font-size: smaller ");
		நிறுத்து;
	நிலை CSS_FONT_SIZE_DIMENSION:
		fprintf(stream, "font-size: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* font-style */
	val = css_computed_font_style(style);
	தேர்வு (val) {
	நிலை CSS_FONT_STYLE_NORMAL:
		fprintf(stream, "font-style: normal ");
		நிறுத்து;
	நிலை CSS_FONT_STYLE_ITALIC:
		fprintf(stream, "font-style: italic ");
		நிறுத்து;
	நிலை CSS_FONT_STYLE_OBLIQUE:
		fprintf(stream, "font-style: oblique ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* font-variant */
	val = css_computed_font_variant(style);
	தேர்வு (val) {
	நிலை CSS_FONT_VARIANT_NORMAL:
		fprintf(stream, "font-variant: normal ");
		நிறுத்து;
	நிலை CSS_FONT_VARIANT_SMALL_CAPS:
		fprintf(stream, "font-variant: small-caps ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* font-weight */
	val = css_computed_font_weight(style);
	தேர்வு (val) {
	நிலை CSS_FONT_WEIGHT_NORMAL:
		fprintf(stream, "font-weight: normal ");
		நிறுத்து;
	நிலை CSS_FONT_WEIGHT_BOLD:
		fprintf(stream, "font-weight: bold ");
		நிறுத்து;
	நிலை CSS_FONT_WEIGHT_BOLDER:
		fprintf(stream, "font-weight: bolder ");
		நிறுத்து;
	நிலை CSS_FONT_WEIGHT_LIGHTER:
		fprintf(stream, "font-weight: lighter ");
		நிறுத்து;
	நிலை CSS_FONT_WEIGHT_100:
		fprintf(stream, "font-weight: 100 ");
		நிறுத்து;
	நிலை CSS_FONT_WEIGHT_200:
		fprintf(stream, "font-weight: 200 ");
		நிறுத்து;
	நிலை CSS_FONT_WEIGHT_300:
		fprintf(stream, "font-weight: 300 ");
		நிறுத்து;
	நிலை CSS_FONT_WEIGHT_400:
		fprintf(stream, "font-weight: 400 ");
		நிறுத்து;
	நிலை CSS_FONT_WEIGHT_500:
		fprintf(stream, "font-weight: 500 ");
		நிறுத்து;
	நிலை CSS_FONT_WEIGHT_600:
		fprintf(stream, "font-weight: 600 ");
		நிறுத்து;
	நிலை CSS_FONT_WEIGHT_700:
		fprintf(stream, "font-weight: 700 ");
		நிறுத்து;
	நிலை CSS_FONT_WEIGHT_800:
		fprintf(stream, "font-weight: 800 ");
		நிறுத்து;
	நிலை CSS_FONT_WEIGHT_900:
		fprintf(stream, "font-weight: 900 ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* height */
	val = css_computed_height(style, &len1, &unit1);
	தேர்வு (val) {
	நிலை CSS_HEIGHT_AUTO:
		fprintf(stream, "height: auto ");
		நிறுத்து;
	நிலை CSS_HEIGHT_SET:
		fprintf(stream, "height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/*இடது*/
	val = css_computed_இடது(style, &len1, &unit1);
	தேர்வு (val) {
	நிலை CSS_இடது_AUTO:
		fprintf(stream, "left: auto ");
		நிறுத்து;
	நிலை CSS_இடது_SET:
		fprintf(stream, "left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* letter-spacing */
	val = css_computed_letter_spacing(style, &len1, &unit1);
	தேர்வு (val) {
	நிலை CSS_LETTER_SPACING_NORMAL:
		fprintf(stream, "letter-spacing: normal ");
		நிறுத்து;
	நிலை CSS_LETTER_SPACING_SET:
		fprintf(stream, "letter-spacing: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* line-height */
	val = css_computed_வரி_height(style, &len1, &unit1);
	தேர்வு (val) {
	நிலை CSS_வரி_HEIGHT_NORMAL:
		fprintf(stream, "line-height: normal ");
		நிறுத்து;
	நிலை CSS_வரி_HEIGHT_எண்:
		fprintf(stream, "line-height: ");

		dump_css_fixed(stream, len1);

		fprintf(stream, " ");
		நிறுத்து;
	நிலை CSS_வரி_HEIGHT_DIMENSION:
		fprintf(stream, "line-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* list-style-image */
	val = css_computed_list_style_image(style, &url);
	எனில் (url != வெற்று) {
		fprintf(stream, "list-style-image: url('%.*s') ",
				(முழு) lwc_string_length(url),
				lwc_string_data(url));
	} இல்லையெனில் எனில் (val == CSS_LIST_STYLE_IMAGE_NONE) {
		fprintf(stream, "list-style-image: none ");
	}

	/* list-style-position */
	val = css_computed_list_style_position(style);
	தேர்வு (val) {
	நிலை CSS_LIST_STYLE_POSITION_INSIDE:
		fprintf(stream, "list-style-position: inside ");
		நிறுத்து;
	நிலை CSS_LIST_STYLE_POSITION_OUTSIDE:
		fprintf(stream, "list-style-position: outside ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* list-style-type */
	val = css_computed_list_style_type(style);
	தேர்வு (val) {
	நிலை CSS_LIST_STYLE_TYPE_DISC:
		fprintf(stream, "list-style-type: disc ");
		நிறுத்து;
	நிலை CSS_LIST_STYLE_TYPE_CIRCLE:
		fprintf(stream, "list-style-type: circle ");
		நிறுத்து;
	நிலை CSS_LIST_STYLE_TYPE_SQUARE:
		fprintf(stream, "list-style-type: square ");
		நிறுத்து;
	நிலை CSS_LIST_STYLE_TYPE_DECIMAL:
		fprintf(stream, "list-style-type: decimal ");
		நிறுத்து;
	நிலை CSS_LIST_STYLE_TYPE_DECIMAL_LEADING_ZERO:
		fprintf(stream, "list-style-type: decimal-leading-zero ");
		நிறுத்து;
	நிலை CSS_LIST_STYLE_TYPE_LOWER_ROMAN:
		fprintf(stream, "list-style-type: lower-roman ");
		நிறுத்து;
	நிலை CSS_LIST_STYLE_TYPE_UPPER_ROMAN:
		fprintf(stream, "list-style-type: upper-roman ");
		நிறுத்து;
	நிலை CSS_LIST_STYLE_TYPE_LOWER_GREEK:
		fprintf(stream, "list-style-type: lower-greek ");
		நிறுத்து;
	நிலை CSS_LIST_STYLE_TYPE_LOWER_LATIN:
		fprintf(stream, "list-style-type: lower-latin ");
		நிறுத்து;
	நிலை CSS_LIST_STYLE_TYPE_UPPER_LATIN:
		fprintf(stream, "list-style-type: upper-latin ");
		நிறுத்து;
	நிலை CSS_LIST_STYLE_TYPE_ARMENIAN:
		fprintf(stream, "list-style-type: armenian ");
		நிறுத்து;
	நிலை CSS_LIST_STYLE_TYPE_GEORGIAN:
		fprintf(stream, "list-style-type: georgian ");
		நிறுத்து;
	நிலை CSS_LIST_STYLE_TYPE_LOWER_ALPHA:
		fprintf(stream, "list-style-type: lower-alpha ");
		நிறுத்து;
	நிலை CSS_LIST_STYLE_TYPE_UPPER_ALPHA:
		fprintf(stream, "list-style-type: upper-alpha ");
		நிறுத்து;
	நிலை CSS_LIST_STYLE_TYPE_NONE:
		fprintf(stream, "list-style-type: none ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* margin-top */
	val = css_computed_margin_உச்சி(style, &len1, &unit1);
	தேர்வு (val) {
	நிலை CSS_MARGIN_AUTO:
		fprintf(stream, "margin-top: auto ");
		நிறுத்து;
	நிலை CSS_MARGIN_SET:
		fprintf(stream, "margin-top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* margin-right */
	val = css_computed_margin_வலது(style, &len1, &unit1);
	தேர்வு (val) {
	நிலை CSS_MARGIN_AUTO:
		fprintf(stream, "margin-right: auto ");
		நிறுத்து;
	நிலை CSS_MARGIN_SET:
		fprintf(stream, "margin-right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* margin-bottom */
	val = css_computed_margin_bottom(style, &len1, &unit1);
	தேர்வு (val) {
	நிலை CSS_MARGIN_AUTO:
		fprintf(stream, "margin-bottom: auto ");
		நிறுத்து;
	நிலை CSS_MARGIN_SET:
		fprintf(stream, "margin-bottom: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* margin-left */
	val = css_computed_margin_இடது(style, &len1, &unit1);
	தேர்வு (val) {
	நிலை CSS_MARGIN_AUTO:
		fprintf(stream, "margin-left: auto ");
		நிறுத்து;
	நிலை CSS_MARGIN_SET:
		fprintf(stream, "margin-left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* max-height */
	val = css_computed_max_height(style, &len1, &unit1);
	தேர்வு (val) {
	நிலை CSS_MAX_HEIGHT_NONE:
		fprintf(stream, "max-height: none ");
		நிறுத்து;
	நிலை CSS_MAX_HEIGHT_SET:
		fprintf(stream, "max-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* max-width */
	val = css_computed_max_width(style, &len1, &unit1);
	தேர்வு (val) {
	நிலை CSS_MAX_WIDTH_NONE:
		fprintf(stream, "max-width: none ");
		நிறுத்து;
	நிலை CSS_MAX_WIDTH_SET:
		fprintf(stream, "max-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* min-height */
	val = ns_computed_min_height(style, &len1, &unit1);
	தேர்வு (val) {
	நிலை CSS_MIN_HEIGHT_SET:
		fprintf(stream, "min-height: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* min-width */
	val = ns_computed_min_width(style, &len1, &unit1);
	தேர்வு (val) {
	நிலை CSS_MIN_WIDTH_SET:
		fprintf(stream, "min-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* opacity */
	val = css_computed_opacity(style, &len1);
	தேர்வு (val) {
	நிலை CSS_OPACITY_SET:
		fprintf(stream, "opacity: ");

		dump_css_fixed(stream, len1);

		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* outline-color */
	val = css_computed_outline_color(style, &color);
	தேர்வு (val) {
	நிலை CSS_OUTLINE_COLOR_INVERT:
		fprintf(stream, "outline-color: invert ");
		நிறுத்து;
	நிலை CSS_OUTLINE_COLOR_COLOR:
		fprintf(stream, "outline-color: #%08"PRIx32" ", color);
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* outline-style */
	val = css_computed_outline_style(style);
	தேர்வு (val) {
	நிலை CSS_OUTLINE_STYLE_NONE:
		fprintf(stream, "outline-style: none ");
		நிறுத்து;
	நிலை CSS_OUTLINE_STYLE_DOTTED:
		fprintf(stream, "outline-style: dotted ");
		நிறுத்து;
	நிலை CSS_OUTLINE_STYLE_DASHED:
		fprintf(stream, "outline-style: dashed ");
		நிறுத்து;
	நிலை CSS_OUTLINE_STYLE_SOLID:
		fprintf(stream, "outline-style: solid ");
		நிறுத்து;
	நிலை CSS_OUTLINE_STYLE_DOUBLE:
		fprintf(stream, "outline-style: double ");
		நிறுத்து;
	நிலை CSS_OUTLINE_STYLE_GROOVE:
		fprintf(stream, "outline-style: groove ");
		நிறுத்து;
	நிலை CSS_OUTLINE_STYLE_RIDGE:
		fprintf(stream, "outline-style: ridge ");
		நிறுத்து;
	நிலை CSS_OUTLINE_STYLE_INSET:
		fprintf(stream, "outline-style: inset ");
		நிறுத்து;
	நிலை CSS_OUTLINE_STYLE_OUTSET:
		fprintf(stream, "outline-style: outset ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* outline-width */
	val = css_computed_outline_width(style, &len1, &unit1);
	தேர்வு (val) {
	நிலை CSS_OUTLINE_WIDTH_THIN:
		fprintf(stream, "outline-width: thin ");
		நிறுத்து;
	நிலை CSS_OUTLINE_WIDTH_MEDIUM:
		fprintf(stream, "outline-width: medium ");
		நிறுத்து;
	நிலை CSS_OUTLINE_WIDTH_THICK:
		fprintf(stream, "outline-width: thick ");
		நிறுத்து;
	நிலை CSS_OUTLINE_WIDTH_WIDTH:
		fprintf(stream, "outline-width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* overflow */
	val = css_computed_overflow_x(style);
	தேர்வு (val) {
	நிலை CSS_OVERFLOW_VISIBLE:
		fprintf(stream, "overflow-x: visible ");
		நிறுத்து;
	நிலை CSS_OVERFLOW_HIDDEN:
		fprintf(stream, "overflow-x: hidden ");
		நிறுத்து;
	நிலை CSS_OVERFLOW_SCROLL:
		fprintf(stream, "overflow-x: scroll ");
		நிறுத்து;
	நிலை CSS_OVERFLOW_AUTO:
		fprintf(stream, "overflow-x auto ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* overflow */
	val = css_computed_overflow_y(style);
	தேர்வு (val) {
	நிலை CSS_OVERFLOW_VISIBLE:
		fprintf(stream, "overflow-y: visible ");
		நிறுத்து;
	நிலை CSS_OVERFLOW_HIDDEN:
		fprintf(stream, "overflow-y: hidden ");
		நிறுத்து;
	நிலை CSS_OVERFLOW_SCROLL:
		fprintf(stream, "overflow-y: scroll ");
		நிறுத்து;
	நிலை CSS_OVERFLOW_AUTO:
		fprintf(stream, "overflow-y: auto ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* padding-top */
	val = css_computed_padding_உச்சி(style, &len1, &unit1);
	தேர்வு (val) {
	நிலை CSS_PADDING_SET:
		fprintf(stream, "padding-top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* padding-right */
	val = css_computed_padding_வலது(style, &len1, &unit1);
	தேர்வு (val) {
	நிலை CSS_PADDING_SET:
		fprintf(stream, "padding-right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* padding-bottom */
	val = css_computed_padding_bottom(style, &len1, &unit1);
	தேர்வு (val) {
	நிலை CSS_PADDING_SET:
		fprintf(stream, "padding-bottom: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* padding-left */
	val = css_computed_padding_இடது(style, &len1, &unit1);
	தேர்வு (val) {
	நிலை CSS_PADDING_SET:
		fprintf(stream, "padding-left: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* position */
	val = css_computed_position(style);
	தேர்வு (val) {
	நிலை CSS_POSITION_STATIC:
		fprintf(stream, "position: static ");
		நிறுத்து;
	நிலை CSS_POSITION_RELATIVE:
		fprintf(stream, "position: relative ");
		நிறுத்து;
	நிலை CSS_POSITION_ABSOLUTE:
		fprintf(stream, "position: absolute ");
		நிறுத்து;
	நிலை CSS_POSITION_FIXED:
		fprintf(stream, "position: fixed ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* quotes */
	val = css_computed_quotes(style, &string_list);
	எனில் (val == CSS_QUOTES_STRING && string_list != வெற்று) {
		fprintf(stream, "quotes:");

		போது (*string_list != வெற்று) {
			fprintf(stream, " \"%.*s\"",
				(முழு) lwc_string_length(*string_list),
				lwc_string_data(*string_list));

			string_list++;
		}

		fprintf(stream, " ");
	} இல்லையெனில் {
		தேர்வு (val) {
		நிலை CSS_QUOTES_NONE:
			fprintf(stream, "quotes: none ");
			நிறுத்து;
		இயல்பு:
			நிறுத்து;
		}
	}

	/*வலது*/
	val = css_computed_வலது(style, &len1, &unit1);
	தேர்வு (val) {
	நிலை CSS_வலது_AUTO:
		fprintf(stream, "right: auto ");
		நிறுத்து;
	நிலை CSS_வலது_SET:
		fprintf(stream, "right: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* table-layout */
	val = css_computed_table_layout(style);
	தேர்வு (val) {
	நிலை CSS_TABLE_LAYOUT_AUTO:
		fprintf(stream, "table-layout: auto ");
		நிறுத்து;
	நிலை CSS_TABLE_LAYOUT_FIXED:
		fprintf(stream, "table-layout: fixed ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* text-align */
	val = css_computed_text_align(style);
	தேர்வு (val) {
	நிலை CSS_TEXT_ALIGN_இடது:
		fprintf(stream, "text-align: left ");
		நிறுத்து;
	நிலை CSS_TEXT_ALIGN_வலது:
		fprintf(stream, "text-align: right ");
		நிறுத்து;
	நிலை CSS_TEXT_ALIGN_CENTER:
		fprintf(stream, "text-align: center ");
		நிறுத்து;
	நிலை CSS_TEXT_ALIGN_JUSTIFY:
		fprintf(stream, "text-align: justify ");
		நிறுத்து;
	நிலை CSS_TEXT_ALIGN_DEFAULT:
		fprintf(stream, "text-align: default ");
		நிறுத்து;
	நிலை CSS_TEXT_ALIGN_LIBCSS_இடது:
		fprintf(stream, "text-align: -libcss-left ");
		நிறுத்து;
	நிலை CSS_TEXT_ALIGN_LIBCSS_CENTER:
		fprintf(stream, "text-align: -libcss-center ");
		நிறுத்து;
	நிலை CSS_TEXT_ALIGN_LIBCSS_வலது:
		fprintf(stream, "text-align: -libcss-right ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* text-decoration */
	val = css_computed_text_decoration(style);
	எனில் (val == CSS_TEXT_DECORATION_NONE) {
		fprintf(stream, "text-decoration: none ");
	} இல்லையெனில் {
		fprintf(stream, "text-decoration:");

		எனில் (val & CSS_TEXT_DECORATION_BLINK) {
			fprintf(stream, " blink");
		}
		எனில் (val & CSS_TEXT_DECORATION_வரி_THROUGH) {
			fprintf(stream, " line-through");
		}
		எனில் (val & CSS_TEXT_DECORATION_OVERLINE) {
			fprintf(stream, " overline");
		}
		எனில் (val & CSS_TEXT_DECORATION_UNDERLINE) {
			fprintf(stream, " underline");
		}

		fprintf(stream, " ");
	}

	/* text-indent */
	val = css_computed_text_indent(style, &len1, &unit1);
	தேர்வு (val) {
	நிலை CSS_TEXT_INDENT_SET:
		fprintf(stream, "text-indent: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* text-transform */
	val = css_computed_text_transform(style);
	தேர்வு (val) {
	நிலை CSS_TEXT_TRANSFORM_CAPITALIZE:
		fprintf(stream, "text-transform: capitalize ");
		நிறுத்து;
	நிலை CSS_TEXT_TRANSFORM_UPPERCASE:
		fprintf(stream, "text-transform: uppercase ");
		நிறுத்து;
	நிலை CSS_TEXT_TRANSFORM_LOWERCASE:
		fprintf(stream, "text-transform: lowercase ");
		நிறுத்து;
	நிலை CSS_TEXT_TRANSFORM_NONE:
		fprintf(stream, "text-transform: none ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/*உச்சி*/
	val = css_computed_உச்சி(style, &len1, &unit1);
	தேர்வு (val) {
	நிலை CSS_உச்சி_AUTO:
		fprintf(stream, "top: auto ");
		நிறுத்து;
	நிலை CSS_உச்சி_SET:
		fprintf(stream, "top: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* unicode-bidi */
	val = css_computed_unicode_bidi(style);
	தேர்வு (val) {
	நிலை CSS_UNICODE_BIDI_NORMAL:
		fprintf(stream, "unicode-bidi: normal ");
		நிறுத்து;
	நிலை CSS_UNICODE_BIDI_EMBED:
		fprintf(stream, "unicode-bidi: embed ");
		நிறுத்து;
	நிலை CSS_UNICODE_BIDI_BIDI_OVERRIDE:
		fprintf(stream, "unicode-bidi: bidi-override ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* vertical-align */
	val = css_computed_vertical_align(style, &len1, &unit1);
	தேர்வு (val) {
	நிலை CSS_VERTICAL_ALIGN_BASELINE:
		fprintf(stream, "vertical-align: baseline ");
		நிறுத்து;
	நிலை CSS_VERTICAL_ALIGN_SUB:
		fprintf(stream, "vertical-align: sub ");
		நிறுத்து;
	நிலை CSS_VERTICAL_ALIGN_SUPER:
		fprintf(stream, "vertical-align: super ");
		நிறுத்து;
	நிலை CSS_VERTICAL_ALIGN_உச்சி:
		fprintf(stream, "vertical-align: top ");
		நிறுத்து;
	நிலை CSS_VERTICAL_ALIGN_TEXT_உச்சி:
		fprintf(stream, "vertical-align: text-top ");
		நிறுத்து;
	நிலை CSS_VERTICAL_ALIGN_நடு:
		fprintf(stream, "vertical-align: middle ");
		நிறுத்து;
	நிலை CSS_VERTICAL_ALIGN_BOTTOM:
		fprintf(stream, "vertical-align: bottom ");
		நிறுத்து;
	நிலை CSS_VERTICAL_ALIGN_TEXT_BOTTOM:
		fprintf(stream, "vertical-align: text-bottom ");
		நிறுத்து;
	நிலை CSS_VERTICAL_ALIGN_SET:
		fprintf(stream, "vertical-align: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* visibility */
	val = css_computed_visibility(style);
	தேர்வு (val) {
	நிலை CSS_VISIBILITY_VISIBLE:
		fprintf(stream, "visibility: visible ");
		நிறுத்து;
	நிலை CSS_VISIBILITY_HIDDEN:
		fprintf(stream, "visibility: hidden ");
		நிறுத்து;
	நிலை CSS_VISIBILITY_COLLAPSE:
		fprintf(stream, "visibility: collapse ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* white-space */
	val = css_computed_white_space(style);
	தேர்வு (val) {
	நிலை CSS_WHITE_SPACE_NORMAL:
		fprintf(stream, "white-space: normal ");
		நிறுத்து;
	நிலை CSS_WHITE_SPACE_PRE:
		fprintf(stream, "white-space: pre ");
		நிறுத்து;
	நிலை CSS_WHITE_SPACE_NOWRAP:
		fprintf(stream, "white-space: nowrap ");
		நிறுத்து;
	நிலை CSS_WHITE_SPACE_PRE_WRAP:
		fprintf(stream, "white-space: pre-wrap ");
		நிறுத்து;
	நிலை CSS_WHITE_SPACE_PRE_வரி:
		fprintf(stream, "white-space: pre-line ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* width */
	val = css_computed_width(style, &len1, &unit1);
	தேர்வு (val) {
	நிலை CSS_WIDTH_AUTO:
		fprintf(stream, "width: auto ");
		நிறுத்து;
	நிலை CSS_WIDTH_SET:
		fprintf(stream, "width: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* word-spacing */
	val = css_computed_word_spacing(style, &len1, &unit1);
	தேர்வு (val) {
	நிலை CSS_WORD_SPACING_NORMAL:
		fprintf(stream, "word-spacing: normal ");
		நிறுத்து;
	நிலை CSS_WORD_SPACING_SET:
		fprintf(stream, "word-spacing: ");

		dump_css_unit(stream, len1, unit1);

		fprintf(stream, " ");
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* z-index */
	val = css_computed_z_index(style, &zindex);
	தேர்வு (val) {
	நிலை CSS_Z_INDEX_AUTO:
		fprintf(stream, "z-index: auto ");
		நிறுத்து;
	நிலை CSS_Z_INDEX_SET:
		fprintf(stream, "z-index: %"PRId32" ", zindex);
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	fprintf(stream, "}");
}
