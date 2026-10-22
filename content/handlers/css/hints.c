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

#شامل <string.h>
#شامل <strings.h>

#شامل "utils/nsoption.h"
#شامل "utils/corestrings.h"
#شامل "utils/log.h"
#شامل "utils/nsurl.h"
#شامل "utils/utils.h"

#شامل "css/hints.h"
#شامل "css/select.h"

#تعریف_کرو LOG_STATS
#تعریف_منسوخ LOG_STATS

/******************************************************************************
 * Utility functions                                                          *
 ******************************************************************************/

/**
 * Determine if a given character is whitespace
 *
 * \param c  Character to consider
 * \return true if character is whitespace, false otherwise
 */
ساکن bool isWhitespace(حرف c)
{
	واپس c == ' ' || c == '\t' || c == '\f' || c == '\r' || c == '\n';
}

/**
 * Determine if a given character is a valid hex digit
 *
 * \param c  Character to consider
 * \return true if character is a valid hex digit, false otherwise
 */
ساکن bool isHex(حرف c)
{
	واپس ('0' <= c && c <= '9') ||
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
ساکن uint8_t char_مقصد_Hex(حرف c)
{
	/* 0-9 */
	c -= '0';

	/* A-F */
	اگر (c > 9)
		c -= 'A' - '9' - 1;

	/* a-f */
	اگر (c > 15)
		c -= 'a' - 'A';

	واپس c;
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
ساکن bool parse_نمبر(ثابت حرف *ڈیٹا, bool maybe_negative, bool real,
		css_fixed *قدر, size_t *consumed)
{
	size_t len;
	ثابت uint8_t *ptr;
	int32_t intpart = 0;
	int32_t fracpart = 0;
	int32_t pwr = 1;
	صحیح sign = 1;

	*consumed = 0;

	len = لڑی_لمبائی(ڈیٹا);
	ptr = (ثابت uint8_t *) ڈیٹا;

	اگر (len == 0)
		واپس false;

	/* Skip leading whitespace */
	جب (len > 0 && isWhitespace(ptr[0])) {
		len--;
		ptr++;
	}

	اگر (len == 0)
		واپس false;

	/* Extract sign, if any */
	اگر (ptr[0] == '+') {
		len--;
		ptr++;
	} ورنہ اگر (ptr[0] == '-' && maybe_negative) {
		sign = -1;
		len--;
		ptr++;
	}

	اگر (len == 0)
		واپس false;

	/* Must have a digit [0,9] */
	اگر ('0' > ptr[0] || ptr[0] > '9')
		واپس false;

	/* Now extract intpart, assuming base 10 */
	جب (len > 0) {
		/* Stop on first non-digit */
		اگر (ptr[0] < '0' || '9' < ptr[0])
			توڑو;

		/* Prevent overflow of 'intpart'; proper clamping below */
		اگر (intpart < (1 << 22)) {
			intpart *= 10;
			intpart += ptr[0] - '0';
		}
		ptr++;
		len--;
	}

	/* And fracpart, again, assuming base 10 */
	اگر (real && len > 1 && ptr[0] == '.' &&
			('0' <= ptr[1] && ptr[1] <= '9')) {
		ptr++;
		len--;

		جب (len > 0) {
			اگر (ptr[0] < '0' || '9' < ptr[0])
				توڑو;

			اگر (pwr < 1000000) {
				pwr *= 10;
				fracpart *= 10;
				fracpart += ptr[0] - '0';
			}
			ptr++;
			len--;
		}

		fracpart = ((1 << 10) * fracpart + pwr/2) / pwr;
		اگر (fracpart >= (1 << 10)) {
			intpart++;
			fracpart &= (1 << 10) - 1;
		}
	}

	اگر (sign > 0) {
		/* If the result is larger than we can represent,
		 * then clamp to the maximum value we can store. */
		اگر (intpart >= (1 << 21)) {
			intpart = (1 << 21) - 1;
			fracpart = (1 << 10) - 1;
		}
	} ورنہ {
		/* If the negated result is smaller than we can represent
		 * then clamp to the minimum value we can store. */
		اگر (intpart >= (1 << 21)) {
			intpart = -(1 << 21);
			fracpart = 0;
		} ورنہ {
			intpart = -intpart;
			اگر (fracpart) {
				fracpart = (1 << 10) - fracpart;
				intpart--;
			}
		}
	}

	*قدر = (intpart << 10) | fracpart;

	*consumed = ptr - (ثابت uint8_t *) ڈیٹا;

	واپس true;
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
ساکن bool parse_dimension(ثابت حرف *ڈیٹا, bool strict, css_fixed *لمبائی,
		css_unit *unit)
{
	size_t len;
	size_t اقرأ;
	css_fixed قدر;

	len = لڑی_لمبائی(ڈیٹا);

	اگر (parse_نمبر(ڈیٹا, false, true, &قدر, &اقرأ) == false)
		واپس false;

	اگر (strict && قدر < INTTOFIX(1))
		واپس false;

	*لمبائی = قدر;

	اگر (len > اقرأ && ڈیٹا[اقرأ] == '%')
		*unit = CSS_UNIT_PCT;
	ورنہ
		*unit = CSS_UNIT_PX;

	واپس true;
}

/**
 * Mapping of colour name to CSS color
 */
ساخت رنگ_map {
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
ساکن صحیح cmp_رنگ_name(ثابت خالی *a, ثابت خالی *b)
{
	ثابت حرف *aa = a;
	ثابت ساخت رنگ_map *bb = b;

	واپس strcasecmp(aa, bb->name);
}

/**
 * Parse a named colour
 *
 * \param name    Name to parse
 * \param result  Pointer to location to receive css_color
 * \return true on success, false on invalid input
 */
ساکن bool parse_named_رنگ(ثابت حرف *name, css_color *نتیجہ)
{
	ساکن ثابت ساخت رنگ_map named_colours[] = {
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
	ثابت ساخت رنگ_map *اندراج_2;

	اندراج_2 = bsearch(name, named_colours,
			حجم(named_colours) / حجم(named_colours[0]),
			حجم(named_colours[0]),
			cmp_رنگ_name);

	اگر (اندراج_2 != تہی)
		*نتیجہ = اندراج_2->color;

	واپس اندراج_2 != تہی;
}

/* exported interface documented in content/handlers/css/hints.h */
bool nscss_parse_رنگ(ثابت حرف *ڈیٹا, css_color *نتیجہ)
{
	size_t len = لڑی_لمبائی(ڈیٹا);
	uint8_t r, g, b;

	/* 2 */
	اگر (len == 0)
		واپس false;

	/* 3 */
	اگر (len == SLEN("transparent") && strcasecmp(ڈیٹا, "transparent") == 0)
		واپس false;

	/* 4 */
	اگر (parse_named_رنگ(ڈیٹا, نتیجہ))
		واپس true;

	/** \todo Implement HTML5's utterly insane legacy colour parsing */

	اگر (ڈیٹا[0] == '#') {
		ڈیٹا++;
		len--;
	}

	اگر (len == 3 && isHex(ڈیٹا[0]) && isHex(ڈیٹا[1]) && isHex(ڈیٹا[2])) {
		r = char_مقصد_Hex(ڈیٹا[0]);
		g = char_مقصد_Hex(ڈیٹا[1]);
		b = char_مقصد_Hex(ڈیٹا[2]);

		r |= (r << 4);
		g |= (g << 4);
		b |= (b << 4);

		*نتیجہ = (0xff << 24) | (r << 16) | (g << 8) | b;

		واپس true;
	} ورنہ اگر (len == 6 && isHex(ڈیٹا[0]) && isHex(ڈیٹا[1]) &&
			isHex(ڈیٹا[2]) && isHex(ڈیٹا[3]) && isHex(ڈیٹا[4]) &&
			isHex(ڈیٹا[5])) {
		r = (char_مقصد_Hex(ڈیٹا[0]) << 4) | char_مقصد_Hex(ڈیٹا[1]);
		g = (char_مقصد_Hex(ڈیٹا[2]) << 4) | char_مقصد_Hex(ڈیٹا[3]);
		b = (char_مقصد_Hex(ڈیٹا[4]) << 4) | char_مقصد_Hex(ڈیٹا[5]);

		*نتیجہ = (0xff << 24) | (r << 16) | (g << 8) | b;

		واپس true;
	}

	واپس false;
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
ساکن bool parse_font_سائز(ثابت حرف *سائز, uint8_t *val,
		css_fixed *len, css_unit *unit)
{
	ساکن ثابت uint8_t سائز_map[] = {
		CSS_FONT_سائز_XX_SMALL,
		CSS_FONT_سائز_SMALL,
		CSS_FONT_سائز_MEDIUM,
		CSS_FONT_سائز_LARGE,
		CSS_FONT_سائز_X_LARGE,
		CSS_FONT_سائز_XX_LARGE,
		CSS_FONT_سائز_DIMENSION	/* xxx-large (see below) */
	};

	ثابت حرف *p = سائز;
	حرف منوال;
	صحیح قدر = 0;

	/* Skip whitespace */
	جب (*p != '\0' && isWhitespace(*p))
		p++;

	منوال = *p;

	/* Skip +/- */
	اگر (منوال == '+' || منوال == '-')
		p++;

	/* Need at least one digit */
	اگر (*p < '0' || *p > '9') {
		واپس false;
	}

	/* Consume digits, computing value */
	جب ('0' <= *p && *p <= '9') {
		قدر = قدر * 10 + (*p - '0');
		p++;
	}

	/* Resolve relative sizes */
	اگر (منوال == '+')
		قدر += 3;
	ورنہ اگر (منوال == '-')
		قدر = 3 - قدر;

	/* Clamp to range [1,7] */
	اگر (قدر < 1)
		قدر = 1;
	ورنہ اگر (قدر > 7)
		قدر = 7;

	اگر (قدر == 7) {
		/* Manufacture xxx-large */
	  *len = FDIV(FMUL(INTTOFIX(3), INTTOFIX(nsoption_int(font_سائز))),
				F_10);
	} ورنہ {
		/* Len is irrelevant */
		*len = 0;
	}

	*unit = CSS_UNIT_PT;
	*val = سائز_map[قدر - 1];

	واپس true;
}


/******************************************************************************
 * Hint context management                                                    *
 ******************************************************************************/

#تعریف_کرو MAX_HINTS_فی_ELEMENT 32

ساخت css_hint_ctx {
	ساخت css_hint *hints;
	uint32_t len;
};

ساخت css_hint_ctx hint_ctx;

nserror css_hint_init(خالی)
{
	hint_ctx.hints = حافظہ_دو(حجم(ساخت css_hint) *
			MAX_HINTS_فی_ELEMENT);
	اگر (hint_ctx.hints == تہی) {
		واپس NSERROR_NOMEM;
	}

	واپس NSERROR_OK;
}

خالی css_hint_fini(خالی)
{
	hint_ctx.len = 0;
	آزاد(hint_ctx.hints);
}

ساکن خالی css_hint_clean(خالی)
{
	hint_ctx.len = 0;
}

ساکن اندرونی خالی css_hint_advance(ساخت css_hint **hint)
{
	hint_ctx.len++;
	أكد(hint_ctx.len < MAX_HINTS_فی_ELEMENT);

	(*hint)++;
}

ساکن خالی css_hint_get_hints(ساخت css_hint **hints, uint32_t *nhints)
{
	*hints = hint_ctx.hints;
	*nhints = hint_ctx.len;
}


/******************************************************************************
 * Presentational hint handlers                                               *
 ******************************************************************************/

ساکن خالی css_hint_جدول_cell_border_padding(
		nscss_select_ctx *ctx,
		dom_گرہ *گرہ)
{
	ساخت css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	css_qname qs;
	dom_string *attr = تہی;
	dom_گرہ *tablenode = تہی;
	dom_exception exc;

	qs.ns = تہی;
	qs.name = lwc_string_ref(corestring_lwc_جدول);
	اگر (named_ancestor_گرہ(ctx, گرہ, &qs,
			(خالی *)&tablenode) != CSS_OK) {
		/* Didn't find, or had error */
		lwc_string_unref(qs.name);
		واپس;
	}
	lwc_string_unref(qs.name);

	اگر (tablenode == تہی) {
		واپس;
	}
	/* No need to unref tablenode, named_ancestor_node does not
	 * return a reffed node to the CSS
	 */

	exc = dom_element_get_attribute(tablenode,
			corestring_dom_border, &attr);

	اگر (exc == DOM_NO_ERR && attr != تہی) {
		uint32_t hint_prop;
		css_hint_لمبائی hint_لمبائی;

		اگر (parse_dimension(
				dom_string_ڈیٹا(attr), false,
						&hint_لمبائی.قدر,
						&hint_لمبائی.unit) &&
				INTTOFIX(0) != hint_لمبائی.قدر) {

			برائے (hint_prop = CSS_PROP_BORDER_چوٹی_STYLE;
			     hint_prop <= CSS_PROP_BORDER_بایاں_STYLE;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->حالت_2 = CSS_BORDER_STYLE_INSET;
				css_hint_advance(&hint);
			}

			برائے (hint_prop = CSS_PROP_BORDER_چوٹی_چوڑائی;
			     hint_prop <= CSS_PROP_BORDER_بایاں_چوڑائی;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->ڈیٹا.لمبائی.قدر = INTTOFIX(1);
				hint->ڈیٹا.لمبائی.unit = CSS_UNIT_PX;
				hint->حالت_2 = CSS_BORDER_چوڑائی_چوڑائی;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(tablenode,
			corestring_dom_bordercolor, &attr);

	اگر (exc == DOM_NO_ERR && attr != تہی) {
		uint32_t hint_prop;
		css_color hint_color;

		اگر (nscss_parse_رنگ(
				(ثابت حرف *)dom_string_ڈیٹا(attr),
				&hint_color)) {

			برائے (hint_prop = CSS_PROP_BORDER_چوٹی_COLOR;
			     hint_prop <= CSS_PROP_BORDER_بایاں_COLOR;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->ڈیٹا.color = hint_color;
				hint->حالت_2 = CSS_BORDER_COLOR_COLOR;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(tablenode,
			corestring_dom_cellpadding, &attr);

	اگر (exc == DOM_NO_ERR && attr != تہی) {
		uint32_t hint_prop;
		css_hint_لمبائی hint_لمبائی;

		اگر (parse_dimension(
				dom_string_ڈیٹا(attr), false,
						&hint_لمبائی.قدر,
						&hint_لمبائی.unit)) {

			برائے (hint_prop = CSS_PROP_PADDING_چوٹی;
			     hint_prop <= CSS_PROP_PADDING_بایاں;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->ڈیٹا.لمبائی.قدر = hint_لمبائی.قدر;
				hint->ڈیٹا.لمبائی.unit = hint_لمبائی.unit;
				hint->حالت_2 = CSS_PADDING_سیٹ;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}
}

ساکن خالی css_hint_vertical_align_جدول_cells(
		nscss_select_ctx *ctx,
		dom_گرہ *گرہ)
{
	ساخت css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = تہی;
	dom_exception err;

	err = dom_element_get_attribute(گرہ,
			corestring_dom_valign, &attr);

	اگر (err == DOM_NO_ERR && attr != تہی) {
		hint->ڈیٹا.لمبائی.قدر = 0;
		hint->ڈیٹا.لمبائی.unit = CSS_UNIT_PX;
		اگر (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_چوٹی)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->حالت_2 = CSS_VERTICAL_ALIGN_چوٹی;
			css_hint_advance(&hint);

		} ورنہ اگر (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_درمیان)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->حالت_2 = CSS_VERTICAL_ALIGN_درمیان;
			css_hint_advance(&hint);

		} ورنہ اگر (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_bottom)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->حالت_2 = CSS_VERTICAL_ALIGN_BOTTOM;
			css_hint_advance(&hint);

		} ورنہ اگر (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_baseline)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->حالت_2 = CSS_VERTICAL_ALIGN_BASELINE;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ساکن خالی css_hint_vertical_align_replaced(
		nscss_select_ctx *ctx,
		dom_گرہ *گرہ)
{
	ساخت css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = تہی;
	dom_exception err;

	err = dom_element_get_attribute(گرہ,
			corestring_dom_valign, &attr);

	اگر (err == DOM_NO_ERR && attr != تہی) {
		اگر (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_چوٹی)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->حالت_2 = CSS_VERTICAL_ALIGN_چوٹی;
			css_hint_advance(&hint);

		} ورنہ اگر (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_bottom) ||
			   dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_baseline)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->حالت_2 = CSS_VERTICAL_ALIGN_BASELINE;
			css_hint_advance(&hint);

		} ورنہ اگر (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_texttop)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->حالت_2 = CSS_VERTICAL_ALIGN_متن_چوٹی;
			css_hint_advance(&hint);

		} ورنہ اگر (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_absmiddle) ||
			   dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_abscenter)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->حالت_2 = CSS_VERTICAL_ALIGN_درمیان;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ساکن خالی css_hint_متن_align_معمول(
		nscss_select_ctx *ctx,
		dom_گرہ *گرہ)
{
	ساخت css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *align = تہی;
	dom_exception err;

	err = dom_element_get_attribute(گرہ,
			corestring_dom_align, &align);
	اگر (err == DOM_NO_ERR && align != تہی) {
		اگر (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_بایاں)) {
			hint->prop = CSS_PROP_متن_ALIGN;
			hint->حالت_2 = CSS_متن_ALIGN_بایاں;
			css_hint_advance(&hint);

		} ورنہ اگر (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_center)) {
			hint->prop = CSS_PROP_متن_ALIGN;
			hint->حالت_2 = CSS_متن_ALIGN_CENTER;
			css_hint_advance(&hint);

		} ورنہ اگر (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_دایاں)) {
			hint->prop = CSS_PROP_متن_ALIGN;
			hint->حالت_2 = CSS_متن_ALIGN_دایاں;
			css_hint_advance(&hint);

		} ورنہ اگر (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_justify)) {
			hint->prop = CSS_PROP_متن_ALIGN;
			hint->حالت_2 = CSS_متن_ALIGN_JUSTIFY;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

ساکن خالی css_hint_متن_align_center(
		nscss_select_ctx *ctx,
		dom_گرہ *گرہ)
{
	ساخت css_hint *hint = &hint_ctx.hints[hint_ctx.len];

	hint->prop = CSS_PROP_متن_ALIGN;
	hint->حالت_2 = CSS_متن_ALIGN_LIBCSS_CENTER;
	css_hint_advance(&hint);
}

ساکن خالی css_hint_margin_بایاں_دایاں_align_center(
		nscss_select_ctx *ctx,
		dom_گرہ *گرہ)
{
	ساخت css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr;
	dom_exception exc;

	exc = dom_element_get_attribute(گرہ,
			corestring_dom_align, &attr);

	اگر (exc == DOM_NO_ERR && attr != تہی) {
		memset(hint, 0, حجم(*hint) * 2);
		اگر (dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_center) ||
				dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_abscenter) ||
				dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_درمیان) ||
				dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_absmiddle)) {
			hint->prop = CSS_PROP_MARGIN_بایاں;
			hint->حالت_2 = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_دایاں;
			hint->حالت_2 = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ساکن خالی css_hint_متن_align_special(
		nscss_select_ctx *ctx,
		dom_گرہ *گرہ)
{
	ساخت css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *align = تہی;
	dom_exception err;

	err = dom_element_get_attribute(گرہ,
			corestring_dom_align, &align);

	اگر (err == DOM_NO_ERR && align != تہی) {
		اگر (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_center)) {
			hint->prop = CSS_PROP_متن_ALIGN;
			hint->حالت_2 = CSS_متن_ALIGN_LIBCSS_CENTER;
			css_hint_advance(&hint);

		} ورنہ اگر (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_بایاں)) {
			hint->prop = CSS_PROP_متن_ALIGN;
			hint->حالت_2 = CSS_متن_ALIGN_LIBCSS_بایاں;
			css_hint_advance(&hint);

		} ورنہ اگر (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_دایاں)) {
			hint->prop = CSS_PROP_متن_ALIGN;
			hint->حالت_2 = CSS_متن_ALIGN_LIBCSS_دایاں;
			css_hint_advance(&hint);

		} ورنہ اگر (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_justify)) {
			hint->prop = CSS_PROP_متن_ALIGN;
			hint->حالت_2 = CSS_متن_ALIGN_JUSTIFY;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

ساکن خالی css_hint_متن_align_جدول_special(
		nscss_select_ctx *ctx,
		dom_گرہ *گرہ)
{
	ساخت css_hint *hint = &hint_ctx.hints[hint_ctx.len];

	hint->prop = CSS_PROP_متن_ALIGN;
	hint->حالت_2 = CSS_متن_ALIGN_INHERIT_IF_NON_MAGIC;
	css_hint_advance(&hint);
}

ساکن خالی css_hint_margin_hspace_vspace(
		nscss_select_ctx *ctx,
		dom_گرہ *گرہ)
{
	ساخت css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = تہی;
	dom_exception exc;

	exc = dom_element_get_attribute(گرہ,
			corestring_dom_vspace, &attr);

	اگر (exc == DOM_NO_ERR && attr != تہی) {
		css_hint_لمبائی hint_لمبائی;
		اگر (parse_dimension(
				dom_string_ڈیٹا(attr), false,
				&hint_لمبائی.قدر,
				&hint_لمبائی.unit)) {
			hint->prop = CSS_PROP_MARGIN_چوٹی;
			hint->ڈیٹا.لمبائی.قدر = hint_لمبائی.قدر;
			hint->ڈیٹا.لمبائی.unit = hint_لمبائی.unit;
			hint->حالت_2 = CSS_MARGIN_سیٹ;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_BOTTOM;
			hint->ڈیٹا.لمبائی.قدر = hint_لمبائی.قدر;
			hint->ڈیٹا.لمبائی.unit = hint_لمبائی.unit;
			hint->حالت_2 = CSS_MARGIN_سیٹ;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(گرہ,
			corestring_dom_hspace, &attr);

	اگر (exc == DOM_NO_ERR && attr != تہی) {
		css_hint_لمبائی hint_لمبائی;
		اگر (parse_dimension(
				dom_string_ڈیٹا(attr), false,
				&hint_لمبائی.قدر,
				&hint_لمبائی.unit)) {
			hint->prop = CSS_PROP_MARGIN_بایاں;
			hint->ڈیٹا.لمبائی.قدر = hint_لمبائی.قدر;
			hint->ڈیٹا.لمبائی.unit = hint_لمبائی.unit;
			hint->حالت_2 = CSS_MARGIN_سیٹ;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_دایاں;
			hint->ڈیٹا.لمبائی.قدر = hint_لمبائی.قدر;
			hint->ڈیٹا.لمبائی.unit = hint_لمبائی.unit;
			hint->حالت_2 = CSS_MARGIN_سیٹ;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ساکن خالی css_hint_margin_بایاں_دایاں_hr(
		nscss_select_ctx *ctx,
		dom_گرہ *گرہ)
{
	ساخت css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr;
	dom_exception exc;

	exc = dom_element_get_attribute(گرہ,
				corestring_dom_align, &attr);

	اگر (exc == DOM_NO_ERR && attr != تہی) {
		memset(hint, 0, حجم(*hint) * 2);
		اگر (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_بایاں)) {
			hint->prop = CSS_PROP_MARGIN_بایاں;
			hint->ڈیٹا.لمبائی.قدر = 0;
			hint->ڈیٹا.لمبائی.unit = CSS_UNIT_PX;
			hint->حالت_2 = CSS_MARGIN_سیٹ;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_دایاں;
			hint->حالت_2 = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

		} ورنہ اگر (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_center)) {
			hint->prop = CSS_PROP_MARGIN_بایاں;
			hint->حالت_2 = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_دایاں;
			hint->حالت_2 = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

		} ورنہ اگر (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_دایاں)) {
			hint->prop = CSS_PROP_MARGIN_بایاں;
			hint->حالت_2 = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_دایاں;
			hint->ڈیٹا.لمبائی.قدر = 0;
			hint->ڈیٹا.لمبائی.unit = CSS_UNIT_PX;
			hint->حالت_2 = CSS_MARGIN_سیٹ;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ساکن خالی css_hint_جدول_spacing_border(
		nscss_select_ctx *ctx,
		dom_گرہ *گرہ)
{
	ساخت css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception exc;
	dom_string *attr = تہی;

	exc = dom_element_get_attribute(گرہ, corestring_dom_border, &attr);

	اگر (exc == DOM_NO_ERR && attr != تہی) {
		uint32_t hint_prop;
		css_hint_لمبائی hint_لمبائی;

		برائے (hint_prop = CSS_PROP_BORDER_چوٹی_STYLE;
			     hint_prop <= CSS_PROP_BORDER_بایاں_STYLE;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->حالت_2 = CSS_BORDER_STYLE_OUTSET;
				css_hint_advance(&hint);
		}

		اگر (parse_dimension(
				dom_string_ڈیٹا(attr), false,
						&hint_لمبائی.قدر,
						&hint_لمبائی.unit)) {

			برائے (hint_prop = CSS_PROP_BORDER_چوٹی_چوڑائی;
			     hint_prop <= CSS_PROP_BORDER_بایاں_چوڑائی;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->ڈیٹا.لمبائی.قدر = hint_لمبائی.قدر;
				hint->ڈیٹا.لمبائی.unit = hint_لمبائی.unit;
				hint->حالت_2 = CSS_BORDER_چوڑائی_چوڑائی;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(گرہ,
			corestring_dom_bordercolor, &attr);

	اگر (exc == DOM_NO_ERR && attr != تہی) {
		uint32_t hint_prop;
		css_color hint_color;

		اگر (nscss_parse_رنگ(
				(ثابت حرف *)dom_string_ڈیٹا(attr),
				&hint_color)) {

			برائے (hint_prop = CSS_PROP_BORDER_چوٹی_COLOR;
			     hint_prop <= CSS_PROP_BORDER_بایاں_COLOR;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->ڈیٹا.color = hint_color;
				hint->حالت_2 = CSS_BORDER_COLOR_COLOR;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(گرہ,
			corestring_dom_cellspacing, &attr);

	اگر (exc == DOM_NO_ERR && attr != تہی) {
		اگر (parse_dimension(
				(ثابت حرف *)dom_string_ڈیٹا(attr), false,
						&hint->ڈیٹا.position.h.قدر,
						&hint->ڈیٹا.position.h.unit)) {
			hint->prop = CSS_PROP_BORDER_SPACING;
			hint->ڈیٹا.position.v = hint->ڈیٹا.position.h;
			hint->حالت_2 = CSS_BORDER_SPACING_سیٹ;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ساکن خالی css_hint_اونچائی(
		nscss_select_ctx *ctx,
		dom_گرہ *گرہ)
{
	ساخت css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = تہی;
	dom_exception err;

	err = dom_element_get_attribute(گرہ,
			corestring_dom_اونچائی, &attr);

	اگر (err == DOM_NO_ERR && attr != تہی) {
		اگر (parse_dimension(
				(ثابت حرف *)dom_string_ڈیٹا(attr), false,
				&hint->ڈیٹا.لمبائی.قدر,
				&hint->ڈیٹا.لمبائی.unit)) {
			hint->prop = CSS_PROP_اونچائی;
			hint->حالت_2 = CSS_اونچائی_سیٹ;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ساکن خالی css_hint_چوڑائی(
		nscss_select_ctx *ctx,
		dom_گرہ *گرہ)
{
	ساخت css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = تہی;
	dom_exception err;

	err = dom_element_get_attribute(گرہ,
			corestring_dom_چوڑائی, &attr);

	اگر (err == DOM_NO_ERR && attr != تہی) {
		اگر (parse_dimension(
				(ثابت حرف *)dom_string_ڈیٹا(attr), false,
				&hint->ڈیٹا.لمبائی.قدر,
				&hint->ڈیٹا.لمبائی.unit)) {
			hint->prop = CSS_PROP_چوڑائی;
			hint->حالت_2 = CSS_چوڑائی_سیٹ;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ساکن خالی css_hint_اونچائی_چوڑائی_textarea(
		nscss_select_ctx *ctx,
		dom_گرہ *گرہ)
{
	ساخت css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = تہی;
	dom_exception err;

	err = dom_element_get_attribute(گرہ,
			corestring_dom_صفیں, &attr);

	اگر (err == DOM_NO_ERR && attr != تہی) {
		اگر (parse_dimension(
				(ثابت حرف *)dom_string_ڈیٹا(attr), false,
				&hint->ڈیٹا.لمبائی.قدر,
				&hint->ڈیٹا.لمبائی.unit)) {
			hint->prop = CSS_PROP_اونچائی;
			hint->ڈیٹا.لمبائی.unit = CSS_UNIT_EM;
			hint->حالت_2 = CSS_اونچائی_سیٹ;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}

	err = dom_element_get_attribute(گرہ,
			corestring_dom_ستون_تعداد, &attr);

	اگر (err == DOM_NO_ERR && attr != تہی) {
		اگر (parse_dimension(
				(ثابت حرف *)dom_string_ڈیٹا(attr), false,
				&hint->ڈیٹا.لمبائی.قدر,
				&hint->ڈیٹا.لمبائی.unit)) {
			hint->prop = CSS_PROP_چوڑائی;
			hint->ڈیٹا.لمبائی.unit = CSS_UNIT_EX;
			hint->حالت_2 = CSS_چوڑائی_سیٹ;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ساکن خالی css_hint_اونچائی_چوڑائی_canvas(
		nscss_select_ctx *ctx,
		dom_گرہ *گرہ)
{
	ساخت css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = تہی;
	dom_exception err;
	bool سیٹ_dim = false;

	err = dom_element_get_attribute(گرہ,
			corestring_dom_اونچائی, &attr);

	اگر (err == DOM_NO_ERR && attr != تہی) {
		اگر (parse_dimension(
				(ثابت حرف *)dom_string_ڈیٹا(attr), true,
				&hint->ڈیٹا.لمبائی.قدر,
				&hint->ڈیٹا.لمبائی.unit)) {
			hint->prop = CSS_PROP_اونچائی;
			hint->ڈیٹا.لمبائی.unit = CSS_UNIT_PX;
			hint->حالت_2 = CSS_اونچائی_سیٹ;
			css_hint_advance(&hint);
			سیٹ_dim = true;
		}
		dom_string_unref(attr);
	}
	اگر (سیٹ_dim == false) {
		/* canvas defaults to 150px tall */
		hint->prop = CSS_PROP_اونچائی;
		hint->ڈیٹا.لمبائی.unit = CSS_UNIT_PX;
		hint->ڈیٹا.لمبائی.قدر = INTTOFIX(150);
		hint->حالت_2 = CSS_اونچائی_سیٹ;
		css_hint_advance(&hint);
	} ورنہ {
		سیٹ_dim = false;
	}

	err = dom_element_get_attribute(گرہ,
			corestring_dom_چوڑائی, &attr);

	اگر (err == DOM_NO_ERR && attr != تہی) {
		اگر (parse_dimension(
				(ثابت حرف *)dom_string_ڈیٹا(attr), true,
				&hint->ڈیٹا.لمبائی.قدر,
				&hint->ڈیٹا.لمبائی.unit)) {
			hint->prop = CSS_PROP_چوڑائی;
			hint->ڈیٹا.لمبائی.unit = CSS_UNIT_PX;
			hint->حالت_2 = CSS_چوڑائی_سیٹ;
			css_hint_advance(&hint);
			سیٹ_dim = true;
		}
		dom_string_unref(attr);
	}
	اگر (سیٹ_dim == false) {
		/* canvas defaults to 300px wide */
		hint->prop = CSS_PROP_چوڑائی;
		hint->ڈیٹا.لمبائی.unit = CSS_UNIT_PX;
		hint->ڈیٹا.لمبائی.قدر = INTTOFIX(300);
		hint->حالت_2 = CSS_چوڑائی_سیٹ;
		css_hint_advance(&hint);
	}
}

ساکن خالی css_hint_چوڑائی_داخلہ(
		nscss_select_ctx *ctx,
		dom_گرہ *گرہ)
{
	ساخت css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_string *attr = تہی;
	dom_exception err;

	err = dom_element_get_attribute(گرہ,
			corestring_dom_سائز, &attr);

	اگر (err == DOM_NO_ERR && attr != تہی) {
		اگر (parse_dimension(
				(ثابت حرف *)dom_string_ڈیٹا(attr), false,
				&hint->ڈیٹا.لمبائی.قدر,
				&hint->ڈیٹا.لمبائی.unit)) {
			dom_string *attr2 = تہی;

			err = dom_element_get_attribute(گرہ,
					corestring_dom_type, &attr2);
			اگر (err == DOM_NO_ERR) {

				hint->prop = CSS_PROP_چوڑائی;
				hint->حالت_2 = CSS_چوڑائی_سیٹ;

				اگر (attr2 == تہی ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_متن) ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_تلاش) ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_password) ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_file)) {
					hint->ڈیٹا.لمبائی.unit = CSS_UNIT_EX;
				}
				اگر (attr2 != تہی) {
					dom_string_unref(attr2);
				}
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}
}

ساکن خالی css_hint_anchor_color(
		nscss_select_ctx *ctx,
		dom_گرہ *گرہ)
{
	ساخت css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	css_error error;
	dom_exception err;
	dom_string *color;
	dom_گرہ *bodynode = تہی;

	/* find body node */
	css_qname qs;
	bool is_دیکھا;

	qs.ns = تہی;
	qs.name = lwc_string_ref(corestring_lwc_body);
	اگر (named_ancestor_گرہ(ctx, گرہ, &qs,
			(خالی *)&bodynode) != CSS_OK) {
		/* Didn't find, or had error */
		lwc_string_unref(qs.name);
		واپس ;
	}
	lwc_string_unref(qs.name);

	اگر (bodynode == تہی) {
		واپس;
	}

	error = گرہ_is_دیکھا(ctx, گرہ, &is_دیکھا);
	اگر (error != CSS_OK)
		واپس;

	اگر (is_دیکھا) {
		err = dom_element_get_attribute(bodynode,
				corestring_dom_vlink, &color);
	} ورنہ {
		err = dom_element_get_attribute(bodynode,
				corestring_dom_link, &color);
	}

	اگر (err == DOM_NO_ERR && color != تہی) {
		اگر (nscss_parse_رنگ(
				(ثابت حرف *)dom_string_ڈیٹا(color),
						&hint->ڈیٹا.color)) {
			hint->prop = CSS_PROP_COLOR;
			hint->حالت_2 = CSS_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(color);
	}
}

ساکن خالی css_hint_body_color(
		nscss_select_ctx *ctx,
		dom_گرہ *گرہ)
{
	ساخت css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *color;

	err = dom_element_get_attribute(گرہ, corestring_dom_متن, &color);

	اگر (err == DOM_NO_ERR && color != تہی) {
		اگر (nscss_parse_رنگ(
				(ثابت حرف *)dom_string_ڈیٹا(color),
						&hint->ڈیٹا.color)) {
			hint->prop = CSS_PROP_COLOR;
			hint->حالت_2 = CSS_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(color);
	}
}

ساکن خالی css_hint_color(
		nscss_select_ctx *ctx,
		dom_گرہ *گرہ)
{
	ساخت css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *color;

	err = dom_element_get_attribute(گرہ, corestring_dom_color, &color);

	اگر (err == DOM_NO_ERR && color != تہی) {
		اگر (nscss_parse_رنگ(
				(ثابت حرف *)dom_string_ڈیٹا(color),
						&hint->ڈیٹا.color)) {
			hint->prop = CSS_PROP_COLOR;
			hint->حالت_2 = CSS_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(color);
	}
}

ساکن خالی css_hint_font_سائز(
		nscss_select_ctx *ctx,
		dom_گرہ *گرہ)
{
	ساخت css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *سائز;

	err = dom_element_get_attribute(گرہ, corestring_dom_سائز, &سائز);
	اگر (err == DOM_NO_ERR && سائز != تہی) {
		اگر (parse_font_سائز(
				(ثابت حرف *)dom_string_ڈیٹا(سائز),
						&hint->حالت_2,
						&hint->ڈیٹا.لمبائی.قدر,
						&hint->ڈیٹا.لمبائی.unit)) {
			hint->prop = CSS_PROP_FONT_سائز;
			css_hint_advance(&hint);
		}
		dom_string_unref(سائز);
	}
}

ساکن خالی css_hint_float(
		nscss_select_ctx *ctx,
		dom_گرہ *گرہ)
{
	ساخت css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *align;

	err = dom_element_get_attribute(گرہ, corestring_dom_align, &align);
	اگر (err == DOM_NO_ERR && align != تہی) {
		اگر (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_بایاں)) {
			hint->prop = CSS_PROP_FLOAT;
			hint->حالت_2 = CSS_FLOAT_بایاں;
			css_hint_advance(&hint);

		} ورنہ اگر (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_دایاں)) {
			hint->prop = CSS_PROP_FLOAT;
			hint->حالت_2 = CSS_FLOAT_دایاں;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

ساکن خالی css_hint_caption_side(
		nscss_select_ctx *ctx,
		dom_گرہ *گرہ)
{
	ساخت css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *align = تہی;

	err = dom_element_get_attribute(گرہ, corestring_dom_align, &align);
	اگر (err == DOM_NO_ERR && align != تہی) {
		اگر (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_bottom)) {
			hint->prop = CSS_PROP_CAPTION_SIDE;
			hint->حالت_2 = CSS_CAPTION_SIDE_BOTTOM;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

ساکن خالی css_hint_bg_color(
		nscss_select_ctx *ctx,
		dom_گرہ *گرہ)
{
	ساخت css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *bgcolor;

	err = dom_element_get_attribute(گرہ,
			corestring_dom_bgcolor, &bgcolor);
	اگر (err == DOM_NO_ERR && bgcolor != تہی) {
		اگر (nscss_parse_رنگ(
				(ثابت حرف *)dom_string_ڈیٹا(bgcolor),
				&hint->ڈیٹا.color)) {
			hint->prop = CSS_PROP_BACKGROUND_COLOR;
			hint->حالت_2 = CSS_BACKGROUND_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(bgcolor);
	}
}

ساکن خالی css_hint_bg_image(
		nscss_select_ctx *ctx,
		dom_گرہ *گرہ)
{
	ساخت css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_exception err;
	dom_string *attr;

	err = dom_element_get_attribute(گرہ,
			corestring_dom_background, &attr);
	اگر (err == DOM_NO_ERR && attr != تہی) {
		nsurl *url;
		nserror error = nsurl_join(ctx->بنیاد_url,
				(ثابت حرف *)dom_string_ڈیٹا(attr), &url);
		dom_string_unref(attr);

		اگر (error == NSERROR_OK) {
			lwc_string *iurl;
			lwc_error lerror = lwc_intern_string(nsurl_access(url),
					nsurl_لمبائی(url), &iurl);
			nsurl_unref(url);

			اگر (lerror == lwc_error_ok) {
				hint->prop = CSS_PROP_BACKGROUND_IMAGE;
				hint->ڈیٹا.string = iurl;
				hint->حالت_2 = CSS_BACKGROUND_IMAGE_IMAGE;
				css_hint_advance(&hint);
			}
		}
	}
}

ساکن خالی css_hint_white_space_nowrap(
		nscss_select_ctx *ctx,
		dom_گرہ *گرہ)
{
	ساخت css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_exception err;
	bool nowrap;

	err = dom_element_has_attribute(گرہ, corestring_dom_nowrap, &nowrap);
	اگر (err == DOM_NO_ERR && nowrap == true) {
		hint->prop = CSS_PROP_WHITE_SPACE;
		hint->حالت_2 = CSS_WHITE_SPACE_NOWRAP;
		css_hint_advance(&hint);
	}
}

ساکن خالی css_hint_list(
		nscss_select_ctx *ctx,
		dom_گرہ *گرہ)
{
	ساخت css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_exception err;
	dom_string *attr;

	err = dom_element_get_attribute(گرہ, corestring_dom_type, &attr);
	اگر (err == DOM_NO_ERR && attr != تہی) {
		ثابت حرف *attr_str = dom_string_ڈیٹا(attr);
		size_t attr_len = dom_string_byte_لمبائی(attr);
		شمار css_list_style_type_e type = CSS_LIST_STYLE_TYPE_INHERIT;

		اگر (attr_len == 1) {
			انتخاب (attr_str[0]) {
			حالت 'a':
				type = CSS_LIST_STYLE_TYPE_زیریں_ALPHA;
				توڑو;
			حالت 'A':
				type = CSS_LIST_STYLE_TYPE_بالائی_ALPHA;
				توڑو;
			حالت 'i':
				type = CSS_LIST_STYLE_TYPE_زیریں_ROMAN;
				توڑو;
			حالت 'I':
				type = CSS_LIST_STYLE_TYPE_بالائی_ROMAN;
				توڑو;
			حالت '1':
				type = CSS_LIST_STYLE_TYPE_DECIMAL;
				توڑو;
			}
		}

		اگر (type != CSS_LIST_STYLE_TYPE_INHERIT) {
			hint->prop = CSS_PROP_LIST_STYLE_TYPE;
			hint->حالت_2 = type;
			css_hint_advance(&hint);
		}

		dom_string_unref(attr);
	}
}


/* Exported function, documented in css/hints.h */
css_error گرہ_presentational_hint(خالی *pw, خالی *گرہ,
		uint32_t *nhints, css_hint **hints)
{
	dom_exception exc;
	dom_html_element_type لیبل_type;

	اگر (nsoption_bool(author_level_css) == false) {
		*nhints = 0;
		*hints = تہی;
		واپس CSS_OK;
	}

	css_hint_clean();

	exc = dom_html_element_get_لیبل_type(گرہ, &لیبل_type);
	اگر (exc != DOM_NO_ERR) {
		لیبل_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	انتخاب (لیبل_type) {
	حالت DOM_HTML_ELEMENT_TYPE_TH:
	حالت DOM_HTML_ELEMENT_TYPE_TD:
		css_hint_چوڑائی(pw, گرہ);
		css_hint_جدول_cell_border_padding(pw, گرہ);
		css_hint_white_space_nowrap(pw, گرہ);
		fallthrough;
	حالت DOM_HTML_ELEMENT_TYPE_TR:
		css_hint_اونچائی(pw, گرہ);
		fallthrough;
	حالت DOM_HTML_ELEMENT_TYPE_THEAD:
	حالت DOM_HTML_ELEMENT_TYPE_TBODY:
	حالت DOM_HTML_ELEMENT_TYPE_TFOOT:
		css_hint_متن_align_special(pw, گرہ);
		fallthrough;
	حالت DOM_HTML_ELEMENT_TYPE_ستون:
		css_hint_vertical_align_جدول_cells(pw, گرہ);
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_APPLET:
	حالت DOM_HTML_ELEMENT_TYPE_IMG:
		css_hint_margin_hspace_vspace(pw, گرہ);
		fallthrough;
	حالت DOM_HTML_ELEMENT_TYPE_EMBED:
	حالت DOM_HTML_ELEMENT_TYPE_IFRAME:
	حالت DOM_HTML_ELEMENT_TYPE_OBJECT:
		css_hint_اونچائی(pw, گرہ);
		css_hint_چوڑائی(pw, گرہ);
		css_hint_vertical_align_replaced(pw, گرہ);
		css_hint_float(pw, گرہ);
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_P:
	حالت DOM_HTML_ELEMENT_TYPE_H1:
	حالت DOM_HTML_ELEMENT_TYPE_H2:
	حالت DOM_HTML_ELEMENT_TYPE_H3:
	حالت DOM_HTML_ELEMENT_TYPE_H4:
	حالت DOM_HTML_ELEMENT_TYPE_H5:
	حالت DOM_HTML_ELEMENT_TYPE_H6:
		css_hint_متن_align_معمول(pw, گرہ);
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_CENTER:
		css_hint_متن_align_center(pw, گرہ);
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_CAPTION:
		css_hint_caption_side(pw, گرہ);
		fallthrough;
	حالت DOM_HTML_ELEMENT_TYPE_DIV:
		css_hint_متن_align_special(pw, گرہ);
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_جدول:
		css_hint_متن_align_جدول_special(pw, گرہ);
		css_hint_جدول_spacing_border(pw, گرہ);
		css_hint_float(pw, گرہ);
		css_hint_margin_بایاں_دایاں_align_center(pw, گرہ);
		css_hint_چوڑائی(pw, گرہ);
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_HR:
		css_hint_چوڑائی(pw, گرہ);
		css_hint_margin_بایاں_دایاں_hr(pw, گرہ);
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		css_hint_اونچائی_چوڑائی_textarea(pw, گرہ);
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_داخلہ:
		css_hint_چوڑائی_داخلہ(pw, گرہ);
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_A:
		css_hint_anchor_color(pw, گرہ);
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_FONT:
		css_hint_font_سائز(pw, گرہ);
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_BODY:
		css_hint_body_color(pw, گرہ);
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_CANVAS:
		css_hint_اونچائی_چوڑائی_canvas(pw, گرہ);
		توڑو;
	حالت DOM_HTML_ELEMENT_TYPE_OL:
		css_hint_list(pw, گرہ);
		توڑو;
	طےشدہ:
		توڑو;
	}

	اگر (لیبل_type != DOM_HTML_ELEMENT_TYPE__UNKNOWN) {
		css_hint_color(pw, گرہ);
		css_hint_bg_color(pw, گرہ);
		css_hint_bg_image(pw, گرہ);
	}

#اگر_تعریف_شدہ LOG_STATS
	NSLOG(netsurf, INFO, "Properties with hints: %i", hint_ctx.len);
#اگر_ختم

	css_hint_get_hints(hints, nhints);

	واپس CSS_OK;
}
