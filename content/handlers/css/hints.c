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

#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <strings.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsoption.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsurl.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "css/hints.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "css/select.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LOG_STATS
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ LOG_STATS

/******************************************************************************
 * Utility functions                                                          *
 ******************************************************************************/

/**
 * Determine if a given character is whitespace
 *
 * \param c  Character to consider
 * \return true if character is whitespace, false otherwise
 */
ਸਥਿਰ bool isWhitespace(ਅੱਖਰ c)
{
	ਵਾਪਸ c == ' ' || c == '\t' || c == '\f' || c == '\r' || c == '\n';
}

/**
 * Determine if a given character is a valid hex digit
 *
 * \param c  Character to consider
 * \return true if character is a valid hex digit, false otherwise
 */
ਸਥਿਰ bool isHex(ਅੱਖਰ c)
{
	ਵਾਪਸ ('0' <= c && c <= '9') ||
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
ਸਥਿਰ uint8_t char_ਮੰਜ਼ਿਲ_Hex(ਅੱਖਰ c)
{
	/* 0-9 */
	c -= '0';

	/* A-F */
	ਜੇ (c > 9)
		c -= 'A' - '9' - 1;

	/* a-f */
	ਜੇ (c > 15)
		c -= 'a' - 'A';

	ਵਾਪਸ c;
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
ਸਥਿਰ bool parse_ਸੰਖਿਆ(ਅਚਲ ਅੱਖਰ *ਅੰਕੜੇ, bool maybe_negative, bool real,
		css_fixed *ਮੁੱਲ, size_t *consumed)
{
	size_t len;
	ਅਚਲ uint8_t *ptr;
	int32_t intpart = 0;
	int32_t fracpart = 0;
	int32_t pwr = 1;
	ਪੂਰਨ sign = 1;

	*consumed = 0;

	len = ਲੜੀ_ਲੰਬਾਈ(ਅੰਕੜੇ);
	ptr = (ਅਚਲ uint8_t *) ਅੰਕੜੇ;

	ਜੇ (len == 0)
		ਵਾਪਸ false;

	/* Skip leading whitespace */
	ਜਦੋਂ (len > 0 && isWhitespace(ptr[0])) {
		len--;
		ptr++;
	}

	ਜੇ (len == 0)
		ਵਾਪਸ false;

	/* Extract sign, if any */
	ਜੇ (ptr[0] == '+') {
		len--;
		ptr++;
	} ਨਹੀਂਤਾਂ ਜੇ (ptr[0] == '-' && maybe_negative) {
		sign = -1;
		len--;
		ptr++;
	}

	ਜੇ (len == 0)
		ਵਾਪਸ false;

	/* Must have a digit [0,9] */
	ਜੇ ('0' > ptr[0] || ptr[0] > '9')
		ਵਾਪਸ false;

	/* Now extract intpart, assuming base 10 */
	ਜਦੋਂ (len > 0) {
		/* Stop on first non-digit */
		ਜੇ (ptr[0] < '0' || '9' < ptr[0])
			ਰੋਕੋ;

		/* Prevent overflow of 'intpart'; proper clamping below */
		ਜੇ (intpart < (1 << 22)) {
			intpart *= 10;
			intpart += ptr[0] - '0';
		}
		ptr++;
		len--;
	}

	/* And fracpart, again, assuming base 10 */
	ਜੇ (real && len > 1 && ptr[0] == '.' &&
			('0' <= ptr[1] && ptr[1] <= '9')) {
		ptr++;
		len--;

		ਜਦੋਂ (len > 0) {
			ਜੇ (ptr[0] < '0' || '9' < ptr[0])
				ਰੋਕੋ;

			ਜੇ (pwr < 1000000) {
				pwr *= 10;
				fracpart *= 10;
				fracpart += ptr[0] - '0';
			}
			ptr++;
			len--;
		}

		fracpart = ((1 << 10) * fracpart + pwr/2) / pwr;
		ਜੇ (fracpart >= (1 << 10)) {
			intpart++;
			fracpart &= (1 << 10) - 1;
		}
	}

	ਜੇ (sign > 0) {
		/* If the result is larger than we can represent,
		 * then clamp to the maximum value we can store. */
		ਜੇ (intpart >= (1 << 21)) {
			intpart = (1 << 21) - 1;
			fracpart = (1 << 10) - 1;
		}
	} ਨਹੀਂਤਾਂ {
		/* If the negated result is smaller than we can represent
		 * then clamp to the minimum value we can store. */
		ਜੇ (intpart >= (1 << 21)) {
			intpart = -(1 << 21);
			fracpart = 0;
		} ਨਹੀਂਤਾਂ {
			intpart = -intpart;
			ਜੇ (fracpart) {
				fracpart = (1 << 10) - fracpart;
				intpart--;
			}
		}
	}

	*ਮੁੱਲ = (intpart << 10) | fracpart;

	*consumed = ptr - (ਅਚਲ uint8_t *) ਅੰਕੜੇ;

	ਵਾਪਸ true;
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
ਸਥਿਰ bool parse_dimension(ਅਚਲ ਅੱਖਰ *ਅੰਕੜੇ, bool strict, css_fixed *ਲੰਮਾਈ,
		css_unit *unit)
{
	size_t len;
	size_t ਪਠ;
	css_fixed ਮੁੱਲ;

	len = ਲੜੀ_ਲੰਬਾਈ(ਅੰਕੜੇ);

	ਜੇ (parse_ਸੰਖਿਆ(ਅੰਕੜੇ, false, true, &ਮੁੱਲ, &ਪਠ) == false)
		ਵਾਪਸ false;

	ਜੇ (strict && ਮੁੱਲ < INTTOFIX(1))
		ਵਾਪਸ false;

	*ਲੰਮਾਈ = ਮੁੱਲ;

	ਜੇ (len > ਪਠ && ਅੰਕੜੇ[ਪਠ] == '%')
		*unit = CSS_UNIT_PCT;
	ਨਹੀਂਤਾਂ
		*unit = CSS_UNIT_PX;

	ਵਾਪਸ true;
}

/**
 * Mapping of colour name to CSS color
 */
ਬਣਤਰ ਰੰਗ_map {
	ਅਚਲ ਅੱਖਰ *name;
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
ਸਥਿਰ ਪੂਰਨ cmp_ਰੰਗ_name(ਅਚਲ ਖਾਲੀ *a, ਅਚਲ ਖਾਲੀ *b)
{
	ਅਚਲ ਅੱਖਰ *aa = a;
	ਅਚਲ ਬਣਤਰ ਰੰਗ_map *bb = b;

	ਵਾਪਸ strcasecmp(aa, bb->name);
}

/**
 * Parse a named colour
 *
 * \param name    Name to parse
 * \param result  Pointer to location to receive css_color
 * \return true on success, false on invalid input
 */
ਸਥਿਰ bool parse_named_ਰੰਗ(ਅਚਲ ਅੱਖਰ *name, css_color *ਨਤੀਜਾ)
{
	ਸਥਿਰ ਅਚਲ ਬਣਤਰ ਰੰਗ_map named_colours[] = {
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
	ਅਚਲ ਬਣਤਰ ਰੰਗ_map *ਇੰਦਰਾਜ;

	ਇੰਦਰਾਜ = bsearch(name, named_colours,
			ਆਕਾਰ(named_colours) / ਆਕਾਰ(named_colours[0]),
			ਆਕਾਰ(named_colours[0]),
			cmp_ਰੰਗ_name);

	ਜੇ (ਇੰਦਰਾਜ != ਨਿਰਾ)
		*ਨਤੀਜਾ = ਇੰਦਰਾਜ->color;

	ਵਾਪਸ ਇੰਦਰਾਜ != ਨਿਰਾ;
}

/* exported interface documented in content/handlers/css/hints.h */
bool nscss_parse_ਰੰਗ(ਅਚਲ ਅੱਖਰ *ਅੰਕੜੇ, css_color *ਨਤੀਜਾ)
{
	size_t len = ਲੜੀ_ਲੰਬਾਈ(ਅੰਕੜੇ);
	uint8_t r, g, b;

	/* 2 */
	ਜੇ (len == 0)
		ਵਾਪਸ false;

	/* 3 */
	ਜੇ (len == SLEN("transparent") && strcasecmp(ਅੰਕੜੇ, "transparent") == 0)
		ਵਾਪਸ false;

	/* 4 */
	ਜੇ (parse_named_ਰੰਗ(ਅੰਕੜੇ, ਨਤੀਜਾ))
		ਵਾਪਸ true;

	/** \todo Implement HTML5's utterly insane legacy colour parsing */

	ਜੇ (ਅੰਕੜੇ[0] == '#') {
		ਅੰਕੜੇ++;
		len--;
	}

	ਜੇ (len == 3 && isHex(ਅੰਕੜੇ[0]) && isHex(ਅੰਕੜੇ[1]) && isHex(ਅੰਕੜੇ[2])) {
		r = char_ਮੰਜ਼ਿਲ_Hex(ਅੰਕੜੇ[0]);
		g = char_ਮੰਜ਼ਿਲ_Hex(ਅੰਕੜੇ[1]);
		b = char_ਮੰਜ਼ਿਲ_Hex(ਅੰਕੜੇ[2]);

		r |= (r << 4);
		g |= (g << 4);
		b |= (b << 4);

		*ਨਤੀਜਾ = (0xff << 24) | (r << 16) | (g << 8) | b;

		ਵਾਪਸ true;
	} ਨਹੀਂਤਾਂ ਜੇ (len == 6 && isHex(ਅੰਕੜੇ[0]) && isHex(ਅੰਕੜੇ[1]) &&
			isHex(ਅੰਕੜੇ[2]) && isHex(ਅੰਕੜੇ[3]) && isHex(ਅੰਕੜੇ[4]) &&
			isHex(ਅੰਕੜੇ[5])) {
		r = (char_ਮੰਜ਼ਿਲ_Hex(ਅੰਕੜੇ[0]) << 4) | char_ਮੰਜ਼ਿਲ_Hex(ਅੰਕੜੇ[1]);
		g = (char_ਮੰਜ਼ਿਲ_Hex(ਅੰਕੜੇ[2]) << 4) | char_ਮੰਜ਼ਿਲ_Hex(ਅੰਕੜੇ[3]);
		b = (char_ਮੰਜ਼ਿਲ_Hex(ਅੰਕੜੇ[4]) << 4) | char_ਮੰਜ਼ਿਲ_Hex(ਅੰਕੜੇ[5]);

		*ਨਤੀਜਾ = (0xff << 24) | (r << 16) | (g << 8) | b;

		ਵਾਪਸ true;
	}

	ਵਾਪਸ false;
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
ਸਥਿਰ bool parse_font_ਮਾਪ(ਅਚਲ ਅੱਖਰ *ਮਾਪ, uint8_t *val,
		css_fixed *len, css_unit *unit)
{
	ਸਥਿਰ ਅਚਲ uint8_t ਮਾਪ_map[] = {
		CSS_FONT_ਮਾਪ_XX_SMALL,
		CSS_FONT_ਮਾਪ_SMALL,
		CSS_FONT_ਮਾਪ_MEDIUM,
		CSS_FONT_ਮਾਪ_LARGE,
		CSS_FONT_ਮਾਪ_X_LARGE,
		CSS_FONT_ਮਾਪ_XX_LARGE,
		CSS_FONT_ਮਾਪ_DIMENSION	/* xxx-large (see below) */
	};

	ਅਚਲ ਅੱਖਰ *p = ਮਾਪ;
	ਅੱਖਰ ਬਹੁਲਕ;
	ਪੂਰਨ ਮੁੱਲ = 0;

	/* Skip whitespace */
	ਜਦੋਂ (*p != '\0' && isWhitespace(*p))
		p++;

	ਬਹੁਲਕ = *p;

	/* Skip +/- */
	ਜੇ (ਬਹੁਲਕ == '+' || ਬਹੁਲਕ == '-')
		p++;

	/* Need at least one digit */
	ਜੇ (*p < '0' || *p > '9') {
		ਵਾਪਸ false;
	}

	/* Consume digits, computing value */
	ਜਦੋਂ ('0' <= *p && *p <= '9') {
		ਮੁੱਲ = ਮੁੱਲ * 10 + (*p - '0');
		p++;
	}

	/* Resolve relative sizes */
	ਜੇ (ਬਹੁਲਕ == '+')
		ਮੁੱਲ += 3;
	ਨਹੀਂਤਾਂ ਜੇ (ਬਹੁਲਕ == '-')
		ਮੁੱਲ = 3 - ਮੁੱਲ;

	/* Clamp to range [1,7] */
	ਜੇ (ਮੁੱਲ < 1)
		ਮੁੱਲ = 1;
	ਨਹੀਂਤਾਂ ਜੇ (ਮੁੱਲ > 7)
		ਮੁੱਲ = 7;

	ਜੇ (ਮੁੱਲ == 7) {
		/* Manufacture xxx-large */
	  *len = FDIV(FMUL(INTTOFIX(3), INTTOFIX(nsoption_int(font_ਮਾਪ))),
				F_10);
	} ਨਹੀਂਤਾਂ {
		/* Len is irrelevant */
		*len = 0;
	}

	*unit = CSS_UNIT_PT;
	*val = ਮਾਪ_map[ਮੁੱਲ - 1];

	ਵਾਪਸ true;
}


/******************************************************************************
 * Hint context management                                                    *
 ******************************************************************************/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_HINTS_ਪ੍ਰਤੀ_ELEMENT 32

ਬਣਤਰ css_hint_ctx {
	ਬਣਤਰ css_hint *hints;
	uint32_t len;
};

ਬਣਤਰ css_hint_ctx hint_ctx;

nserror css_hint_init(ਖਾਲੀ)
{
	hint_ctx.hints = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਬਣਤਰ css_hint) *
			MAX_HINTS_ਪ੍ਰਤੀ_ELEMENT);
	ਜੇ (hint_ctx.hints == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	ਵਾਪਸ NSERROR_OK;
}

ਖਾਲੀ css_hint_fini(ਖਾਲੀ)
{
	hint_ctx.len = 0;
	ਮੁਕਤ(hint_ctx.hints);
}

ਸਥਿਰ ਖਾਲੀ css_hint_clean(ਖਾਲੀ)
{
	hint_ctx.len = 0;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ css_hint_advance(ਬਣਤਰ css_hint **hint)
{
	hint_ctx.len++;
	ਪ੍ਰਤਿਜ੍ਞਾ(hint_ctx.len < MAX_HINTS_ਪ੍ਰਤੀ_ELEMENT);

	(*hint)++;
}

ਸਥਿਰ ਖਾਲੀ css_hint_get_hints(ਬਣਤਰ css_hint **hints, uint32_t *nhints)
{
	*hints = hint_ctx.hints;
	*nhints = hint_ctx.len;
}


/******************************************************************************
 * Presentational hint handlers                                               *
 ******************************************************************************/

ਸਥਿਰ ਖਾਲੀ css_hint_ਸਾਰਣੀ_cell_border_padding(
		nscss_select_ctx *ctx,
		dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	css_qname qs;
	dom_string *attr = ਨਿਰਾ;
	dom_ਗੰਢ *tablenode = ਨਿਰਾ;
	dom_exception exc;

	qs.ns = ਨਿਰਾ;
	qs.name = lwc_string_ref(corestring_lwc_ਸਾਰਣੀ);
	ਜੇ (named_ancestor_ਗੰਢ(ctx, ਗੰਢ, &qs,
			(ਖਾਲੀ *)&tablenode) != CSS_OK) {
		/* Didn't find, or had error */
		lwc_string_unref(qs.name);
		ਵਾਪਸ;
	}
	lwc_string_unref(qs.name);

	ਜੇ (tablenode == ਨਿਰਾ) {
		ਵਾਪਸ;
	}
	/* No need to unref tablenode, named_ancestor_node does not
	 * return a reffed node to the CSS
	 */

	exc = dom_element_get_attribute(tablenode,
			corestring_dom_border, &attr);

	ਜੇ (exc == DOM_NO_ERR && attr != ਨਿਰਾ) {
		uint32_t hint_prop;
		css_hint_ਲੰਮਾਈ hint_ਲੰਮਾਈ;

		ਜੇ (parse_dimension(
				dom_string_ਅੰਕੜੇ(attr), false,
						&hint_ਲੰਮਾਈ.ਮੁੱਲ,
						&hint_ਲੰਮਾਈ.unit) &&
				INTTOFIX(0) != hint_ਲੰਮਾਈ.ਮੁੱਲ) {

			ਲਈ (hint_prop = CSS_PROP_BORDER_ਸਿਖਰ_STYLE;
			     hint_prop <= CSS_PROP_BORDER_ਖੱਬਾ_STYLE;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->ਸਥਿਤੀ = CSS_BORDER_STYLE_INSET;
				css_hint_advance(&hint);
			}

			ਲਈ (hint_prop = CSS_PROP_BORDER_ਸਿਖਰ_ਚੌੜਾਈ;
			     hint_prop <= CSS_PROP_BORDER_ਖੱਬਾ_ਚੌੜਾਈ;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->ਅੰਕੜੇ.ਲੰਮਾਈ.ਮੁੱਲ = INTTOFIX(1);
				hint->ਅੰਕੜੇ.ਲੰਮਾਈ.unit = CSS_UNIT_PX;
				hint->ਸਥਿਤੀ = CSS_BORDER_ਚੌੜਾਈ_ਚੌੜਾਈ;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(tablenode,
			corestring_dom_bordercolor, &attr);

	ਜੇ (exc == DOM_NO_ERR && attr != ਨਿਰਾ) {
		uint32_t hint_prop;
		css_color hint_color;

		ਜੇ (nscss_parse_ਰੰਗ(
				(ਅਚਲ ਅੱਖਰ *)dom_string_ਅੰਕੜੇ(attr),
				&hint_color)) {

			ਲਈ (hint_prop = CSS_PROP_BORDER_ਸਿਖਰ_COLOR;
			     hint_prop <= CSS_PROP_BORDER_ਖੱਬਾ_COLOR;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->ਅੰਕੜੇ.color = hint_color;
				hint->ਸਥਿਤੀ = CSS_BORDER_COLOR_COLOR;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(tablenode,
			corestring_dom_cellpadding, &attr);

	ਜੇ (exc == DOM_NO_ERR && attr != ਨਿਰਾ) {
		uint32_t hint_prop;
		css_hint_ਲੰਮਾਈ hint_ਲੰਮਾਈ;

		ਜੇ (parse_dimension(
				dom_string_ਅੰਕੜੇ(attr), false,
						&hint_ਲੰਮਾਈ.ਮੁੱਲ,
						&hint_ਲੰਮਾਈ.unit)) {

			ਲਈ (hint_prop = CSS_PROP_PADDING_ਸਿਖਰ;
			     hint_prop <= CSS_PROP_PADDING_ਖੱਬਾ;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->ਅੰਕੜੇ.ਲੰਮਾਈ.ਮੁੱਲ = hint_ਲੰਮਾਈ.ਮੁੱਲ;
				hint->ਅੰਕੜੇ.ਲੰਮਾਈ.unit = hint_ਲੰਮਾਈ.unit;
				hint->ਸਥਿਤੀ = CSS_PADDING_ਸੈੱਟ;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}
}

ਸਥਿਰ ਖਾਲੀ css_hint_vertical_align_ਸਾਰਣੀ_cells(
		nscss_select_ctx *ctx,
		dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = ਨਿਰਾ;
	dom_exception err;

	err = dom_element_get_attribute(ਗੰਢ,
			corestring_dom_valign, &attr);

	ਜੇ (err == DOM_NO_ERR && attr != ਨਿਰਾ) {
		hint->ਅੰਕੜੇ.ਲੰਮਾਈ.ਮੁੱਲ = 0;
		hint->ਅੰਕੜੇ.ਲੰਮਾਈ.unit = CSS_UNIT_PX;
		ਜੇ (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_ਸਿਖਰ)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->ਸਥਿਤੀ = CSS_VERTICAL_ALIGN_ਸਿਖਰ;
			css_hint_advance(&hint);

		} ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_ਵਿਚਕਾਰ)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->ਸਥਿਤੀ = CSS_VERTICAL_ALIGN_ਵਿਚਕਾਰ;
			css_hint_advance(&hint);

		} ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_bottom)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->ਸਥਿਤੀ = CSS_VERTICAL_ALIGN_BOTTOM;
			css_hint_advance(&hint);

		} ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_baseline)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->ਸਥਿਤੀ = CSS_VERTICAL_ALIGN_BASELINE;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ਸਥਿਰ ਖਾਲੀ css_hint_vertical_align_replaced(
		nscss_select_ctx *ctx,
		dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = ਨਿਰਾ;
	dom_exception err;

	err = dom_element_get_attribute(ਗੰਢ,
			corestring_dom_valign, &attr);

	ਜੇ (err == DOM_NO_ERR && attr != ਨਿਰਾ) {
		ਜੇ (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_ਸਿਖਰ)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->ਸਥਿਤੀ = CSS_VERTICAL_ALIGN_ਸਿਖਰ;
			css_hint_advance(&hint);

		} ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_bottom) ||
			   dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_baseline)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->ਸਥਿਤੀ = CSS_VERTICAL_ALIGN_BASELINE;
			css_hint_advance(&hint);

		} ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_texttop)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->ਸਥਿਤੀ = CSS_VERTICAL_ALIGN_ਪਾਠ_ਸਿਖਰ;
			css_hint_advance(&hint);

		} ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_absmiddle) ||
			   dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_abscenter)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->ਸਥਿਤੀ = CSS_VERTICAL_ALIGN_ਵਿਚਕਾਰ;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ਸਥਿਰ ਖਾਲੀ css_hint_ਪਾਠ_align_ਸਾਧਾਰਨ(
		nscss_select_ctx *ctx,
		dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *align = ਨਿਰਾ;
	dom_exception err;

	err = dom_element_get_attribute(ਗੰਢ,
			corestring_dom_align, &align);
	ਜੇ (err == DOM_NO_ERR && align != ਨਿਰਾ) {
		ਜੇ (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_ਖੱਬਾ)) {
			hint->prop = CSS_PROP_ਪਾਠ_ALIGN;
			hint->ਸਥਿਤੀ = CSS_ਪਾਠ_ALIGN_ਖੱਬਾ;
			css_hint_advance(&hint);

		} ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_center)) {
			hint->prop = CSS_PROP_ਪਾਠ_ALIGN;
			hint->ਸਥਿਤੀ = CSS_ਪਾਠ_ALIGN_CENTER;
			css_hint_advance(&hint);

		} ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_ਸੱਜਾ)) {
			hint->prop = CSS_PROP_ਪਾਠ_ALIGN;
			hint->ਸਥਿਤੀ = CSS_ਪਾਠ_ALIGN_ਸੱਜਾ;
			css_hint_advance(&hint);

		} ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_justify)) {
			hint->prop = CSS_PROP_ਪਾਠ_ALIGN;
			hint->ਸਥਿਤੀ = CSS_ਪਾਠ_ALIGN_JUSTIFY;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

ਸਥਿਰ ਖਾਲੀ css_hint_ਪਾਠ_align_center(
		nscss_select_ctx *ctx,
		dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ css_hint *hint = &hint_ctx.hints[hint_ctx.len];

	hint->prop = CSS_PROP_ਪਾਠ_ALIGN;
	hint->ਸਥਿਤੀ = CSS_ਪਾਠ_ALIGN_LIBCSS_CENTER;
	css_hint_advance(&hint);
}

ਸਥਿਰ ਖਾਲੀ css_hint_margin_ਖੱਬਾ_ਸੱਜਾ_align_center(
		nscss_select_ctx *ctx,
		dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr;
	dom_exception exc;

	exc = dom_element_get_attribute(ਗੰਢ,
			corestring_dom_align, &attr);

	ਜੇ (exc == DOM_NO_ERR && attr != ਨਿਰਾ) {
		memset(hint, 0, ਆਕਾਰ(*hint) * 2);
		ਜੇ (dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_center) ||
				dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_abscenter) ||
				dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_ਵਿਚਕਾਰ) ||
				dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_absmiddle)) {
			hint->prop = CSS_PROP_MARGIN_ਖੱਬਾ;
			hint->ਸਥਿਤੀ = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_ਸੱਜਾ;
			hint->ਸਥਿਤੀ = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ਸਥਿਰ ਖਾਲੀ css_hint_ਪਾਠ_align_special(
		nscss_select_ctx *ctx,
		dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *align = ਨਿਰਾ;
	dom_exception err;

	err = dom_element_get_attribute(ਗੰਢ,
			corestring_dom_align, &align);

	ਜੇ (err == DOM_NO_ERR && align != ਨਿਰਾ) {
		ਜੇ (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_center)) {
			hint->prop = CSS_PROP_ਪਾਠ_ALIGN;
			hint->ਸਥਿਤੀ = CSS_ਪਾਠ_ALIGN_LIBCSS_CENTER;
			css_hint_advance(&hint);

		} ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_ਖੱਬਾ)) {
			hint->prop = CSS_PROP_ਪਾਠ_ALIGN;
			hint->ਸਥਿਤੀ = CSS_ਪਾਠ_ALIGN_LIBCSS_ਖੱਬਾ;
			css_hint_advance(&hint);

		} ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_ਸੱਜਾ)) {
			hint->prop = CSS_PROP_ਪਾਠ_ALIGN;
			hint->ਸਥਿਤੀ = CSS_ਪਾਠ_ALIGN_LIBCSS_ਸੱਜਾ;
			css_hint_advance(&hint);

		} ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_justify)) {
			hint->prop = CSS_PROP_ਪਾਠ_ALIGN;
			hint->ਸਥਿਤੀ = CSS_ਪਾਠ_ALIGN_JUSTIFY;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

ਸਥਿਰ ਖਾਲੀ css_hint_ਪਾਠ_align_ਸਾਰਣੀ_special(
		nscss_select_ctx *ctx,
		dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ css_hint *hint = &hint_ctx.hints[hint_ctx.len];

	hint->prop = CSS_PROP_ਪਾਠ_ALIGN;
	hint->ਸਥਿਤੀ = CSS_ਪਾਠ_ALIGN_INHERIT_IF_NON_MAGIC;
	css_hint_advance(&hint);
}

ਸਥਿਰ ਖਾਲੀ css_hint_margin_hspace_vspace(
		nscss_select_ctx *ctx,
		dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = ਨਿਰਾ;
	dom_exception exc;

	exc = dom_element_get_attribute(ਗੰਢ,
			corestring_dom_vspace, &attr);

	ਜੇ (exc == DOM_NO_ERR && attr != ਨਿਰਾ) {
		css_hint_ਲੰਮਾਈ hint_ਲੰਮਾਈ;
		ਜੇ (parse_dimension(
				dom_string_ਅੰਕੜੇ(attr), false,
				&hint_ਲੰਮਾਈ.ਮੁੱਲ,
				&hint_ਲੰਮਾਈ.unit)) {
			hint->prop = CSS_PROP_MARGIN_ਸਿਖਰ;
			hint->ਅੰਕੜੇ.ਲੰਮਾਈ.ਮੁੱਲ = hint_ਲੰਮਾਈ.ਮੁੱਲ;
			hint->ਅੰਕੜੇ.ਲੰਮਾਈ.unit = hint_ਲੰਮਾਈ.unit;
			hint->ਸਥਿਤੀ = CSS_MARGIN_ਸੈੱਟ;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_BOTTOM;
			hint->ਅੰਕੜੇ.ਲੰਮਾਈ.ਮੁੱਲ = hint_ਲੰਮਾਈ.ਮੁੱਲ;
			hint->ਅੰਕੜੇ.ਲੰਮਾਈ.unit = hint_ਲੰਮਾਈ.unit;
			hint->ਸਥਿਤੀ = CSS_MARGIN_ਸੈੱਟ;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(ਗੰਢ,
			corestring_dom_hspace, &attr);

	ਜੇ (exc == DOM_NO_ERR && attr != ਨਿਰਾ) {
		css_hint_ਲੰਮਾਈ hint_ਲੰਮਾਈ;
		ਜੇ (parse_dimension(
				dom_string_ਅੰਕੜੇ(attr), false,
				&hint_ਲੰਮਾਈ.ਮੁੱਲ,
				&hint_ਲੰਮਾਈ.unit)) {
			hint->prop = CSS_PROP_MARGIN_ਖੱਬਾ;
			hint->ਅੰਕੜੇ.ਲੰਮਾਈ.ਮੁੱਲ = hint_ਲੰਮਾਈ.ਮੁੱਲ;
			hint->ਅੰਕੜੇ.ਲੰਮਾਈ.unit = hint_ਲੰਮਾਈ.unit;
			hint->ਸਥਿਤੀ = CSS_MARGIN_ਸੈੱਟ;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_ਸੱਜਾ;
			hint->ਅੰਕੜੇ.ਲੰਮਾਈ.ਮੁੱਲ = hint_ਲੰਮਾਈ.ਮੁੱਲ;
			hint->ਅੰਕੜੇ.ਲੰਮਾਈ.unit = hint_ਲੰਮਾਈ.unit;
			hint->ਸਥਿਤੀ = CSS_MARGIN_ਸੈੱਟ;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ਸਥਿਰ ਖਾਲੀ css_hint_margin_ਖੱਬਾ_ਸੱਜਾ_hr(
		nscss_select_ctx *ctx,
		dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr;
	dom_exception exc;

	exc = dom_element_get_attribute(ਗੰਢ,
				corestring_dom_align, &attr);

	ਜੇ (exc == DOM_NO_ERR && attr != ਨਿਰਾ) {
		memset(hint, 0, ਆਕਾਰ(*hint) * 2);
		ਜੇ (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_ਖੱਬਾ)) {
			hint->prop = CSS_PROP_MARGIN_ਖੱਬਾ;
			hint->ਅੰਕੜੇ.ਲੰਮਾਈ.ਮੁੱਲ = 0;
			hint->ਅੰਕੜੇ.ਲੰਮਾਈ.unit = CSS_UNIT_PX;
			hint->ਸਥਿਤੀ = CSS_MARGIN_ਸੈੱਟ;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_ਸੱਜਾ;
			hint->ਸਥਿਤੀ = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

		} ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_center)) {
			hint->prop = CSS_PROP_MARGIN_ਖੱਬਾ;
			hint->ਸਥਿਤੀ = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_ਸੱਜਾ;
			hint->ਸਥਿਤੀ = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

		} ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_ਸੱਜਾ)) {
			hint->prop = CSS_PROP_MARGIN_ਖੱਬਾ;
			hint->ਸਥਿਤੀ = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_ਸੱਜਾ;
			hint->ਅੰਕੜੇ.ਲੰਮਾਈ.ਮੁੱਲ = 0;
			hint->ਅੰਕੜੇ.ਲੰਮਾਈ.unit = CSS_UNIT_PX;
			hint->ਸਥਿਤੀ = CSS_MARGIN_ਸੈੱਟ;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ਸਥਿਰ ਖਾਲੀ css_hint_ਸਾਰਣੀ_spacing_border(
		nscss_select_ctx *ctx,
		dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception exc;
	dom_string *attr = ਨਿਰਾ;

	exc = dom_element_get_attribute(ਗੰਢ, corestring_dom_border, &attr);

	ਜੇ (exc == DOM_NO_ERR && attr != ਨਿਰਾ) {
		uint32_t hint_prop;
		css_hint_ਲੰਮਾਈ hint_ਲੰਮਾਈ;

		ਲਈ (hint_prop = CSS_PROP_BORDER_ਸਿਖਰ_STYLE;
			     hint_prop <= CSS_PROP_BORDER_ਖੱਬਾ_STYLE;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->ਸਥਿਤੀ = CSS_BORDER_STYLE_OUTSET;
				css_hint_advance(&hint);
		}

		ਜੇ (parse_dimension(
				dom_string_ਅੰਕੜੇ(attr), false,
						&hint_ਲੰਮਾਈ.ਮੁੱਲ,
						&hint_ਲੰਮਾਈ.unit)) {

			ਲਈ (hint_prop = CSS_PROP_BORDER_ਸਿਖਰ_ਚੌੜਾਈ;
			     hint_prop <= CSS_PROP_BORDER_ਖੱਬਾ_ਚੌੜਾਈ;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->ਅੰਕੜੇ.ਲੰਮਾਈ.ਮੁੱਲ = hint_ਲੰਮਾਈ.ਮੁੱਲ;
				hint->ਅੰਕੜੇ.ਲੰਮਾਈ.unit = hint_ਲੰਮਾਈ.unit;
				hint->ਸਥਿਤੀ = CSS_BORDER_ਚੌੜਾਈ_ਚੌੜਾਈ;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(ਗੰਢ,
			corestring_dom_bordercolor, &attr);

	ਜੇ (exc == DOM_NO_ERR && attr != ਨਿਰਾ) {
		uint32_t hint_prop;
		css_color hint_color;

		ਜੇ (nscss_parse_ਰੰਗ(
				(ਅਚਲ ਅੱਖਰ *)dom_string_ਅੰਕੜੇ(attr),
				&hint_color)) {

			ਲਈ (hint_prop = CSS_PROP_BORDER_ਸਿਖਰ_COLOR;
			     hint_prop <= CSS_PROP_BORDER_ਖੱਬਾ_COLOR;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->ਅੰਕੜੇ.color = hint_color;
				hint->ਸਥਿਤੀ = CSS_BORDER_COLOR_COLOR;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(ਗੰਢ,
			corestring_dom_cellspacing, &attr);

	ਜੇ (exc == DOM_NO_ERR && attr != ਨਿਰਾ) {
		ਜੇ (parse_dimension(
				(ਅਚਲ ਅੱਖਰ *)dom_string_ਅੰਕੜੇ(attr), false,
						&hint->ਅੰਕੜੇ.position.h.ਮੁੱਲ,
						&hint->ਅੰਕੜੇ.position.h.unit)) {
			hint->prop = CSS_PROP_BORDER_SPACING;
			hint->ਅੰਕੜੇ.position.v = hint->ਅੰਕੜੇ.position.h;
			hint->ਸਥਿਤੀ = CSS_BORDER_SPACING_ਸੈੱਟ;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ਸਥਿਰ ਖਾਲੀ css_hint_ਉਚਾਈ(
		nscss_select_ctx *ctx,
		dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = ਨਿਰਾ;
	dom_exception err;

	err = dom_element_get_attribute(ਗੰਢ,
			corestring_dom_ਉਚਾਈ, &attr);

	ਜੇ (err == DOM_NO_ERR && attr != ਨਿਰਾ) {
		ਜੇ (parse_dimension(
				(ਅਚਲ ਅੱਖਰ *)dom_string_ਅੰਕੜੇ(attr), false,
				&hint->ਅੰਕੜੇ.ਲੰਮਾਈ.ਮੁੱਲ,
				&hint->ਅੰਕੜੇ.ਲੰਮਾਈ.unit)) {
			hint->prop = CSS_PROP_ਉਚਾਈ;
			hint->ਸਥਿਤੀ = CSS_ਉਚਾਈ_ਸੈੱਟ;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ਸਥਿਰ ਖਾਲੀ css_hint_ਚੌੜਾਈ(
		nscss_select_ctx *ctx,
		dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = ਨਿਰਾ;
	dom_exception err;

	err = dom_element_get_attribute(ਗੰਢ,
			corestring_dom_ਚੌੜਾਈ, &attr);

	ਜੇ (err == DOM_NO_ERR && attr != ਨਿਰਾ) {
		ਜੇ (parse_dimension(
				(ਅਚਲ ਅੱਖਰ *)dom_string_ਅੰਕੜੇ(attr), false,
				&hint->ਅੰਕੜੇ.ਲੰਮਾਈ.ਮੁੱਲ,
				&hint->ਅੰਕੜੇ.ਲੰਮਾਈ.unit)) {
			hint->prop = CSS_PROP_ਚੌੜਾਈ;
			hint->ਸਥਿਤੀ = CSS_ਚੌੜਾਈ_ਸੈੱਟ;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ਸਥਿਰ ਖਾਲੀ css_hint_ਉਚਾਈ_ਚੌੜਾਈ_textarea(
		nscss_select_ctx *ctx,
		dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = ਨਿਰਾ;
	dom_exception err;

	err = dom_element_get_attribute(ਗੰਢ,
			corestring_dom_ਪੰਕਤੀਆਂ, &attr);

	ਜੇ (err == DOM_NO_ERR && attr != ਨਿਰਾ) {
		ਜੇ (parse_dimension(
				(ਅਚਲ ਅੱਖਰ *)dom_string_ਅੰਕੜੇ(attr), false,
				&hint->ਅੰਕੜੇ.ਲੰਮਾਈ.ਮੁੱਲ,
				&hint->ਅੰਕੜੇ.ਲੰਮਾਈ.unit)) {
			hint->prop = CSS_PROP_ਉਚਾਈ;
			hint->ਅੰਕੜੇ.ਲੰਮਾਈ.unit = CSS_UNIT_EM;
			hint->ਸਥਿਤੀ = CSS_ਉਚਾਈ_ਸੈੱਟ;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}

	err = dom_element_get_attribute(ਗੰਢ,
			corestring_dom_ਖਾਨੇ_ਗਿਣਤੀ, &attr);

	ਜੇ (err == DOM_NO_ERR && attr != ਨਿਰਾ) {
		ਜੇ (parse_dimension(
				(ਅਚਲ ਅੱਖਰ *)dom_string_ਅੰਕੜੇ(attr), false,
				&hint->ਅੰਕੜੇ.ਲੰਮਾਈ.ਮੁੱਲ,
				&hint->ਅੰਕੜੇ.ਲੰਮਾਈ.unit)) {
			hint->prop = CSS_PROP_ਚੌੜਾਈ;
			hint->ਅੰਕੜੇ.ਲੰਮਾਈ.unit = CSS_UNIT_EX;
			hint->ਸਥਿਤੀ = CSS_ਚੌੜਾਈ_ਸੈੱਟ;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ਸਥਿਰ ਖਾਲੀ css_hint_ਉਚਾਈ_ਚੌੜਾਈ_canvas(
		nscss_select_ctx *ctx,
		dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = ਨਿਰਾ;
	dom_exception err;
	bool ਸੈੱਟ_dim = false;

	err = dom_element_get_attribute(ਗੰਢ,
			corestring_dom_ਉਚਾਈ, &attr);

	ਜੇ (err == DOM_NO_ERR && attr != ਨਿਰਾ) {
		ਜੇ (parse_dimension(
				(ਅਚਲ ਅੱਖਰ *)dom_string_ਅੰਕੜੇ(attr), true,
				&hint->ਅੰਕੜੇ.ਲੰਮਾਈ.ਮੁੱਲ,
				&hint->ਅੰਕੜੇ.ਲੰਮਾਈ.unit)) {
			hint->prop = CSS_PROP_ਉਚਾਈ;
			hint->ਅੰਕੜੇ.ਲੰਮਾਈ.unit = CSS_UNIT_PX;
			hint->ਸਥਿਤੀ = CSS_ਉਚਾਈ_ਸੈੱਟ;
			css_hint_advance(&hint);
			ਸੈੱਟ_dim = true;
		}
		dom_string_unref(attr);
	}
	ਜੇ (ਸੈੱਟ_dim == false) {
		/* canvas defaults to 150px tall */
		hint->prop = CSS_PROP_ਉਚਾਈ;
		hint->ਅੰਕੜੇ.ਲੰਮਾਈ.unit = CSS_UNIT_PX;
		hint->ਅੰਕੜੇ.ਲੰਮਾਈ.ਮੁੱਲ = INTTOFIX(150);
		hint->ਸਥਿਤੀ = CSS_ਉਚਾਈ_ਸੈੱਟ;
		css_hint_advance(&hint);
	} ਨਹੀਂਤਾਂ {
		ਸੈੱਟ_dim = false;
	}

	err = dom_element_get_attribute(ਗੰਢ,
			corestring_dom_ਚੌੜਾਈ, &attr);

	ਜੇ (err == DOM_NO_ERR && attr != ਨਿਰਾ) {
		ਜੇ (parse_dimension(
				(ਅਚਲ ਅੱਖਰ *)dom_string_ਅੰਕੜੇ(attr), true,
				&hint->ਅੰਕੜੇ.ਲੰਮਾਈ.ਮੁੱਲ,
				&hint->ਅੰਕੜੇ.ਲੰਮਾਈ.unit)) {
			hint->prop = CSS_PROP_ਚੌੜਾਈ;
			hint->ਅੰਕੜੇ.ਲੰਮਾਈ.unit = CSS_UNIT_PX;
			hint->ਸਥਿਤੀ = CSS_ਚੌੜਾਈ_ਸੈੱਟ;
			css_hint_advance(&hint);
			ਸੈੱਟ_dim = true;
		}
		dom_string_unref(attr);
	}
	ਜੇ (ਸੈੱਟ_dim == false) {
		/* canvas defaults to 300px wide */
		hint->prop = CSS_PROP_ਚੌੜਾਈ;
		hint->ਅੰਕੜੇ.ਲੰਮਾਈ.unit = CSS_UNIT_PX;
		hint->ਅੰਕੜੇ.ਲੰਮਾਈ.ਮੁੱਲ = INTTOFIX(300);
		hint->ਸਥਿਤੀ = CSS_ਚੌੜਾਈ_ਸੈੱਟ;
		css_hint_advance(&hint);
	}
}

ਸਥਿਰ ਖਾਲੀ css_hint_ਚੌੜਾਈ_ਨਿਵੇਸ਼(
		nscss_select_ctx *ctx,
		dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_string *attr = ਨਿਰਾ;
	dom_exception err;

	err = dom_element_get_attribute(ਗੰਢ,
			corestring_dom_ਮਾਪ, &attr);

	ਜੇ (err == DOM_NO_ERR && attr != ਨਿਰਾ) {
		ਜੇ (parse_dimension(
				(ਅਚਲ ਅੱਖਰ *)dom_string_ਅੰਕੜੇ(attr), false,
				&hint->ਅੰਕੜੇ.ਲੰਮਾਈ.ਮੁੱਲ,
				&hint->ਅੰਕੜੇ.ਲੰਮਾਈ.unit)) {
			dom_string *attr2 = ਨਿਰਾ;

			err = dom_element_get_attribute(ਗੰਢ,
					corestring_dom_type, &attr2);
			ਜੇ (err == DOM_NO_ERR) {

				hint->prop = CSS_PROP_ਚੌੜਾਈ;
				hint->ਸਥਿਤੀ = CSS_ਚੌੜਾਈ_ਸੈੱਟ;

				ਜੇ (attr2 == ਨਿਰਾ ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_ਪਾਠ) ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_ਖੋਜ) ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_password) ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_file)) {
					hint->ਅੰਕੜੇ.ਲੰਮਾਈ.unit = CSS_UNIT_EX;
				}
				ਜੇ (attr2 != ਨਿਰਾ) {
					dom_string_unref(attr2);
				}
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}
}

ਸਥਿਰ ਖਾਲੀ css_hint_anchor_color(
		nscss_select_ctx *ctx,
		dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	css_error error;
	dom_exception err;
	dom_string *color;
	dom_ਗੰਢ *bodynode = ਨਿਰਾ;

	/* find body node */
	css_qname qs;
	bool is_ਦੇਖਿਆ;

	qs.ns = ਨਿਰਾ;
	qs.name = lwc_string_ref(corestring_lwc_body);
	ਜੇ (named_ancestor_ਗੰਢ(ctx, ਗੰਢ, &qs,
			(ਖਾਲੀ *)&bodynode) != CSS_OK) {
		/* Didn't find, or had error */
		lwc_string_unref(qs.name);
		ਵਾਪਸ ;
	}
	lwc_string_unref(qs.name);

	ਜੇ (bodynode == ਨਿਰਾ) {
		ਵਾਪਸ;
	}

	error = ਗੰਢ_is_ਦੇਖਿਆ(ctx, ਗੰਢ, &is_ਦੇਖਿਆ);
	ਜੇ (error != CSS_OK)
		ਵਾਪਸ;

	ਜੇ (is_ਦੇਖਿਆ) {
		err = dom_element_get_attribute(bodynode,
				corestring_dom_vlink, &color);
	} ਨਹੀਂਤਾਂ {
		err = dom_element_get_attribute(bodynode,
				corestring_dom_link, &color);
	}

	ਜੇ (err == DOM_NO_ERR && color != ਨਿਰਾ) {
		ਜੇ (nscss_parse_ਰੰਗ(
				(ਅਚਲ ਅੱਖਰ *)dom_string_ਅੰਕੜੇ(color),
						&hint->ਅੰਕੜੇ.color)) {
			hint->prop = CSS_PROP_COLOR;
			hint->ਸਥਿਤੀ = CSS_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(color);
	}
}

ਸਥਿਰ ਖਾਲੀ css_hint_body_color(
		nscss_select_ctx *ctx,
		dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *color;

	err = dom_element_get_attribute(ਗੰਢ, corestring_dom_ਪਾਠ, &color);

	ਜੇ (err == DOM_NO_ERR && color != ਨਿਰਾ) {
		ਜੇ (nscss_parse_ਰੰਗ(
				(ਅਚਲ ਅੱਖਰ *)dom_string_ਅੰਕੜੇ(color),
						&hint->ਅੰਕੜੇ.color)) {
			hint->prop = CSS_PROP_COLOR;
			hint->ਸਥਿਤੀ = CSS_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(color);
	}
}

ਸਥਿਰ ਖਾਲੀ css_hint_color(
		nscss_select_ctx *ctx,
		dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *color;

	err = dom_element_get_attribute(ਗੰਢ, corestring_dom_color, &color);

	ਜੇ (err == DOM_NO_ERR && color != ਨਿਰਾ) {
		ਜੇ (nscss_parse_ਰੰਗ(
				(ਅਚਲ ਅੱਖਰ *)dom_string_ਅੰਕੜੇ(color),
						&hint->ਅੰਕੜੇ.color)) {
			hint->prop = CSS_PROP_COLOR;
			hint->ਸਥਿਤੀ = CSS_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(color);
	}
}

ਸਥਿਰ ਖਾਲੀ css_hint_font_ਮਾਪ(
		nscss_select_ctx *ctx,
		dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *ਮਾਪ;

	err = dom_element_get_attribute(ਗੰਢ, corestring_dom_ਮਾਪ, &ਮਾਪ);
	ਜੇ (err == DOM_NO_ERR && ਮਾਪ != ਨਿਰਾ) {
		ਜੇ (parse_font_ਮਾਪ(
				(ਅਚਲ ਅੱਖਰ *)dom_string_ਅੰਕੜੇ(ਮਾਪ),
						&hint->ਸਥਿਤੀ,
						&hint->ਅੰਕੜੇ.ਲੰਮਾਈ.ਮੁੱਲ,
						&hint->ਅੰਕੜੇ.ਲੰਮਾਈ.unit)) {
			hint->prop = CSS_PROP_FONT_ਮਾਪ;
			css_hint_advance(&hint);
		}
		dom_string_unref(ਮਾਪ);
	}
}

ਸਥਿਰ ਖਾਲੀ css_hint_float(
		nscss_select_ctx *ctx,
		dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *align;

	err = dom_element_get_attribute(ਗੰਢ, corestring_dom_align, &align);
	ਜੇ (err == DOM_NO_ERR && align != ਨਿਰਾ) {
		ਜੇ (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_ਖੱਬਾ)) {
			hint->prop = CSS_PROP_FLOAT;
			hint->ਸਥਿਤੀ = CSS_FLOAT_ਖੱਬਾ;
			css_hint_advance(&hint);

		} ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_ਸੱਜਾ)) {
			hint->prop = CSS_PROP_FLOAT;
			hint->ਸਥਿਤੀ = CSS_FLOAT_ਸੱਜਾ;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

ਸਥਿਰ ਖਾਲੀ css_hint_caption_side(
		nscss_select_ctx *ctx,
		dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *align = ਨਿਰਾ;

	err = dom_element_get_attribute(ਗੰਢ, corestring_dom_align, &align);
	ਜੇ (err == DOM_NO_ERR && align != ਨਿਰਾ) {
		ਜੇ (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_bottom)) {
			hint->prop = CSS_PROP_CAPTION_SIDE;
			hint->ਸਥਿਤੀ = CSS_CAPTION_SIDE_BOTTOM;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

ਸਥਿਰ ਖਾਲੀ css_hint_bg_color(
		nscss_select_ctx *ctx,
		dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *bgcolor;

	err = dom_element_get_attribute(ਗੰਢ,
			corestring_dom_bgcolor, &bgcolor);
	ਜੇ (err == DOM_NO_ERR && bgcolor != ਨਿਰਾ) {
		ਜੇ (nscss_parse_ਰੰਗ(
				(ਅਚਲ ਅੱਖਰ *)dom_string_ਅੰਕੜੇ(bgcolor),
				&hint->ਅੰਕੜੇ.color)) {
			hint->prop = CSS_PROP_BACKGROUND_COLOR;
			hint->ਸਥਿਤੀ = CSS_BACKGROUND_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(bgcolor);
	}
}

ਸਥਿਰ ਖਾਲੀ css_hint_bg_image(
		nscss_select_ctx *ctx,
		dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_exception err;
	dom_string *attr;

	err = dom_element_get_attribute(ਗੰਢ,
			corestring_dom_background, &attr);
	ਜੇ (err == DOM_NO_ERR && attr != ਨਿਰਾ) {
		nsurl *url;
		nserror error = nsurl_join(ctx->ਅਧਾਰ_url,
				(ਅਚਲ ਅੱਖਰ *)dom_string_ਅੰਕੜੇ(attr), &url);
		dom_string_unref(attr);

		ਜੇ (error == NSERROR_OK) {
			lwc_string *iurl;
			lwc_error lerror = lwc_intern_string(nsurl_access(url),
					nsurl_ਲੰਮਾਈ(url), &iurl);
			nsurl_unref(url);

			ਜੇ (lerror == lwc_error_ok) {
				hint->prop = CSS_PROP_BACKGROUND_IMAGE;
				hint->ਅੰਕੜੇ.string = iurl;
				hint->ਸਥਿਤੀ = CSS_BACKGROUND_IMAGE_IMAGE;
				css_hint_advance(&hint);
			}
		}
	}
}

ਸਥਿਰ ਖਾਲੀ css_hint_white_space_nowrap(
		nscss_select_ctx *ctx,
		dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_exception err;
	bool nowrap;

	err = dom_element_has_attribute(ਗੰਢ, corestring_dom_nowrap, &nowrap);
	ਜੇ (err == DOM_NO_ERR && nowrap == true) {
		hint->prop = CSS_PROP_WHITE_SPACE;
		hint->ਸਥਿਤੀ = CSS_WHITE_SPACE_NOWRAP;
		css_hint_advance(&hint);
	}
}

ਸਥਿਰ ਖਾਲੀ css_hint_list(
		nscss_select_ctx *ctx,
		dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_exception err;
	dom_string *attr;

	err = dom_element_get_attribute(ਗੰਢ, corestring_dom_type, &attr);
	ਜੇ (err == DOM_NO_ERR && attr != ਨਿਰਾ) {
		ਅਚਲ ਅੱਖਰ *attr_str = dom_string_ਅੰਕੜੇ(attr);
		size_t attr_len = dom_string_byte_ਲੰਮਾਈ(attr);
		ਗਿਣਤੀ css_list_style_type_e type = CSS_LIST_STYLE_TYPE_INHERIT;

		ਜੇ (attr_len == 1) {
			ਚੋਣ (attr_str[0]) {
			ਹਾਲਤ 'a':
				type = CSS_LIST_STYLE_TYPE_ਹੇਠਲਾ_ALPHA;
				ਰੋਕੋ;
			ਹਾਲਤ 'A':
				type = CSS_LIST_STYLE_TYPE_ਉੱਤਲਾ_ALPHA;
				ਰੋਕੋ;
			ਹਾਲਤ 'i':
				type = CSS_LIST_STYLE_TYPE_ਹੇਠਲਾ_ROMAN;
				ਰੋਕੋ;
			ਹਾਲਤ 'I':
				type = CSS_LIST_STYLE_TYPE_ਉੱਤਲਾ_ROMAN;
				ਰੋਕੋ;
			ਹਾਲਤ '1':
				type = CSS_LIST_STYLE_TYPE_DECIMAL;
				ਰੋਕੋ;
			}
		}

		ਜੇ (type != CSS_LIST_STYLE_TYPE_INHERIT) {
			hint->prop = CSS_PROP_LIST_STYLE_TYPE;
			hint->ਸਥਿਤੀ = type;
			css_hint_advance(&hint);
		}

		dom_string_unref(attr);
	}
}


/* Exported function, documented in css/hints.h */
css_error ਗੰਢ_presentational_hint(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		uint32_t *nhints, css_hint **hints)
{
	dom_exception exc;
	dom_html_element_type ਟੈਗ_type;

	ਜੇ (nsoption_bool(author_level_css) == false) {
		*nhints = 0;
		*hints = ਨਿਰਾ;
		ਵਾਪਸ CSS_OK;
	}

	css_hint_clean();

	exc = dom_html_element_get_ਟੈਗ_type(ਗੰਢ, &ਟੈਗ_type);
	ਜੇ (exc != DOM_NO_ERR) {
		ਟੈਗ_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	ਚੋਣ (ਟੈਗ_type) {
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_TH:
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_TD:
		css_hint_ਚੌੜਾਈ(pw, ਗੰਢ);
		css_hint_ਸਾਰਣੀ_cell_border_padding(pw, ਗੰਢ);
		css_hint_white_space_nowrap(pw, ਗੰਢ);
		fallthrough;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_TR:
		css_hint_ਉਚਾਈ(pw, ਗੰਢ);
		fallthrough;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_THEAD:
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_TBODY:
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_TFOOT:
		css_hint_ਪਾਠ_align_special(pw, ਗੰਢ);
		fallthrough;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_ਖਾਨਾ:
		css_hint_vertical_align_ਸਾਰਣੀ_cells(pw, ਗੰਢ);
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_APPLET:
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_IMG:
		css_hint_margin_hspace_vspace(pw, ਗੰਢ);
		fallthrough;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_EMBED:
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_IFRAME:
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_OBJECT:
		css_hint_ਉਚਾਈ(pw, ਗੰਢ);
		css_hint_ਚੌੜਾਈ(pw, ਗੰਢ);
		css_hint_vertical_align_replaced(pw, ਗੰਢ);
		css_hint_float(pw, ਗੰਢ);
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_P:
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_H1:
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_H2:
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_H3:
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_H4:
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_H5:
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_H6:
		css_hint_ਪਾਠ_align_ਸਾਧਾਰਨ(pw, ਗੰਢ);
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_CENTER:
		css_hint_ਪਾਠ_align_center(pw, ਗੰਢ);
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_CAPTION:
		css_hint_caption_side(pw, ਗੰਢ);
		fallthrough;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_DIV:
		css_hint_ਪਾਠ_align_special(pw, ਗੰਢ);
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_ਸਾਰਣੀ:
		css_hint_ਪਾਠ_align_ਸਾਰਣੀ_special(pw, ਗੰਢ);
		css_hint_ਸਾਰਣੀ_spacing_border(pw, ਗੰਢ);
		css_hint_float(pw, ਗੰਢ);
		css_hint_margin_ਖੱਬਾ_ਸੱਜਾ_align_center(pw, ਗੰਢ);
		css_hint_ਚੌੜਾਈ(pw, ਗੰਢ);
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_HR:
		css_hint_ਚੌੜਾਈ(pw, ਗੰਢ);
		css_hint_margin_ਖੱਬਾ_ਸੱਜਾ_hr(pw, ਗੰਢ);
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		css_hint_ਉਚਾਈ_ਚੌੜਾਈ_textarea(pw, ਗੰਢ);
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_ਨਿਵੇਸ਼:
		css_hint_ਚੌੜਾਈ_ਨਿਵੇਸ਼(pw, ਗੰਢ);
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_A:
		css_hint_anchor_color(pw, ਗੰਢ);
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_FONT:
		css_hint_font_ਮਾਪ(pw, ਗੰਢ);
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_BODY:
		css_hint_body_color(pw, ਗੰਢ);
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_CANVAS:
		css_hint_ਉਚਾਈ_ਚੌੜਾਈ_canvas(pw, ਗੰਢ);
		ਰੋਕੋ;
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_OL:
		css_hint_list(pw, ਗੰਢ);
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	ਜੇ (ਟੈਗ_type != DOM_HTML_ELEMENT_TYPE__UNKNOWN) {
		css_hint_color(pw, ਗੰਢ);
		css_hint_bg_color(pw, ਗੰਢ);
		css_hint_bg_image(pw, ਗੰਢ);
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ LOG_STATS
	NSLOG(netsurf, INFO, "Properties with hints: %i", hint_ctx.len);
#ਜੇ_ਅੰਤ

	css_hint_get_hints(hints, nhints);

	ਵਾਪਸ CSS_OK;
}
