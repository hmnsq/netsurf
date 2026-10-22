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

#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <strings.h>

#ಅನ್ತರ್ಭಾವ "utils/nsoption.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/nsurl.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"

#ಅನ್ತರ್ಭಾವ "css/hints.h"
#ಅನ್ತರ್ಭಾವ "css/select.h"

#ವ್ಯಾಖ್ಯಾನಿಸು LOG_STATS
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು LOG_STATS

/******************************************************************************
 * Utility functions                                                          *
 ******************************************************************************/

/**
 * Determine if a given character is whitespace
 *
 * \param c  Character to consider
 * \return true if character is whitespace, false otherwise
 */
ಸ್ಥಿರ bool isWhitespace(ಅಕ್ಷರ c)
{
	ಹಿಂತಿರುಗಿ c == ' ' || c == '\t' || c == '\f' || c == '\r' || c == '\n';
}

/**
 * Determine if a given character is a valid hex digit
 *
 * \param c  Character to consider
 * \return true if character is a valid hex digit, false otherwise
 */
ಸ್ಥಿರ bool isHex(ಅಕ್ಷರ c)
{
	ಹಿಂತಿರುಗಿ ('0' <= c && c <= '9') ||
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
ಸ್ಥಿರ uint8_t char_ಗಮ್ಯ_Hex(ಅಕ್ಷರ c)
{
	/* 0-9 */
	c -= '0';

	/* A-F */
	ಆದರೆ (c > 9)
		c -= 'A' - '9' - 1;

	/* a-f */
	ಆದರೆ (c > 15)
		c -= 'a' - 'A';

	ಹಿಂತಿರುಗಿ c;
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
ಸ್ಥಿರ bool parse_ಸಂಖ್ಯೆ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ದತ್ತಾಂಶ, bool maybe_negative, bool real,
		css_fixed *ಮೌಲ್ಯ, size_t *consumed)
{
	size_t len;
	ಸ್ಥಿರಾಂಕ uint8_t *ptr;
	int32_t intpart = 0;
	int32_t fracpart = 0;
	int32_t pwr = 1;
	ಪೂರ್ಣಾಂಕ sign = 1;

	*consumed = 0;

	len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ದತ್ತಾಂಶ);
	ptr = (ಸ್ಥಿರಾಂಕ uint8_t *) ದತ್ತಾಂಶ;

	ಆದರೆ (len == 0)
		ಹಿಂತಿರುಗಿ false;

	/* Skip leading whitespace */
	ಆಗ (len > 0 && isWhitespace(ptr[0])) {
		len--;
		ptr++;
	}

	ಆದರೆ (len == 0)
		ಹಿಂತಿರುಗಿ false;

	/* Extract sign, if any */
	ಆದರೆ (ptr[0] == '+') {
		len--;
		ptr++;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ptr[0] == '-' && maybe_negative) {
		sign = -1;
		len--;
		ptr++;
	}

	ಆದರೆ (len == 0)
		ಹಿಂತಿರುಗಿ false;

	/* Must have a digit [0,9] */
	ಆದರೆ ('0' > ptr[0] || ptr[0] > '9')
		ಹಿಂತಿರುಗಿ false;

	/* Now extract intpart, assuming base 10 */
	ಆಗ (len > 0) {
		/* Stop on first non-digit */
		ಆದರೆ (ptr[0] < '0' || '9' < ptr[0])
			ನಿಲ್ಲಿಸು;

		/* Prevent overflow of 'intpart'; proper clamping below */
		ಆದರೆ (intpart < (1 << 22)) {
			intpart *= 10;
			intpart += ptr[0] - '0';
		}
		ptr++;
		len--;
	}

	/* And fracpart, again, assuming base 10 */
	ಆದರೆ (real && len > 1 && ptr[0] == '.' &&
			('0' <= ptr[1] && ptr[1] <= '9')) {
		ptr++;
		len--;

		ಆಗ (len > 0) {
			ಆದರೆ (ptr[0] < '0' || '9' < ptr[0])
				ನಿಲ್ಲಿಸು;

			ಆದರೆ (pwr < 1000000) {
				pwr *= 10;
				fracpart *= 10;
				fracpart += ptr[0] - '0';
			}
			ptr++;
			len--;
		}

		fracpart = ((1 << 10) * fracpart + pwr/2) / pwr;
		ಆದರೆ (fracpart >= (1 << 10)) {
			intpart++;
			fracpart &= (1 << 10) - 1;
		}
	}

	ಆದರೆ (sign > 0) {
		/* If the result is larger than we can represent,
		 * then clamp to the maximum value we can store. */
		ಆದರೆ (intpart >= (1 << 21)) {
			intpart = (1 << 21) - 1;
			fracpart = (1 << 10) - 1;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* If the negated result is smaller than we can represent
		 * then clamp to the minimum value we can store. */
		ಆದರೆ (intpart >= (1 << 21)) {
			intpart = -(1 << 21);
			fracpart = 0;
		} ಇಲ್ಲದಿದ್ದರೆ {
			intpart = -intpart;
			ಆದರೆ (fracpart) {
				fracpart = (1 << 10) - fracpart;
				intpart--;
			}
		}
	}

	*ಮೌಲ್ಯ = (intpart << 10) | fracpart;

	*consumed = ptr - (ಸ್ಥಿರಾಂಕ uint8_t *) ದತ್ತಾಂಶ;

	ಹಿಂತಿರುಗಿ true;
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
ಸ್ಥಿರ bool parse_dimension(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ದತ್ತಾಂಶ, bool strict, css_fixed *ಉದ್ದಳತೆ,
		css_unit *unit)
{
	size_t len;
	size_t ಪಠ;
	css_fixed ಮೌಲ್ಯ;

	len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ದತ್ತಾಂಶ);

	ಆದರೆ (parse_ಸಂಖ್ಯೆ(ದತ್ತಾಂಶ, false, true, &ಮೌಲ್ಯ, &ಪಠ) == false)
		ಹಿಂತಿರುಗಿ false;

	ಆದರೆ (strict && ಮೌಲ್ಯ < INTTOFIX(1))
		ಹಿಂತಿರುಗಿ false;

	*ಉದ್ದಳತೆ = ಮೌಲ್ಯ;

	ಆದರೆ (len > ಪಠ && ದತ್ತಾಂಶ[ಪಠ] == '%')
		*unit = CSS_UNIT_PCT;
	ಇಲ್ಲದಿದ್ದರೆ
		*unit = CSS_UNIT_PX;

	ಹಿಂತಿರುಗಿ true;
}

/**
 * Mapping of colour name to CSS color
 */
ರಚನೆ ಬಣ್ಣ_map {
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cmp_ಬಣ್ಣ_name(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *a, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *b)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *aa = a;
	ಸ್ಥಿರಾಂಕ ರಚನೆ ಬಣ್ಣ_map *bb = b;

	ಹಿಂತಿರುಗಿ strcasecmp(aa, bb->name);
}

/**
 * Parse a named colour
 *
 * \param name    Name to parse
 * \param result  Pointer to location to receive css_color
 * \return true on success, false on invalid input
 */
ಸ್ಥಿರ bool parse_named_ಬಣ್ಣ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, css_color *ಫಲಿತಾಂಶ)
{
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ ಬಣ್ಣ_map named_colours[] = {
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
	ಸ್ಥಿರಾಂಕ ರಚನೆ ಬಣ್ಣ_map *ನಮೂದು;

	ನಮೂದು = bsearch(name, named_colours,
			ಗಾತ್ರ(named_colours) / ಗಾತ್ರ(named_colours[0]),
			ಗಾತ್ರ(named_colours[0]),
			cmp_ಬಣ್ಣ_name);

	ಆದರೆ (ನಮೂದು != ಖಾಲಿ)
		*ಫಲಿತಾಂಶ = ನಮೂದು->color;

	ಹಿಂತಿರುಗಿ ನಮೂದು != ಖಾಲಿ;
}

/* exported interface documented in content/handlers/css/hints.h */
bool nscss_parse_ಬಣ್ಣ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ದತ್ತಾಂಶ, css_color *ಫಲಿತಾಂಶ)
{
	size_t len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ದತ್ತಾಂಶ);
	uint8_t r, g, b;

	/* 2 */
	ಆದರೆ (len == 0)
		ಹಿಂತಿರುಗಿ false;

	/* 3 */
	ಆದರೆ (len == SLEN("transparent") && strcasecmp(ದತ್ತಾಂಶ, "transparent") == 0)
		ಹಿಂತಿರುಗಿ false;

	/* 4 */
	ಆದರೆ (parse_named_ಬಣ್ಣ(ದತ್ತಾಂಶ, ಫಲಿತಾಂಶ))
		ಹಿಂತಿರುಗಿ true;

	/** \todo Implement HTML5's utterly insane legacy colour parsing */

	ಆದರೆ (ದತ್ತಾಂಶ[0] == '#') {
		ದತ್ತಾಂಶ++;
		len--;
	}

	ಆದರೆ (len == 3 && isHex(ದತ್ತಾಂಶ[0]) && isHex(ದತ್ತಾಂಶ[1]) && isHex(ದತ್ತಾಂಶ[2])) {
		r = char_ಗಮ್ಯ_Hex(ದತ್ತಾಂಶ[0]);
		g = char_ಗಮ್ಯ_Hex(ದತ್ತಾಂಶ[1]);
		b = char_ಗಮ್ಯ_Hex(ದತ್ತಾಂಶ[2]);

		r |= (r << 4);
		g |= (g << 4);
		b |= (b << 4);

		*ಫಲಿತಾಂಶ = (0xff << 24) | (r << 16) | (g << 8) | b;

		ಹಿಂತಿರುಗಿ true;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (len == 6 && isHex(ದತ್ತಾಂಶ[0]) && isHex(ದತ್ತಾಂಶ[1]) &&
			isHex(ದತ್ತಾಂಶ[2]) && isHex(ದತ್ತಾಂಶ[3]) && isHex(ದತ್ತಾಂಶ[4]) &&
			isHex(ದತ್ತಾಂಶ[5])) {
		r = (char_ಗಮ್ಯ_Hex(ದತ್ತಾಂಶ[0]) << 4) | char_ಗಮ್ಯ_Hex(ದತ್ತಾಂಶ[1]);
		g = (char_ಗಮ್ಯ_Hex(ದತ್ತಾಂಶ[2]) << 4) | char_ಗಮ್ಯ_Hex(ದತ್ತಾಂಶ[3]);
		b = (char_ಗಮ್ಯ_Hex(ದತ್ತಾಂಶ[4]) << 4) | char_ಗಮ್ಯ_Hex(ದತ್ತಾಂಶ[5]);

		*ಫಲಿತಾಂಶ = (0xff << 24) | (r << 16) | (g << 8) | b;

		ಹಿಂತಿರುಗಿ true;
	}

	ಹಿಂತಿರುಗಿ false;
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
ಸ್ಥಿರ bool parse_font_ಗಾತ್ರ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಗಾತ್ರ_2, uint8_t *val,
		css_fixed *len, css_unit *unit)
{
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ uint8_t ಗಾತ್ರ_map[] = {
		CSS_FONT_ಗಾತ್ರ_XX_SMALL,
		CSS_FONT_ಗಾತ್ರ_SMALL,
		CSS_FONT_ಗಾತ್ರ_MEDIUM,
		CSS_FONT_ಗಾತ್ರ_LARGE,
		CSS_FONT_ಗಾತ್ರ_X_LARGE,
		CSS_FONT_ಗಾತ್ರ_XX_LARGE,
		CSS_FONT_ಗಾತ್ರ_DIMENSION	/* xxx-large (see below) */
	};

	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = ಗಾತ್ರ_2;
	ಅಕ್ಷರ ಬಹುಲಕ;
	ಪೂರ್ಣಾಂಕ ಮೌಲ್ಯ = 0;

	/* Skip whitespace */
	ಆಗ (*p != '\0' && isWhitespace(*p))
		p++;

	ಬಹುಲಕ = *p;

	/* Skip +/- */
	ಆದರೆ (ಬಹುಲಕ == '+' || ಬಹುಲಕ == '-')
		p++;

	/* Need at least one digit */
	ಆದರೆ (*p < '0' || *p > '9') {
		ಹಿಂತಿರುಗಿ false;
	}

	/* Consume digits, computing value */
	ಆಗ ('0' <= *p && *p <= '9') {
		ಮೌಲ್ಯ = ಮೌಲ್ಯ * 10 + (*p - '0');
		p++;
	}

	/* Resolve relative sizes */
	ಆದರೆ (ಬಹುಲಕ == '+')
		ಮೌಲ್ಯ += 3;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಬಹುಲಕ == '-')
		ಮೌಲ್ಯ = 3 - ಮೌಲ್ಯ;

	/* Clamp to range [1,7] */
	ಆದರೆ (ಮೌಲ್ಯ < 1)
		ಮೌಲ್ಯ = 1;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಮೌಲ್ಯ > 7)
		ಮೌಲ್ಯ = 7;

	ಆದರೆ (ಮೌಲ್ಯ == 7) {
		/* Manufacture xxx-large */
	  *len = FDIV(FMUL(INTTOFIX(3), INTTOFIX(nsoption_int(font_ಗಾತ್ರ))),
				F_10);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Len is irrelevant */
		*len = 0;
	}

	*unit = CSS_UNIT_PT;
	*val = ಗಾತ್ರ_map[ಮೌಲ್ಯ - 1];

	ಹಿಂತಿರುಗಿ true;
}


/******************************************************************************
 * Hint context management                                                    *
 ******************************************************************************/

#ವ್ಯಾಖ್ಯಾನಿಸು MAX_HINTS_ಪ್ರತಿ_ELEMENT 32

ರಚನೆ css_hint_ctx {
	ರಚನೆ css_hint *hints;
	uint32_t len;
};

ರಚನೆ css_hint_ctx hint_ctx;

nserror css_hint_init(ಶೂನ್ಯ)
{
	hint_ctx.hints = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ರಚನೆ css_hint) *
			MAX_HINTS_ಪ್ರತಿ_ELEMENT);
	ಆದರೆ (hint_ctx.hints == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

ಶೂನ್ಯ css_hint_fini(ಶೂನ್ಯ)
{
	hint_ctx.len = 0;
	ಬಿಡುಗಡೆ(hint_ctx.hints);
}

ಸ್ಥಿರ ಶೂನ್ಯ css_hint_clean(ಶೂನ್ಯ)
{
	hint_ctx.len = 0;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ css_hint_advance(ರಚನೆ css_hint **hint)
{
	hint_ctx.len++;
	ಪ್ರತಿಜ್ಞಾ(hint_ctx.len < MAX_HINTS_ಪ್ರತಿ_ELEMENT);

	(*hint)++;
}

ಸ್ಥಿರ ಶೂನ್ಯ css_hint_get_hints(ರಚನೆ css_hint **hints, uint32_t *nhints)
{
	*hints = hint_ctx.hints;
	*nhints = hint_ctx.len;
}


/******************************************************************************
 * Presentational hint handlers                                               *
 ******************************************************************************/

ಸ್ಥಿರ ಶೂನ್ಯ css_hint_ಕೋಷ್ಟಕ_cell_border_padding(
		nscss_select_ctx *ctx,
		dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	css_qname qs;
	dom_string *attr = ಖಾಲಿ;
	dom_ಗಂಟು *tablenode = ಖಾಲಿ;
	dom_exception exc;

	qs.ns = ಖಾಲಿ;
	qs.name = lwc_string_ref(corestring_lwc_ಕೋಷ್ಟಕ);
	ಆದರೆ (named_ancestor_ಗಂಟು(ctx, ಗಂಟು, &qs,
			(ಶೂನ್ಯ *)&tablenode) != CSS_OK) {
		/* Didn't find, or had error */
		lwc_string_unref(qs.name);
		ಹಿಂತಿರುಗಿ;
	}
	lwc_string_unref(qs.name);

	ಆದರೆ (tablenode == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ;
	}
	/* No need to unref tablenode, named_ancestor_node does not
	 * return a reffed node to the CSS
	 */

	exc = dom_element_get_attribute(tablenode,
			corestring_dom_border, &attr);

	ಆದರೆ (exc == DOM_NO_ERR && attr != ಖಾಲಿ) {
		uint32_t hint_prop;
		css_hint_ಉದ್ದಳತೆ hint_ಉದ್ದಳತೆ;

		ಆದರೆ (parse_dimension(
				dom_string_ದತ್ತಾಂಶ(attr), false,
						&hint_ಉದ್ದಳತೆ.ಮೌಲ್ಯ,
						&hint_ಉದ್ದಳತೆ.unit) &&
				INTTOFIX(0) != hint_ಉದ್ದಳತೆ.ಮೌಲ್ಯ) {

			ಗಾಗಿ (hint_prop = CSS_PROP_BORDER_ತುದಿ_STYLE;
			     hint_prop <= CSS_PROP_BORDER_ಎಡ_STYLE;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->ಸ್ಥಿತಿ_2 = CSS_BORDER_STYLE_INSET;
				css_hint_advance(&hint);
			}

			ಗಾಗಿ (hint_prop = CSS_PROP_BORDER_ತುದಿ_ಅಗಲ;
			     hint_prop <= CSS_PROP_BORDER_ಎಡ_ಅಗಲ;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.ಮೌಲ್ಯ = INTTOFIX(1);
				hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.unit = CSS_UNIT_PX;
				hint->ಸ್ಥಿತಿ_2 = CSS_BORDER_ಅಗಲ_ಅಗಲ;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(tablenode,
			corestring_dom_bordercolor, &attr);

	ಆದರೆ (exc == DOM_NO_ERR && attr != ಖಾಲಿ) {
		uint32_t hint_prop;
		css_color hint_color;

		ಆದರೆ (nscss_parse_ಬಣ್ಣ(
				(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)dom_string_ದತ್ತಾಂಶ(attr),
				&hint_color)) {

			ಗಾಗಿ (hint_prop = CSS_PROP_BORDER_ತುದಿ_COLOR;
			     hint_prop <= CSS_PROP_BORDER_ಎಡ_COLOR;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->ದತ್ತಾಂಶ.color = hint_color;
				hint->ಸ್ಥಿತಿ_2 = CSS_BORDER_COLOR_COLOR;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(tablenode,
			corestring_dom_cellpadding, &attr);

	ಆದರೆ (exc == DOM_NO_ERR && attr != ಖಾಲಿ) {
		uint32_t hint_prop;
		css_hint_ಉದ್ದಳತೆ hint_ಉದ್ದಳತೆ;

		ಆದರೆ (parse_dimension(
				dom_string_ದತ್ತಾಂಶ(attr), false,
						&hint_ಉದ್ದಳತೆ.ಮೌಲ್ಯ,
						&hint_ಉದ್ದಳತೆ.unit)) {

			ಗಾಗಿ (hint_prop = CSS_PROP_PADDING_ತುದಿ;
			     hint_prop <= CSS_PROP_PADDING_ಎಡ;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.ಮೌಲ್ಯ = hint_ಉದ್ದಳತೆ.ಮೌಲ್ಯ;
				hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.unit = hint_ಉದ್ದಳತೆ.unit;
				hint->ಸ್ಥಿತಿ_2 = CSS_PADDING_ಗಣ;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ css_hint_vertical_align_ಕೋಷ್ಟಕ_cells(
		nscss_select_ctx *ctx,
		dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = ಖಾಲಿ;
	dom_exception err;

	err = dom_element_get_attribute(ಗಂಟು,
			corestring_dom_valign, &attr);

	ಆದರೆ (err == DOM_NO_ERR && attr != ಖಾಲಿ) {
		hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.ಮೌಲ್ಯ = 0;
		hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.unit = CSS_UNIT_PX;
		ಆದರೆ (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_ತುದಿ)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->ಸ್ಥಿತಿ_2 = CSS_VERTICAL_ALIGN_ತುದಿ;
			css_hint_advance(&hint);

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_ಮಧ್ಯ)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->ಸ್ಥಿತಿ_2 = CSS_VERTICAL_ALIGN_ಮಧ್ಯ;
			css_hint_advance(&hint);

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_bottom)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->ಸ್ಥಿತಿ_2 = CSS_VERTICAL_ALIGN_BOTTOM;
			css_hint_advance(&hint);

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_baseline)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->ಸ್ಥಿತಿ_2 = CSS_VERTICAL_ALIGN_BASELINE;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ css_hint_vertical_align_replaced(
		nscss_select_ctx *ctx,
		dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = ಖಾಲಿ;
	dom_exception err;

	err = dom_element_get_attribute(ಗಂಟು,
			corestring_dom_valign, &attr);

	ಆದರೆ (err == DOM_NO_ERR && attr != ಖಾಲಿ) {
		ಆದರೆ (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_ತುದಿ)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->ಸ್ಥಿತಿ_2 = CSS_VERTICAL_ALIGN_ತುದಿ;
			css_hint_advance(&hint);

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_bottom) ||
			   dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_baseline)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->ಸ್ಥಿತಿ_2 = CSS_VERTICAL_ALIGN_BASELINE;
			css_hint_advance(&hint);

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_texttop)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->ಸ್ಥಿತಿ_2 = CSS_VERTICAL_ALIGN_ಪಠ್ಯ_ತುದಿ;
			css_hint_advance(&hint);

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_absmiddle) ||
			   dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_abscenter)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->ಸ್ಥಿತಿ_2 = CSS_VERTICAL_ALIGN_ಮಧ್ಯ;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ css_hint_ಪಠ್ಯ_align_ಸಾಮಾನ್ಯ(
		nscss_select_ctx *ctx,
		dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *align = ಖಾಲಿ;
	dom_exception err;

	err = dom_element_get_attribute(ಗಂಟು,
			corestring_dom_align, &align);
	ಆದರೆ (err == DOM_NO_ERR && align != ಖಾಲಿ) {
		ಆದರೆ (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_ಎಡ)) {
			hint->prop = CSS_PROP_ಪಠ್ಯ_ALIGN;
			hint->ಸ್ಥಿತಿ_2 = CSS_ಪಠ್ಯ_ALIGN_ಎಡ;
			css_hint_advance(&hint);

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_center)) {
			hint->prop = CSS_PROP_ಪಠ್ಯ_ALIGN;
			hint->ಸ್ಥಿತಿ_2 = CSS_ಪಠ್ಯ_ALIGN_CENTER;
			css_hint_advance(&hint);

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_ಬಲ)) {
			hint->prop = CSS_PROP_ಪಠ್ಯ_ALIGN;
			hint->ಸ್ಥಿತಿ_2 = CSS_ಪಠ್ಯ_ALIGN_ಬಲ;
			css_hint_advance(&hint);

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_justify)) {
			hint->prop = CSS_PROP_ಪಠ್ಯ_ALIGN;
			hint->ಸ್ಥಿತಿ_2 = CSS_ಪಠ್ಯ_ALIGN_JUSTIFY;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ css_hint_ಪಠ್ಯ_align_center(
		nscss_select_ctx *ctx,
		dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ css_hint *hint = &hint_ctx.hints[hint_ctx.len];

	hint->prop = CSS_PROP_ಪಠ್ಯ_ALIGN;
	hint->ಸ್ಥಿತಿ_2 = CSS_ಪಠ್ಯ_ALIGN_LIBCSS_CENTER;
	css_hint_advance(&hint);
}

ಸ್ಥಿರ ಶೂನ್ಯ css_hint_margin_ಎಡ_ಬಲ_align_center(
		nscss_select_ctx *ctx,
		dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr;
	dom_exception exc;

	exc = dom_element_get_attribute(ಗಂಟು,
			corestring_dom_align, &attr);

	ಆದರೆ (exc == DOM_NO_ERR && attr != ಖಾಲಿ) {
		memset(hint, 0, ಗಾತ್ರ(*hint) * 2);
		ಆದರೆ (dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_center) ||
				dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_abscenter) ||
				dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_ಮಧ್ಯ) ||
				dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_absmiddle)) {
			hint->prop = CSS_PROP_MARGIN_ಎಡ;
			hint->ಸ್ಥಿತಿ_2 = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_ಬಲ;
			hint->ಸ್ಥಿತಿ_2 = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ css_hint_ಪಠ್ಯ_align_special(
		nscss_select_ctx *ctx,
		dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *align = ಖಾಲಿ;
	dom_exception err;

	err = dom_element_get_attribute(ಗಂಟು,
			corestring_dom_align, &align);

	ಆದರೆ (err == DOM_NO_ERR && align != ಖಾಲಿ) {
		ಆದರೆ (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_center)) {
			hint->prop = CSS_PROP_ಪಠ್ಯ_ALIGN;
			hint->ಸ್ಥಿತಿ_2 = CSS_ಪಠ್ಯ_ALIGN_LIBCSS_CENTER;
			css_hint_advance(&hint);

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_ಎಡ)) {
			hint->prop = CSS_PROP_ಪಠ್ಯ_ALIGN;
			hint->ಸ್ಥಿತಿ_2 = CSS_ಪಠ್ಯ_ALIGN_LIBCSS_ಎಡ;
			css_hint_advance(&hint);

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_ಬಲ)) {
			hint->prop = CSS_PROP_ಪಠ್ಯ_ALIGN;
			hint->ಸ್ಥಿತಿ_2 = CSS_ಪಠ್ಯ_ALIGN_LIBCSS_ಬಲ;
			css_hint_advance(&hint);

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_justify)) {
			hint->prop = CSS_PROP_ಪಠ್ಯ_ALIGN;
			hint->ಸ್ಥಿತಿ_2 = CSS_ಪಠ್ಯ_ALIGN_JUSTIFY;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ css_hint_ಪಠ್ಯ_align_ಕೋಷ್ಟಕ_special(
		nscss_select_ctx *ctx,
		dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ css_hint *hint = &hint_ctx.hints[hint_ctx.len];

	hint->prop = CSS_PROP_ಪಠ್ಯ_ALIGN;
	hint->ಸ್ಥಿತಿ_2 = CSS_ಪಠ್ಯ_ALIGN_INHERIT_IF_NON_MAGIC;
	css_hint_advance(&hint);
}

ಸ್ಥಿರ ಶೂನ್ಯ css_hint_margin_hspace_vspace(
		nscss_select_ctx *ctx,
		dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = ಖಾಲಿ;
	dom_exception exc;

	exc = dom_element_get_attribute(ಗಂಟು,
			corestring_dom_vspace, &attr);

	ಆದರೆ (exc == DOM_NO_ERR && attr != ಖಾಲಿ) {
		css_hint_ಉದ್ದಳತೆ hint_ಉದ್ದಳತೆ;
		ಆದರೆ (parse_dimension(
				dom_string_ದತ್ತಾಂಶ(attr), false,
				&hint_ಉದ್ದಳತೆ.ಮೌಲ್ಯ,
				&hint_ಉದ್ದಳತೆ.unit)) {
			hint->prop = CSS_PROP_MARGIN_ತುದಿ;
			hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.ಮೌಲ್ಯ = hint_ಉದ್ದಳತೆ.ಮೌಲ್ಯ;
			hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.unit = hint_ಉದ್ದಳತೆ.unit;
			hint->ಸ್ಥಿತಿ_2 = CSS_MARGIN_ಗಣ;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_BOTTOM;
			hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.ಮೌಲ್ಯ = hint_ಉದ್ದಳತೆ.ಮೌಲ್ಯ;
			hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.unit = hint_ಉದ್ದಳತೆ.unit;
			hint->ಸ್ಥಿತಿ_2 = CSS_MARGIN_ಗಣ;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(ಗಂಟು,
			corestring_dom_hspace, &attr);

	ಆದರೆ (exc == DOM_NO_ERR && attr != ಖಾಲಿ) {
		css_hint_ಉದ್ದಳತೆ hint_ಉದ್ದಳತೆ;
		ಆದರೆ (parse_dimension(
				dom_string_ದತ್ತಾಂಶ(attr), false,
				&hint_ಉದ್ದಳತೆ.ಮೌಲ್ಯ,
				&hint_ಉದ್ದಳತೆ.unit)) {
			hint->prop = CSS_PROP_MARGIN_ಎಡ;
			hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.ಮೌಲ್ಯ = hint_ಉದ್ದಳತೆ.ಮೌಲ್ಯ;
			hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.unit = hint_ಉದ್ದಳತೆ.unit;
			hint->ಸ್ಥಿತಿ_2 = CSS_MARGIN_ಗಣ;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_ಬಲ;
			hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.ಮೌಲ್ಯ = hint_ಉದ್ದಳತೆ.ಮೌಲ್ಯ;
			hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.unit = hint_ಉದ್ದಳತೆ.unit;
			hint->ಸ್ಥಿತಿ_2 = CSS_MARGIN_ಗಣ;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ css_hint_margin_ಎಡ_ಬಲ_hr(
		nscss_select_ctx *ctx,
		dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr;
	dom_exception exc;

	exc = dom_element_get_attribute(ಗಂಟು,
				corestring_dom_align, &attr);

	ಆದರೆ (exc == DOM_NO_ERR && attr != ಖಾಲಿ) {
		memset(hint, 0, ಗಾತ್ರ(*hint) * 2);
		ಆದರೆ (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_ಎಡ)) {
			hint->prop = CSS_PROP_MARGIN_ಎಡ;
			hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.ಮೌಲ್ಯ = 0;
			hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.unit = CSS_UNIT_PX;
			hint->ಸ್ಥಿತಿ_2 = CSS_MARGIN_ಗಣ;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_ಬಲ;
			hint->ಸ್ಥಿತಿ_2 = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_center)) {
			hint->prop = CSS_PROP_MARGIN_ಎಡ;
			hint->ಸ್ಥಿತಿ_2 = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_ಬಲ;
			hint->ಸ್ಥಿತಿ_2 = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_ಬಲ)) {
			hint->prop = CSS_PROP_MARGIN_ಎಡ;
			hint->ಸ್ಥಿತಿ_2 = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_ಬಲ;
			hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.ಮೌಲ್ಯ = 0;
			hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.unit = CSS_UNIT_PX;
			hint->ಸ್ಥಿತಿ_2 = CSS_MARGIN_ಗಣ;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ css_hint_ಕೋಷ್ಟಕ_spacing_border(
		nscss_select_ctx *ctx,
		dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception exc;
	dom_string *attr = ಖಾಲಿ;

	exc = dom_element_get_attribute(ಗಂಟು, corestring_dom_border, &attr);

	ಆದರೆ (exc == DOM_NO_ERR && attr != ಖಾಲಿ) {
		uint32_t hint_prop;
		css_hint_ಉದ್ದಳತೆ hint_ಉದ್ದಳತೆ;

		ಗಾಗಿ (hint_prop = CSS_PROP_BORDER_ತುದಿ_STYLE;
			     hint_prop <= CSS_PROP_BORDER_ಎಡ_STYLE;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->ಸ್ಥಿತಿ_2 = CSS_BORDER_STYLE_OUTSET;
				css_hint_advance(&hint);
		}

		ಆದರೆ (parse_dimension(
				dom_string_ದತ್ತಾಂಶ(attr), false,
						&hint_ಉದ್ದಳತೆ.ಮೌಲ್ಯ,
						&hint_ಉದ್ದಳತೆ.unit)) {

			ಗಾಗಿ (hint_prop = CSS_PROP_BORDER_ತುದಿ_ಅಗಲ;
			     hint_prop <= CSS_PROP_BORDER_ಎಡ_ಅಗಲ;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.ಮೌಲ್ಯ = hint_ಉದ್ದಳತೆ.ಮೌಲ್ಯ;
				hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.unit = hint_ಉದ್ದಳತೆ.unit;
				hint->ಸ್ಥಿತಿ_2 = CSS_BORDER_ಅಗಲ_ಅಗಲ;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(ಗಂಟು,
			corestring_dom_bordercolor, &attr);

	ಆದರೆ (exc == DOM_NO_ERR && attr != ಖಾಲಿ) {
		uint32_t hint_prop;
		css_color hint_color;

		ಆದರೆ (nscss_parse_ಬಣ್ಣ(
				(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)dom_string_ದತ್ತಾಂಶ(attr),
				&hint_color)) {

			ಗಾಗಿ (hint_prop = CSS_PROP_BORDER_ತುದಿ_COLOR;
			     hint_prop <= CSS_PROP_BORDER_ಎಡ_COLOR;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->ದತ್ತಾಂಶ.color = hint_color;
				hint->ಸ್ಥಿತಿ_2 = CSS_BORDER_COLOR_COLOR;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(ಗಂಟು,
			corestring_dom_cellspacing, &attr);

	ಆದರೆ (exc == DOM_NO_ERR && attr != ಖಾಲಿ) {
		ಆದರೆ (parse_dimension(
				(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)dom_string_ದತ್ತಾಂಶ(attr), false,
						&hint->ದತ್ತಾಂಶ.position.h.ಮೌಲ್ಯ,
						&hint->ದತ್ತಾಂಶ.position.h.unit)) {
			hint->prop = CSS_PROP_BORDER_SPACING;
			hint->ದತ್ತಾಂಶ.position.v = hint->ದತ್ತಾಂಶ.position.h;
			hint->ಸ್ಥಿತಿ_2 = CSS_BORDER_SPACING_ಗಣ;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ css_hint_ಎತ್ತರ(
		nscss_select_ctx *ctx,
		dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = ಖಾಲಿ;
	dom_exception err;

	err = dom_element_get_attribute(ಗಂಟು,
			corestring_dom_ಎತ್ತರ, &attr);

	ಆದರೆ (err == DOM_NO_ERR && attr != ಖಾಲಿ) {
		ಆದರೆ (parse_dimension(
				(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)dom_string_ದತ್ತಾಂಶ(attr), false,
				&hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.ಮೌಲ್ಯ,
				&hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.unit)) {
			hint->prop = CSS_PROP_ಎತ್ತರ;
			hint->ಸ್ಥಿತಿ_2 = CSS_ಎತ್ತರ_ಗಣ;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ css_hint_ಅಗಲ(
		nscss_select_ctx *ctx,
		dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = ಖಾಲಿ;
	dom_exception err;

	err = dom_element_get_attribute(ಗಂಟು,
			corestring_dom_ಅಗಲ, &attr);

	ಆದರೆ (err == DOM_NO_ERR && attr != ಖಾಲಿ) {
		ಆದರೆ (parse_dimension(
				(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)dom_string_ದತ್ತಾಂಶ(attr), false,
				&hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.ಮೌಲ್ಯ,
				&hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.unit)) {
			hint->prop = CSS_PROP_ಅಗಲ;
			hint->ಸ್ಥಿತಿ_2 = CSS_ಅಗಲ_ಗಣ;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ css_hint_ಎತ್ತರ_ಅಗಲ_textarea(
		nscss_select_ctx *ctx,
		dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = ಖಾಲಿ;
	dom_exception err;

	err = dom_element_get_attribute(ಗಂಟು,
			corestring_dom_ಅಡ್ಡಸಾಲುಗಳು, &attr);

	ಆದರೆ (err == DOM_NO_ERR && attr != ಖಾಲಿ) {
		ಆದರೆ (parse_dimension(
				(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)dom_string_ದತ್ತಾಂಶ(attr), false,
				&hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.ಮೌಲ್ಯ,
				&hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.unit)) {
			hint->prop = CSS_PROP_ಎತ್ತರ;
			hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.unit = CSS_UNIT_EM;
			hint->ಸ್ಥಿತಿ_2 = CSS_ಎತ್ತರ_ಗಣ;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}

	err = dom_element_get_attribute(ಗಂಟು,
			corestring_dom_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ, &attr);

	ಆದರೆ (err == DOM_NO_ERR && attr != ಖಾಲಿ) {
		ಆದರೆ (parse_dimension(
				(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)dom_string_ದತ್ತಾಂಶ(attr), false,
				&hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.ಮೌಲ್ಯ,
				&hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.unit)) {
			hint->prop = CSS_PROP_ಅಗಲ;
			hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.unit = CSS_UNIT_EX;
			hint->ಸ್ಥಿತಿ_2 = CSS_ಅಗಲ_ಗಣ;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ css_hint_ಎತ್ತರ_ಅಗಲ_canvas(
		nscss_select_ctx *ctx,
		dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = ಖಾಲಿ;
	dom_exception err;
	bool ಗಣ_dim = false;

	err = dom_element_get_attribute(ಗಂಟು,
			corestring_dom_ಎತ್ತರ, &attr);

	ಆದರೆ (err == DOM_NO_ERR && attr != ಖಾಲಿ) {
		ಆದರೆ (parse_dimension(
				(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)dom_string_ದತ್ತಾಂಶ(attr), true,
				&hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.ಮೌಲ್ಯ,
				&hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.unit)) {
			hint->prop = CSS_PROP_ಎತ್ತರ;
			hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.unit = CSS_UNIT_PX;
			hint->ಸ್ಥಿತಿ_2 = CSS_ಎತ್ತರ_ಗಣ;
			css_hint_advance(&hint);
			ಗಣ_dim = true;
		}
		dom_string_unref(attr);
	}
	ಆದರೆ (ಗಣ_dim == false) {
		/* canvas defaults to 150px tall */
		hint->prop = CSS_PROP_ಎತ್ತರ;
		hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.unit = CSS_UNIT_PX;
		hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.ಮೌಲ್ಯ = INTTOFIX(150);
		hint->ಸ್ಥಿತಿ_2 = CSS_ಎತ್ತರ_ಗಣ;
		css_hint_advance(&hint);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಗಣ_dim = false;
	}

	err = dom_element_get_attribute(ಗಂಟು,
			corestring_dom_ಅಗಲ, &attr);

	ಆದರೆ (err == DOM_NO_ERR && attr != ಖಾಲಿ) {
		ಆದರೆ (parse_dimension(
				(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)dom_string_ದತ್ತಾಂಶ(attr), true,
				&hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.ಮೌಲ್ಯ,
				&hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.unit)) {
			hint->prop = CSS_PROP_ಅಗಲ;
			hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.unit = CSS_UNIT_PX;
			hint->ಸ್ಥಿತಿ_2 = CSS_ಅಗಲ_ಗಣ;
			css_hint_advance(&hint);
			ಗಣ_dim = true;
		}
		dom_string_unref(attr);
	}
	ಆದರೆ (ಗಣ_dim == false) {
		/* canvas defaults to 300px wide */
		hint->prop = CSS_PROP_ಅಗಲ;
		hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.unit = CSS_UNIT_PX;
		hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.ಮೌಲ್ಯ = INTTOFIX(300);
		hint->ಸ್ಥಿತಿ_2 = CSS_ಅಗಲ_ಗಣ;
		css_hint_advance(&hint);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ css_hint_ಅಗಲ_ಒಳಹರಿವು(
		nscss_select_ctx *ctx,
		dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_string *attr = ಖಾಲಿ;
	dom_exception err;

	err = dom_element_get_attribute(ಗಂಟು,
			corestring_dom_ಗಾತ್ರ, &attr);

	ಆದರೆ (err == DOM_NO_ERR && attr != ಖಾಲಿ) {
		ಆದರೆ (parse_dimension(
				(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)dom_string_ದತ್ತಾಂಶ(attr), false,
				&hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.ಮೌಲ್ಯ,
				&hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.unit)) {
			dom_string *attr2 = ಖಾಲಿ;

			err = dom_element_get_attribute(ಗಂಟು,
					corestring_dom_type, &attr2);
			ಆದರೆ (err == DOM_NO_ERR) {

				hint->prop = CSS_PROP_ಅಗಲ;
				hint->ಸ್ಥಿತಿ_2 = CSS_ಅಗಲ_ಗಣ;

				ಆದರೆ (attr2 == ಖಾಲಿ ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_ಪಠ್ಯ) ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_ಹುಡುಕು) ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_password) ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_file)) {
					hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.unit = CSS_UNIT_EX;
				}
				ಆದರೆ (attr2 != ಖಾಲಿ) {
					dom_string_unref(attr2);
				}
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ css_hint_anchor_color(
		nscss_select_ctx *ctx,
		dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	css_error error;
	dom_exception err;
	dom_string *color;
	dom_ಗಂಟು *bodynode = ಖಾಲಿ;

	/* find body node */
	css_qname qs;
	bool is_ಭೇಟಿಯಾದ;

	qs.ns = ಖಾಲಿ;
	qs.name = lwc_string_ref(corestring_lwc_body);
	ಆದರೆ (named_ancestor_ಗಂಟು(ctx, ಗಂಟು, &qs,
			(ಶೂನ್ಯ *)&bodynode) != CSS_OK) {
		/* Didn't find, or had error */
		lwc_string_unref(qs.name);
		ಹಿಂತಿರುಗಿ ;
	}
	lwc_string_unref(qs.name);

	ಆದರೆ (bodynode == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ;
	}

	error = ಗಂಟು_is_ಭೇಟಿಯಾದ(ctx, ಗಂಟು, &is_ಭೇಟಿಯಾದ);
	ಆದರೆ (error != CSS_OK)
		ಹಿಂತಿರುಗಿ;

	ಆದರೆ (is_ಭೇಟಿಯಾದ) {
		err = dom_element_get_attribute(bodynode,
				corestring_dom_vlink, &color);
	} ಇಲ್ಲದಿದ್ದರೆ {
		err = dom_element_get_attribute(bodynode,
				corestring_dom_link, &color);
	}

	ಆದರೆ (err == DOM_NO_ERR && color != ಖಾಲಿ) {
		ಆದರೆ (nscss_parse_ಬಣ್ಣ(
				(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)dom_string_ದತ್ತಾಂಶ(color),
						&hint->ದತ್ತಾಂಶ.color)) {
			hint->prop = CSS_PROP_COLOR;
			hint->ಸ್ಥಿತಿ_2 = CSS_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(color);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ css_hint_body_color(
		nscss_select_ctx *ctx,
		dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *color;

	err = dom_element_get_attribute(ಗಂಟು, corestring_dom_ಪಠ್ಯ, &color);

	ಆದರೆ (err == DOM_NO_ERR && color != ಖಾಲಿ) {
		ಆದರೆ (nscss_parse_ಬಣ್ಣ(
				(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)dom_string_ದತ್ತಾಂಶ(color),
						&hint->ದತ್ತಾಂಶ.color)) {
			hint->prop = CSS_PROP_COLOR;
			hint->ಸ್ಥಿತಿ_2 = CSS_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(color);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ css_hint_color(
		nscss_select_ctx *ctx,
		dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *color;

	err = dom_element_get_attribute(ಗಂಟು, corestring_dom_color, &color);

	ಆದರೆ (err == DOM_NO_ERR && color != ಖಾಲಿ) {
		ಆದರೆ (nscss_parse_ಬಣ್ಣ(
				(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)dom_string_ದತ್ತಾಂಶ(color),
						&hint->ದತ್ತಾಂಶ.color)) {
			hint->prop = CSS_PROP_COLOR;
			hint->ಸ್ಥಿತಿ_2 = CSS_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(color);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ css_hint_font_ಗಾತ್ರ(
		nscss_select_ctx *ctx,
		dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *ಗಾತ್ರ_2;

	err = dom_element_get_attribute(ಗಂಟು, corestring_dom_ಗಾತ್ರ, &ಗಾತ್ರ_2);
	ಆದರೆ (err == DOM_NO_ERR && ಗಾತ್ರ_2 != ಖಾಲಿ) {
		ಆದರೆ (parse_font_ಗಾತ್ರ(
				(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)dom_string_ದತ್ತಾಂಶ(ಗಾತ್ರ_2),
						&hint->ಸ್ಥಿತಿ_2,
						&hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.ಮೌಲ್ಯ,
						&hint->ದತ್ತಾಂಶ.ಉದ್ದಳತೆ.unit)) {
			hint->prop = CSS_PROP_FONT_ಗಾತ್ರ;
			css_hint_advance(&hint);
		}
		dom_string_unref(ಗಾತ್ರ_2);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ css_hint_float(
		nscss_select_ctx *ctx,
		dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *align;

	err = dom_element_get_attribute(ಗಂಟು, corestring_dom_align, &align);
	ಆದರೆ (err == DOM_NO_ERR && align != ಖಾಲಿ) {
		ಆದರೆ (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_ಎಡ)) {
			hint->prop = CSS_PROP_FLOAT;
			hint->ಸ್ಥಿತಿ_2 = CSS_FLOAT_ಎಡ;
			css_hint_advance(&hint);

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_ಬಲ)) {
			hint->prop = CSS_PROP_FLOAT;
			hint->ಸ್ಥಿತಿ_2 = CSS_FLOAT_ಬಲ;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ css_hint_caption_side(
		nscss_select_ctx *ctx,
		dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *align = ಖಾಲಿ;

	err = dom_element_get_attribute(ಗಂಟು, corestring_dom_align, &align);
	ಆದರೆ (err == DOM_NO_ERR && align != ಖಾಲಿ) {
		ಆದರೆ (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_bottom)) {
			hint->prop = CSS_PROP_CAPTION_SIDE;
			hint->ಸ್ಥಿತಿ_2 = CSS_CAPTION_SIDE_BOTTOM;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ css_hint_bg_color(
		nscss_select_ctx *ctx,
		dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *bgcolor;

	err = dom_element_get_attribute(ಗಂಟು,
			corestring_dom_bgcolor, &bgcolor);
	ಆದರೆ (err == DOM_NO_ERR && bgcolor != ಖಾಲಿ) {
		ಆದರೆ (nscss_parse_ಬಣ್ಣ(
				(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)dom_string_ದತ್ತಾಂಶ(bgcolor),
				&hint->ದತ್ತಾಂಶ.color)) {
			hint->prop = CSS_PROP_BACKGROUND_COLOR;
			hint->ಸ್ಥಿತಿ_2 = CSS_BACKGROUND_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(bgcolor);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ css_hint_bg_image(
		nscss_select_ctx *ctx,
		dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_exception err;
	dom_string *attr;

	err = dom_element_get_attribute(ಗಂಟು,
			corestring_dom_background, &attr);
	ಆದರೆ (err == DOM_NO_ERR && attr != ಖಾಲಿ) {
		nsurl *url;
		nserror error = nsurl_join(ctx->ತಳ_url,
				(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)dom_string_ದತ್ತಾಂಶ(attr), &url);
		dom_string_unref(attr);

		ಆದರೆ (error == NSERROR_OK) {
			lwc_string *iurl;
			lwc_error lerror = lwc_intern_string(nsurl_access(url),
					nsurl_ಉದ್ದಳತೆ(url), &iurl);
			nsurl_unref(url);

			ಆದರೆ (lerror == lwc_error_ok) {
				hint->prop = CSS_PROP_BACKGROUND_IMAGE;
				hint->ದತ್ತಾಂಶ.string = iurl;
				hint->ಸ್ಥಿತಿ_2 = CSS_BACKGROUND_IMAGE_IMAGE;
				css_hint_advance(&hint);
			}
		}
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ css_hint_white_space_nowrap(
		nscss_select_ctx *ctx,
		dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_exception err;
	bool nowrap;

	err = dom_element_has_attribute(ಗಂಟು, corestring_dom_nowrap, &nowrap);
	ಆದರೆ (err == DOM_NO_ERR && nowrap == true) {
		hint->prop = CSS_PROP_WHITE_SPACE;
		hint->ಸ್ಥಿತಿ_2 = CSS_WHITE_SPACE_NOWRAP;
		css_hint_advance(&hint);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ css_hint_list(
		nscss_select_ctx *ctx,
		dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_exception err;
	dom_string *attr;

	err = dom_element_get_attribute(ಗಂಟು, corestring_dom_type, &attr);
	ಆದರೆ (err == DOM_NO_ERR && attr != ಖಾಲಿ) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *attr_str = dom_string_ದತ್ತಾಂಶ(attr);
		size_t attr_len = dom_string_byte_ಉದ್ದಳತೆ(attr);
		ಎಣಿಕೆ css_list_style_type_e type = CSS_LIST_STYLE_TYPE_INHERIT;

		ಆದರೆ (attr_len == 1) {
			ಆಯ್ಕೆ (attr_str[0]) {
			ಸ್ಥಿತಿ 'a':
				type = CSS_LIST_STYLE_TYPE_ಅಧೋ_ALPHA;
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ 'A':
				type = CSS_LIST_STYLE_TYPE_ಊರ್ಧ್ವ_ALPHA;
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ 'i':
				type = CSS_LIST_STYLE_TYPE_ಅಧೋ_ROMAN;
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ 'I':
				type = CSS_LIST_STYLE_TYPE_ಊರ್ಧ್ವ_ROMAN;
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ '1':
				type = CSS_LIST_STYLE_TYPE_DECIMAL;
				ನಿಲ್ಲಿಸು;
			}
		}

		ಆದರೆ (type != CSS_LIST_STYLE_TYPE_INHERIT) {
			hint->prop = CSS_PROP_LIST_STYLE_TYPE;
			hint->ಸ್ಥಿತಿ_2 = type;
			css_hint_advance(&hint);
		}

		dom_string_unref(attr);
	}
}


/* Exported function, documented in css/hints.h */
css_error ಗಂಟು_presentational_hint(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		uint32_t *nhints, css_hint **hints)
{
	dom_exception exc;
	dom_html_element_type ಟ್ಯಾಗ್_type;

	ಆದರೆ (nsoption_bool(author_level_css) == false) {
		*nhints = 0;
		*hints = ಖಾಲಿ;
		ಹಿಂತಿರುಗಿ CSS_OK;
	}

	css_hint_clean();

	exc = dom_html_element_get_ಟ್ಯಾಗ್_type(ಗಂಟು, &ಟ್ಯಾಗ್_type);
	ಆದರೆ (exc != DOM_NO_ERR) {
		ಟ್ಯಾಗ್_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	ಆಯ್ಕೆ (ಟ್ಯಾಗ್_type) {
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_TH:
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_TD:
		css_hint_ಅಗಲ(pw, ಗಂಟು);
		css_hint_ಕೋಷ್ಟಕ_cell_border_padding(pw, ಗಂಟು);
		css_hint_white_space_nowrap(pw, ಗಂಟು);
		fallthrough;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_TR:
		css_hint_ಎತ್ತರ(pw, ಗಂಟು);
		fallthrough;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_THEAD:
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_TBODY:
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_TFOOT:
		css_hint_ಪಠ್ಯ_align_special(pw, ಗಂಟು);
		fallthrough;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_ಲಂಬಸಾಲು:
		css_hint_vertical_align_ಕೋಷ್ಟಕ_cells(pw, ಗಂಟು);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_APPLET:
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_IMG:
		css_hint_margin_hspace_vspace(pw, ಗಂಟು);
		fallthrough;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_EMBED:
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_IFRAME:
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_OBJECT:
		css_hint_ಎತ್ತರ(pw, ಗಂಟು);
		css_hint_ಅಗಲ(pw, ಗಂಟು);
		css_hint_vertical_align_replaced(pw, ಗಂಟು);
		css_hint_float(pw, ಗಂಟು);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_P:
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_H1:
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_H2:
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_H3:
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_H4:
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_H5:
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_H6:
		css_hint_ಪಠ್ಯ_align_ಸಾಮಾನ್ಯ(pw, ಗಂಟು);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_CENTER:
		css_hint_ಪಠ್ಯ_align_center(pw, ಗಂಟು);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_CAPTION:
		css_hint_caption_side(pw, ಗಂಟು);
		fallthrough;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_DIV:
		css_hint_ಪಠ್ಯ_align_special(pw, ಗಂಟು);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_ಕೋಷ್ಟಕ:
		css_hint_ಪಠ್ಯ_align_ಕೋಷ್ಟಕ_special(pw, ಗಂಟು);
		css_hint_ಕೋಷ್ಟಕ_spacing_border(pw, ಗಂಟು);
		css_hint_float(pw, ಗಂಟು);
		css_hint_margin_ಎಡ_ಬಲ_align_center(pw, ಗಂಟು);
		css_hint_ಅಗಲ(pw, ಗಂಟು);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_HR:
		css_hint_ಅಗಲ(pw, ಗಂಟು);
		css_hint_margin_ಎಡ_ಬಲ_hr(pw, ಗಂಟು);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		css_hint_ಎತ್ತರ_ಅಗಲ_textarea(pw, ಗಂಟು);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_ಒಳಹರಿವು:
		css_hint_ಅಗಲ_ಒಳಹರಿವು(pw, ಗಂಟು);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_A:
		css_hint_anchor_color(pw, ಗಂಟು);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_FONT:
		css_hint_font_ಗಾತ್ರ(pw, ಗಂಟು);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_BODY:
		css_hint_body_color(pw, ಗಂಟು);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_CANVAS:
		css_hint_ಎತ್ತರ_ಅಗಲ_canvas(pw, ಗಂಟು);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_OL:
		css_hint_list(pw, ಗಂಟು);
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	ಆದರೆ (ಟ್ಯಾಗ್_type != DOM_HTML_ELEMENT_TYPE__UNKNOWN) {
		css_hint_color(pw, ಗಂಟು);
		css_hint_bg_color(pw, ಗಂಟು);
		css_hint_bg_image(pw, ಗಂಟು);
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ LOG_STATS
	NSLOG(netsurf, INFO, "Properties with hints: %i", hint_ctx.len);
#ಆದರೆ_ಅಂತ್ಯ

	css_hint_get_hints(hints, nhints);

	ಹಿಂತಿರುಗಿ CSS_OK;
}
