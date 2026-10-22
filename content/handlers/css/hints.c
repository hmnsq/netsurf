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

#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <strings.h>

#ଅନ୍ତର୍ଭାଵ "utils/nsoption.h"
#ଅନ୍ତର୍ଭାଵ "utils/corestrings.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsurl.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"

#ଅନ୍ତର୍ଭାଵ "css/hints.h"
#ଅନ୍ତର୍ଭାଵ "css/select.h"

#define LOG_STATS
#undef LOG_STATS

/******************************************************************************
 * Utility functions                                                          *
 ******************************************************************************/

/**
 * Determine if a given character is whitespace
 *
 * \param c  Character to consider
 * \return true if character is whitespace, false otherwise
 */
ସ୍ଥିର bool isWhitespace(ଅକ୍ଷର c)
{
	ଫେରନ୍ତୁ c == ' ' || c == '\t' || c == '\f' || c == '\r' || c == '\n';
}

/**
 * Determine if a given character is a valid hex digit
 *
 * \param c  Character to consider
 * \return true if character is a valid hex digit, false otherwise
 */
ସ୍ଥିର bool isHex(ଅକ୍ଷର c)
{
	ଫେରନ୍ତୁ ('0' <= c && c <= '9') ||
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
ସ୍ଥିର uint8_t charToHex(ଅକ୍ଷର c)
{
	/* 0-9 */
	c -= '0';

	/* A-F */
	ଯଦି (c > 9)
		c -= 'A' - '9' - 1;

	/* a-f */
	ଯଦି (c > 15)
		c -= 'a' - 'A';

	ଫେରନ୍ତୁ c;
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
ସ୍ଥିର bool parse_ସଂଖ୍ୟା(ଅଚଳ ଅକ୍ଷର *data, bool maybe_negative, bool real,
		css_fixed *ମୂଲ୍ୟ, size_t *consumed)
{
	size_t len;
	ଅଚଳ uint8_t *ptr;
	int32_t intpart = 0;
	int32_t fracpart = 0;
	int32_t pwr = 1;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା sign = 1;

	*consumed = 0;

	len = strlen(data);
	ptr = (ଅଚଳ uint8_t *) data;

	ଯଦି (len == 0)
		ଫେରନ୍ତୁ false;

	/* Skip leading whitespace */
	ଯେତେବେଳେ (len > 0 && isWhitespace(ptr[0])) {
		len--;
		ptr++;
	}

	ଯଦି (len == 0)
		ଫେରନ୍ତୁ false;

	/* Extract sign, if any */
	ଯଦି (ptr[0] == '+') {
		len--;
		ptr++;
	} ଅନ୍ୟଥା ଯଦି (ptr[0] == '-' && maybe_negative) {
		sign = -1;
		len--;
		ptr++;
	}

	ଯଦି (len == 0)
		ଫେରନ୍ତୁ false;

	/* Must have a digit [0,9] */
	ଯଦି ('0' > ptr[0] || ptr[0] > '9')
		ଫେରନ୍ତୁ false;

	/* Now extract intpart, assuming base 10 */
	ଯେତେବେଳେ (len > 0) {
		/* Stop on first non-digit */
		ଯଦି (ptr[0] < '0' || '9' < ptr[0])
			ଭାଙ୍ଗ;

		/* Prevent overflow of 'intpart'; proper clamping below */
		ଯଦି (intpart < (1 << 22)) {
			intpart *= 10;
			intpart += ptr[0] - '0';
		}
		ptr++;
		len--;
	}

	/* And fracpart, again, assuming base 10 */
	ଯଦି (real && len > 1 && ptr[0] == '.' &&
			('0' <= ptr[1] && ptr[1] <= '9')) {
		ptr++;
		len--;

		ଯେତେବେଳେ (len > 0) {
			ଯଦି (ptr[0] < '0' || '9' < ptr[0])
				ଭାଙ୍ଗ;

			ଯଦି (pwr < 1000000) {
				pwr *= 10;
				fracpart *= 10;
				fracpart += ptr[0] - '0';
			}
			ptr++;
			len--;
		}

		fracpart = ((1 << 10) * fracpart + pwr/2) / pwr;
		ଯଦି (fracpart >= (1 << 10)) {
			intpart++;
			fracpart &= (1 << 10) - 1;
		}
	}

	ଯଦି (sign > 0) {
		/* If the result is larger than we can represent,
		 * then clamp to the maximum value we can store. */
		ଯଦି (intpart >= (1 << 21)) {
			intpart = (1 << 21) - 1;
			fracpart = (1 << 10) - 1;
		}
	} ଅନ୍ୟଥା {
		/* If the negated result is smaller than we can represent
		 * then clamp to the minimum value we can store. */
		ଯଦି (intpart >= (1 << 21)) {
			intpart = -(1 << 21);
			fracpart = 0;
		} ଅନ୍ୟଥା {
			intpart = -intpart;
			ଯଦି (fracpart) {
				fracpart = (1 << 10) - fracpart;
				intpart--;
			}
		}
	}

	*ମୂଲ୍ୟ = (intpart << 10) | fracpart;

	*consumed = ptr - (ଅଚଳ uint8_t *) data;

	ଫେରନ୍ତୁ true;
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
ସ୍ଥିର bool parse_dimension(ଅଚଳ ଅକ୍ଷର *data, bool strict, css_fixed *length,
		css_unit *unit)
{
	size_t len;
	size_t ପଠ;
	css_fixed ମୂଲ୍ୟ;

	len = strlen(data);

	ଯଦି (parse_ସଂଖ୍ୟା(data, false, true, &ମୂଲ୍ୟ, &ପଠ) == false)
		ଫେରନ୍ତୁ false;

	ଯଦି (strict && ମୂଲ୍ୟ < INTTOFIX(1))
		ଫେରନ୍ତୁ false;

	*length = ମୂଲ୍ୟ;

	ଯଦି (len > ପଠ && data[ପଠ] == '%')
		*unit = CSS_UNIT_PCT;
	ଅନ୍ୟଥା
		*unit = CSS_UNIT_PX;

	ଫେରନ୍ତୁ true;
}

/**
 * Mapping of colour name to CSS color
 */
ସଂରଚନା colour_map {
	ଅଚଳ ଅକ୍ଷର *name;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmp_colour_name(ଅଚଳ ଶୂନ୍ୟ *a, ଅଚଳ ଶୂନ୍ୟ *b)
{
	ଅଚଳ ଅକ୍ଷର *aa = a;
	ଅଚଳ ସଂରଚନା colour_map *bb = b;

	ଫେରନ୍ତୁ strcasecmp(aa, bb->name);
}

/**
 * Parse a named colour
 *
 * \param name    Name to parse
 * \param result  Pointer to location to receive css_color
 * \return true on success, false on invalid input
 */
ସ୍ଥିର bool parse_named_colour(ଅଚଳ ଅକ୍ଷର *name, css_color *result)
{
	ସ୍ଥିର ଅଚଳ ସଂରଚନା colour_map named_colours[] = {
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
	ଅଚଳ ସଂରଚନା colour_map *entry;

	entry = bsearch(name, named_colours,
			ଆକାର(named_colours) / ଆକାର(named_colours[0]),
			ଆକାର(named_colours[0]),
			cmp_colour_name);

	ଯଦି (entry != ଖାଲି)
		*result = entry->color;

	ଫେରନ୍ତୁ entry != ଖାଲି;
}

/* exported interface documented in content/handlers/css/hints.h */
bool nscss_parse_colour(ଅଚଳ ଅକ୍ଷର *data, css_color *result)
{
	size_t len = strlen(data);
	uint8_t r, g, b;

	/* 2 */
	ଯଦି (len == 0)
		ଫେରନ୍ତୁ false;

	/* 3 */
	ଯଦି (len == SLEN("transparent") && strcasecmp(data, "transparent") == 0)
		ଫେରନ୍ତୁ false;

	/* 4 */
	ଯଦି (parse_named_colour(data, result))
		ଫେରନ୍ତୁ true;

	/** \todo Implement HTML5's utterly insane legacy colour parsing */

	ଯଦି (data[0] == '#') {
		data++;
		len--;
	}

	ଯଦି (len == 3 && isHex(data[0]) && isHex(data[1]) && isHex(data[2])) {
		r = charToHex(data[0]);
		g = charToHex(data[1]);
		b = charToHex(data[2]);

		r |= (r << 4);
		g |= (g << 4);
		b |= (b << 4);

		*result = (0xff << 24) | (r << 16) | (g << 8) | b;

		ଫେରନ୍ତୁ true;
	} ଅନ୍ୟଥା ଯଦି (len == 6 && isHex(data[0]) && isHex(data[1]) &&
			isHex(data[2]) && isHex(data[3]) && isHex(data[4]) &&
			isHex(data[5])) {
		r = (charToHex(data[0]) << 4) | charToHex(data[1]);
		g = (charToHex(data[2]) << 4) | charToHex(data[3]);
		b = (charToHex(data[4]) << 4) | charToHex(data[5]);

		*result = (0xff << 24) | (r << 16) | (g << 8) | b;

		ଫେରନ୍ତୁ true;
	}

	ଫେରନ୍ତୁ false;
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
ସ୍ଥିର bool parse_font_size(ଅଚଳ ଅକ୍ଷର *size, uint8_t *val,
		css_fixed *len, css_unit *unit)
{
	ସ୍ଥିର ଅଚଳ uint8_t size_map[] = {
		CSS_FONT_SIZE_XX_SMALL,
		CSS_FONT_SIZE_SMALL,
		CSS_FONT_SIZE_MEDIUM,
		CSS_FONT_SIZE_LARGE,
		CSS_FONT_SIZE_X_LARGE,
		CSS_FONT_SIZE_XX_LARGE,
		CSS_FONT_SIZE_DIMENSION	/* xxx-large (see below) */
	};

	ଅଚଳ ଅକ୍ଷର *p = size;
	ଅକ୍ଷର mode;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୂଲ୍ୟ = 0;

	/* Skip whitespace */
	ଯେତେବେଳେ (*p != '\0' && isWhitespace(*p))
		p++;

	mode = *p;

	/* Skip +/- */
	ଯଦି (mode == '+' || mode == '-')
		p++;

	/* Need at least one digit */
	ଯଦି (*p < '0' || *p > '9') {
		ଫେରନ୍ତୁ false;
	}

	/* Consume digits, computing value */
	ଯେତେବେଳେ ('0' <= *p && *p <= '9') {
		ମୂଲ୍ୟ = ମୂଲ୍ୟ * 10 + (*p - '0');
		p++;
	}

	/* Resolve relative sizes */
	ଯଦି (mode == '+')
		ମୂଲ୍ୟ += 3;
	ଅନ୍ୟଥା ଯଦି (mode == '-')
		ମୂଲ୍ୟ = 3 - ମୂଲ୍ୟ;

	/* Clamp to range [1,7] */
	ଯଦି (ମୂଲ୍ୟ < 1)
		ମୂଲ୍ୟ = 1;
	ଅନ୍ୟଥା ଯଦି (ମୂଲ୍ୟ > 7)
		ମୂଲ୍ୟ = 7;

	ଯଦି (ମୂଲ୍ୟ == 7) {
		/* Manufacture xxx-large */
	  *len = FDIV(FMUL(INTTOFIX(3), INTTOFIX(nsoption_int(font_size))),
				F_10);
	} ଅନ୍ୟଥା {
		/* Len is irrelevant */
		*len = 0;
	}

	*unit = CSS_UNIT_PT;
	*val = size_map[ମୂଲ୍ୟ - 1];

	ଫେରନ୍ତୁ true;
}


/******************************************************************************
 * Hint context management                                                    *
 ******************************************************************************/

#define MAX_HINTS_PER_ELEMENT 32

ସଂରଚନା css_hint_ctx {
	ସଂରଚନା css_hint *hints;
	uint32_t len;
};

ସଂରଚନା css_hint_ctx hint_ctx;

nserror css_hint_init(ଶୂନ୍ୟ)
{
	hint_ctx.hints = malloc(ଆକାର(ସଂରଚନା css_hint) *
			MAX_HINTS_PER_ELEMENT);
	ଯଦି (hint_ctx.hints == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}

ଶୂନ୍ୟ css_hint_fini(ଶୂନ୍ୟ)
{
	hint_ctx.len = 0;
	ମୁକ୍ତ(hint_ctx.hints);
}

ସ୍ଥିର ଶୂନ୍ୟ css_hint_clean(ଶୂନ୍ୟ)
{
	hint_ctx.len = 0;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ css_hint_advance(ସଂରଚନା css_hint **hint)
{
	hint_ctx.len++;
	ପ୍ରତିଜ୍ଞା(hint_ctx.len < MAX_HINTS_PER_ELEMENT);

	(*hint)++;
}

ସ୍ଥିର ଶୂନ୍ୟ css_hint_get_hints(ସଂରଚନା css_hint **hints, uint32_t *nhints)
{
	*hints = hint_ctx.hints;
	*nhints = hint_ctx.len;
}


/******************************************************************************
 * Presentational hint handlers                                               *
 ******************************************************************************/

ସ୍ଥିର ଶୂନ୍ୟ css_hint_table_cell_border_padding(
		nscss_select_ctx *ctx,
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	css_qname qs;
	dom_string *attr = ଖାଲି;
	dom_ଗଣ୍ଠି *tablenode = ଖାଲି;
	dom_exception exc;

	qs.ns = ଖାଲି;
	qs.name = lwc_string_ref(corestring_lwc_table);
	ଯଦି (named_ancestor_ଗଣ୍ଠି(ctx, ଗଣ୍ଠି, &qs,
			(ଶୂନ୍ୟ *)&tablenode) != CSS_OK) {
		/* Didn't find, or had error */
		lwc_string_unref(qs.name);
		ଫେରନ୍ତୁ;
	}
	lwc_string_unref(qs.name);

	ଯଦି (tablenode == ଖାଲି) {
		ଫେରନ୍ତୁ;
	}
	/* No need to unref tablenode, named_ancestor_node does not
	 * return a reffed node to the CSS
	 */

	exc = dom_element_get_attribute(tablenode,
			corestring_dom_border, &attr);

	ଯଦି (exc == DOM_NO_ERR && attr != ଖାଲି) {
		uint32_t hint_prop;
		css_hint_length hint_length;

		ଯଦି (parse_dimension(
				dom_string_data(attr), false,
						&hint_length.ମୂଲ୍ୟ,
						&hint_length.unit) &&
				INTTOFIX(0) != hint_length.ମୂଲ୍ୟ) {

			ପାଇଁ (hint_prop = CSS_PROP_BORDER_ଶୀର୍ଷ_STYLE;
			     hint_prop <= CSS_PROP_BORDER_ବାମ_STYLE;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->status = CSS_BORDER_STYLE_INSET;
				css_hint_advance(&hint);
			}

			ପାଇଁ (hint_prop = CSS_PROP_BORDER_ଶୀର୍ଷ_WIDTH;
			     hint_prop <= CSS_PROP_BORDER_ବାମ_WIDTH;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->data.length.ମୂଲ୍ୟ = INTTOFIX(1);
				hint->data.length.unit = CSS_UNIT_PX;
				hint->status = CSS_BORDER_WIDTH_WIDTH;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(tablenode,
			corestring_dom_bordercolor, &attr);

	ଯଦି (exc == DOM_NO_ERR && attr != ଖାଲି) {
		uint32_t hint_prop;
		css_color hint_color;

		ଯଦି (nscss_parse_colour(
				(ଅଚଳ ଅକ୍ଷର *)dom_string_data(attr),
				&hint_color)) {

			ପାଇଁ (hint_prop = CSS_PROP_BORDER_ଶୀର୍ଷ_COLOR;
			     hint_prop <= CSS_PROP_BORDER_ବାମ_COLOR;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->data.color = hint_color;
				hint->status = CSS_BORDER_COLOR_COLOR;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(tablenode,
			corestring_dom_cellpadding, &attr);

	ଯଦି (exc == DOM_NO_ERR && attr != ଖାଲି) {
		uint32_t hint_prop;
		css_hint_length hint_length;

		ଯଦି (parse_dimension(
				dom_string_data(attr), false,
						&hint_length.ମୂଲ୍ୟ,
						&hint_length.unit)) {

			ପାଇଁ (hint_prop = CSS_PROP_PADDING_ଶୀର୍ଷ;
			     hint_prop <= CSS_PROP_PADDING_ବାମ;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->data.length.ମୂଲ୍ୟ = hint_length.ମୂଲ୍ୟ;
				hint->data.length.unit = hint_length.unit;
				hint->status = CSS_PADDING_SET;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}
}

ସ୍ଥିର ଶୂନ୍ୟ css_hint_vertical_align_table_cells(
		nscss_select_ctx *ctx,
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = ଖାଲି;
	dom_exception err;

	err = dom_element_get_attribute(ଗଣ୍ଠି,
			corestring_dom_valign, &attr);

	ଯଦି (err == DOM_NO_ERR && attr != ଖାଲି) {
		hint->data.length.ମୂଲ୍ୟ = 0;
		hint->data.length.unit = CSS_UNIT_PX;
		ଯଦି (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_ଶୀର୍ଷ)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->status = CSS_VERTICAL_ALIGN_ଶୀର୍ଷ;
			css_hint_advance(&hint);

		} ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_ମଧ୍ୟ)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->status = CSS_VERTICAL_ALIGN_ମଧ୍ୟ;
			css_hint_advance(&hint);

		} ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_bottom)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->status = CSS_VERTICAL_ALIGN_BOTTOM;
			css_hint_advance(&hint);

		} ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_baseline)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->status = CSS_VERTICAL_ALIGN_BASELINE;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ସ୍ଥିର ଶୂନ୍ୟ css_hint_vertical_align_replaced(
		nscss_select_ctx *ctx,
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = ଖାଲି;
	dom_exception err;

	err = dom_element_get_attribute(ଗଣ୍ଠି,
			corestring_dom_valign, &attr);

	ଯଦି (err == DOM_NO_ERR && attr != ଖାଲି) {
		ଯଦି (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_ଶୀର୍ଷ)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->status = CSS_VERTICAL_ALIGN_ଶୀର୍ଷ;
			css_hint_advance(&hint);

		} ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_bottom) ||
			   dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_baseline)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->status = CSS_VERTICAL_ALIGN_BASELINE;
			css_hint_advance(&hint);

		} ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_texttop)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->status = CSS_VERTICAL_ALIGN_TEXT_ଶୀର୍ଷ;
			css_hint_advance(&hint);

		} ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_absmiddle) ||
			   dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_abscenter)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->status = CSS_VERTICAL_ALIGN_ମଧ୍ୟ;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ସ୍ଥିର ଶୂନ୍ୟ css_hint_text_align_normal(
		nscss_select_ctx *ctx,
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *align = ଖାଲି;
	dom_exception err;

	err = dom_element_get_attribute(ଗଣ୍ଠି,
			corestring_dom_align, &align);
	ଯଦି (err == DOM_NO_ERR && align != ଖାଲି) {
		ଯଦି (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_ବାମ)) {
			hint->prop = CSS_PROP_TEXT_ALIGN;
			hint->status = CSS_TEXT_ALIGN_ବାମ;
			css_hint_advance(&hint);

		} ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_center)) {
			hint->prop = CSS_PROP_TEXT_ALIGN;
			hint->status = CSS_TEXT_ALIGN_CENTER;
			css_hint_advance(&hint);

		} ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_ଡାହାଣ)) {
			hint->prop = CSS_PROP_TEXT_ALIGN;
			hint->status = CSS_TEXT_ALIGN_ଡାହାଣ;
			css_hint_advance(&hint);

		} ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_justify)) {
			hint->prop = CSS_PROP_TEXT_ALIGN;
			hint->status = CSS_TEXT_ALIGN_JUSTIFY;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

ସ୍ଥିର ଶୂନ୍ୟ css_hint_text_align_center(
		nscss_select_ctx *ctx,
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା css_hint *hint = &hint_ctx.hints[hint_ctx.len];

	hint->prop = CSS_PROP_TEXT_ALIGN;
	hint->status = CSS_TEXT_ALIGN_LIBCSS_CENTER;
	css_hint_advance(&hint);
}

ସ୍ଥିର ଶୂନ୍ୟ css_hint_margin_ବାମ_ଡାହାଣ_align_center(
		nscss_select_ctx *ctx,
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr;
	dom_exception exc;

	exc = dom_element_get_attribute(ଗଣ୍ଠି,
			corestring_dom_align, &attr);

	ଯଦି (exc == DOM_NO_ERR && attr != ଖାଲି) {
		memset(hint, 0, ଆକାର(*hint) * 2);
		ଯଦି (dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_center) ||
				dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_abscenter) ||
				dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_ମଧ୍ୟ) ||
				dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_absmiddle)) {
			hint->prop = CSS_PROP_MARGIN_ବାମ;
			hint->status = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_ଡାହାଣ;
			hint->status = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ସ୍ଥିର ଶୂନ୍ୟ css_hint_text_align_special(
		nscss_select_ctx *ctx,
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *align = ଖାଲି;
	dom_exception err;

	err = dom_element_get_attribute(ଗଣ୍ଠି,
			corestring_dom_align, &align);

	ଯଦି (err == DOM_NO_ERR && align != ଖାଲି) {
		ଯଦି (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_center)) {
			hint->prop = CSS_PROP_TEXT_ALIGN;
			hint->status = CSS_TEXT_ALIGN_LIBCSS_CENTER;
			css_hint_advance(&hint);

		} ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_ବାମ)) {
			hint->prop = CSS_PROP_TEXT_ALIGN;
			hint->status = CSS_TEXT_ALIGN_LIBCSS_ବାମ;
			css_hint_advance(&hint);

		} ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_ଡାହାଣ)) {
			hint->prop = CSS_PROP_TEXT_ALIGN;
			hint->status = CSS_TEXT_ALIGN_LIBCSS_ଡାହାଣ;
			css_hint_advance(&hint);

		} ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_justify)) {
			hint->prop = CSS_PROP_TEXT_ALIGN;
			hint->status = CSS_TEXT_ALIGN_JUSTIFY;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

ସ୍ଥିର ଶୂନ୍ୟ css_hint_text_align_table_special(
		nscss_select_ctx *ctx,
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା css_hint *hint = &hint_ctx.hints[hint_ctx.len];

	hint->prop = CSS_PROP_TEXT_ALIGN;
	hint->status = CSS_TEXT_ALIGN_INHERIT_IF_NON_MAGIC;
	css_hint_advance(&hint);
}

ସ୍ଥିର ଶୂନ୍ୟ css_hint_margin_hspace_vspace(
		nscss_select_ctx *ctx,
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = ଖାଲି;
	dom_exception exc;

	exc = dom_element_get_attribute(ଗଣ୍ଠି,
			corestring_dom_vspace, &attr);

	ଯଦି (exc == DOM_NO_ERR && attr != ଖାଲି) {
		css_hint_length hint_length;
		ଯଦି (parse_dimension(
				dom_string_data(attr), false,
				&hint_length.ମୂଲ୍ୟ,
				&hint_length.unit)) {
			hint->prop = CSS_PROP_MARGIN_ଶୀର୍ଷ;
			hint->data.length.ମୂଲ୍ୟ = hint_length.ମୂଲ୍ୟ;
			hint->data.length.unit = hint_length.unit;
			hint->status = CSS_MARGIN_SET;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_BOTTOM;
			hint->data.length.ମୂଲ୍ୟ = hint_length.ମୂଲ୍ୟ;
			hint->data.length.unit = hint_length.unit;
			hint->status = CSS_MARGIN_SET;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(ଗଣ୍ଠି,
			corestring_dom_hspace, &attr);

	ଯଦି (exc == DOM_NO_ERR && attr != ଖାଲି) {
		css_hint_length hint_length;
		ଯଦି (parse_dimension(
				dom_string_data(attr), false,
				&hint_length.ମୂଲ୍ୟ,
				&hint_length.unit)) {
			hint->prop = CSS_PROP_MARGIN_ବାମ;
			hint->data.length.ମୂଲ୍ୟ = hint_length.ମୂଲ୍ୟ;
			hint->data.length.unit = hint_length.unit;
			hint->status = CSS_MARGIN_SET;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_ଡାହାଣ;
			hint->data.length.ମୂଲ୍ୟ = hint_length.ମୂଲ୍ୟ;
			hint->data.length.unit = hint_length.unit;
			hint->status = CSS_MARGIN_SET;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ସ୍ଥିର ଶୂନ୍ୟ css_hint_margin_ବାମ_ଡାହାଣ_hr(
		nscss_select_ctx *ctx,
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr;
	dom_exception exc;

	exc = dom_element_get_attribute(ଗଣ୍ଠି,
				corestring_dom_align, &attr);

	ଯଦି (exc == DOM_NO_ERR && attr != ଖାଲି) {
		memset(hint, 0, ଆକାର(*hint) * 2);
		ଯଦି (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_ବାମ)) {
			hint->prop = CSS_PROP_MARGIN_ବାମ;
			hint->data.length.ମୂଲ୍ୟ = 0;
			hint->data.length.unit = CSS_UNIT_PX;
			hint->status = CSS_MARGIN_SET;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_ଡାହାଣ;
			hint->status = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

		} ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_center)) {
			hint->prop = CSS_PROP_MARGIN_ବାମ;
			hint->status = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_ଡାହାଣ;
			hint->status = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

		} ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_ଡାହାଣ)) {
			hint->prop = CSS_PROP_MARGIN_ବାମ;
			hint->status = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_ଡାହାଣ;
			hint->data.length.ମୂଲ୍ୟ = 0;
			hint->data.length.unit = CSS_UNIT_PX;
			hint->status = CSS_MARGIN_SET;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ସ୍ଥିର ଶୂନ୍ୟ css_hint_table_spacing_border(
		nscss_select_ctx *ctx,
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception exc;
	dom_string *attr = ଖାଲି;

	exc = dom_element_get_attribute(ଗଣ୍ଠି, corestring_dom_border, &attr);

	ଯଦି (exc == DOM_NO_ERR && attr != ଖାଲି) {
		uint32_t hint_prop;
		css_hint_length hint_length;

		ପାଇଁ (hint_prop = CSS_PROP_BORDER_ଶୀର୍ଷ_STYLE;
			     hint_prop <= CSS_PROP_BORDER_ବାମ_STYLE;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->status = CSS_BORDER_STYLE_OUTSET;
				css_hint_advance(&hint);
		}

		ଯଦି (parse_dimension(
				dom_string_data(attr), false,
						&hint_length.ମୂଲ୍ୟ,
						&hint_length.unit)) {

			ପାଇଁ (hint_prop = CSS_PROP_BORDER_ଶୀର୍ଷ_WIDTH;
			     hint_prop <= CSS_PROP_BORDER_ବାମ_WIDTH;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->data.length.ମୂଲ୍ୟ = hint_length.ମୂଲ୍ୟ;
				hint->data.length.unit = hint_length.unit;
				hint->status = CSS_BORDER_WIDTH_WIDTH;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(ଗଣ୍ଠି,
			corestring_dom_bordercolor, &attr);

	ଯଦି (exc == DOM_NO_ERR && attr != ଖାଲି) {
		uint32_t hint_prop;
		css_color hint_color;

		ଯଦି (nscss_parse_colour(
				(ଅଚଳ ଅକ୍ଷର *)dom_string_data(attr),
				&hint_color)) {

			ପାଇଁ (hint_prop = CSS_PROP_BORDER_ଶୀର୍ଷ_COLOR;
			     hint_prop <= CSS_PROP_BORDER_ବାମ_COLOR;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->data.color = hint_color;
				hint->status = CSS_BORDER_COLOR_COLOR;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(ଗଣ୍ଠି,
			corestring_dom_cellspacing, &attr);

	ଯଦି (exc == DOM_NO_ERR && attr != ଖାଲି) {
		ଯଦି (parse_dimension(
				(ଅଚଳ ଅକ୍ଷର *)dom_string_data(attr), false,
						&hint->data.position.h.ମୂଲ୍ୟ,
						&hint->data.position.h.unit)) {
			hint->prop = CSS_PROP_BORDER_SPACING;
			hint->data.position.v = hint->data.position.h;
			hint->status = CSS_BORDER_SPACING_SET;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ସ୍ଥିର ଶୂନ୍ୟ css_hint_height(
		nscss_select_ctx *ctx,
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = ଖାଲି;
	dom_exception err;

	err = dom_element_get_attribute(ଗଣ୍ଠି,
			corestring_dom_height, &attr);

	ଯଦି (err == DOM_NO_ERR && attr != ଖାଲି) {
		ଯଦି (parse_dimension(
				(ଅଚଳ ଅକ୍ଷର *)dom_string_data(attr), false,
				&hint->data.length.ମୂଲ୍ୟ,
				&hint->data.length.unit)) {
			hint->prop = CSS_PROP_HEIGHT;
			hint->status = CSS_HEIGHT_SET;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ସ୍ଥିର ଶୂନ୍ୟ css_hint_width(
		nscss_select_ctx *ctx,
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = ଖାଲି;
	dom_exception err;

	err = dom_element_get_attribute(ଗଣ୍ଠି,
			corestring_dom_width, &attr);

	ଯଦି (err == DOM_NO_ERR && attr != ଖାଲି) {
		ଯଦି (parse_dimension(
				(ଅଚଳ ଅକ୍ଷର *)dom_string_data(attr), false,
				&hint->data.length.ମୂଲ୍ୟ,
				&hint->data.length.unit)) {
			hint->prop = CSS_PROP_WIDTH;
			hint->status = CSS_WIDTH_SET;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ସ୍ଥିର ଶୂନ୍ୟ css_hint_height_width_textarea(
		nscss_select_ctx *ctx,
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = ଖାଲି;
	dom_exception err;

	err = dom_element_get_attribute(ଗଣ୍ଠି,
			corestring_dom_rows, &attr);

	ଯଦି (err == DOM_NO_ERR && attr != ଖାଲି) {
		ଯଦି (parse_dimension(
				(ଅଚଳ ଅକ୍ଷର *)dom_string_data(attr), false,
				&hint->data.length.ମୂଲ୍ୟ,
				&hint->data.length.unit)) {
			hint->prop = CSS_PROP_HEIGHT;
			hint->data.length.unit = CSS_UNIT_EM;
			hint->status = CSS_HEIGHT_SET;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}

	err = dom_element_get_attribute(ଗଣ୍ଠି,
			corestring_dom_cols, &attr);

	ଯଦି (err == DOM_NO_ERR && attr != ଖାଲି) {
		ଯଦି (parse_dimension(
				(ଅଚଳ ଅକ୍ଷର *)dom_string_data(attr), false,
				&hint->data.length.ମୂଲ୍ୟ,
				&hint->data.length.unit)) {
			hint->prop = CSS_PROP_WIDTH;
			hint->data.length.unit = CSS_UNIT_EX;
			hint->status = CSS_WIDTH_SET;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

ସ୍ଥିର ଶୂନ୍ୟ css_hint_height_width_canvas(
		nscss_select_ctx *ctx,
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = ଖାଲି;
	dom_exception err;
	bool set_dim = false;

	err = dom_element_get_attribute(ଗଣ୍ଠି,
			corestring_dom_height, &attr);

	ଯଦି (err == DOM_NO_ERR && attr != ଖାଲି) {
		ଯଦି (parse_dimension(
				(ଅଚଳ ଅକ୍ଷର *)dom_string_data(attr), true,
				&hint->data.length.ମୂଲ୍ୟ,
				&hint->data.length.unit)) {
			hint->prop = CSS_PROP_HEIGHT;
			hint->data.length.unit = CSS_UNIT_PX;
			hint->status = CSS_HEIGHT_SET;
			css_hint_advance(&hint);
			set_dim = true;
		}
		dom_string_unref(attr);
	}
	ଯଦି (set_dim == false) {
		/* canvas defaults to 150px tall */
		hint->prop = CSS_PROP_HEIGHT;
		hint->data.length.unit = CSS_UNIT_PX;
		hint->data.length.ମୂଲ୍ୟ = INTTOFIX(150);
		hint->status = CSS_HEIGHT_SET;
		css_hint_advance(&hint);
	} ଅନ୍ୟଥା {
		set_dim = false;
	}

	err = dom_element_get_attribute(ଗଣ୍ଠି,
			corestring_dom_width, &attr);

	ଯଦି (err == DOM_NO_ERR && attr != ଖାଲି) {
		ଯଦି (parse_dimension(
				(ଅଚଳ ଅକ୍ଷର *)dom_string_data(attr), true,
				&hint->data.length.ମୂଲ୍ୟ,
				&hint->data.length.unit)) {
			hint->prop = CSS_PROP_WIDTH;
			hint->data.length.unit = CSS_UNIT_PX;
			hint->status = CSS_WIDTH_SET;
			css_hint_advance(&hint);
			set_dim = true;
		}
		dom_string_unref(attr);
	}
	ଯଦି (set_dim == false) {
		/* canvas defaults to 300px wide */
		hint->prop = CSS_PROP_WIDTH;
		hint->data.length.unit = CSS_UNIT_PX;
		hint->data.length.ମୂଲ୍ୟ = INTTOFIX(300);
		hint->status = CSS_WIDTH_SET;
		css_hint_advance(&hint);
	}
}

ସ୍ଥିର ଶୂନ୍ୟ css_hint_width_ନିବେଶ(
		nscss_select_ctx *ctx,
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_string *attr = ଖାଲି;
	dom_exception err;

	err = dom_element_get_attribute(ଗଣ୍ଠି,
			corestring_dom_size, &attr);

	ଯଦି (err == DOM_NO_ERR && attr != ଖାଲି) {
		ଯଦି (parse_dimension(
				(ଅଚଳ ଅକ୍ଷର *)dom_string_data(attr), false,
				&hint->data.length.ମୂଲ୍ୟ,
				&hint->data.length.unit)) {
			dom_string *attr2 = ଖାଲି;

			err = dom_element_get_attribute(ଗଣ୍ଠି,
					corestring_dom_type, &attr2);
			ଯଦି (err == DOM_NO_ERR) {

				hint->prop = CSS_PROP_WIDTH;
				hint->status = CSS_WIDTH_SET;

				ଯଦି (attr2 == ଖାଲି ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_text) ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_ସନ୍ଧାନ) ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_password) ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_file)) {
					hint->data.length.unit = CSS_UNIT_EX;
				}
				ଯଦି (attr2 != ଖାଲି) {
					dom_string_unref(attr2);
				}
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}
}

ସ୍ଥିର ଶୂନ୍ୟ css_hint_anchor_color(
		nscss_select_ctx *ctx,
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	css_error error;
	dom_exception err;
	dom_string *color;
	dom_ଗଣ୍ଠି *bodynode = ଖାଲି;

	/* find body node */
	css_qname qs;
	bool is_visited;

	qs.ns = ଖାଲି;
	qs.name = lwc_string_ref(corestring_lwc_body);
	ଯଦି (named_ancestor_ଗଣ୍ଠି(ctx, ଗଣ୍ଠି, &qs,
			(ଶୂନ୍ୟ *)&bodynode) != CSS_OK) {
		/* Didn't find, or had error */
		lwc_string_unref(qs.name);
		ଫେରନ୍ତୁ ;
	}
	lwc_string_unref(qs.name);

	ଯଦି (bodynode == ଖାଲି) {
		ଫେରନ୍ତୁ;
	}

	error = ଗଣ୍ଠି_is_visited(ctx, ଗଣ୍ଠି, &is_visited);
	ଯଦି (error != CSS_OK)
		ଫେରନ୍ତୁ;

	ଯଦି (is_visited) {
		err = dom_element_get_attribute(bodynode,
				corestring_dom_vlink, &color);
	} ଅନ୍ୟଥା {
		err = dom_element_get_attribute(bodynode,
				corestring_dom_link, &color);
	}

	ଯଦି (err == DOM_NO_ERR && color != ଖାଲି) {
		ଯଦି (nscss_parse_colour(
				(ଅଚଳ ଅକ୍ଷର *)dom_string_data(color),
						&hint->data.color)) {
			hint->prop = CSS_PROP_COLOR;
			hint->status = CSS_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(color);
	}
}

ସ୍ଥିର ଶୂନ୍ୟ css_hint_body_color(
		nscss_select_ctx *ctx,
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *color;

	err = dom_element_get_attribute(ଗଣ୍ଠି, corestring_dom_text, &color);

	ଯଦି (err == DOM_NO_ERR && color != ଖାଲି) {
		ଯଦି (nscss_parse_colour(
				(ଅଚଳ ଅକ୍ଷର *)dom_string_data(color),
						&hint->data.color)) {
			hint->prop = CSS_PROP_COLOR;
			hint->status = CSS_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(color);
	}
}

ସ୍ଥିର ଶୂନ୍ୟ css_hint_color(
		nscss_select_ctx *ctx,
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *color;

	err = dom_element_get_attribute(ଗଣ୍ଠି, corestring_dom_color, &color);

	ଯଦି (err == DOM_NO_ERR && color != ଖାଲି) {
		ଯଦି (nscss_parse_colour(
				(ଅଚଳ ଅକ୍ଷର *)dom_string_data(color),
						&hint->data.color)) {
			hint->prop = CSS_PROP_COLOR;
			hint->status = CSS_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(color);
	}
}

ସ୍ଥିର ଶୂନ୍ୟ css_hint_font_size(
		nscss_select_ctx *ctx,
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *size;

	err = dom_element_get_attribute(ଗଣ୍ଠି, corestring_dom_size, &size);
	ଯଦି (err == DOM_NO_ERR && size != ଖାଲି) {
		ଯଦି (parse_font_size(
				(ଅଚଳ ଅକ୍ଷର *)dom_string_data(size),
						&hint->status,
						&hint->data.length.ମୂଲ୍ୟ,
						&hint->data.length.unit)) {
			hint->prop = CSS_PROP_FONT_SIZE;
			css_hint_advance(&hint);
		}
		dom_string_unref(size);
	}
}

ସ୍ଥିର ଶୂନ୍ୟ css_hint_float(
		nscss_select_ctx *ctx,
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *align;

	err = dom_element_get_attribute(ଗଣ୍ଠି, corestring_dom_align, &align);
	ଯଦି (err == DOM_NO_ERR && align != ଖାଲି) {
		ଯଦି (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_ବାମ)) {
			hint->prop = CSS_PROP_FLOAT;
			hint->status = CSS_FLOAT_ବାମ;
			css_hint_advance(&hint);

		} ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_ଡାହାଣ)) {
			hint->prop = CSS_PROP_FLOAT;
			hint->status = CSS_FLOAT_ଡାହାଣ;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

ସ୍ଥିର ଶୂନ୍ୟ css_hint_caption_side(
		nscss_select_ctx *ctx,
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *align = ଖାଲି;

	err = dom_element_get_attribute(ଗଣ୍ଠି, corestring_dom_align, &align);
	ଯଦି (err == DOM_NO_ERR && align != ଖାଲି) {
		ଯଦି (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_bottom)) {
			hint->prop = CSS_PROP_CAPTION_SIDE;
			hint->status = CSS_CAPTION_SIDE_BOTTOM;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

ସ୍ଥିର ଶୂନ୍ୟ css_hint_bg_color(
		nscss_select_ctx *ctx,
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *bgcolor;

	err = dom_element_get_attribute(ଗଣ୍ଠି,
			corestring_dom_bgcolor, &bgcolor);
	ଯଦି (err == DOM_NO_ERR && bgcolor != ଖାଲି) {
		ଯଦି (nscss_parse_colour(
				(ଅଚଳ ଅକ୍ଷର *)dom_string_data(bgcolor),
				&hint->data.color)) {
			hint->prop = CSS_PROP_BACKGROUND_COLOR;
			hint->status = CSS_BACKGROUND_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(bgcolor);
	}
}

ସ୍ଥିର ଶୂନ୍ୟ css_hint_bg_image(
		nscss_select_ctx *ctx,
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_exception err;
	dom_string *attr;

	err = dom_element_get_attribute(ଗଣ୍ଠି,
			corestring_dom_background, &attr);
	ଯଦି (err == DOM_NO_ERR && attr != ଖାଲି) {
		nsurl *url;
		nserror error = nsurl_join(ctx->base_url,
				(ଅଚଳ ଅକ୍ଷର *)dom_string_data(attr), &url);
		dom_string_unref(attr);

		ଯଦି (error == NSERROR_OK) {
			lwc_string *iurl;
			lwc_error lerror = lwc_intern_string(nsurl_access(url),
					nsurl_length(url), &iurl);
			nsurl_unref(url);

			ଯଦି (lerror == lwc_error_ok) {
				hint->prop = CSS_PROP_BACKGROUND_IMAGE;
				hint->data.string = iurl;
				hint->status = CSS_BACKGROUND_IMAGE_IMAGE;
				css_hint_advance(&hint);
			}
		}
	}
}

ସ୍ଥିର ଶୂନ୍ୟ css_hint_white_space_nowrap(
		nscss_select_ctx *ctx,
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_exception err;
	bool nowrap;

	err = dom_element_has_attribute(ଗଣ୍ଠି, corestring_dom_nowrap, &nowrap);
	ଯଦି (err == DOM_NO_ERR && nowrap == true) {
		hint->prop = CSS_PROP_WHITE_SPACE;
		hint->status = CSS_WHITE_SPACE_NOWRAP;
		css_hint_advance(&hint);
	}
}

ସ୍ଥିର ଶୂନ୍ୟ css_hint_list(
		nscss_select_ctx *ctx,
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_exception err;
	dom_string *attr;

	err = dom_element_get_attribute(ଗଣ୍ଠି, corestring_dom_type, &attr);
	ଯଦି (err == DOM_NO_ERR && attr != ଖାଲି) {
		ଅଚଳ ଅକ୍ଷର *attr_str = dom_string_data(attr);
		size_t attr_len = dom_string_byte_length(attr);
		ଗଣନା css_list_style_type_e type = CSS_LIST_STYLE_TYPE_INHERIT;

		ଯଦି (attr_len == 1) {
			ବାଛ (attr_str[0]) {
			ସ୍ଥିତି 'a':
				type = CSS_LIST_STYLE_TYPE_LOWER_ALPHA;
				ଭାଙ୍ଗ;
			ସ୍ଥିତି 'A':
				type = CSS_LIST_STYLE_TYPE_UPPER_ALPHA;
				ଭାଙ୍ଗ;
			ସ୍ଥିତି 'i':
				type = CSS_LIST_STYLE_TYPE_LOWER_ROMAN;
				ଭାଙ୍ଗ;
			ସ୍ଥିତି 'I':
				type = CSS_LIST_STYLE_TYPE_UPPER_ROMAN;
				ଭାଙ୍ଗ;
			ସ୍ଥିତି '1':
				type = CSS_LIST_STYLE_TYPE_DECIMAL;
				ଭାଙ୍ଗ;
			}
		}

		ଯଦି (type != CSS_LIST_STYLE_TYPE_INHERIT) {
			hint->prop = CSS_PROP_LIST_STYLE_TYPE;
			hint->status = type;
			css_hint_advance(&hint);
		}

		dom_string_unref(attr);
	}
}


/* Exported function, documented in css/hints.h */
css_error ଗଣ୍ଠି_presentational_hint(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		uint32_t *nhints, css_hint **hints)
{
	dom_exception exc;
	dom_html_element_type tag_type;

	ଯଦି (nsoption_bool(author_level_css) == false) {
		*nhints = 0;
		*hints = ଖାଲି;
		ଫେରନ୍ତୁ CSS_OK;
	}

	css_hint_clean();

	exc = dom_html_element_get_tag_type(ଗଣ୍ଠି, &tag_type);
	ଯଦି (exc != DOM_NO_ERR) {
		tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	ବାଛ (tag_type) {
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_TH:
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_TD:
		css_hint_width(pw, ଗଣ୍ଠି);
		css_hint_table_cell_border_padding(pw, ଗଣ୍ଠି);
		css_hint_white_space_nowrap(pw, ଗଣ୍ଠି);
		fallthrough;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_TR:
		css_hint_height(pw, ଗଣ୍ଠି);
		fallthrough;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_THEAD:
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_TBODY:
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_TFOOT:
		css_hint_text_align_special(pw, ଗଣ୍ଠି);
		fallthrough;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_COL:
		css_hint_vertical_align_table_cells(pw, ଗଣ୍ଠି);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_APPLET:
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_IMG:
		css_hint_margin_hspace_vspace(pw, ଗଣ୍ଠି);
		fallthrough;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_EMBED:
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_IFRAME:
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_OBJECT:
		css_hint_height(pw, ଗଣ୍ଠି);
		css_hint_width(pw, ଗଣ୍ଠି);
		css_hint_vertical_align_replaced(pw, ଗଣ୍ଠି);
		css_hint_float(pw, ଗଣ୍ଠି);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_P:
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_H1:
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_H2:
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_H3:
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_H4:
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_H5:
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_H6:
		css_hint_text_align_normal(pw, ଗଣ୍ଠି);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_CENTER:
		css_hint_text_align_center(pw, ଗଣ୍ଠି);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_CAPTION:
		css_hint_caption_side(pw, ଗଣ୍ଠି);
		fallthrough;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_DIV:
		css_hint_text_align_special(pw, ଗଣ୍ଠି);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_TABLE:
		css_hint_text_align_table_special(pw, ଗଣ୍ଠି);
		css_hint_table_spacing_border(pw, ଗଣ୍ଠି);
		css_hint_float(pw, ଗଣ୍ଠି);
		css_hint_margin_ବାମ_ଡାହାଣ_align_center(pw, ଗଣ୍ଠି);
		css_hint_width(pw, ଗଣ୍ଠି);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_HR:
		css_hint_width(pw, ଗଣ୍ଠି);
		css_hint_margin_ବାମ_ଡାହାଣ_hr(pw, ଗଣ୍ଠି);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		css_hint_height_width_textarea(pw, ଗଣ୍ଠି);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_ନିବେଶ:
		css_hint_width_ନିବେଶ(pw, ଗଣ୍ଠି);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_A:
		css_hint_anchor_color(pw, ଗଣ୍ଠି);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_FONT:
		css_hint_font_size(pw, ଗଣ୍ଠି);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_BODY:
		css_hint_body_color(pw, ଗଣ୍ଠି);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_CANVAS:
		css_hint_height_width_canvas(pw, ଗଣ୍ଠି);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_OL:
		css_hint_list(pw, ଗଣ୍ଠି);
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	ଯଦି (tag_type != DOM_HTML_ELEMENT_TYPE__UNKNOWN) {
		css_hint_color(pw, ଗଣ୍ଠି);
		css_hint_bg_color(pw, ଗଣ୍ଠି);
		css_hint_bg_image(pw, ଗଣ୍ଠି);
	}

#ifdef LOG_STATS
	NSLOG(netsurf, INFO, "Properties with hints: %i", hint_ctx.len);
#endif

	css_hint_get_hints(hints, nhints);

	ଫେରନ୍ତୁ CSS_OK;
}
