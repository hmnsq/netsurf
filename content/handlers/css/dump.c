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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <libcss/libcss.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/inttypes.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "css/dump.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "css/utils.h"

/**
 * Dump a fixed point value to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param f       Value to write
 */
ਸਥਿਰ ਖਾਲੀ dump_css_fixed(FILE *ਧਾਰਾ, css_fixed f)
{
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NSCSS_ABS(x) (uint32_t)((x) < 0 ? -(x) : (x))
	uint32_t uintpart = FIXTOINT(NSCSS_ABS(f));
	/* + 500 to ensure round to nearest (division will truncate) */
	uint32_t fracpart = ((NSCSS_ABS(f) & 0x3ff) * 1000 + 500) / (1 << 10);
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ NSCSS_ABS

	fprintf(ਧਾਰਾ, "%s%"PRIu32".%03"PRIu32, f < 0 ? "-" : "", uintpart, fracpart);
}

/**
 * Dump a numeric value to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 */
ਸਥਿਰ ਖਾਲੀ dump_css_ਸੰਖਿਆ(FILE *ਧਾਰਾ, css_fixed val)
{
	ਜੇ (INTTOFIX(FIXTOINT(val)) == val)
		fprintf(ਧਾਰਾ, "%"PRId32, FIXTOINT(val));
	ਨਹੀਂਤਾਂ
		dump_css_fixed(ਧਾਰਾ, val);
}

/**
 * Dump a dimension to the stream in a textual form.
 *
 * \param stream  Stream to write to
 * \param val     Value to write
 * \param unit    Unit to write
 */
ਸਥਿਰ ਖਾਲੀ dump_css_unit(FILE *ਧਾਰਾ, css_fixed val, css_unit unit)
{
	dump_css_ਸੰਖਿਆ(ਧਾਰਾ, val);

	ਚੋਣ (unit) {
	ਹਾਲਤ CSS_UNIT_PX:
		fprintf(ਧਾਰਾ, "px");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_UNIT_EX:
		fprintf(ਧਾਰਾ, "ex");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_UNIT_EM:
		fprintf(ਧਾਰਾ, "em");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_UNIT_ਵਿੱਚ:
		fprintf(ਧਾਰਾ, "in");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_UNIT_CM:
		fprintf(ਧਾਰਾ, "cm");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_UNIT_MM:
		fprintf(ਧਾਰਾ, "mm");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_UNIT_PT:
		fprintf(ਧਾਰਾ, "pt");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_UNIT_PC:
		fprintf(ਧਾਰਾ, "pc");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_UNIT_PCT:
		fprintf(ਧਾਰਾ, "%%");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_UNIT_DEG:
		fprintf(ਧਾਰਾ, "deg");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_UNIT_GRAD:
		fprintf(ਧਾਰਾ, "grad");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_UNIT_RAD:
		fprintf(ਧਾਰਾ, "rad");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_UNIT_MS:
		fprintf(ਧਾਰਾ, "ms");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_UNIT_S:
		fprintf(ਧਾਰਾ, "s");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_UNIT_HZ:
		fprintf(ਧਾਰਾ, "Hz");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_UNIT_KHZ:
		fprintf(ਧਾਰਾ, "kHz");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_UNIT_CH:
		fprintf(ਧਾਰਾ, "ch");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_UNIT_REM:
		fprintf(ਧਾਰਾ, "rem");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_UNIT_LH:
		fprintf(ਧਾਰਾ, "lh");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_UNIT_VH:
		fprintf(ਧਾਰਾ, "vh");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_UNIT_VW:
		fprintf(ਧਾਰਾ, "vw");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_UNIT_VI:
		fprintf(ਧਾਰਾ, "vi");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_UNIT_VB:
		fprintf(ਧਾਰਾ, "vb");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_UNIT_VMIN:
		fprintf(ਧਾਰਾ, "vmin");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_UNIT_VMAX:
		fprintf(ਧਾਰਾ, "vmax");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_UNIT_Q:
		fprintf(ਧਾਰਾ, "q");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_UNIT_CALC:
		fprintf(ਧਾਰਾ, "calc()");
		ਰੋਕੋ;
	}
}

/* exported interface documented in content/handlers/css/dump.h */
ਖਾਲੀ nscss_dump_computed_style(FILE *ਧਾਰਾ, ਅਚਲ css_computed_style *style)
{
	uint8_t val;
	css_color color = 0;
	lwc_string *url = ਨਿਰਾ;
	css_fixed len1 = 0, len2 = 0;
	css_unit unit1 = CSS_UNIT_PX, unit2 = CSS_UNIT_PX;
	css_computed_clip_rect rect = { 0, 0, 0, 0, CSS_UNIT_PX, CSS_UNIT_PX,
					CSS_UNIT_PX, CSS_UNIT_PX, true, true,
					true, true };
	ਅਚਲ css_computed_content_ਵਸਤੂ *content = ਨਿਰਾ;
	ਅਚਲ css_computed_counter *counter = ਨਿਰਾ;
	lwc_string **string_list = ਨਿਰਾ;
	int32_t zindex = 0;

	fprintf(ਧਾਰਾ, "{ ");

	/* background-attachment */
	val = css_computed_background_attachment(style);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_BACKGROUND_ATTACHMENT_FIXED:
		fprintf(ਧਾਰਾ, "background-attachment: fixed ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BACKGROUND_ATTACHMENT_SCROLL:
		fprintf(ਧਾਰਾ, "background-attachment: scroll ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* background-color */
	val = css_computed_background_color(style, &color);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_BACKGROUND_COLOR_COLOR:
		fprintf(ਧਾਰਾ, "background-color: #%08"PRIx32" ", color);
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* background-image */
	val = css_computed_background_image(style, &url);
	ਜੇ (val == CSS_BACKGROUND_IMAGE_IMAGE && url != ਨਿਰਾ) {
		fprintf(ਧਾਰਾ, "background-image: url('%.*s') ",
				(ਪੂਰਨ) lwc_string_ਲੰਮਾਈ(url),
				lwc_string_ਅੰਕੜੇ(url));
	} ਨਹੀਂਤਾਂ ਜੇ (val == CSS_BACKGROUND_IMAGE_NONE) {
		fprintf(ਧਾਰਾ, "background-image: none ");
	}

	/* background-position */
	val = css_computed_background_position(style, &len1, &unit1,
			&len2, &unit2);
	ਜੇ (val == CSS_BACKGROUND_POSITION_ਸੈੱਟ) {
		fprintf(ਧਾਰਾ, "background-position: ");
		dump_css_unit(ਧਾਰਾ, len1, unit1);
		fprintf(ਧਾਰਾ, " ");
		dump_css_unit(ਧਾਰਾ, len2, unit2);
		fprintf(ਧਾਰਾ, " ");
	}

	/* background-repeat */
	val = css_computed_background_repeat(style);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_BACKGROUND_REPEAT_REPEAT_X:
		fprintf(ਧਾਰਾ, "background-repeat: repeat-x ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BACKGROUND_REPEAT_REPEAT_Y:
		fprintf(ਧਾਰਾ, "background-repeat: repeat-y ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BACKGROUND_REPEAT_REPEAT:
		fprintf(ਧਾਰਾ, "background-repeat: repeat ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BACKGROUND_REPEAT_NO_REPEAT:
		fprintf(ਧਾਰਾ, "background-repeat: no-repeat ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* border-collapse */
	val = css_computed_border_collapse(style);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_BORDER_COLLAPSE_SEPARATE:
		fprintf(ਧਾਰਾ, "border-collapse: separate ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_COLLAPSE_COLLAPSE:
		fprintf(ਧਾਰਾ, "border-collapse: collapse ");
		ਰੋਕੋ;
	ਮੂਲ:

		ਰੋਕੋ;
	}

	/* border-spacing */
	val = css_computed_border_spacing(style, &len1, &unit1, &len2, &unit2);
	ਜੇ (val == CSS_BORDER_SPACING_ਸੈੱਟ) {
		fprintf(ਧਾਰਾ, "border-spacing: ");
		dump_css_unit(ਧਾਰਾ, len1, unit1);
		fprintf(ਧਾਰਾ, " ");
		dump_css_unit(ਧਾਰਾ, len2, unit2);
		fprintf(ਧਾਰਾ, " ");
	}

	/* border-top-color */
	val = css_computed_border_ਸਿਖਰ_color(style, &color);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_BORDER_COLOR_COLOR:
		fprintf(ਧਾਰਾ, "border-top-color: #%08"PRIx32" ", color);
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* border-right-color */
	val = css_computed_border_ਸੱਜਾ_color(style, &color);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_BORDER_COLOR_COLOR:
		fprintf(ਧਾਰਾ, "border-right-color: #%08"PRIx32" ", color);
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* border-bottom-color */
	val = css_computed_border_bottom_color(style, &color);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_BORDER_COLOR_COLOR:
		fprintf(ਧਾਰਾ, "border-bottom-color: #%08"PRIx32" ", color);
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* border-left-color */
	val = css_computed_border_ਖੱਬਾ_color(style, &color);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_BORDER_COLOR_COLOR:
		fprintf(ਧਾਰਾ, "border-left-color: #%08"PRIx32" ", color);
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* border-top-style */
	val = css_computed_border_ਸਿਖਰ_style(style);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_BORDER_STYLE_NONE:
		fprintf(ਧਾਰਾ, "border-top-style: none ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_HIDDEN:
		fprintf(ਧਾਰਾ, "border-top-style: hidden ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_DOTTED:
		fprintf(ਧਾਰਾ, "border-top-style: dotted ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_DASHED:
		fprintf(ਧਾਰਾ, "border-top-style: dashed ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_SOLID:
		fprintf(ਧਾਰਾ, "border-top-style: solid ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_DOUBLE:
		fprintf(ਧਾਰਾ, "border-top-style: double ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_GROOVE:
		fprintf(ਧਾਰਾ, "border-top-style: groove ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_RIDGE:
		fprintf(ਧਾਰਾ, "border-top-style: ridge ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_INSET:
		fprintf(ਧਾਰਾ, "border-top-style: inset ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_OUTSET:
		fprintf(ਧਾਰਾ, "border-top-style: outset ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* border-right-style */
	val = css_computed_border_ਸੱਜਾ_style(style);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_BORDER_STYLE_NONE:
		fprintf(ਧਾਰਾ, "border-right-style: none ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_HIDDEN:
		fprintf(ਧਾਰਾ, "border-right-style: hidden ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_DOTTED:
		fprintf(ਧਾਰਾ, "border-right-style: dotted ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_DASHED:
		fprintf(ਧਾਰਾ, "border-right-style: dashed ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_SOLID:
		fprintf(ਧਾਰਾ, "border-right-style: solid ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_DOUBLE:
		fprintf(ਧਾਰਾ, "border-right-style: double ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_GROOVE:
		fprintf(ਧਾਰਾ, "border-right-style: groove ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_RIDGE:
		fprintf(ਧਾਰਾ, "border-right-style: ridge ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_INSET:
		fprintf(ਧਾਰਾ, "border-right-style: inset ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_OUTSET:
		fprintf(ਧਾਰਾ, "border-right-style: outset ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* border-bottom-style */
	val = css_computed_border_bottom_style(style);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_BORDER_STYLE_NONE:
		fprintf(ਧਾਰਾ, "border-bottom-style: none ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_HIDDEN:
		fprintf(ਧਾਰਾ, "border-bottom-style: hidden ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_DOTTED:
		fprintf(ਧਾਰਾ, "border-bottom-style: dotted ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_DASHED:
		fprintf(ਧਾਰਾ, "border-bottom-style: dashed ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_SOLID:
		fprintf(ਧਾਰਾ, "border-bottom-style: solid ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_DOUBLE:
		fprintf(ਧਾਰਾ, "border-bottom-style: double ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_GROOVE:
		fprintf(ਧਾਰਾ, "border-bottom-style: groove ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_RIDGE:
		fprintf(ਧਾਰਾ, "border-bottom-style: ridge ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_INSET:
		fprintf(ਧਾਰਾ, "border-bottom-style: inset ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_OUTSET:
		fprintf(ਧਾਰਾ, "border-bottom-style: outset ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* border-left-style */
	val = css_computed_border_ਖੱਬਾ_style(style);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_BORDER_STYLE_NONE:
		fprintf(ਧਾਰਾ, "border-left-style: none ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_HIDDEN:
		fprintf(ਧਾਰਾ, "border-left-style: hidden ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_DOTTED:
		fprintf(ਧਾਰਾ, "border-left-style: dotted ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_DASHED:
		fprintf(ਧਾਰਾ, "border-left-style: dashed ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_SOLID:
		fprintf(ਧਾਰਾ, "border-left-style: solid ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_DOUBLE:
		fprintf(ਧਾਰਾ, "border-left-style: double ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_GROOVE:
		fprintf(ਧਾਰਾ, "border-left-style: groove ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_RIDGE:
		fprintf(ਧਾਰਾ, "border-left-style: ridge ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_INSET:
		fprintf(ਧਾਰਾ, "border-left-style: inset ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_STYLE_OUTSET:
		fprintf(ਧਾਰਾ, "border-left-style: outset ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* border-top-width */
	val = css_computed_border_ਸਿਖਰ_ਚੌੜਾਈ(style, &len1, &unit1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_BORDER_ਚੌੜਾਈ_THIN:
		fprintf(ਧਾਰਾ, "border-top-width: thin ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_ਚੌੜਾਈ_MEDIUM:
		fprintf(ਧਾਰਾ, "border-top-width: medium ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_ਚੌੜਾਈ_THICK:
		fprintf(ਧਾਰਾ, "border-top-width: thick ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_ਚੌੜਾਈ_ਚੌੜਾਈ:
		fprintf(ਧਾਰਾ, "border-top-width: ");
		dump_css_unit(ਧਾਰਾ, len1, unit1);
		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* border-right-width */
	val = css_computed_border_ਸੱਜਾ_ਚੌੜਾਈ(style, &len1, &unit1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_BORDER_ਚੌੜਾਈ_THIN:
		fprintf(ਧਾਰਾ, "border-right-width: thin ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_ਚੌੜਾਈ_MEDIUM:
		fprintf(ਧਾਰਾ, "border-right-width: medium ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_ਚੌੜਾਈ_THICK:
		fprintf(ਧਾਰਾ, "border-right-width: thick ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_ਚੌੜਾਈ_ਚੌੜਾਈ:
		fprintf(ਧਾਰਾ, "border-right-width: ");
		dump_css_unit(ਧਾਰਾ, len1, unit1);
		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* border-bottom-width */
	val = css_computed_border_bottom_ਚੌੜਾਈ(style, &len1, &unit1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_BORDER_ਚੌੜਾਈ_THIN:
		fprintf(ਧਾਰਾ, "border-bottom-width: thin ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_ਚੌੜਾਈ_MEDIUM:
		fprintf(ਧਾਰਾ, "border-bottom-width: medium ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_ਚੌੜਾਈ_THICK:
		fprintf(ਧਾਰਾ, "border-bottom-width: thick ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_ਚੌੜਾਈ_ਚੌੜਾਈ:
		fprintf(ਧਾਰਾ, "border-bottom-width: ");
		dump_css_unit(ਧਾਰਾ, len1, unit1);
		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* border-left-width */
	val = css_computed_border_ਖੱਬਾ_ਚੌੜਾਈ(style, &len1, &unit1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_BORDER_ਚੌੜਾਈ_THIN:
		fprintf(ਧਾਰਾ, "border-left-width: thin ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_ਚੌੜਾਈ_MEDIUM:
		fprintf(ਧਾਰਾ, "border-left-width: medium ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_ਚੌੜਾਈ_THICK:
		fprintf(ਧਾਰਾ, "border-left-width: thick ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BORDER_ਚੌੜਾਈ_ਚੌੜਾਈ:
		fprintf(ਧਾਰਾ, "border-left-width: ");
		dump_css_unit(ਧਾਰਾ, len1, unit1);
		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* bottom */
	val = css_computed_bottom(style, &len1, &unit1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_BOTTOM_AUTO:
		fprintf(ਧਾਰਾ, "bottom: auto ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_BOTTOM_ਸੈੱਟ:
		fprintf(ਧਾਰਾ, "bottom: ");
		dump_css_unit(ਧਾਰਾ, len1, unit1);
		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* caption-side */
	val = css_computed_caption_side(style);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_CAPTION_SIDE_ਸਿਖਰ:
		fprintf(ਧਾਰਾ, "caption_side: top ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CAPTION_SIDE_BOTTOM:
		fprintf(ਧਾਰਾ, "caption_side: bottom ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* clear */
	val = css_computed_clear(style);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_CLEAR_NONE:
		fprintf(ਧਾਰਾ, "clear: none ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CLEAR_ਖੱਬਾ:
		fprintf(ਧਾਰਾ, "clear: left ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CLEAR_ਸੱਜਾ:
		fprintf(ਧਾਰਾ, "clear: right ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CLEAR_BOTH:
		fprintf(ਧਾਰਾ, "clear: both ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* clip */
	val = css_computed_clip(style, &rect);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_CLIP_AUTO:
		fprintf(ਧਾਰਾ, "clip: auto ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CLIP_RECT:
		fprintf(ਧਾਰਾ, "clip: rect( ");

		ਜੇ (rect.ਸਿਖਰ_auto)
			fprintf(ਧਾਰਾ, "auto");
		ਨਹੀਂਤਾਂ
			dump_css_unit(ਧਾਰਾ, rect.ਸਿਖਰ, rect.tunit);
		fprintf(ਧਾਰਾ, ", ");

		ਜੇ (rect.ਸੱਜਾ_auto)
			fprintf(ਧਾਰਾ, "auto");
		ਨਹੀਂਤਾਂ
			dump_css_unit(ਧਾਰਾ, rect.ਸੱਜਾ, rect.runit);
		fprintf(ਧਾਰਾ, ", ");

		ਜੇ (rect.bottom_auto)
			fprintf(ਧਾਰਾ, "auto");
		ਨਹੀਂਤਾਂ
			dump_css_unit(ਧਾਰਾ, rect.bottom, rect.bunit);
		fprintf(ਧਾਰਾ, ", ");

		ਜੇ (rect.ਖੱਬਾ_auto)
			fprintf(ਧਾਰਾ, "auto");
		ਨਹੀਂਤਾਂ
			dump_css_unit(ਧਾਰਾ, rect.ਖੱਬਾ, rect.lunit);
		fprintf(ਧਾਰਾ, ") ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* color */
	val = css_computed_color(style, &color);
	ਜੇ (val == CSS_COLOR_COLOR) {
		fprintf(ਧਾਰਾ, "color: #%08"PRIx32" ", color);
	}

	/* content */
	val = css_computed_content(style, &content);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_CONTENT_NONE:
		fprintf(ਧਾਰਾ, "content: none ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CONTENT_ਸਾਧਾਰਨ:
		fprintf(ਧਾਰਾ, "content: normal ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CONTENT_ਸੈੱਟ:
		fprintf(ਧਾਰਾ, "content:");

		ਜਦੋਂ (content->type != CSS_COMPUTED_CONTENT_NONE) {
			fprintf(ਧਾਰਾ, " ");

			ਚੋਣ (content->type) {
			ਹਾਲਤ CSS_COMPUTED_CONTENT_STRING:
				fprintf(ਧਾਰਾ,	"\"%.*s\"",
						(ਪੂਰਨ) lwc_string_ਲੰਮਾਈ(
						content->ਅੰਕੜੇ.string),
						lwc_string_ਅੰਕੜੇ(
						content->ਅੰਕੜੇ.string));
				ਰੋਕੋ;
			ਹਾਲਤ CSS_COMPUTED_CONTENT_URI:
				fprintf(ਧਾਰਾ,	"uri(\"%.*s\")",
						(ਪੂਰਨ) lwc_string_ਲੰਮਾਈ(
						content->ਅੰਕੜੇ.uri),
						lwc_string_ਅੰਕੜੇ(
						content->ਅੰਕੜੇ.uri));
				ਰੋਕੋ;
			ਹਾਲਤ CSS_COMPUTED_CONTENT_COUNTER:
				fprintf(ਧਾਰਾ, "counter(%.*s)",
						(ਪੂਰਨ) lwc_string_ਲੰਮਾਈ(
						content->ਅੰਕੜੇ.counter.name),
						lwc_string_ਅੰਕੜੇ(
						content->ਅੰਕੜੇ.counter.name));
				ਰੋਕੋ;
			ਹਾਲਤ CSS_COMPUTED_CONTENT_COUNTERS:
				fprintf(ਧਾਰਾ, "counters(%.*s, \"%.*s\")",
						(ਪੂਰਨ) lwc_string_ਲੰਮਾਈ(
						content->ਅੰਕੜੇ.counters.name),
						lwc_string_ਅੰਕੜੇ(
						content->ਅੰਕੜੇ.counters.name),
						(ਪੂਰਨ) lwc_string_ਲੰਮਾਈ(
						content->ਅੰਕੜੇ.counters.sep),
						lwc_string_ਅੰਕੜੇ(
						content->ਅੰਕੜੇ.counters.sep));
				ਰੋਕੋ;
			ਹਾਲਤ CSS_COMPUTED_CONTENT_ATTR:
				fprintf(ਧਾਰਾ, "attr(%.*s)",
						(ਪੂਰਨ) lwc_string_ਲੰਮਾਈ(
						content->ਅੰਕੜੇ.attr),
						lwc_string_ਅੰਕੜੇ(
						content->ਅੰਕੜੇ.attr));
				ਰੋਕੋ;
			ਹਾਲਤ CSS_COMPUTED_CONTENT_OPEN_QUOTE:
				fprintf(ਧਾਰਾ, "open-quote");
				ਰੋਕੋ;
			ਹਾਲਤ CSS_COMPUTED_CONTENT_CLOSE_QUOTE:
				fprintf(ਧਾਰਾ, "close-quote");
				ਰੋਕੋ;
			ਹਾਲਤ CSS_COMPUTED_CONTENT_NO_OPEN_QUOTE:
				fprintf(ਧਾਰਾ, "no-open-quote");
				ਰੋਕੋ;
			ਹਾਲਤ CSS_COMPUTED_CONTENT_NO_CLOSE_QUOTE:
				fprintf(ਧਾਰਾ, "no-close-quote");
				ਰੋਕੋ;
			}

			content++;
		}

		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* counter-increment */
	val = css_computed_counter_increment(style, &counter);
	ਜੇ ((val == CSS_COUNTER_INCREMENT_NONE) || (counter == ਨਿਰਾ)) {
		fprintf(ਧਾਰਾ, "counter-increment: none ");
	} ਨਹੀਂਤਾਂ {
		fprintf(ਧਾਰਾ, "counter-increment:");

		ਜਦੋਂ (counter->name != ਨਿਰਾ) {
			fprintf(ਧਾਰਾ, " %.*s ",
					(ਪੂਰਨ) lwc_string_ਲੰਮਾਈ(counter->name),
					lwc_string_ਅੰਕੜੇ(counter->name));

			dump_css_fixed(ਧਾਰਾ, counter->ਮੁੱਲ);

			counter++;
		}

		fprintf(ਧਾਰਾ, " ");
	}

	/* counter-reset */
	val = css_computed_counter_reset(style, &counter);
	ਜੇ ((val == CSS_COUNTER_RESET_NONE) || (counter == ਨਿਰਾ)) {
		fprintf(ਧਾਰਾ, "counter-reset: none ");
	} ਨਹੀਂਤਾਂ {
		fprintf(ਧਾਰਾ, "counter-reset:");

		ਜਦੋਂ (counter->name != ਨਿਰਾ) {
			fprintf(ਧਾਰਾ, " %.*s ",
					(ਪੂਰਨ) lwc_string_ਲੰਮਾਈ(counter->name),
					lwc_string_ਅੰਕੜੇ(counter->name));

			dump_css_fixed(ਧਾਰਾ, counter->ਮੁੱਲ);

			counter++;
		}

		fprintf(ਧਾਰਾ, " ");
	}

	/* cursor */
	val = css_computed_cursor(style, &string_list);
	fprintf(ਧਾਰਾ, "cursor:");

	ਜੇ (string_list != ਨਿਰਾ) {
		ਜਦੋਂ (*string_list != ਨਿਰਾ) {
			fprintf(ਧਾਰਾ, " url\"%.*s\")",
					(ਪੂਰਨ) lwc_string_ਲੰਮਾਈ(*string_list),
					lwc_string_ਅੰਕੜੇ(*string_list));

			string_list++;
		}
	}
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_CURSOR_AUTO:
		fprintf(ਧਾਰਾ, " auto ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_CROSSHAIR:
		fprintf(ਧਾਰਾ, " crosshair ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_DEFAULT:
		fprintf(ਧਾਰਾ, " default ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_ਸੂਚਕ:
		fprintf(ਧਾਰਾ, " pointer ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_MOVE:
		fprintf(ਧਾਰਾ, " move ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_E_RESIZE:
		fprintf(ਧਾਰਾ, " e-resize ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_NE_RESIZE:
		fprintf(ਧਾਰਾ, " ne-resize ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_NW_RESIZE:
		fprintf(ਧਾਰਾ, " nw-resize ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_N_RESIZE:
		fprintf(ਧਾਰਾ, " n-resize ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_SE_RESIZE:
		fprintf(ਧਾਰਾ, " se-resize ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_SW_RESIZE:
		fprintf(ਧਾਰਾ, " sw-resize ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_S_RESIZE:
		fprintf(ਧਾਰਾ, " s-resize ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_W_RESIZE:
		fprintf(ਧਾਰਾ, " w-resize ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_ਪਾਠ:
		fprintf(ਧਾਰਾ, " text ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_WAIT:
		fprintf(ਧਾਰਾ, " wait ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_HELP:
		fprintf(ਧਾਰਾ, " help ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_CURSOR_ਤਰੱਕੀ:
		fprintf(ਧਾਰਾ, " progress ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* direction */
	val = css_computed_direction(style);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_DIRECTION_LTR:
		fprintf(ਧਾਰਾ, "direction: ltr ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_DIRECTION_RTL:
		fprintf(ਧਾਰਾ, "direction: rtl ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* display */
	val = ns_computed_display_static(style);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_DISPLAY_INLINE:
		fprintf(ਧਾਰਾ, "display: inline ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_DISPLAY_ਖੰਡ:
		fprintf(ਧਾਰਾ, "display: block ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_DISPLAY_LIST_ਵਸਤੂ:
		fprintf(ਧਾਰਾ, "display: list-item ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_DISPLAY_ਲੜੀਵਾਰ_ਵਿੱਚ:
		fprintf(ਧਾਰਾ, "display: run-in ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_DISPLAY_INLINE_ਖੰਡ:
		fprintf(ਧਾਰਾ, "display: inline-block ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_DISPLAY_ਸਾਰਣੀ:
		fprintf(ਧਾਰਾ, "display: table ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_DISPLAY_INLINE_ਸਾਰਣੀ:
		fprintf(ਧਾਰਾ, "display: inline-table ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_DISPLAY_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ:
		fprintf(ਧਾਰਾ, "display: table-row-group ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_DISPLAY_ਸਾਰਣੀ_ਸਿਰਲੇਖ_ਸਮੂਹ:
		fprintf(ਧਾਰਾ, "display: table-header-group ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_DISPLAY_ਸਾਰਣੀ_FOOTER_ਸਮੂਹ:
		fprintf(ਧਾਰਾ, "display: table-footer-group ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_DISPLAY_ਸਾਰਣੀ_ਪੰਕਤੀ:
		fprintf(ਧਾਰਾ, "display: table-row ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_DISPLAY_ਸਾਰਣੀ_ਕਾਲਮ_ਸਮੂਹ:
		fprintf(ਧਾਰਾ, "display: table-column-group ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_DISPLAY_ਸਾਰਣੀ_ਕਾਲਮ:
		fprintf(ਧਾਰਾ, "display: table-column ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_DISPLAY_ਸਾਰਣੀ_CELL:
		fprintf(ਧਾਰਾ, "display: table-cell ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_DISPLAY_ਸਾਰਣੀ_CAPTION:
		fprintf(ਧਾਰਾ, "display: table-caption ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_DISPLAY_NONE:
		fprintf(ਧਾਰਾ, "display: none ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_DISPLAY_FLEX:
		fprintf(ਧਾਰਾ, "display: flex ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_DISPLAY_INLINE_FLEX:
		fprintf(ਧਾਰਾ, "display: inline-flex ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* empty-cells */
	val = css_computed_ਖਾਲੀ_ਥਾਵਾਂ_cells(style);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_ਖਾਲੀ_ਥਾਵਾਂ_CELLS_SHOW:
		fprintf(ਧਾਰਾ, "empty-cells: show ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_ਖਾਲੀ_ਥਾਵਾਂ_CELLS_HIDE:
		fprintf(ਧਾਰਾ, "empty-cells: hide ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* float */
	val = css_computed_float(style);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_FLOAT_ਖੱਬਾ:
		fprintf(ਧਾਰਾ, "float: left ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FLOAT_ਸੱਜਾ:
		fprintf(ਧਾਰਾ, "float: right ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FLOAT_NONE:
		fprintf(ਧਾਰਾ, "float: none ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* font-family */
	val = css_computed_font_family(style, &string_list);
	ਜੇ (val != CSS_FONT_FAMILY_INHERIT) {
		fprintf(ਧਾਰਾ, "font-family:");

		ਜੇ (string_list != ਨਿਰਾ) {
			ਜਦੋਂ (*string_list != ਨਿਰਾ) {
				fprintf(ਧਾਰਾ, " \"%.*s\"",
					(ਪੂਰਨ) lwc_string_ਲੰਮਾਈ(*string_list),
					lwc_string_ਅੰਕੜੇ(*string_list));

				string_list++;
			}
		}
		ਚੋਣ (val) {
		ਹਾਲਤ CSS_FONT_FAMILY_SERIF:
			fprintf(ਧਾਰਾ, " serif ");
			ਰੋਕੋ;
		ਹਾਲਤ CSS_FONT_FAMILY_SANS_SERIF:
			fprintf(ਧਾਰਾ, " sans-serif ");
			ਰੋਕੋ;
		ਹਾਲਤ CSS_FONT_FAMILY_CURSIVE:
			fprintf(ਧਾਰਾ, " cursive ");
			ਰੋਕੋ;
		ਹਾਲਤ CSS_FONT_FAMILY_FANTASY:
			fprintf(ਧਾਰਾ, " fantasy ");
			ਰੋਕੋ;
		ਹਾਲਤ CSS_FONT_FAMILY_MONOSPACE:
			fprintf(ਧਾਰਾ, " monospace ");
			ਰੋਕੋ;
		}
	}

	/* font-size */
	val = css_computed_font_ਮਾਪ(style, &len1, &unit1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_FONT_ਮਾਪ_XX_SMALL:
		fprintf(ਧਾਰਾ, "font-size: xx-small ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_ਮਾਪ_X_SMALL:
		fprintf(ਧਾਰਾ, "font-size: x-small ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_ਮਾਪ_SMALL:
		fprintf(ਧਾਰਾ, "font-size: small ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_ਮਾਪ_MEDIUM:
		fprintf(ਧਾਰਾ, "font-size: medium ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_ਮਾਪ_LARGE:
		fprintf(ਧਾਰਾ, "font-size: large ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_ਮਾਪ_X_LARGE:
		fprintf(ਧਾਰਾ, "font-size: x-large ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_ਮਾਪ_XX_LARGE:
		fprintf(ਧਾਰਾ, "font-size: xx-large ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_ਮਾਪ_LARGER:
		fprintf(ਧਾਰਾ, "font-size: larger ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_ਮਾਪ_SMALLER:
		fprintf(ਧਾਰਾ, "font-size: smaller ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_ਮਾਪ_DIMENSION:
		fprintf(ਧਾਰਾ, "font-size: ");

		dump_css_unit(ਧਾਰਾ, len1, unit1);

		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* font-style */
	val = css_computed_font_style(style);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_FONT_STYLE_ਸਾਧਾਰਨ:
		fprintf(ਧਾਰਾ, "font-style: normal ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_STYLE_ITALIC:
		fprintf(ਧਾਰਾ, "font-style: italic ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_STYLE_OBLIQUE:
		fprintf(ਧਾਰਾ, "font-style: oblique ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* font-variant */
	val = css_computed_font_variant(style);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_FONT_VARIANT_ਸਾਧਾਰਨ:
		fprintf(ਧਾਰਾ, "font-variant: normal ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_VARIANT_SMALL_CAPS:
		fprintf(ਧਾਰਾ, "font-variant: small-caps ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* font-weight */
	val = css_computed_font_ਭਾਰ(style);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_FONT_ਭਾਰ_ਸਾਧਾਰਨ:
		fprintf(ਧਾਰਾ, "font-weight: normal ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_ਭਾਰ_BOLD:
		fprintf(ਧਾਰਾ, "font-weight: bold ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_ਭਾਰ_BOLDER:
		fprintf(ਧਾਰਾ, "font-weight: bolder ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_ਭਾਰ_LIGHTER:
		fprintf(ਧਾਰਾ, "font-weight: lighter ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_ਭਾਰ_100:
		fprintf(ਧਾਰਾ, "font-weight: 100 ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_ਭਾਰ_200:
		fprintf(ਧਾਰਾ, "font-weight: 200 ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_ਭਾਰ_300:
		fprintf(ਧਾਰਾ, "font-weight: 300 ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_ਭਾਰ_400:
		fprintf(ਧਾਰਾ, "font-weight: 400 ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_ਭਾਰ_500:
		fprintf(ਧਾਰਾ, "font-weight: 500 ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_ਭਾਰ_600:
		fprintf(ਧਾਰਾ, "font-weight: 600 ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_ਭਾਰ_700:
		fprintf(ਧਾਰਾ, "font-weight: 700 ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_ਭਾਰ_800:
		fprintf(ਧਾਰਾ, "font-weight: 800 ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_FONT_ਭਾਰ_900:
		fprintf(ਧਾਰਾ, "font-weight: 900 ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/*ਉਚਾਈ*/
	val = css_computed_ਉਚਾਈ(style, &len1, &unit1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_ਉਚਾਈ_AUTO:
		fprintf(ਧਾਰਾ, "height: auto ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_ਉਚਾਈ_ਸੈੱਟ:
		fprintf(ਧਾਰਾ, "height: ");

		dump_css_unit(ਧਾਰਾ, len1, unit1);

		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/*ਖੱਬਾ*/
	val = css_computed_ਖੱਬਾ(style, &len1, &unit1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_ਖੱਬਾ_AUTO:
		fprintf(ਧਾਰਾ, "left: auto ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_ਖੱਬਾ_ਸੈੱਟ:
		fprintf(ਧਾਰਾ, "left: ");

		dump_css_unit(ਧਾਰਾ, len1, unit1);

		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* letter-spacing */
	val = css_computed_letter_spacing(style, &len1, &unit1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_LETTER_SPACING_ਸਾਧਾਰਨ:
		fprintf(ਧਾਰਾ, "letter-spacing: normal ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_LETTER_SPACING_ਸੈੱਟ:
		fprintf(ਧਾਰਾ, "letter-spacing: ");

		dump_css_unit(ਧਾਰਾ, len1, unit1);

		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* line-height */
	val = css_computed_ਸਤਰ_ਉਚਾਈ(style, &len1, &unit1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_ਸਤਰ_ਉਚਾਈ_ਸਾਧਾਰਨ:
		fprintf(ਧਾਰਾ, "line-height: normal ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_ਸਤਰ_ਉਚਾਈ_ਸੰਖਿਆ:
		fprintf(ਧਾਰਾ, "line-height: ");

		dump_css_fixed(ਧਾਰਾ, len1);

		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_ਸਤਰ_ਉਚਾਈ_DIMENSION:
		fprintf(ਧਾਰਾ, "line-height: ");

		dump_css_unit(ਧਾਰਾ, len1, unit1);

		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* list-style-image */
	val = css_computed_list_style_image(style, &url);
	ਜੇ (url != ਨਿਰਾ) {
		fprintf(ਧਾਰਾ, "list-style-image: url('%.*s') ",
				(ਪੂਰਨ) lwc_string_ਲੰਮਾਈ(url),
				lwc_string_ਅੰਕੜੇ(url));
	} ਨਹੀਂਤਾਂ ਜੇ (val == CSS_LIST_STYLE_IMAGE_NONE) {
		fprintf(ਧਾਰਾ, "list-style-image: none ");
	}

	/* list-style-position */
	val = css_computed_list_style_position(style);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_LIST_STYLE_POSITION_INSIDE:
		fprintf(ਧਾਰਾ, "list-style-position: inside ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_LIST_STYLE_POSITION_OUTSIDE:
		fprintf(ਧਾਰਾ, "list-style-position: outside ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* list-style-type */
	val = css_computed_list_style_type(style);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_LIST_STYLE_TYPE_DISC:
		fprintf(ਧਾਰਾ, "list-style-type: disc ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_LIST_STYLE_TYPE_CIRCLE:
		fprintf(ਧਾਰਾ, "list-style-type: circle ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_LIST_STYLE_TYPE_ਵਰਗਫਲ:
		fprintf(ਧਾਰਾ, "list-style-type: square ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_LIST_STYLE_TYPE_DECIMAL:
		fprintf(ਧਾਰਾ, "list-style-type: decimal ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_LIST_STYLE_TYPE_DECIMAL_LEADING_ZERO:
		fprintf(ਧਾਰਾ, "list-style-type: decimal-leading-zero ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_LIST_STYLE_TYPE_ਹੇਠਲਾ_ROMAN:
		fprintf(ਧਾਰਾ, "list-style-type: lower-roman ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_LIST_STYLE_TYPE_ਉੱਤਲਾ_ROMAN:
		fprintf(ਧਾਰਾ, "list-style-type: upper-roman ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_LIST_STYLE_TYPE_ਹੇਠਲਾ_GREEK:
		fprintf(ਧਾਰਾ, "list-style-type: lower-greek ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_LIST_STYLE_TYPE_ਹੇਠਲਾ_LATIN:
		fprintf(ਧਾਰਾ, "list-style-type: lower-latin ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_LIST_STYLE_TYPE_ਉੱਤਲਾ_LATIN:
		fprintf(ਧਾਰਾ, "list-style-type: upper-latin ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_LIST_STYLE_TYPE_ARMENIAN:
		fprintf(ਧਾਰਾ, "list-style-type: armenian ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_LIST_STYLE_TYPE_GEORGIAN:
		fprintf(ਧਾਰਾ, "list-style-type: georgian ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_LIST_STYLE_TYPE_ਹੇਠਲਾ_ALPHA:
		fprintf(ਧਾਰਾ, "list-style-type: lower-alpha ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_LIST_STYLE_TYPE_ਉੱਤਲਾ_ALPHA:
		fprintf(ਧਾਰਾ, "list-style-type: upper-alpha ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_LIST_STYLE_TYPE_NONE:
		fprintf(ਧਾਰਾ, "list-style-type: none ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* margin-top */
	val = css_computed_margin_ਸਿਖਰ(style, &len1, &unit1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_MARGIN_AUTO:
		fprintf(ਧਾਰਾ, "margin-top: auto ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_MARGIN_ਸੈੱਟ:
		fprintf(ਧਾਰਾ, "margin-top: ");

		dump_css_unit(ਧਾਰਾ, len1, unit1);

		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* margin-right */
	val = css_computed_margin_ਸੱਜਾ(style, &len1, &unit1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_MARGIN_AUTO:
		fprintf(ਧਾਰਾ, "margin-right: auto ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_MARGIN_ਸੈੱਟ:
		fprintf(ਧਾਰਾ, "margin-right: ");

		dump_css_unit(ਧਾਰਾ, len1, unit1);

		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* margin-bottom */
	val = css_computed_margin_bottom(style, &len1, &unit1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_MARGIN_AUTO:
		fprintf(ਧਾਰਾ, "margin-bottom: auto ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_MARGIN_ਸੈੱਟ:
		fprintf(ਧਾਰਾ, "margin-bottom: ");

		dump_css_unit(ਧਾਰਾ, len1, unit1);

		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* margin-left */
	val = css_computed_margin_ਖੱਬਾ(style, &len1, &unit1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_MARGIN_AUTO:
		fprintf(ਧਾਰਾ, "margin-left: auto ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_MARGIN_ਸੈੱਟ:
		fprintf(ਧਾਰਾ, "margin-left: ");

		dump_css_unit(ਧਾਰਾ, len1, unit1);

		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* max-height */
	val = css_computed_max_ਉਚਾਈ(style, &len1, &unit1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_MAX_ਉਚਾਈ_NONE:
		fprintf(ਧਾਰਾ, "max-height: none ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_MAX_ਉਚਾਈ_ਸੈੱਟ:
		fprintf(ਧਾਰਾ, "max-height: ");

		dump_css_unit(ਧਾਰਾ, len1, unit1);

		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* max-width */
	val = css_computed_max_ਚੌੜਾਈ(style, &len1, &unit1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_MAX_ਚੌੜਾਈ_NONE:
		fprintf(ਧਾਰਾ, "max-width: none ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_MAX_ਚੌੜਾਈ_ਸੈੱਟ:
		fprintf(ਧਾਰਾ, "max-width: ");

		dump_css_unit(ਧਾਰਾ, len1, unit1);

		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* min-height */
	val = ns_computed_min_ਉਚਾਈ(style, &len1, &unit1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_MIN_ਉਚਾਈ_ਸੈੱਟ:
		fprintf(ਧਾਰਾ, "min-height: ");

		dump_css_unit(ਧਾਰਾ, len1, unit1);

		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* min-width */
	val = ns_computed_min_ਚੌੜਾਈ(style, &len1, &unit1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_MIN_ਚੌੜਾਈ_ਸੈੱਟ:
		fprintf(ਧਾਰਾ, "min-width: ");

		dump_css_unit(ਧਾਰਾ, len1, unit1);

		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* opacity */
	val = css_computed_opacity(style, &len1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_OPACITY_ਸੈੱਟ:
		fprintf(ਧਾਰਾ, "opacity: ");

		dump_css_fixed(ਧਾਰਾ, len1);

		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* outline-color */
	val = css_computed_outline_color(style, &color);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_OUTLINE_COLOR_INVERT:
		fprintf(ਧਾਰਾ, "outline-color: invert ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_OUTLINE_COLOR_COLOR:
		fprintf(ਧਾਰਾ, "outline-color: #%08"PRIx32" ", color);
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* outline-style */
	val = css_computed_outline_style(style);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_OUTLINE_STYLE_NONE:
		fprintf(ਧਾਰਾ, "outline-style: none ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_OUTLINE_STYLE_DOTTED:
		fprintf(ਧਾਰਾ, "outline-style: dotted ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_OUTLINE_STYLE_DASHED:
		fprintf(ਧਾਰਾ, "outline-style: dashed ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_OUTLINE_STYLE_SOLID:
		fprintf(ਧਾਰਾ, "outline-style: solid ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_OUTLINE_STYLE_DOUBLE:
		fprintf(ਧਾਰਾ, "outline-style: double ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_OUTLINE_STYLE_GROOVE:
		fprintf(ਧਾਰਾ, "outline-style: groove ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_OUTLINE_STYLE_RIDGE:
		fprintf(ਧਾਰਾ, "outline-style: ridge ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_OUTLINE_STYLE_INSET:
		fprintf(ਧਾਰਾ, "outline-style: inset ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_OUTLINE_STYLE_OUTSET:
		fprintf(ਧਾਰਾ, "outline-style: outset ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* outline-width */
	val = css_computed_outline_ਚੌੜਾਈ(style, &len1, &unit1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_OUTLINE_ਚੌੜਾਈ_THIN:
		fprintf(ਧਾਰਾ, "outline-width: thin ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_OUTLINE_ਚੌੜਾਈ_MEDIUM:
		fprintf(ਧਾਰਾ, "outline-width: medium ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_OUTLINE_ਚੌੜਾਈ_THICK:
		fprintf(ਧਾਰਾ, "outline-width: thick ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_OUTLINE_ਚੌੜਾਈ_ਚੌੜਾਈ:
		fprintf(ਧਾਰਾ, "outline-width: ");

		dump_css_unit(ਧਾਰਾ, len1, unit1);

		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/*ਉਛਾਲ*/
	val = css_computed_ਉਛਾਲ_x(style);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_ਉਛਾਲ_VISIBLE:
		fprintf(ਧਾਰਾ, "overflow-x: visible ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_ਉਛਾਲ_HIDDEN:
		fprintf(ਧਾਰਾ, "overflow-x: hidden ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_ਉਛਾਲ_SCROLL:
		fprintf(ਧਾਰਾ, "overflow-x: scroll ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_ਉਛਾਲ_AUTO:
		fprintf(ਧਾਰਾ, "overflow-x auto ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/*ਉਛਾਲ*/
	val = css_computed_ਉਛਾਲ_y(style);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_ਉਛਾਲ_VISIBLE:
		fprintf(ਧਾਰਾ, "overflow-y: visible ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_ਉਛਾਲ_HIDDEN:
		fprintf(ਧਾਰਾ, "overflow-y: hidden ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_ਉਛਾਲ_SCROLL:
		fprintf(ਧਾਰਾ, "overflow-y: scroll ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_ਉਛਾਲ_AUTO:
		fprintf(ਧਾਰਾ, "overflow-y: auto ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* padding-top */
	val = css_computed_padding_ਸਿਖਰ(style, &len1, &unit1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_PADDING_ਸੈੱਟ:
		fprintf(ਧਾਰਾ, "padding-top: ");

		dump_css_unit(ਧਾਰਾ, len1, unit1);

		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* padding-right */
	val = css_computed_padding_ਸੱਜਾ(style, &len1, &unit1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_PADDING_ਸੈੱਟ:
		fprintf(ਧਾਰਾ, "padding-right: ");

		dump_css_unit(ਧਾਰਾ, len1, unit1);

		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* padding-bottom */
	val = css_computed_padding_bottom(style, &len1, &unit1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_PADDING_ਸੈੱਟ:
		fprintf(ਧਾਰਾ, "padding-bottom: ");

		dump_css_unit(ਧਾਰਾ, len1, unit1);

		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* padding-left */
	val = css_computed_padding_ਖੱਬਾ(style, &len1, &unit1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_PADDING_ਸੈੱਟ:
		fprintf(ਧਾਰਾ, "padding-left: ");

		dump_css_unit(ਧਾਰਾ, len1, unit1);

		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* position */
	val = css_computed_position(style);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_POSITION_STATIC:
		fprintf(ਧਾਰਾ, "position: static ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_POSITION_RELATIVE:
		fprintf(ਧਾਰਾ, "position: relative ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_POSITION_ABSOLUTE:
		fprintf(ਧਾਰਾ, "position: absolute ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_POSITION_FIXED:
		fprintf(ਧਾਰਾ, "position: fixed ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* quotes */
	val = css_computed_quotes(style, &string_list);
	ਜੇ (val == CSS_QUOTES_STRING && string_list != ਨਿਰਾ) {
		fprintf(ਧਾਰਾ, "quotes:");

		ਜਦੋਂ (*string_list != ਨਿਰਾ) {
			fprintf(ਧਾਰਾ, " \"%.*s\"",
				(ਪੂਰਨ) lwc_string_ਲੰਮਾਈ(*string_list),
				lwc_string_ਅੰਕੜੇ(*string_list));

			string_list++;
		}

		fprintf(ਧਾਰਾ, " ");
	} ਨਹੀਂਤਾਂ {
		ਚੋਣ (val) {
		ਹਾਲਤ CSS_QUOTES_NONE:
			fprintf(ਧਾਰਾ, "quotes: none ");
			ਰੋਕੋ;
		ਮੂਲ:
			ਰੋਕੋ;
		}
	}

	/*ਸੱਜਾ*/
	val = css_computed_ਸੱਜਾ(style, &len1, &unit1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_ਸੱਜਾ_AUTO:
		fprintf(ਧਾਰਾ, "right: auto ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_ਸੱਜਾ_ਸੈੱਟ:
		fprintf(ਧਾਰਾ, "right: ");

		dump_css_unit(ਧਾਰਾ, len1, unit1);

		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* table-layout */
	val = css_computed_ਸਾਰਣੀ_layout(style);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_ਸਾਰਣੀ_LAYOUT_AUTO:
		fprintf(ਧਾਰਾ, "table-layout: auto ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_ਸਾਰਣੀ_LAYOUT_FIXED:
		fprintf(ਧਾਰਾ, "table-layout: fixed ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* text-align */
	val = css_computed_ਪਾਠ_align(style);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_ਪਾਠ_ALIGN_ਖੱਬਾ:
		fprintf(ਧਾਰਾ, "text-align: left ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_ਪਾਠ_ALIGN_ਸੱਜਾ:
		fprintf(ਧਾਰਾ, "text-align: right ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_ਪਾਠ_ALIGN_CENTER:
		fprintf(ਧਾਰਾ, "text-align: center ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_ਪਾਠ_ALIGN_JUSTIFY:
		fprintf(ਧਾਰਾ, "text-align: justify ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_ਪਾਠ_ALIGN_DEFAULT:
		fprintf(ਧਾਰਾ, "text-align: default ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_ਪਾਠ_ALIGN_LIBCSS_ਖੱਬਾ:
		fprintf(ਧਾਰਾ, "text-align: -libcss-left ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_ਪਾਠ_ALIGN_LIBCSS_CENTER:
		fprintf(ਧਾਰਾ, "text-align: -libcss-center ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_ਪਾਠ_ALIGN_LIBCSS_ਸੱਜਾ:
		fprintf(ਧਾਰਾ, "text-align: -libcss-right ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* text-decoration */
	val = css_computed_ਪਾਠ_decoration(style);
	ਜੇ (val == CSS_ਪਾਠ_DECORATION_NONE) {
		fprintf(ਧਾਰਾ, "text-decoration: none ");
	} ਨਹੀਂਤਾਂ {
		fprintf(ਧਾਰਾ, "text-decoration:");

		ਜੇ (val & CSS_ਪਾਠ_DECORATION_BLINK) {
			fprintf(ਧਾਰਾ, " blink");
		}
		ਜੇ (val & CSS_ਪਾਠ_DECORATION_ਸਤਰ_THROUGH) {
			fprintf(ਧਾਰਾ, " line-through");
		}
		ਜੇ (val & CSS_ਪਾਠ_DECORATION_OVERLINE) {
			fprintf(ਧਾਰਾ, " overline");
		}
		ਜੇ (val & CSS_ਪਾਠ_DECORATION_UNDERLINE) {
			fprintf(ਧਾਰਾ, " underline");
		}

		fprintf(ਧਾਰਾ, " ");
	}

	/* text-indent */
	val = css_computed_ਪਾਠ_indent(style, &len1, &unit1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_ਪਾਠ_INDENT_ਸੈੱਟ:
		fprintf(ਧਾਰਾ, "text-indent: ");

		dump_css_unit(ਧਾਰਾ, len1, unit1);

		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* text-transform */
	val = css_computed_ਪਾਠ_transform(style);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_ਪਾਠ_TRANSFORM_CAPITALIZE:
		fprintf(ਧਾਰਾ, "text-transform: capitalize ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_ਪਾਠ_TRANSFORM_UPPERCASE:
		fprintf(ਧਾਰਾ, "text-transform: uppercase ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_ਪਾਠ_TRANSFORM_LOWERCASE:
		fprintf(ਧਾਰਾ, "text-transform: lowercase ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_ਪਾਠ_TRANSFORM_NONE:
		fprintf(ਧਾਰਾ, "text-transform: none ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/*ਸਿਖਰ*/
	val = css_computed_ਸਿਖਰ(style, &len1, &unit1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_ਸਿਖਰ_AUTO:
		fprintf(ਧਾਰਾ, "top: auto ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_ਸਿਖਰ_ਸੈੱਟ:
		fprintf(ਧਾਰਾ, "top: ");

		dump_css_unit(ਧਾਰਾ, len1, unit1);

		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* unicode-bidi */
	val = css_computed_unicode_bidi(style);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_UNICODE_BIDI_ਸਾਧਾਰਨ:
		fprintf(ਧਾਰਾ, "unicode-bidi: normal ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_UNICODE_BIDI_EMBED:
		fprintf(ਧਾਰਾ, "unicode-bidi: embed ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_UNICODE_BIDI_BIDI_OVERRIDE:
		fprintf(ਧਾਰਾ, "unicode-bidi: bidi-override ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* vertical-align */
	val = css_computed_vertical_align(style, &len1, &unit1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_VERTICAL_ALIGN_BASELINE:
		fprintf(ਧਾਰਾ, "vertical-align: baseline ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_VERTICAL_ALIGN_SUB:
		fprintf(ਧਾਰਾ, "vertical-align: sub ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_VERTICAL_ALIGN_SUPER:
		fprintf(ਧਾਰਾ, "vertical-align: super ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_VERTICAL_ALIGN_ਸਿਖਰ:
		fprintf(ਧਾਰਾ, "vertical-align: top ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_VERTICAL_ALIGN_ਪਾਠ_ਸਿਖਰ:
		fprintf(ਧਾਰਾ, "vertical-align: text-top ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_VERTICAL_ALIGN_ਵਿਚਕਾਰ:
		fprintf(ਧਾਰਾ, "vertical-align: middle ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_VERTICAL_ALIGN_BOTTOM:
		fprintf(ਧਾਰਾ, "vertical-align: bottom ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_VERTICAL_ALIGN_ਪਾਠ_BOTTOM:
		fprintf(ਧਾਰਾ, "vertical-align: text-bottom ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_VERTICAL_ALIGN_ਸੈੱਟ:
		fprintf(ਧਾਰਾ, "vertical-align: ");

		dump_css_unit(ਧਾਰਾ, len1, unit1);

		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* visibility */
	val = css_computed_visibility(style);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_VISIBILITY_VISIBLE:
		fprintf(ਧਾਰਾ, "visibility: visible ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_VISIBILITY_HIDDEN:
		fprintf(ਧਾਰਾ, "visibility: hidden ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_VISIBILITY_COLLAPSE:
		fprintf(ਧਾਰਾ, "visibility: collapse ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* white-space */
	val = css_computed_white_space(style);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_WHITE_SPACE_ਸਾਧਾਰਨ:
		fprintf(ਧਾਰਾ, "white-space: normal ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_WHITE_SPACE_PRE:
		fprintf(ਧਾਰਾ, "white-space: pre ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_WHITE_SPACE_NOWRAP:
		fprintf(ਧਾਰਾ, "white-space: nowrap ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_WHITE_SPACE_PRE_WRAP:
		fprintf(ਧਾਰਾ, "white-space: pre-wrap ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_WHITE_SPACE_PRE_ਸਤਰ:
		fprintf(ਧਾਰਾ, "white-space: pre-line ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/*ਚੌੜਾਈ*/
	val = css_computed_ਚੌੜਾਈ(style, &len1, &unit1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_ਚੌੜਾਈ_AUTO:
		fprintf(ਧਾਰਾ, "width: auto ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_ਚੌੜਾਈ_ਸੈੱਟ:
		fprintf(ਧਾਰਾ, "width: ");

		dump_css_unit(ਧਾਰਾ, len1, unit1);

		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* word-spacing */
	val = css_computed_ਸ਼ਬਦ_spacing(style, &len1, &unit1);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_ਸ਼ਬਦ_SPACING_ਸਾਧਾਰਨ:
		fprintf(ਧਾਰਾ, "word-spacing: normal ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_ਸ਼ਬਦ_SPACING_ਸੈੱਟ:
		fprintf(ਧਾਰਾ, "word-spacing: ");

		dump_css_unit(ਧਾਰਾ, len1, unit1);

		fprintf(ਧਾਰਾ, " ");
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* z-index */
	val = css_computed_z_ਸੂਚਕਾਂਕ(style, &zindex);
	ਚੋਣ (val) {
	ਹਾਲਤ CSS_Z_ਸੂਚਕਾਂਕ_AUTO:
		fprintf(ਧਾਰਾ, "z-index: auto ");
		ਰੋਕੋ;
	ਹਾਲਤ CSS_Z_ਸੂਚਕਾਂਕ_ਸੈੱਟ:
		fprintf(ਧਾਰਾ, "z-index: %"PRId32" ", zindex);
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	fprintf(ਧਾਰਾ, "}");
}
