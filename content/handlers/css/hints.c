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

#ضمن <string.h>
#ضمن <strings.h>

#ضمن "utils/nsoption.h"
#ضمن "utils/corestrings.h"
#ضمن "utils/log.h"
#ضمن "utils/nsurl.h"
#ضمن "utils/utils.h"

#ضمن "css/hints.h"
#ضمن "css/select.h"

#تعریف_کن LOG_STATS
#لغو_تعریف LOG_STATS

/******************************************************************************
 * Utility functions                                                          *
 ******************************************************************************/

/**
 * Determine if a given character is whitespace
 *
 * \param c  Character to consider
 * \return true if character is whitespace, false otherwise
 */
ساكن bool isWhitespace(حرف c)
{
	أرجع c == ' ' || c == '\t' || c == '\f' || c == '\r' || c == '\n';
}

/**
 * Determine if a given character is a valid hex digit
 *
 * \param c  Character to consider
 * \return true if character is a valid hex digit, false otherwise
 */
ساكن bool isHex(حرف c)
{
	أرجع ('0' <= c && c <= '9') ||
			('A' <= (c & ~0x20) && (c & ~0x20) <= 'F');
}

/**
 * Convert a character representing a hex digit to the corresponding hex value
 *
 * \param c  Character to convert
 * \return Hex value represented by character
 *
 * \note This function assumes an ASCII-compatible character set
 */
ساكن uint8_t char_مقصد_Hex(حرف c)
{
	/* 0-9 */
	c -= '0';

	/* A-F */
	إذا (c > 9)
		c -= 'A' - '9' - 1;

	/* a-f */
	إذا (c > 15)
		c -= 'a' - 'A';

	أرجع c;
}


/******************************************************************************
 * Common parsing functions                                                   *
 ******************************************************************************/

/**
 * Parse a number string
 *
 * \param data  Data to parse (NUL-terminated)
 * \param maybe_negative  Negative numbers permitted
 * \param real            Floating point numbers permitted
 * \param value           Pointer to location to receive numeric value
 * \param consumed        Pointer to location to receive number of input
 *                        bytes consumed
 * \return true on success, false on invalid input
 */
ساكن bool parse_عدد(ثابت حرف *داده, bool maybe_negative, bool real,
		css_fixed *مقدار, size_t *consumed)
{
	size_t len;
	ثابت uint8_t *ptr;
	int32_t intpart = 0;
	int32_t fracpart = 0;
	int32_t pwr = 1;
	صحيح sign = 1;

	*consumed = 0;

	len = طول_رشته(داده);
	ptr = (ثابت uint8_t *) داده;

	إذا (len == 0)
		أرجع false;

	/* Skip leading whitespace */
	طالما (len > 0 && isWhitespace(ptr[0])) {
		len--;
		ptr++;
	}

	إذا (len == 0)
		أرجع false;

	/* Extract sign, if any */
	إذا (ptr[0] == '+') {
		len--;
		ptr++;
	} وإلا إذا (ptr[0] == '-' && maybe_negative) {
		sign = -1;
		len--;
		ptr++;
	}

	إذا (len == 0)
		أرجع false;

	/* Must have a digit [0,9] */
	إذا ('0' > ptr[0] || ptr[0] > '9')
		أرجع false;

	/* Now extract intpart, assuming base 10 */
	طالما (len > 0) {
		/* Stop on first non-digit */
		إذا (ptr[0] < '0' || '9' < ptr[0])
			اكسر;

		/* Prevent overflow of 'intpart'; proper clamping below */
		إذا (intpart < (1 << 22)) {
			intpart *= 10;
			intpart += ptr[0] - '0';
		}
		ptr++;
		len--;
	}

	/* And fracpart, again, assuming base 10 */
	إذا (real && len > 1 && ptr[0] == '.' &&
			('0' <= ptr[1] && ptr[1] <= '9')) {
		ptr++;
		len--;

		طالما (len > 0) {
			إذا (ptr[0] < '0' || '9' < ptr[0])
				اكسر;

			إذا (pwr < 1000000) {
				pwr *= 10;
				fracpart *= 10;
				fracpart += ptr[0] - '0';
			}
			ptr++;
			len--;
		}

		fracpart = ((1 << 10) * fracpart + pwr/2) / pwr;
		إذا (fracpart >= (1 << 10)) {
			intpart++;
			fracpart &= (1 << 10) - 1;
		}
	}

	إذا (sign > 0) {
		/* If the result is larger than we can represent,
		 * then clamp to the maximum value we can store. */
		إذا (intpart >= (1 << 21)) {
			intpart = (1 << 21) - 1;
			fracpart = (1 << 10) - 1;
		}
	} وإلا {
		/* If the negated result is smaller than we can represent
		 * then clamp to the minimum value we can store. */
		إذا (intpart >= (1 << 21)) {
			intpart = -(1 << 21);
			fracpart = 0;
		} وإلا {
			intpart = -intpart;
			إذا (fracpart) {
				fracpart = (1 << 10) - fracpart;
				intpart--;
			}
		}
	}

	*مقدار = (intpart << 10) | fracpart;

	*consumed = ptr - (ثابت uint8_t *) داده;

	أرجع true;
}

/**
 * Parse a dimension string
 *
 * \param data    Data to parse (NUL-terminated)
 * \param strict  Whether to enforce strict parsing rules
 * \param length  Pointer to location to receive dimension's length
 * \param unit    Pointer to location to receive dimension's unit
 * \return true on success, false on invalid input
 */
ساكن bool parse_dimension(ثابت حرف *داده, bool strict, css_fixed *درازی,
		css_unit *unit)
{
	size_t len;
	size_t اقرأ;
	css_fixed مقدار;

	len = طول_رشته(داده);

	إذا (parse_عدد(داده, false, true, &مقدار, &اقرأ) == false)
		أرجع false;

	إذا (strict && مقدار < INTTOFIX(1))
		أرجع false;

	*درازی = مقدار;

	إذا (len > اقرأ && داده[اقرأ] == '%')
		*unit = CSS_UNIT_PCT;
	وإلا
		*unit = CSS_UNIT_PX;

	أرجع true;
}

/**
 * Mapping of colour name to CSS color
 */
بنية رنگ_map {
	ثابت حرف *name;
	css_color color;
};

/**
 * Name comparator for named colour matching
 *
 * \param a  Name to match
 * \param b  Colour map entry to consider
 * \return 0   on match,
 *         < 0 if a < b,
 *         > 0 if b > a.
 */
ساكن صحيح cmp_رنگ_name(ثابت فراغ *a, ثابت فراغ *b)
{
	ثابت حرف *aa = a;
	ثابت بنية رنگ_map *bb = b;

	أرجع strcasecmp(aa, bb->name);
}

/**
 * Parse a named colour
 *
 * \param name    Name to parse
 * \param result  Pointer to location to receive css_color
 * \return true on success, false on invalid input
 */
ساكن bool parse_named_رنگ(ثابت حرف *name, css_color *نتیجه)
{
	ساكن ثابت بنية رنگ_map named_colours[] = {
		{ "aliceblue",		0xfff0f8ff },
		{ "antiquewhite",	0xfffaebd7 },
		{ "aqua",		0xff00ffff },
		{ "aquamarine",		0xff7fffd4 },
		{ "azure",		0xfff0ffff },
		{ "beige",		0xfff5f5dc },
		{ "bisque",		0xffffe4c4 },
		{ "black",		0xff000000 },
		{ "blanchedalmond",	0xffffebcd },
		{ "blue",		0xff0000ff },
		{ "blueviolet",		0xff8a2be2 },
		{ "brown",		0xffa52a2a },
		{ "burlywood",		0xffdeb887 },
		{ "cadetblue",		0xff5f9ea0 },
		{ "chartreuse",		0xff7fff00 },
		{ "chocolate",		0xffd2691e },
		{ "coral",		0xffff7f50 },
		{ "cornflowerblue",	0xff6495ed },
		{ "cornsilk",		0xfffff8dc },
		{ "crimson",		0xffdc143c },
		{ "cyan",		0xff00ffff },
		{ "darkblue",		0xff00008b },
		{ "darkcyan",		0xff008b8b },
		{ "darkgoldenrod",	0xffb8860b },
		{ "darkgray",		0xffa9a9a9 },
		{ "darkgreen",		0xff006400 },
		{ "darkgrey",		0xffa9a9a9 },
		{ "darkkhaki",		0xffbdb76b },
		{ "darkmagenta",	0xff8b008b },
		{ "darkolivegreen",	0xff556b2f },
		{ "darkorange",		0xffff8c00 },
		{ "darkorchid",		0xff9932cc },
		{ "darkred",		0xff8b0000 },
		{ "darksalmon",		0xffe9967a },
		{ "darkseagreen",	0xff8fbc8f },
		{ "darkslateblue",	0xff483d8b },
		{ "darkslategray",	0xff2f4f4f },
		{ "darkslategrey",	0xff2f4f4f },
		{ "darkturquoise",	0xff00ced1 },
		{ "darkviolet",		0xff9400d3 },
		{ "deeppink",		0xffff1493 },
		{ "deepskyblue",	0xff00bfff },
		{ "dimgray",		0xff696969 },
		{ "dimgrey",		0xff696969 },
		{ "dodgerblue",		0xff1e90ff },
		{ "feldspar",		0xffd19275 },
		{ "firebrick",		0xffb22222 },
		{ "floralwhite",	0xfffffaf0 },
		{ "forestgreen",	0xff228b22 },
		{ "fuchsia",		0xffff00ff },
		{ "gainsboro",		0xffdcdcdc },
		{ "ghostwhite",		0xfff8f8ff },
		{ "gold",		0xffffd700 },
		{ "goldenrod",		0xffdaa520 },
		{ "gray",		0xff808080 },
		{ "green",		0xff008000 },
		{ "greenyellow",	0xffadff2f },
		{ "grey",		0xff808080 },
		{ "honeydew",		0xfff0fff0 },
		{ "hotpink",		0xffff69b4 },
		{ "indianred",		0xffcd5c5c },
		{ "indigo",		0xff4b0082 },
		{ "ivory",		0xfffffff0 },
		{ "khaki",		0xfff0e68c },
		{ "lavender",		0xffe6e6fa },
		{ "lavenderblush",	0xfffff0f5 },
		{ "lawngreen",		0xff7cfc00 },
		{ "lemonchiffon",	0xfffffacd },
		{ "lightblue",		0xffadd8e6 },
		{ "lightcoral",		0xfff08080 },
		{ "lightcyan",		0xffe0ffff },
		{ "lightgoldenrodyellow",	0xfffafad2 },
		{ "lightgray",		0xffd3d3d3 },
		{ "lightgreen",		0xff90ee90 },
		{ "lightgrey",		0xffd3d3d3 },
		{ "lightpink",		0xffffb6c1 },
		{ "lightsalmon",	0xffffa07a },
		{ "lightseagreen",	0xff20b2aa },
		{ "lightskyblue",	0xff87cefa },
		{ "lightslateblue",	0xff8470ff },
		{ "lightslategray",	0xff778899 },
		{ "lightslategrey",	0xff778899 },
		{ "lightsteelblue",	0xffb0c4de },
		{ "lightyellow",	0xffffffe0 },
		{ "lime",		0xff00ff00 },
		{ "limegreen",		0xff32cd32 },
		{ "linen",		0xfffaf0e6 },
		{ "magenta",		0xffff00ff },
		{ "maroon",		0xff800000 },
		{ "mediumaquamarine",	0xff66cdaa },
		{ "mediumblue",		0xff0000cd },
		{ "mediumorchid",	0xffba55d3 },
		{ "mediumpurple",	0xff9370db },
		{ "mediumseagreen",	0xff3cb371 },
		{ "mediumslateblue",	0xff7b68ee },
		{ "mediumspringgreen",	0xff00fa9a },
		{ "mediumturquoise",	0xff48d1cc },
		{ "mediumvioletred",	0xffc71585 },
		{ "midnightblue",	0xff191970 },
		{ "mintcream",		0xfff5fffa },
		{ "mistyrose",		0xffffe4e1 },
		{ "moccasin",		0xffffe4b5 },
		{ "navajowhite",	0xffffdead },
		{ "navy",		0xff000080 },
		{ "oldlace",		0xfffdf5e6 },
		{ "olive",		0xff808000 },
		{ "olivedrab",		0xff6b8e23 },
		{ "orange",		0xffffa500 },
		{ "orangered",		0xffff4500 },
		{ "orchid",		0xffda70d6 },
		{ "palegoldenrod",	0xffeee8aa },
		{ "palegreen",		0xff98fb98 },
		{ "paleturquoise",	0xffafeeee },
		{ "palevioletred",	0xffdb7093 },
		{ "papayawhip",		0xffffefd5 },
		{ "peachpuff",		0xffffdab9 },
		{ "peru",		0xffcd853f },
		{ "pink",		0xffffc0cb },
		{ "plum",		0xffdda0dd },
		{ "powderblue",		0xffb0e0e6 },
		{ "purple",		0xff800080 },
		{ "red",		0xffff0000 },
		{ "rosybrown",		0xffbc8f8f },
		{ "royalblue",		0xff4169e1 },
		{ "saddlebrown",	0xff8b4513 },
		{ "salmon",		0xfffa8072 },
		{ "sandybrown",		0xfff4a460 },
		{ "seagreen",		0xff2e8b57 },
		{ "seashell",		0xfffff5ee },
		{ "sienna",		0xffa0522d },
		{ "silver",		0xffc0c0c0 },
		{ "skyblue",		0xff87ceeb },
		{ "slateblue",		0xff6a5acd },
		{ "slategray",		0xff708090 },
		{ "slategrey",		0xff708090 },
		{ "snow",		0xfffffafa },
		{ "springgreen",	0xff00ff7f },
		{ "steelblue",		0xff4682b4 },
		{ "tan",		0xffd2b48c },
		{ "teal",		0xff008080 },
		{ "thistle",		0xffd8bfd8 },
		{ "tomato",		0xffff6347 },
		{ "turquoise",		0xff40e0d0 },
		{ "violet",		0xffee82ee },
		{ "violetred",		0xffd02090 },
		{ "wheat",		0xfff5deb3 },
		{ "white",		0xffffffff },
		{ "whitesmoke",		0xfff5f5f5 },
		{ "yellow",		0xffffff00 },
		{ "yellowgreen",	0xff9acd32 }
	};
	ثابت بنية رنگ_map *مدخل;

	مدخل = bsearch(name, named_colours,
			حجم(named_colours) / حجم(named_colours[0]),
			حجم(named_colours[0]),
			cmp_رنگ_name);

	إذا (مدخل != عدم)
		*نتیجه = مدخل->color;

	أرجع مدخل != عدم;
}

/* exported interface documented in content/handlers/css/hints.h */
bool nscss_parse_رنگ(ثابت حرف *داده, css_color *نتیجه)
{
	size_t len = طول_رشته(داده);
	uint8_t r, g, b;

	/* 2 */
	إذا (len == 0)
		أرجع false;

	/* 3 */
	إذا (len == SLEN("transparent") && strcasecmp(داده, "transparent") == 0)
		أرجع false;

	/* 4 */
	إذا (parse_named_رنگ(داده, نتیجه))
		أرجع true;

	/** \todo Implement HTML5's utterly insane legacy colour parsing */

	إذا (داده[0] == '#') {
		داده++;
		len--;
	}

	إذا (len == 3 && isHex(داده[0]) && isHex(داده[1]) && isHex(داده[2])) {
		r = char_مقصد_Hex(داده[0]);
		g = char_مقصد_Hex(داده[1]);
		b = char_مقصد_Hex(داده[2]);

		r |= (r << 4);
		g |= (g << 4);
		b |= (b << 4);

		*نتیجه = (0xff << 24) | (r << 16) | (g << 8) | b;

		أرجع true;
	} وإلا إذا (len == 6 && isHex(داده[0]) && isHex(داده[1]) &&
			isHex(داده[2]) && isHex(داده[3]) && isHex(داده[4]) &&
			isHex(داده[5])) {
		r = (char_مقصد_Hex(داده[0]) << 4) | char_مقصد_Hex(داده[1]);
		g = (char_مقصد_Hex(داده[2]) << 4) | char_مقصد_Hex(داده[3]);
		b = (char_مقصد_Hex(داده[4]) << 4) | char_مقصد_Hex(داده[5]);

		*نتیجه = (0xff << 24) | (r << 16) | (g << 8) | b;

		أرجع true;
	}

	أرجع false;
}

/**
 * Parse a font \@size attribute
 *
 * \param size  Data to parse (NUL-terminated)
 * \param val   Pointer to location to receive enum value
 * \param len   Pointer to location to receive length
 * \param unit  Pointer to location to receive unit
 * \return True on success, false on failure
 */
ساكن bool parse_font_اندازه(ثابت حرف *اندازه, uint8_t *val,
		css_fixed *len, css_unit *unit)
{
	ساكن ثابت uint8_t اندازه_map[] = {
		CSS_FONT_اندازه_XX_SMALL,
		CSS_FONT_اندازه_SMALL,
		CSS_FONT_اندازه_MEDIUM,
		CSS_FONT_اندازه_LARGE,
		CSS_FONT_اندازه_X_LARGE,
		CSS_FONT_اندازه_XX_LARGE,
		CSS_FONT_اندازه_DIMENSION	/* xxx-large (see below) */
	};

	ثابت حرف *p = اندازه;
	حرف نما;
	صحيح مقدار = 0;

	/* Skip whitespace */
	طالما (*p != '\0' && isWhitespace(*p))
		p++;

	نما = *p;

	/* Skip +/- */
	إذا (نما == '+' || نما == '-')
		p++;

	/* Need at least one digit */
	إذا (*p < '0' || *p > '9') {
		أرجع false;
	}

	/* Consume digits, computing value */
	طالما ('0' <= *p && *p <= '9') {
		مقدار = مقدار * 10 + (*p - '0');
		p++;
	}

	/* Resolve relative sizes */
	إذا (نما == '+')
		مقدار += 3;
	وإلا إذا (نما == '-')
		مقدار = 3 - مقدار;

	/* Clamp to range [1,7] */
	إذا (مقدار < 1)
		مقدار = 1;
	وإلا إذا (مقدار > 7)
		مقدار = 7;

	إذا (مقدار == 7) {
		/* Manufacture xxx-large */
	  *len = FDIV(FMUL(INTTOFIX(3), INTTOFIX(nsoption_int(font_اندازه))),
				F_10);
	} وإلا {
		/* Len is irrelevant */
		*len = 0;
	}

	*unit = CSS_UNIT_PT;
	*val = اندازه_map[مقدار - 1];

	أرجع true;
}


/******************************************************************************
 * Hint context management                                                    *
 ******************************************************************************/

#تعریف_کن MAX_HINTS_در_هر_ELEMENT 32

بنية css_hint_ctx {
	بنية css_hint *hints;
	uint32_t len;
};

بنية css_hint_ctx hint_ctx;

nserror css_hint_init(فراغ)
{
	hint_ctx.hints = حافظه_بده(حجم(بنية css_hint) *
			MAX_HINTS_در_هر_ELEMENT);
	إذا (hint_ctx.hints == عدم) {
		أرجع NSERROR_NOMEM;
	}

	أرجع NSERROR_OK;
}

فراغ css_hint_fini(فراغ)
{
	hint_ctx.len = 0;
	حرر(hint_ctx.hints);
}

ساكن فراغ css_hint_clean(فراغ)
{
	hint_ctx.len = 0;
}

ساكن مضمن فراغ css_hint_advance(بنية css_hint **hint)
{
	hint_ctx.len++;
	أكد(hint_ctx.len < MAX_HINTS_در_هر_ELEMENT);

	(*hint)++;
}

ساكن فراغ css_hint_get_hints(بنية css_hint **hints, uint32_t *nhints)
{
	*hints = hint_ctx.hints;
	*nhints = hint_ctx.len;
}


/******************************************************************************
 * Presentational hint handlers                                               *
 ******************************************************************************/

ساكن فراغ css_hint_جدول_cell_border_padding(
		nscss_select_ctx *ctx,
		dom_گره *گره)
{
	بنية css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	css_qname qs;
	dom_string *attr = عدم;
	dom_گره *tablenode = عدم;
	dom_exception exc;

	qs.ns = عدم;
	qs.name = lwc_string_ref(corestring_lwc_جدول);
	إذا (named_ancestor_گره(ctx, گره, &qs,
			(فراغ *)&tablenode) != CSS_OK) {
		/* Didn't find, or had error */
		lwc_string_unref(qs.name);
		أرجع;
	}
	lwc_string_unref(qs.name);

	إذا (tablenode == عدم) {
		أرجع;
	}
	/* No need to unref tablenode, named_ancestor_node does not
	 * return a reffed node to the CSS
	 */

	exc = dom_element_get_attribute(tablenode,
			corestring_dom_border, &attr);

	إذا (exc == DOM_NO_ERR && attr != عدم) {
		uint32_t hint_prop;
		css_hint_درازی hint_درازی;

		إذا (parse_dimension(
				dom_string_داده(attr), false,
						&hint_درازی.مقدار,
						&hint_درازی.unit) &&
				INTTOFIX(0) != hint_درازی.مقدار) {

			لكل (hint_prop = CSS_PROP_BORDER_بالاترین_STYLE;
			     hint_prop <= CSS_PROP_BORDER_چپ_STYLE;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->وضعیت = CSS_BORDER_STYLE_INSET;
				css_hint_advance(&hint);
			}

			لكل (hint_prop = CSS_PROP_BORDER_بالاترین_عرض;
			     hint_prop <= CSS_PROP_BORDER_چپ_عرض;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->داده.درازی.مقدار = INTTOFIX(1);
				hint->داده.درازی.unit = CSS_UNIT_PX;
				hint->وضعیت = CSS_BORDER_عرض_عرض;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(tablenode,
			corestring_dom_bordercolor, &attr);

	إذا (exc == DOM_NO_ERR && attr != عدم) {
		uint32_t hint_prop;
		css_color hint_color;

		إذا (nscss_parse_رنگ(
				(ثابت حرف *)dom_string_داده(attr),
				&hint_color)) {

			لكل (hint_prop = CSS_PROP_BORDER_بالاترین_COLOR;
			     hint_prop <= CSS_PROP_BORDER_چپ_COLOR;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->داده.color = hint_color;
				hint->وضعیت = CSS_BORDER_COLOR_COLOR;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(tablenode,
			corestring_dom_cellpadding, &attr);

	إذا (exc == DOM_NO_ERR && attr != عدم) {
		uint32_t hint_prop;
		css_hint_درازی hint_درازی;

		إذا (parse_dimension(
				dom_string_داده(attr), false,
						&hint_درازی.مقدار,
						&hint_درازی.unit)) {

			لكل (hint_prop = CSS_PROP_PADDING_بالاترین;
			     hint_prop <= CSS_PROP_PADDING_چپ;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->داده.درازی.مقدار = hint_درازی.مقدار;
				hint->داده.درازی.unit = hint_درازی.unit;
				hint->وضعیت = CSS_PADDING_مجموعه;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}
}

ساكن فراغ css_hint_vertical_align_جدول_cells(
		nscss_select_ctx *ctx,
		dom_گره *گره)
{
	بنية css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = عدم;
	dom_exception err;

	err = dom_element_get_attribute(گره,
			corestring_dom_valign, &attr);

	إذا (err == DOM_NO_ERR && attr != عدم) {
		hint->داده.درازی.مقدار = 0;
		hint->داده.درازی.unit = CSS_UNIT_PX;
		إذا (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_بالاترین)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->وضعیت = CSS_VERTICAL_ALIGN_بالاترین;
			css_hint_advance(&hint);

		} وإلا إذا (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_میانه)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->وضعیت = CSS_VERTICAL_ALIGN_میانه;
			css_hint_advance(&hint);

		} وإلا إذا (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_bottom)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->وضعیت = CSS_VERTICAL_ALIGN_BOTTOM;
			css_hint_advance(&hint);

		} وإلا إذا (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_baseline)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->وضعیت = CSS_VERTICAL_ALIGN_BASELINE;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ساكن فراغ css_hint_vertical_align_replaced(
		nscss_select_ctx *ctx,
		dom_گره *گره)
{
	بنية css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = عدم;
	dom_exception err;

	err = dom_element_get_attribute(گره,
			corestring_dom_valign, &attr);

	إذا (err == DOM_NO_ERR && attr != عدم) {
		إذا (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_بالاترین)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->وضعیت = CSS_VERTICAL_ALIGN_بالاترین;
			css_hint_advance(&hint);

		} وإلا إذا (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_bottom) ||
			   dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_baseline)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->وضعیت = CSS_VERTICAL_ALIGN_BASELINE;
			css_hint_advance(&hint);

		} وإلا إذا (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_texttop)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->وضعیت = CSS_VERTICAL_ALIGN_متن_بالاترین;
			css_hint_advance(&hint);

		} وإلا إذا (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_absmiddle) ||
			   dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_abscenter)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->وضعیت = CSS_VERTICAL_ALIGN_میانه;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ساكن فراغ css_hint_متن_align_نرمال(
		nscss_select_ctx *ctx,
		dom_گره *گره)
{
	بنية css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *align = عدم;
	dom_exception err;

	err = dom_element_get_attribute(گره,
			corestring_dom_align, &align);
	إذا (err == DOM_NO_ERR && align != عدم) {
		إذا (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_چپ)) {
			hint->prop = CSS_PROP_متن_ALIGN;
			hint->وضعیت = CSS_متن_ALIGN_چپ;
			css_hint_advance(&hint);

		} وإلا إذا (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_center)) {
			hint->prop = CSS_PROP_متن_ALIGN;
			hint->وضعیت = CSS_متن_ALIGN_CENTER;
			css_hint_advance(&hint);

		} وإلا إذا (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_راست)) {
			hint->prop = CSS_PROP_متن_ALIGN;
			hint->وضعیت = CSS_متن_ALIGN_راست;
			css_hint_advance(&hint);

		} وإلا إذا (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_justify)) {
			hint->prop = CSS_PROP_متن_ALIGN;
			hint->وضعیت = CSS_متن_ALIGN_JUSTIFY;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

ساكن فراغ css_hint_متن_align_center(
		nscss_select_ctx *ctx,
		dom_گره *گره)
{
	بنية css_hint *hint = &hint_ctx.hints[hint_ctx.len];

	hint->prop = CSS_PROP_متن_ALIGN;
	hint->وضعیت = CSS_متن_ALIGN_LIBCSS_CENTER;
	css_hint_advance(&hint);
}

ساكن فراغ css_hint_margin_چپ_راست_align_center(
		nscss_select_ctx *ctx,
		dom_گره *گره)
{
	بنية css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr;
	dom_exception exc;

	exc = dom_element_get_attribute(گره,
			corestring_dom_align, &attr);

	إذا (exc == DOM_NO_ERR && attr != عدم) {
		memset(hint, 0, حجم(*hint) * 2);
		إذا (dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_center) ||
				dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_abscenter) ||
				dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_میانه) ||
				dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_absmiddle)) {
			hint->prop = CSS_PROP_MARGIN_چپ;
			hint->وضعیت = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_راست;
			hint->وضعیت = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ساكن فراغ css_hint_متن_align_special(
		nscss_select_ctx *ctx,
		dom_گره *گره)
{
	بنية css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *align = عدم;
	dom_exception err;

	err = dom_element_get_attribute(گره,
			corestring_dom_align, &align);

	إذا (err == DOM_NO_ERR && align != عدم) {
		إذا (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_center)) {
			hint->prop = CSS_PROP_متن_ALIGN;
			hint->وضعیت = CSS_متن_ALIGN_LIBCSS_CENTER;
			css_hint_advance(&hint);

		} وإلا إذا (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_چپ)) {
			hint->prop = CSS_PROP_متن_ALIGN;
			hint->وضعیت = CSS_متن_ALIGN_LIBCSS_چپ;
			css_hint_advance(&hint);

		} وإلا إذا (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_راست)) {
			hint->prop = CSS_PROP_متن_ALIGN;
			hint->وضعیت = CSS_متن_ALIGN_LIBCSS_راست;
			css_hint_advance(&hint);

		} وإلا إذا (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_justify)) {
			hint->prop = CSS_PROP_متن_ALIGN;
			hint->وضعیت = CSS_متن_ALIGN_JUSTIFY;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

ساكن فراغ css_hint_متن_align_جدول_special(
		nscss_select_ctx *ctx,
		dom_گره *گره)
{
	بنية css_hint *hint = &hint_ctx.hints[hint_ctx.len];

	hint->prop = CSS_PROP_متن_ALIGN;
	hint->وضعیت = CSS_متن_ALIGN_INHERIT_IF_NON_MAGIC;
	css_hint_advance(&hint);
}

ساكن فراغ css_hint_margin_hspace_vspace(
		nscss_select_ctx *ctx,
		dom_گره *گره)
{
	بنية css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = عدم;
	dom_exception exc;

	exc = dom_element_get_attribute(گره,
			corestring_dom_vspace, &attr);

	إذا (exc == DOM_NO_ERR && attr != عدم) {
		css_hint_درازی hint_درازی;
		إذا (parse_dimension(
				dom_string_داده(attr), false,
				&hint_درازی.مقدار,
				&hint_درازی.unit)) {
			hint->prop = CSS_PROP_MARGIN_بالاترین;
			hint->داده.درازی.مقدار = hint_درازی.مقدار;
			hint->داده.درازی.unit = hint_درازی.unit;
			hint->وضعیت = CSS_MARGIN_مجموعه;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_BOTTOM;
			hint->داده.درازی.مقدار = hint_درازی.مقدار;
			hint->داده.درازی.unit = hint_درازی.unit;
			hint->وضعیت = CSS_MARGIN_مجموعه;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(گره,
			corestring_dom_hspace, &attr);

	إذا (exc == DOM_NO_ERR && attr != عدم) {
		css_hint_درازی hint_درازی;
		إذا (parse_dimension(
				dom_string_داده(attr), false,
				&hint_درازی.مقدار,
				&hint_درازی.unit)) {
			hint->prop = CSS_PROP_MARGIN_چپ;
			hint->داده.درازی.مقدار = hint_درازی.مقدار;
			hint->داده.درازی.unit = hint_درازی.unit;
			hint->وضعیت = CSS_MARGIN_مجموعه;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_راست;
			hint->داده.درازی.مقدار = hint_درازی.مقدار;
			hint->داده.درازی.unit = hint_درازی.unit;
			hint->وضعیت = CSS_MARGIN_مجموعه;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ساكن فراغ css_hint_margin_چپ_راست_hr(
		nscss_select_ctx *ctx,
		dom_گره *گره)
{
	بنية css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr;
	dom_exception exc;

	exc = dom_element_get_attribute(گره,
				corestring_dom_align, &attr);

	إذا (exc == DOM_NO_ERR && attr != عدم) {
		memset(hint, 0, حجم(*hint) * 2);
		إذا (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_چپ)) {
			hint->prop = CSS_PROP_MARGIN_چپ;
			hint->داده.درازی.مقدار = 0;
			hint->داده.درازی.unit = CSS_UNIT_PX;
			hint->وضعیت = CSS_MARGIN_مجموعه;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_راست;
			hint->وضعیت = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

		} وإلا إذا (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_center)) {
			hint->prop = CSS_PROP_MARGIN_چپ;
			hint->وضعیت = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_راست;
			hint->وضعیت = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

		} وإلا إذا (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_راست)) {
			hint->prop = CSS_PROP_MARGIN_چپ;
			hint->وضعیت = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_راست;
			hint->داده.درازی.مقدار = 0;
			hint->داده.درازی.unit = CSS_UNIT_PX;
			hint->وضعیت = CSS_MARGIN_مجموعه;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ساكن فراغ css_hint_جدول_spacing_border(
		nscss_select_ctx *ctx,
		dom_گره *گره)
{
	بنية css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception exc;
	dom_string *attr = عدم;

	exc = dom_element_get_attribute(گره, corestring_dom_border, &attr);

	إذا (exc == DOM_NO_ERR && attr != عدم) {
		uint32_t hint_prop;
		css_hint_درازی hint_درازی;

		لكل (hint_prop = CSS_PROP_BORDER_بالاترین_STYLE;
			     hint_prop <= CSS_PROP_BORDER_چپ_STYLE;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->وضعیت = CSS_BORDER_STYLE_OUTSET;
				css_hint_advance(&hint);
		}

		إذا (parse_dimension(
				dom_string_داده(attr), false,
						&hint_درازی.مقدار,
						&hint_درازی.unit)) {

			لكل (hint_prop = CSS_PROP_BORDER_بالاترین_عرض;
			     hint_prop <= CSS_PROP_BORDER_چپ_عرض;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->داده.درازی.مقدار = hint_درازی.مقدار;
				hint->داده.درازی.unit = hint_درازی.unit;
				hint->وضعیت = CSS_BORDER_عرض_عرض;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(گره,
			corestring_dom_bordercolor, &attr);

	إذا (exc == DOM_NO_ERR && attr != عدم) {
		uint32_t hint_prop;
		css_color hint_color;

		إذا (nscss_parse_رنگ(
				(ثابت حرف *)dom_string_داده(attr),
				&hint_color)) {

			لكل (hint_prop = CSS_PROP_BORDER_بالاترین_COLOR;
			     hint_prop <= CSS_PROP_BORDER_چپ_COLOR;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->داده.color = hint_color;
				hint->وضعیت = CSS_BORDER_COLOR_COLOR;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(گره,
			corestring_dom_cellspacing, &attr);

	إذا (exc == DOM_NO_ERR && attr != عدم) {
		إذا (parse_dimension(
				(ثابت حرف *)dom_string_داده(attr), false,
						&hint->داده.position.h.مقدار,
						&hint->داده.position.h.unit)) {
			hint->prop = CSS_PROP_BORDER_SPACING;
			hint->داده.position.v = hint->داده.position.h;
			hint->وضعیت = CSS_BORDER_SPACING_مجموعه;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ساكن فراغ css_hint_ارتفاع(
		nscss_select_ctx *ctx,
		dom_گره *گره)
{
	بنية css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = عدم;
	dom_exception err;

	err = dom_element_get_attribute(گره,
			corestring_dom_ارتفاع, &attr);

	إذا (err == DOM_NO_ERR && attr != عدم) {
		إذا (parse_dimension(
				(ثابت حرف *)dom_string_داده(attr), false,
				&hint->داده.درازی.مقدار,
				&hint->داده.درازی.unit)) {
			hint->prop = CSS_PROP_ارتفاع;
			hint->وضعیت = CSS_ارتفاع_مجموعه;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ساكن فراغ css_hint_عرض(
		nscss_select_ctx *ctx,
		dom_گره *گره)
{
	بنية css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = عدم;
	dom_exception err;

	err = dom_element_get_attribute(گره,
			corestring_dom_عرض, &attr);

	إذا (err == DOM_NO_ERR && attr != عدم) {
		إذا (parse_dimension(
				(ثابت حرف *)dom_string_داده(attr), false,
				&hint->داده.درازی.مقدار,
				&hint->داده.درازی.unit)) {
			hint->prop = CSS_PROP_عرض;
			hint->وضعیت = CSS_عرض_مجموعه;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ساكن فراغ css_hint_ارتفاع_عرض_textarea(
		nscss_select_ctx *ctx,
		dom_گره *گره)
{
	بنية css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = عدم;
	dom_exception err;

	err = dom_element_get_attribute(گره,
			corestring_dom_ردیفها, &attr);

	إذا (err == DOM_NO_ERR && attr != عدم) {
		إذا (parse_dimension(
				(ثابت حرف *)dom_string_داده(attr), false,
				&hint->داده.درازی.مقدار,
				&hint->داده.درازی.unit)) {
			hint->prop = CSS_PROP_ارتفاع;
			hint->داده.درازی.unit = CSS_UNIT_EM;
			hint->وضعیت = CSS_ارتفاع_مجموعه;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}

	err = dom_element_get_attribute(گره,
			corestring_dom_تعداد_ستون, &attr);

	إذا (err == DOM_NO_ERR && attr != عدم) {
		إذا (parse_dimension(
				(ثابت حرف *)dom_string_داده(attr), false,
				&hint->داده.درازی.مقدار,
				&hint->داده.درازی.unit)) {
			hint->prop = CSS_PROP_عرض;
			hint->داده.درازی.unit = CSS_UNIT_EX;
			hint->وضعیت = CSS_عرض_مجموعه;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ساكن فراغ css_hint_ارتفاع_عرض_canvas(
		nscss_select_ctx *ctx,
		dom_گره *گره)
{
	بنية css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = عدم;
	dom_exception err;
	bool مجموعه_dim = false;

	err = dom_element_get_attribute(گره,
			corestring_dom_ارتفاع, &attr);

	إذا (err == DOM_NO_ERR && attr != عدم) {
		إذا (parse_dimension(
				(ثابت حرف *)dom_string_داده(attr), true,
				&hint->داده.درازی.مقدار,
				&hint->داده.درازی.unit)) {
			hint->prop = CSS_PROP_ارتفاع;
			hint->داده.درازی.unit = CSS_UNIT_PX;
			hint->وضعیت = CSS_ارتفاع_مجموعه;
			css_hint_advance(&hint);
			مجموعه_dim = true;
		}
		dom_string_unref(attr);
	}
	إذا (مجموعه_dim == false) {
		/* canvas defaults to 150px tall */
		hint->prop = CSS_PROP_ارتفاع;
		hint->داده.درازی.unit = CSS_UNIT_PX;
		hint->داده.درازی.مقدار = INTTOFIX(150);
		hint->وضعیت = CSS_ارتفاع_مجموعه;
		css_hint_advance(&hint);
	} وإلا {
		مجموعه_dim = false;
	}

	err = dom_element_get_attribute(گره,
			corestring_dom_عرض, &attr);

	إذا (err == DOM_NO_ERR && attr != عدم) {
		إذا (parse_dimension(
				(ثابت حرف *)dom_string_داده(attr), true,
				&hint->داده.درازی.مقدار,
				&hint->داده.درازی.unit)) {
			hint->prop = CSS_PROP_عرض;
			hint->داده.درازی.unit = CSS_UNIT_PX;
			hint->وضعیت = CSS_عرض_مجموعه;
			css_hint_advance(&hint);
			مجموعه_dim = true;
		}
		dom_string_unref(attr);
	}
	إذا (مجموعه_dim == false) {
		/* canvas defaults to 300px wide */
		hint->prop = CSS_PROP_عرض;
		hint->داده.درازی.unit = CSS_UNIT_PX;
		hint->داده.درازی.مقدار = INTTOFIX(300);
		hint->وضعیت = CSS_عرض_مجموعه;
		css_hint_advance(&hint);
	}
}

ساكن فراغ css_hint_عرض_درون_داد(
		nscss_select_ctx *ctx,
		dom_گره *گره)
{
	بنية css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_string *attr = عدم;
	dom_exception err;

	err = dom_element_get_attribute(گره,
			corestring_dom_اندازه, &attr);

	إذا (err == DOM_NO_ERR && attr != عدم) {
		إذا (parse_dimension(
				(ثابت حرف *)dom_string_داده(attr), false,
				&hint->داده.درازی.مقدار,
				&hint->داده.درازی.unit)) {
			dom_string *attr2 = عدم;

			err = dom_element_get_attribute(گره,
					corestring_dom_type, &attr2);
			إذا (err == DOM_NO_ERR) {

				hint->prop = CSS_PROP_عرض;
				hint->وضعیت = CSS_عرض_مجموعه;

				إذا (attr2 == عدم ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_متن) ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_جستجو) ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_password) ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_file)) {
					hint->داده.درازی.unit = CSS_UNIT_EX;
				}
				إذا (attr2 != عدم) {
					dom_string_unref(attr2);
				}
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}
}

ساكن فراغ css_hint_anchor_color(
		nscss_select_ctx *ctx,
		dom_گره *گره)
{
	بنية css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	css_error error;
	dom_exception err;
	dom_string *color;
	dom_گره *bodynode = عدم;

	/* find body node */
	css_qname qs;
	bool is_دیدهشده;

	qs.ns = عدم;
	qs.name = lwc_string_ref(corestring_lwc_body);
	إذا (named_ancestor_گره(ctx, گره, &qs,
			(فراغ *)&bodynode) != CSS_OK) {
		/* Didn't find, or had error */
		lwc_string_unref(qs.name);
		أرجع ;
	}
	lwc_string_unref(qs.name);

	إذا (bodynode == عدم) {
		أرجع;
	}

	error = گره_is_دیدهشده(ctx, گره, &is_دیدهشده);
	إذا (error != CSS_OK)
		أرجع;

	إذا (is_دیدهشده) {
		err = dom_element_get_attribute(bodynode,
				corestring_dom_vlink, &color);
	} وإلا {
		err = dom_element_get_attribute(bodynode,
				corestring_dom_link, &color);
	}

	إذا (err == DOM_NO_ERR && color != عدم) {
		إذا (nscss_parse_رنگ(
				(ثابت حرف *)dom_string_داده(color),
						&hint->داده.color)) {
			hint->prop = CSS_PROP_COLOR;
			hint->وضعیت = CSS_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(color);
	}
}

ساكن فراغ css_hint_body_color(
		nscss_select_ctx *ctx,
		dom_گره *گره)
{
	بنية css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *color;

	err = dom_element_get_attribute(گره, corestring_dom_متن, &color);

	إذا (err == DOM_NO_ERR && color != عدم) {
		إذا (nscss_parse_رنگ(
				(ثابت حرف *)dom_string_داده(color),
						&hint->داده.color)) {
			hint->prop = CSS_PROP_COLOR;
			hint->وضعیت = CSS_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(color);
	}
}

ساكن فراغ css_hint_color(
		nscss_select_ctx *ctx,
		dom_گره *گره)
{
	بنية css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *color;

	err = dom_element_get_attribute(گره, corestring_dom_color, &color);

	إذا (err == DOM_NO_ERR && color != عدم) {
		إذا (nscss_parse_رنگ(
				(ثابت حرف *)dom_string_داده(color),
						&hint->داده.color)) {
			hint->prop = CSS_PROP_COLOR;
			hint->وضعیت = CSS_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(color);
	}
}

ساكن فراغ css_hint_font_اندازه(
		nscss_select_ctx *ctx,
		dom_گره *گره)
{
	بنية css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *اندازه;

	err = dom_element_get_attribute(گره, corestring_dom_اندازه, &اندازه);
	إذا (err == DOM_NO_ERR && اندازه != عدم) {
		إذا (parse_font_اندازه(
				(ثابت حرف *)dom_string_داده(اندازه),
						&hint->وضعیت,
						&hint->داده.درازی.مقدار,
						&hint->داده.درازی.unit)) {
			hint->prop = CSS_PROP_FONT_اندازه;
			css_hint_advance(&hint);
		}
		dom_string_unref(اندازه);
	}
}

ساكن فراغ css_hint_float(
		nscss_select_ctx *ctx,
		dom_گره *گره)
{
	بنية css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *align;

	err = dom_element_get_attribute(گره, corestring_dom_align, &align);
	إذا (err == DOM_NO_ERR && align != عدم) {
		إذا (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_چپ)) {
			hint->prop = CSS_PROP_FLOAT;
			hint->وضعیت = CSS_FLOAT_چپ;
			css_hint_advance(&hint);

		} وإلا إذا (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_راست)) {
			hint->prop = CSS_PROP_FLOAT;
			hint->وضعیت = CSS_FLOAT_راست;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

ساكن فراغ css_hint_caption_side(
		nscss_select_ctx *ctx,
		dom_گره *گره)
{
	بنية css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *align = عدم;

	err = dom_element_get_attribute(گره, corestring_dom_align, &align);
	إذا (err == DOM_NO_ERR && align != عدم) {
		إذا (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_bottom)) {
			hint->prop = CSS_PROP_CAPTION_SIDE;
			hint->وضعیت = CSS_CAPTION_SIDE_BOTTOM;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

ساكن فراغ css_hint_bg_color(
		nscss_select_ctx *ctx,
		dom_گره *گره)
{
	بنية css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *bgcolor;

	err = dom_element_get_attribute(گره,
			corestring_dom_bgcolor, &bgcolor);
	إذا (err == DOM_NO_ERR && bgcolor != عدم) {
		إذا (nscss_parse_رنگ(
				(ثابت حرف *)dom_string_داده(bgcolor),
				&hint->داده.color)) {
			hint->prop = CSS_PROP_BACKGROUND_COLOR;
			hint->وضعیت = CSS_BACKGROUND_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(bgcolor);
	}
}

ساكن فراغ css_hint_bg_image(
		nscss_select_ctx *ctx,
		dom_گره *گره)
{
	بنية css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_exception err;
	dom_string *attr;

	err = dom_element_get_attribute(گره,
			corestring_dom_background, &attr);
	إذا (err == DOM_NO_ERR && attr != عدم) {
		nsurl *url;
		nserror error = nsurl_join(ctx->پایه_url,
				(ثابت حرف *)dom_string_داده(attr), &url);
		dom_string_unref(attr);

		إذا (error == NSERROR_OK) {
			lwc_string *iurl;
			lwc_error lerror = lwc_intern_string(nsurl_access(url),
					nsurl_درازی(url), &iurl);
			nsurl_unref(url);

			إذا (lerror == lwc_error_ok) {
				hint->prop = CSS_PROP_BACKGROUND_IMAGE;
				hint->داده.string = iurl;
				hint->وضعیت = CSS_BACKGROUND_IMAGE_IMAGE;
				css_hint_advance(&hint);
			}
		}
	}
}

ساكن فراغ css_hint_white_space_nowrap(
		nscss_select_ctx *ctx,
		dom_گره *گره)
{
	بنية css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_exception err;
	bool nowrap;

	err = dom_element_has_attribute(گره, corestring_dom_nowrap, &nowrap);
	إذا (err == DOM_NO_ERR && nowrap == true) {
		hint->prop = CSS_PROP_WHITE_SPACE;
		hint->وضعیت = CSS_WHITE_SPACE_NOWRAP;
		css_hint_advance(&hint);
	}
}

ساكن فراغ css_hint_list(
		nscss_select_ctx *ctx,
		dom_گره *گره)
{
	بنية css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_exception err;
	dom_string *attr;

	err = dom_element_get_attribute(گره, corestring_dom_type, &attr);
	إذا (err == DOM_NO_ERR && attr != عدم) {
		ثابت حرف *attr_str = dom_string_داده(attr);
		size_t attr_len = dom_string_byte_درازی(attr);
		تعداد css_list_style_type_e type = CSS_LIST_STYLE_TYPE_INHERIT;

		إذا (attr_len == 1) {
			بدل (attr_str[0]) {
			حالة 'a':
				type = CSS_LIST_STYLE_TYPE_پایینی_ALPHA;
				اكسر;
			حالة 'A':
				type = CSS_LIST_STYLE_TYPE_فوقانی_ALPHA;
				اكسر;
			حالة 'i':
				type = CSS_LIST_STYLE_TYPE_پایینی_ROMAN;
				اكسر;
			حالة 'I':
				type = CSS_LIST_STYLE_TYPE_فوقانی_ROMAN;
				اكسر;
			حالة '1':
				type = CSS_LIST_STYLE_TYPE_DECIMAL;
				اكسر;
			}
		}

		إذا (type != CSS_LIST_STYLE_TYPE_INHERIT) {
			hint->prop = CSS_PROP_LIST_STYLE_TYPE;
			hint->وضعیت = type;
			css_hint_advance(&hint);
		}

		dom_string_unref(attr);
	}
}


/* Exported function, documented in css/hints.h */
css_error گره_presentational_hint(فراغ *pw, فراغ *گره,
		uint32_t *nhints, css_hint **hints)
{
	dom_exception exc;
	dom_html_element_type برچسب_type;

	إذا (nsoption_bool(author_level_css) == false) {
		*nhints = 0;
		*hints = عدم;
		أرجع CSS_OK;
	}

	css_hint_clean();

	exc = dom_html_element_get_برچسب_type(گره, &برچسب_type);
	إذا (exc != DOM_NO_ERR) {
		برچسب_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	بدل (برچسب_type) {
	حالة DOM_HTML_ELEMENT_TYPE_TH:
	حالة DOM_HTML_ELEMENT_TYPE_TD:
		css_hint_عرض(pw, گره);
		css_hint_جدول_cell_border_padding(pw, گره);
		css_hint_white_space_nowrap(pw, گره);
		fallthrough;
	حالة DOM_HTML_ELEMENT_TYPE_TR:
		css_hint_ارتفاع(pw, گره);
		fallthrough;
	حالة DOM_HTML_ELEMENT_TYPE_THEAD:
	حالة DOM_HTML_ELEMENT_TYPE_TBODY:
	حالة DOM_HTML_ELEMENT_TYPE_TFOOT:
		css_hint_متن_align_special(pw, گره);
		fallthrough;
	حالة DOM_HTML_ELEMENT_TYPE_ستون:
		css_hint_vertical_align_جدول_cells(pw, گره);
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_APPLET:
	حالة DOM_HTML_ELEMENT_TYPE_IMG:
		css_hint_margin_hspace_vspace(pw, گره);
		fallthrough;
	حالة DOM_HTML_ELEMENT_TYPE_EMBED:
	حالة DOM_HTML_ELEMENT_TYPE_IFRAME:
	حالة DOM_HTML_ELEMENT_TYPE_OBJECT:
		css_hint_ارتفاع(pw, گره);
		css_hint_عرض(pw, گره);
		css_hint_vertical_align_replaced(pw, گره);
		css_hint_float(pw, گره);
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_P:
	حالة DOM_HTML_ELEMENT_TYPE_H1:
	حالة DOM_HTML_ELEMENT_TYPE_H2:
	حالة DOM_HTML_ELEMENT_TYPE_H3:
	حالة DOM_HTML_ELEMENT_TYPE_H4:
	حالة DOM_HTML_ELEMENT_TYPE_H5:
	حالة DOM_HTML_ELEMENT_TYPE_H6:
		css_hint_متن_align_نرمال(pw, گره);
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_CENTER:
		css_hint_متن_align_center(pw, گره);
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_CAPTION:
		css_hint_caption_side(pw, گره);
		fallthrough;
	حالة DOM_HTML_ELEMENT_TYPE_DIV:
		css_hint_متن_align_special(pw, گره);
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_جدول:
		css_hint_متن_align_جدول_special(pw, گره);
		css_hint_جدول_spacing_border(pw, گره);
		css_hint_float(pw, گره);
		css_hint_margin_چپ_راست_align_center(pw, گره);
		css_hint_عرض(pw, گره);
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_HR:
		css_hint_عرض(pw, گره);
		css_hint_margin_چپ_راست_hr(pw, گره);
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		css_hint_ارتفاع_عرض_textarea(pw, گره);
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_درون_داد:
		css_hint_عرض_درون_داد(pw, گره);
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_A:
		css_hint_anchor_color(pw, گره);
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_FONT:
		css_hint_font_اندازه(pw, گره);
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_BODY:
		css_hint_body_color(pw, گره);
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_CANVAS:
		css_hint_ارتفاع_عرض_canvas(pw, گره);
		اكسر;
	حالة DOM_HTML_ELEMENT_TYPE_OL:
		css_hint_list(pw, گره);
		اكسر;
	افتراضي:
		اكسر;
	}

	إذا (برچسب_type != DOM_HTML_ELEMENT_TYPE__UNKNOWN) {
		css_hint_color(pw, گره);
		css_hint_bg_color(pw, گره);
		css_hint_bg_image(pw, گره);
	}

#اگر_تعریف_شده LOG_STATS
	NSLOG(netsurf, INFO, "Properties with hints: %i", hint_ctx.len);
#پایان_اگر

	css_hint_get_hints(hints, nhints);

	أرجع CSS_OK;
}
