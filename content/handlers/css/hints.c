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

#अन्तर्भाव <string.h>
#अन्तर्भाव <strings.h>

#अन्तर्भाव "utils/nsoption.h"
#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/nsurl.h"
#अन्तर्भाव "utils/utils.h"

#अन्तर्भाव "css/hints.h"
#अन्तर्भाव "css/select.h"

#परिभाषय LOG_STATS
#अपरिभाषय LOG_STATS

/******************************************************************************
 * Utility functions                                                          *
 ******************************************************************************/

/**
 * Determine if a given character is whitespace
 *
 * \param c  Character to consider
 * \return true if character is whitespace, false otherwise
 */
स्थिर bool isWhitespace(वर्ण c)
{
	प्रत्यागम c == ' ' || c == '\t' || c == '\f' || c == '\r' || c == '\n';
}

/**
 * Determine if a given character is a valid hex digit
 *
 * \param c  Character to consider
 * \return true if character is a valid hex digit, false otherwise
 */
स्थिर bool isHex(वर्ण c)
{
	प्रत्यागम ('0' <= c && c <= '9') ||
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
स्थिर uint8_t char_गन्तव्यम्_Hex(वर्ण c)
{
	/* 0-9 */
	c -= '0';

	/* A-F */
	यदि (c > 9)
		c -= 'A' - '9' - 1;

	/* a-f */
	यदि (c > 15)
		c -= 'a' - 'A';

	प्रत्यागम c;
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
स्थिर bool parse_सङ्ख्या(ध्रुव वर्ण *दत्त, bool maybe_negative, bool real,
		css_fixed *मानम्, size_t *consumed)
{
	size_t len;
	ध्रुव uint8_t *ptr;
	int32_t intpart = 0;
	int32_t fracpart = 0;
	int32_t pwr = 1;
	पूर्णाङ्क sign = 1;

	*consumed = 0;

	len = वाक्यदैर्घ्यम्(दत्त);
	ptr = (ध्रुव uint8_t *) दत्त;

	यदि (len == 0)
		प्रत्यागम false;

	/* Skip leading whitespace */
	यावत् (len > 0 && isWhitespace(ptr[0])) {
		len--;
		ptr++;
	}

	यदि (len == 0)
		प्रत्यागम false;

	/* Extract sign, if any */
	यदि (ptr[0] == '+') {
		len--;
		ptr++;
	} अन्यथा यदि (ptr[0] == '-' && maybe_negative) {
		sign = -1;
		len--;
		ptr++;
	}

	यदि (len == 0)
		प्रत्यागम false;

	/* Must have a digit [0,9] */
	यदि ('0' > ptr[0] || ptr[0] > '9')
		प्रत्यागम false;

	/* Now extract intpart, assuming base 10 */
	यावत् (len > 0) {
		/* Stop on first non-digit */
		यदि (ptr[0] < '0' || '9' < ptr[0])
			भङ्ग;

		/* Prevent overflow of 'intpart'; proper clamping below */
		यदि (intpart < (1 << 22)) {
			intpart *= 10;
			intpart += ptr[0] - '0';
		}
		ptr++;
		len--;
	}

	/* And fracpart, again, assuming base 10 */
	यदि (real && len > 1 && ptr[0] == '.' &&
			('0' <= ptr[1] && ptr[1] <= '9')) {
		ptr++;
		len--;

		यावत् (len > 0) {
			यदि (ptr[0] < '0' || '9' < ptr[0])
				भङ्ग;

			यदि (pwr < 1000000) {
				pwr *= 10;
				fracpart *= 10;
				fracpart += ptr[0] - '0';
			}
			ptr++;
			len--;
		}

		fracpart = ((1 << 10) * fracpart + pwr/2) / pwr;
		यदि (fracpart >= (1 << 10)) {
			intpart++;
			fracpart &= (1 << 10) - 1;
		}
	}

	यदि (sign > 0) {
		/* If the result is larger than we can represent,
		 * then clamp to the maximum value we can store. */
		यदि (intpart >= (1 << 21)) {
			intpart = (1 << 21) - 1;
			fracpart = (1 << 10) - 1;
		}
	} अन्यथा {
		/* If the negated result is smaller than we can represent
		 * then clamp to the minimum value we can store. */
		यदि (intpart >= (1 << 21)) {
			intpart = -(1 << 21);
			fracpart = 0;
		} अन्यथा {
			intpart = -intpart;
			यदि (fracpart) {
				fracpart = (1 << 10) - fracpart;
				intpart--;
			}
		}
	}

	*मानम् = (intpart << 10) | fracpart;

	*consumed = ptr - (ध्रुव uint8_t *) दत्त;

	प्रत्यागम true;
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
स्थिर bool parse_dimension(ध्रुव वर्ण *दत्त, bool strict, css_fixed *दीर्घता,
		css_unit *unit)
{
	size_t len;
	size_t पठ;
	css_fixed मानम्;

	len = वाक्यदैर्घ्यम्(दत्त);

	यदि (parse_सङ्ख्या(दत्त, false, true, &मानम्, &पठ) == false)
		प्रत्यागम false;

	यदि (strict && मानम् < INTTOFIX(1))
		प्रत्यागम false;

	*दीर्घता = मानम्;

	यदि (len > पठ && दत्त[पठ] == '%')
		*unit = CSS_UNIT_PCT;
	अन्यथा
		*unit = CSS_UNIT_PX;

	प्रत्यागम true;
}

/**
 * Mapping of colour name to CSS color
 */
संरचना रङ्गः_map {
	ध्रुव वर्ण *name;
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
स्थिर पूर्णाङ्क cmp_रङ्गः_name(ध्रुव शून्य *a, ध्रुव शून्य *b)
{
	ध्रुव वर्ण *aa = a;
	ध्रुव संरचना रङ्गः_map *bb = b;

	प्रत्यागम strcasecmp(aa, bb->name);
}

/**
 * Parse a named colour
 *
 * \param name    Name to parse
 * \param result  Pointer to location to receive css_color
 * \return true on success, false on invalid input
 */
स्थिर bool parse_named_रङ्गः(ध्रुव वर्ण *name, css_color *परिणाम)
{
	स्थिर ध्रुव संरचना रङ्गः_map named_colours[] = {
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
	ध्रुव संरचना रङ्गः_map *प्रविष्टिः;

	प्रविष्टिः = bsearch(name, named_colours,
			परिमाण(named_colours) / परिमाण(named_colours[0]),
			परिमाण(named_colours[0]),
			cmp_रङ्गः_name);

	यदि (प्रविष्टिः != रिक्त)
		*परिणाम = प्रविष्टिः->color;

	प्रत्यागम प्रविष्टिः != रिक्त;
}

/* exported interface documented in content/handlers/css/hints.h */
bool nscss_parse_रङ्गः(ध्रुव वर्ण *दत्त, css_color *परिणाम)
{
	size_t len = वाक्यदैर्घ्यम्(दत्त);
	uint8_t r, g, b;

	/* 2 */
	यदि (len == 0)
		प्रत्यागम false;

	/* 3 */
	यदि (len == SLEN("transparent") && strcasecmp(दत्त, "transparent") == 0)
		प्रत्यागम false;

	/* 4 */
	यदि (parse_named_रङ्गः(दत्त, परिणाम))
		प्रत्यागम true;

	/** \todo Implement HTML5's utterly insane legacy colour parsing */

	यदि (दत्त[0] == '#') {
		दत्त++;
		len--;
	}

	यदि (len == 3 && isHex(दत्त[0]) && isHex(दत्त[1]) && isHex(दत्त[2])) {
		r = char_गन्तव्यम्_Hex(दत्त[0]);
		g = char_गन्तव्यम्_Hex(दत्त[1]);
		b = char_गन्तव्यम्_Hex(दत्त[2]);

		r |= (r << 4);
		g |= (g << 4);
		b |= (b << 4);

		*परिणाम = (0xff << 24) | (r << 16) | (g << 8) | b;

		प्रत्यागम true;
	} अन्यथा यदि (len == 6 && isHex(दत्त[0]) && isHex(दत्त[1]) &&
			isHex(दत्त[2]) && isHex(दत्त[3]) && isHex(दत्त[4]) &&
			isHex(दत्त[5])) {
		r = (char_गन्तव्यम्_Hex(दत्त[0]) << 4) | char_गन्तव्यम्_Hex(दत्त[1]);
		g = (char_गन्तव्यम्_Hex(दत्त[2]) << 4) | char_गन्तव्यम्_Hex(दत्त[3]);
		b = (char_गन्तव्यम्_Hex(दत्त[4]) << 4) | char_गन्तव्यम्_Hex(दत्त[5]);

		*परिणाम = (0xff << 24) | (r << 16) | (g << 8) | b;

		प्रत्यागम true;
	}

	प्रत्यागम false;
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
स्थिर bool parse_font_आकार(ध्रुव वर्ण *आकार, uint8_t *val,
		css_fixed *len, css_unit *unit)
{
	स्थिर ध्रुव uint8_t आकार_map[] = {
		CSS_FONT_आकार_XX_SMALL,
		CSS_FONT_आकार_SMALL,
		CSS_FONT_आकार_MEDIUM,
		CSS_FONT_आकार_LARGE,
		CSS_FONT_आकार_X_LARGE,
		CSS_FONT_आकार_XX_LARGE,
		CSS_FONT_आकार_DIMENSION	/* xxx-large (see below) */
	};

	ध्रुव वर्ण *p = आकार;
	वर्ण बहुलकम्;
	पूर्णाङ्क मानम् = 0;

	/* Skip whitespace */
	यावत् (*p != '\0' && isWhitespace(*p))
		p++;

	बहुलकम् = *p;

	/* Skip +/- */
	यदि (बहुलकम् == '+' || बहुलकम् == '-')
		p++;

	/* Need at least one digit */
	यदि (*p < '0' || *p > '9') {
		प्रत्यागम false;
	}

	/* Consume digits, computing value */
	यावत् ('0' <= *p && *p <= '9') {
		मानम् = मानम् * 10 + (*p - '0');
		p++;
	}

	/* Resolve relative sizes */
	यदि (बहुलकम् == '+')
		मानम् += 3;
	अन्यथा यदि (बहुलकम् == '-')
		मानम् = 3 - मानम्;

	/* Clamp to range [1,7] */
	यदि (मानम् < 1)
		मानम् = 1;
	अन्यथा यदि (मानम् > 7)
		मानम् = 7;

	यदि (मानम् == 7) {
		/* Manufacture xxx-large */
	  *len = FDIV(FMUL(INTTOFIX(3), INTTOFIX(nsoption_int(font_आकार))),
				F_10);
	} अन्यथा {
		/* Len is irrelevant */
		*len = 0;
	}

	*unit = CSS_UNIT_PT;
	*val = आकार_map[मानम् - 1];

	प्रत्यागम true;
}


/******************************************************************************
 * Hint context management                                                    *
 ******************************************************************************/

#परिभाषय MAX_HINTS_प्रत्येकम्_ELEMENT 32

संरचना css_hint_ctx {
	संरचना css_hint *hints;
	uint32_t len;
};

संरचना css_hint_ctx hint_ctx;

nserror css_hint_init(शून्य)
{
	hint_ctx.hints = स्मृतिं_देहि(परिमाण(संरचना css_hint) *
			MAX_HINTS_प्रत्येकम्_ELEMENT);
	यदि (hint_ctx.hints == रिक्त) {
		प्रत्यागम NSERROR_NOMEM;
	}

	प्रत्यागम NSERROR_OK;
}

शून्य css_hint_fini(शून्य)
{
	hint_ctx.len = 0;
	मुक्त(hint_ctx.hints);
}

स्थिर शून्य css_hint_clean(शून्य)
{
	hint_ctx.len = 0;
}

स्थिर अन्तःस्थ शून्य css_hint_advance(संरचना css_hint **hint)
{
	hint_ctx.len++;
	प्रतिज्ञा(hint_ctx.len < MAX_HINTS_प्रत्येकम्_ELEMENT);

	(*hint)++;
}

स्थिर शून्य css_hint_get_hints(संरचना css_hint **hints, uint32_t *nhints)
{
	*hints = hint_ctx.hints;
	*nhints = hint_ctx.len;
}


/******************************************************************************
 * Presentational hint handlers                                               *
 ******************************************************************************/

स्थिर शून्य css_hint_सारणी_cell_border_padding(
		nscss_select_ctx *ctx,
		dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	css_qname qs;
	dom_string *attr = रिक्त;
	dom_ग्रन्थिः *tablenode = रिक्त;
	dom_exception exc;

	qs.ns = रिक्त;
	qs.name = lwc_string_ref(corestring_lwc_सारणी);
	यदि (named_ancestor_ग्रन्थिः(ctx, ग्रन्थिः, &qs,
			(शून्य *)&tablenode) != CSS_OK) {
		/* Didn't find, or had error */
		lwc_string_unref(qs.name);
		प्रत्यागम;
	}
	lwc_string_unref(qs.name);

	यदि (tablenode == रिक्त) {
		प्रत्यागम;
	}
	/* No need to unref tablenode, named_ancestor_node does not
	 * return a reffed node to the CSS
	 */

	exc = dom_element_get_attribute(tablenode,
			corestring_dom_border, &attr);

	यदि (exc == DOM_NO_ERR && attr != रिक्त) {
		uint32_t hint_prop;
		css_hint_दीर्घता hint_दीर्घता;

		यदि (parse_dimension(
				dom_string_दत्त(attr), false,
						&hint_दीर्घता.मानम्,
						&hint_दीर्घता.unit) &&
				INTTOFIX(0) != hint_दीर्घता.मानम्) {

			प्रति (hint_prop = CSS_PROP_BORDER_शिखरम्_STYLE;
			     hint_prop <= CSS_PROP_BORDER_वामः_STYLE;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->स्थितिः = CSS_BORDER_STYLE_INSET;
				css_hint_advance(&hint);
			}

			प्रति (hint_prop = CSS_PROP_BORDER_शिखरम्_विस्तारः;
			     hint_prop <= CSS_PROP_BORDER_वामः_विस्तारः;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->दत्त.दीर्घता.मानम् = INTTOFIX(1);
				hint->दत्त.दीर्घता.unit = CSS_UNIT_PX;
				hint->स्थितिः = CSS_BORDER_विस्तारः_विस्तारः;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(tablenode,
			corestring_dom_bordercolor, &attr);

	यदि (exc == DOM_NO_ERR && attr != रिक्त) {
		uint32_t hint_prop;
		css_color hint_color;

		यदि (nscss_parse_रङ्गः(
				(ध्रुव वर्ण *)dom_string_दत्त(attr),
				&hint_color)) {

			प्रति (hint_prop = CSS_PROP_BORDER_शिखरम्_COLOR;
			     hint_prop <= CSS_PROP_BORDER_वामः_COLOR;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->दत्त.color = hint_color;
				hint->स्थितिः = CSS_BORDER_COLOR_COLOR;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(tablenode,
			corestring_dom_cellpadding, &attr);

	यदि (exc == DOM_NO_ERR && attr != रिक्त) {
		uint32_t hint_prop;
		css_hint_दीर्घता hint_दीर्घता;

		यदि (parse_dimension(
				dom_string_दत्त(attr), false,
						&hint_दीर्घता.मानम्,
						&hint_दीर्घता.unit)) {

			प्रति (hint_prop = CSS_PROP_PADDING_शिखरम्;
			     hint_prop <= CSS_PROP_PADDING_वामः;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->दत्त.दीर्घता.मानम् = hint_दीर्घता.मानम्;
				hint->दत्त.दीर्घता.unit = hint_दीर्घता.unit;
				hint->स्थितिः = CSS_PADDING_समुच्चयः;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}
}

स्थिर शून्य css_hint_vertical_align_सारणी_cells(
		nscss_select_ctx *ctx,
		dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = रिक्त;
	dom_exception err;

	err = dom_element_get_attribute(ग्रन्थिः,
			corestring_dom_valign, &attr);

	यदि (err == DOM_NO_ERR && attr != रिक्त) {
		hint->दत्त.दीर्घता.मानम् = 0;
		hint->दत्त.दीर्घता.unit = CSS_UNIT_PX;
		यदि (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_शिखरम्)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->स्थितिः = CSS_VERTICAL_ALIGN_शिखरम्;
			css_hint_advance(&hint);

		} अन्यथा यदि (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_मध्यः)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->स्थितिः = CSS_VERTICAL_ALIGN_मध्यः;
			css_hint_advance(&hint);

		} अन्यथा यदि (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_bottom)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->स्थितिः = CSS_VERTICAL_ALIGN_BOTTOM;
			css_hint_advance(&hint);

		} अन्यथा यदि (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_baseline)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->स्थितिः = CSS_VERTICAL_ALIGN_BASELINE;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

स्थिर शून्य css_hint_vertical_align_replaced(
		nscss_select_ctx *ctx,
		dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = रिक्त;
	dom_exception err;

	err = dom_element_get_attribute(ग्रन्थिः,
			corestring_dom_valign, &attr);

	यदि (err == DOM_NO_ERR && attr != रिक्त) {
		यदि (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_शिखरम्)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->स्थितिः = CSS_VERTICAL_ALIGN_शिखरम्;
			css_hint_advance(&hint);

		} अन्यथा यदि (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_bottom) ||
			   dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_baseline)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->स्थितिः = CSS_VERTICAL_ALIGN_BASELINE;
			css_hint_advance(&hint);

		} अन्यथा यदि (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_texttop)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->स्थितिः = CSS_VERTICAL_ALIGN_TEXT_शिखरम्;
			css_hint_advance(&hint);

		} अन्यथा यदि (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_absmiddle) ||
			   dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_abscenter)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->स्थितिः = CSS_VERTICAL_ALIGN_मध्यः;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

स्थिर शून्य css_hint_text_align_प्रसामान्यम्(
		nscss_select_ctx *ctx,
		dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *align = रिक्त;
	dom_exception err;

	err = dom_element_get_attribute(ग्रन्थिः,
			corestring_dom_align, &align);
	यदि (err == DOM_NO_ERR && align != रिक्त) {
		यदि (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_वामः)) {
			hint->prop = CSS_PROP_TEXT_ALIGN;
			hint->स्थितिः = CSS_TEXT_ALIGN_वामः;
			css_hint_advance(&hint);

		} अन्यथा यदि (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_center)) {
			hint->prop = CSS_PROP_TEXT_ALIGN;
			hint->स्थितिः = CSS_TEXT_ALIGN_CENTER;
			css_hint_advance(&hint);

		} अन्यथा यदि (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_दक्षिणः)) {
			hint->prop = CSS_PROP_TEXT_ALIGN;
			hint->स्थितिः = CSS_TEXT_ALIGN_दक्षिणः;
			css_hint_advance(&hint);

		} अन्यथा यदि (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_justify)) {
			hint->prop = CSS_PROP_TEXT_ALIGN;
			hint->स्थितिः = CSS_TEXT_ALIGN_JUSTIFY;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

स्थिर शून्य css_hint_text_align_center(
		nscss_select_ctx *ctx,
		dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना css_hint *hint = &hint_ctx.hints[hint_ctx.len];

	hint->prop = CSS_PROP_TEXT_ALIGN;
	hint->स्थितिः = CSS_TEXT_ALIGN_LIBCSS_CENTER;
	css_hint_advance(&hint);
}

स्थिर शून्य css_hint_margin_वामः_दक्षिणः_align_center(
		nscss_select_ctx *ctx,
		dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr;
	dom_exception exc;

	exc = dom_element_get_attribute(ग्रन्थिः,
			corestring_dom_align, &attr);

	यदि (exc == DOM_NO_ERR && attr != रिक्त) {
		स्मृतिं_पूरय(hint, 0, परिमाण(*hint) * 2);
		यदि (dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_center) ||
				dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_abscenter) ||
				dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_मध्यः) ||
				dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_absmiddle)) {
			hint->prop = CSS_PROP_MARGIN_वामः;
			hint->स्थितिः = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_दक्षिणः;
			hint->स्थितिः = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

स्थिर शून्य css_hint_text_align_special(
		nscss_select_ctx *ctx,
		dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *align = रिक्त;
	dom_exception err;

	err = dom_element_get_attribute(ग्रन्थिः,
			corestring_dom_align, &align);

	यदि (err == DOM_NO_ERR && align != रिक्त) {
		यदि (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_center)) {
			hint->prop = CSS_PROP_TEXT_ALIGN;
			hint->स्थितिः = CSS_TEXT_ALIGN_LIBCSS_CENTER;
			css_hint_advance(&hint);

		} अन्यथा यदि (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_वामः)) {
			hint->prop = CSS_PROP_TEXT_ALIGN;
			hint->स्थितिः = CSS_TEXT_ALIGN_LIBCSS_वामः;
			css_hint_advance(&hint);

		} अन्यथा यदि (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_दक्षिणः)) {
			hint->prop = CSS_PROP_TEXT_ALIGN;
			hint->स्थितिः = CSS_TEXT_ALIGN_LIBCSS_दक्षिणः;
			css_hint_advance(&hint);

		} अन्यथा यदि (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_justify)) {
			hint->prop = CSS_PROP_TEXT_ALIGN;
			hint->स्थितिः = CSS_TEXT_ALIGN_JUSTIFY;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

स्थिर शून्य css_hint_text_align_सारणी_special(
		nscss_select_ctx *ctx,
		dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना css_hint *hint = &hint_ctx.hints[hint_ctx.len];

	hint->prop = CSS_PROP_TEXT_ALIGN;
	hint->स्थितिः = CSS_TEXT_ALIGN_INHERIT_IF_NON_MAGIC;
	css_hint_advance(&hint);
}

स्थिर शून्य css_hint_margin_hspace_vspace(
		nscss_select_ctx *ctx,
		dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = रिक्त;
	dom_exception exc;

	exc = dom_element_get_attribute(ग्रन्थिः,
			corestring_dom_vspace, &attr);

	यदि (exc == DOM_NO_ERR && attr != रिक्त) {
		css_hint_दीर्घता hint_दीर्घता;
		यदि (parse_dimension(
				dom_string_दत्त(attr), false,
				&hint_दीर्घता.मानम्,
				&hint_दीर्घता.unit)) {
			hint->prop = CSS_PROP_MARGIN_शिखरम्;
			hint->दत्त.दीर्घता.मानम् = hint_दीर्घता.मानम्;
			hint->दत्त.दीर्घता.unit = hint_दीर्घता.unit;
			hint->स्थितिः = CSS_MARGIN_समुच्चयः;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_BOTTOM;
			hint->दत्त.दीर्घता.मानम् = hint_दीर्घता.मानम्;
			hint->दत्त.दीर्घता.unit = hint_दीर्घता.unit;
			hint->स्थितिः = CSS_MARGIN_समुच्चयः;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(ग्रन्थिः,
			corestring_dom_hspace, &attr);

	यदि (exc == DOM_NO_ERR && attr != रिक्त) {
		css_hint_दीर्घता hint_दीर्घता;
		यदि (parse_dimension(
				dom_string_दत्त(attr), false,
				&hint_दीर्घता.मानम्,
				&hint_दीर्घता.unit)) {
			hint->prop = CSS_PROP_MARGIN_वामः;
			hint->दत्त.दीर्घता.मानम् = hint_दीर्घता.मानम्;
			hint->दत्त.दीर्घता.unit = hint_दीर्घता.unit;
			hint->स्थितिः = CSS_MARGIN_समुच्चयः;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_दक्षिणः;
			hint->दत्त.दीर्घता.मानम् = hint_दीर्घता.मानम्;
			hint->दत्त.दीर्घता.unit = hint_दीर्घता.unit;
			hint->स्थितिः = CSS_MARGIN_समुच्चयः;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

स्थिर शून्य css_hint_margin_वामः_दक्षिणः_hr(
		nscss_select_ctx *ctx,
		dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr;
	dom_exception exc;

	exc = dom_element_get_attribute(ग्रन्थिः,
				corestring_dom_align, &attr);

	यदि (exc == DOM_NO_ERR && attr != रिक्त) {
		स्मृतिं_पूरय(hint, 0, परिमाण(*hint) * 2);
		यदि (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_वामः)) {
			hint->prop = CSS_PROP_MARGIN_वामः;
			hint->दत्त.दीर्घता.मानम् = 0;
			hint->दत्त.दीर्घता.unit = CSS_UNIT_PX;
			hint->स्थितिः = CSS_MARGIN_समुच्चयः;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_दक्षिणः;
			hint->स्थितिः = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

		} अन्यथा यदि (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_center)) {
			hint->prop = CSS_PROP_MARGIN_वामः;
			hint->स्थितिः = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_दक्षिणः;
			hint->स्थितिः = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

		} अन्यथा यदि (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_दक्षिणः)) {
			hint->prop = CSS_PROP_MARGIN_वामः;
			hint->स्थितिः = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_दक्षिणः;
			hint->दत्त.दीर्घता.मानम् = 0;
			hint->दत्त.दीर्घता.unit = CSS_UNIT_PX;
			hint->स्थितिः = CSS_MARGIN_समुच्चयः;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

स्थिर शून्य css_hint_सारणी_spacing_border(
		nscss_select_ctx *ctx,
		dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception exc;
	dom_string *attr = रिक्त;

	exc = dom_element_get_attribute(ग्रन्थिः, corestring_dom_border, &attr);

	यदि (exc == DOM_NO_ERR && attr != रिक्त) {
		uint32_t hint_prop;
		css_hint_दीर्घता hint_दीर्घता;

		प्रति (hint_prop = CSS_PROP_BORDER_शिखरम्_STYLE;
			     hint_prop <= CSS_PROP_BORDER_वामः_STYLE;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->स्थितिः = CSS_BORDER_STYLE_OUTSET;
				css_hint_advance(&hint);
		}

		यदि (parse_dimension(
				dom_string_दत्त(attr), false,
						&hint_दीर्घता.मानम्,
						&hint_दीर्घता.unit)) {

			प्रति (hint_prop = CSS_PROP_BORDER_शिखरम्_विस्तारः;
			     hint_prop <= CSS_PROP_BORDER_वामः_विस्तारः;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->दत्त.दीर्घता.मानम् = hint_दीर्घता.मानम्;
				hint->दत्त.दीर्घता.unit = hint_दीर्घता.unit;
				hint->स्थितिः = CSS_BORDER_विस्तारः_विस्तारः;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(ग्रन्थिः,
			corestring_dom_bordercolor, &attr);

	यदि (exc == DOM_NO_ERR && attr != रिक्त) {
		uint32_t hint_prop;
		css_color hint_color;

		यदि (nscss_parse_रङ्गः(
				(ध्रुव वर्ण *)dom_string_दत्त(attr),
				&hint_color)) {

			प्रति (hint_prop = CSS_PROP_BORDER_शिखरम्_COLOR;
			     hint_prop <= CSS_PROP_BORDER_वामः_COLOR;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->दत्त.color = hint_color;
				hint->स्थितिः = CSS_BORDER_COLOR_COLOR;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(ग्रन्थिः,
			corestring_dom_cellspacing, &attr);

	यदि (exc == DOM_NO_ERR && attr != रिक्त) {
		यदि (parse_dimension(
				(ध्रुव वर्ण *)dom_string_दत्त(attr), false,
						&hint->दत्त.position.h.मानम्,
						&hint->दत्त.position.h.unit)) {
			hint->prop = CSS_PROP_BORDER_SPACING;
			hint->दत्त.position.v = hint->दत्त.position.h;
			hint->स्थितिः = CSS_BORDER_SPACING_समुच्चयः;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

स्थिर शून्य css_hint_height(
		nscss_select_ctx *ctx,
		dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = रिक्त;
	dom_exception err;

	err = dom_element_get_attribute(ग्रन्थिः,
			corestring_dom_height, &attr);

	यदि (err == DOM_NO_ERR && attr != रिक्त) {
		यदि (parse_dimension(
				(ध्रुव वर्ण *)dom_string_दत्त(attr), false,
				&hint->दत्त.दीर्घता.मानम्,
				&hint->दत्त.दीर्घता.unit)) {
			hint->prop = CSS_PROP_HEIGHT;
			hint->स्थितिः = CSS_HEIGHT_समुच्चयः;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

स्थिर शून्य css_hint_विस्तारः(
		nscss_select_ctx *ctx,
		dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = रिक्त;
	dom_exception err;

	err = dom_element_get_attribute(ग्रन्थिः,
			corestring_dom_विस्तारः, &attr);

	यदि (err == DOM_NO_ERR && attr != रिक्त) {
		यदि (parse_dimension(
				(ध्रुव वर्ण *)dom_string_दत्त(attr), false,
				&hint->दत्त.दीर्घता.मानम्,
				&hint->दत्त.दीर्घता.unit)) {
			hint->prop = CSS_PROP_विस्तारः;
			hint->स्थितिः = CSS_विस्तारः_समुच्चयः;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

स्थिर शून्य css_hint_height_विस्तारः_textarea(
		nscss_select_ctx *ctx,
		dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = रिक्त;
	dom_exception err;

	err = dom_element_get_attribute(ग्रन्थिः,
			corestring_dom_पङ्क्तिकाः, &attr);

	यदि (err == DOM_NO_ERR && attr != रिक्त) {
		यदि (parse_dimension(
				(ध्रुव वर्ण *)dom_string_दत्त(attr), false,
				&hint->दत्त.दीर्घता.मानम्,
				&hint->दत्त.दीर्घता.unit)) {
			hint->prop = CSS_PROP_HEIGHT;
			hint->दत्त.दीर्घता.unit = CSS_UNIT_EM;
			hint->स्थितिः = CSS_HEIGHT_समुच्चयः;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}

	err = dom_element_get_attribute(ग्रन्थिः,
			corestring_dom_स्तम्भसङ्ख्या, &attr);

	यदि (err == DOM_NO_ERR && attr != रिक्त) {
		यदि (parse_dimension(
				(ध्रुव वर्ण *)dom_string_दत्त(attr), false,
				&hint->दत्त.दीर्घता.मानम्,
				&hint->दत्त.दीर्घता.unit)) {
			hint->prop = CSS_PROP_विस्तारः;
			hint->दत्त.दीर्घता.unit = CSS_UNIT_EX;
			hint->स्थितिः = CSS_विस्तारः_समुच्चयः;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

स्थिर शून्य css_hint_height_विस्तारः_canvas(
		nscss_select_ctx *ctx,
		dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = रिक्त;
	dom_exception err;
	bool समुच्चयः_dim = false;

	err = dom_element_get_attribute(ग्रन्थिः,
			corestring_dom_height, &attr);

	यदि (err == DOM_NO_ERR && attr != रिक्त) {
		यदि (parse_dimension(
				(ध्रुव वर्ण *)dom_string_दत्त(attr), true,
				&hint->दत्त.दीर्घता.मानम्,
				&hint->दत्त.दीर्घता.unit)) {
			hint->prop = CSS_PROP_HEIGHT;
			hint->दत्त.दीर्घता.unit = CSS_UNIT_PX;
			hint->स्थितिः = CSS_HEIGHT_समुच्चयः;
			css_hint_advance(&hint);
			समुच्चयः_dim = true;
		}
		dom_string_unref(attr);
	}
	यदि (समुच्चयः_dim == false) {
		/* canvas defaults to 150px tall */
		hint->prop = CSS_PROP_HEIGHT;
		hint->दत्त.दीर्घता.unit = CSS_UNIT_PX;
		hint->दत्त.दीर्घता.मानम् = INTTOFIX(150);
		hint->स्थितिः = CSS_HEIGHT_समुच्चयः;
		css_hint_advance(&hint);
	} अन्यथा {
		समुच्चयः_dim = false;
	}

	err = dom_element_get_attribute(ग्रन्थिः,
			corestring_dom_विस्तारः, &attr);

	यदि (err == DOM_NO_ERR && attr != रिक्त) {
		यदि (parse_dimension(
				(ध्रुव वर्ण *)dom_string_दत्त(attr), true,
				&hint->दत्त.दीर्घता.मानम्,
				&hint->दत्त.दीर्घता.unit)) {
			hint->prop = CSS_PROP_विस्तारः;
			hint->दत्त.दीर्घता.unit = CSS_UNIT_PX;
			hint->स्थितिः = CSS_विस्तारः_समुच्चयः;
			css_hint_advance(&hint);
			समुच्चयः_dim = true;
		}
		dom_string_unref(attr);
	}
	यदि (समुच्चयः_dim == false) {
		/* canvas defaults to 300px wide */
		hint->prop = CSS_PROP_विस्तारः;
		hint->दत्त.दीर्घता.unit = CSS_UNIT_PX;
		hint->दत्त.दीर्घता.मानम् = INTTOFIX(300);
		hint->स्थितिः = CSS_विस्तारः_समुच्चयः;
		css_hint_advance(&hint);
	}
}

स्थिर शून्य css_hint_विस्तारः_निवेशः(
		nscss_select_ctx *ctx,
		dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_string *attr = रिक्त;
	dom_exception err;

	err = dom_element_get_attribute(ग्रन्थिः,
			corestring_dom_आकार, &attr);

	यदि (err == DOM_NO_ERR && attr != रिक्त) {
		यदि (parse_dimension(
				(ध्रुव वर्ण *)dom_string_दत्त(attr), false,
				&hint->दत्त.दीर्घता.मानम्,
				&hint->दत्त.दीर्घता.unit)) {
			dom_string *attr2 = रिक्त;

			err = dom_element_get_attribute(ग्रन्थिः,
					corestring_dom_type, &attr2);
			यदि (err == DOM_NO_ERR) {

				hint->prop = CSS_PROP_विस्तारः;
				hint->स्थितिः = CSS_विस्तारः_समुच्चयः;

				यदि (attr2 == रिक्त ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_text) ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_अन्वेषणम्) ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_password) ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_file)) {
					hint->दत्त.दीर्घता.unit = CSS_UNIT_EX;
				}
				यदि (attr2 != रिक्त) {
					dom_string_unref(attr2);
				}
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}
}

स्थिर शून्य css_hint_anchor_color(
		nscss_select_ctx *ctx,
		dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	css_error error;
	dom_exception err;
	dom_string *color;
	dom_ग्रन्थिः *bodynode = रिक्त;

	/* find body node */
	css_qname qs;
	bool is_visited;

	qs.ns = रिक्त;
	qs.name = lwc_string_ref(corestring_lwc_body);
	यदि (named_ancestor_ग्रन्थिः(ctx, ग्रन्थिः, &qs,
			(शून्य *)&bodynode) != CSS_OK) {
		/* Didn't find, or had error */
		lwc_string_unref(qs.name);
		प्रत्यागम ;
	}
	lwc_string_unref(qs.name);

	यदि (bodynode == रिक्त) {
		प्रत्यागम;
	}

	error = ग्रन्थिः_is_visited(ctx, ग्रन्थिः, &is_visited);
	यदि (error != CSS_OK)
		प्रत्यागम;

	यदि (is_visited) {
		err = dom_element_get_attribute(bodynode,
				corestring_dom_vlink, &color);
	} अन्यथा {
		err = dom_element_get_attribute(bodynode,
				corestring_dom_link, &color);
	}

	यदि (err == DOM_NO_ERR && color != रिक्त) {
		यदि (nscss_parse_रङ्गः(
				(ध्रुव वर्ण *)dom_string_दत्त(color),
						&hint->दत्त.color)) {
			hint->prop = CSS_PROP_COLOR;
			hint->स्थितिः = CSS_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(color);
	}
}

स्थिर शून्य css_hint_body_color(
		nscss_select_ctx *ctx,
		dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *color;

	err = dom_element_get_attribute(ग्रन्थिः, corestring_dom_text, &color);

	यदि (err == DOM_NO_ERR && color != रिक्त) {
		यदि (nscss_parse_रङ्गः(
				(ध्रुव वर्ण *)dom_string_दत्त(color),
						&hint->दत्त.color)) {
			hint->prop = CSS_PROP_COLOR;
			hint->स्थितिः = CSS_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(color);
	}
}

स्थिर शून्य css_hint_color(
		nscss_select_ctx *ctx,
		dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *color;

	err = dom_element_get_attribute(ग्रन्थिः, corestring_dom_color, &color);

	यदि (err == DOM_NO_ERR && color != रिक्त) {
		यदि (nscss_parse_रङ्गः(
				(ध्रुव वर्ण *)dom_string_दत्त(color),
						&hint->दत्त.color)) {
			hint->prop = CSS_PROP_COLOR;
			hint->स्थितिः = CSS_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(color);
	}
}

स्थिर शून्य css_hint_font_आकार(
		nscss_select_ctx *ctx,
		dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *आकार;

	err = dom_element_get_attribute(ग्रन्थिः, corestring_dom_आकार, &आकार);
	यदि (err == DOM_NO_ERR && आकार != रिक्त) {
		यदि (parse_font_आकार(
				(ध्रुव वर्ण *)dom_string_दत्त(आकार),
						&hint->स्थितिः,
						&hint->दत्त.दीर्घता.मानम्,
						&hint->दत्त.दीर्घता.unit)) {
			hint->prop = CSS_PROP_FONT_आकार;
			css_hint_advance(&hint);
		}
		dom_string_unref(आकार);
	}
}

स्थिर शून्य css_hint_float(
		nscss_select_ctx *ctx,
		dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *align;

	err = dom_element_get_attribute(ग्रन्थिः, corestring_dom_align, &align);
	यदि (err == DOM_NO_ERR && align != रिक्त) {
		यदि (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_वामः)) {
			hint->prop = CSS_PROP_FLOAT;
			hint->स्थितिः = CSS_FLOAT_वामः;
			css_hint_advance(&hint);

		} अन्यथा यदि (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_दक्षिणः)) {
			hint->prop = CSS_PROP_FLOAT;
			hint->स्थितिः = CSS_FLOAT_दक्षिणः;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

स्थिर शून्य css_hint_caption_side(
		nscss_select_ctx *ctx,
		dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *align = रिक्त;

	err = dom_element_get_attribute(ग्रन्थिः, corestring_dom_align, &align);
	यदि (err == DOM_NO_ERR && align != रिक्त) {
		यदि (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_bottom)) {
			hint->prop = CSS_PROP_CAPTION_SIDE;
			hint->स्थितिः = CSS_CAPTION_SIDE_BOTTOM;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

स्थिर शून्य css_hint_bg_color(
		nscss_select_ctx *ctx,
		dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *bgcolor;

	err = dom_element_get_attribute(ग्रन्थिः,
			corestring_dom_bgcolor, &bgcolor);
	यदि (err == DOM_NO_ERR && bgcolor != रिक्त) {
		यदि (nscss_parse_रङ्गः(
				(ध्रुव वर्ण *)dom_string_दत्त(bgcolor),
				&hint->दत्त.color)) {
			hint->prop = CSS_PROP_BACKGROUND_COLOR;
			hint->स्थितिः = CSS_BACKGROUND_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(bgcolor);
	}
}

स्थिर शून्य css_hint_bg_image(
		nscss_select_ctx *ctx,
		dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_exception err;
	dom_string *attr;

	err = dom_element_get_attribute(ग्रन्थिः,
			corestring_dom_background, &attr);
	यदि (err == DOM_NO_ERR && attr != रिक्त) {
		nsurl *url;
		nserror error = nsurl_join(ctx->आधारः_url,
				(ध्रुव वर्ण *)dom_string_दत्त(attr), &url);
		dom_string_unref(attr);

		यदि (error == NSERROR_OK) {
			lwc_string *iurl;
			lwc_error lerror = lwc_intern_string(nsurl_access(url),
					nsurl_दीर्घता(url), &iurl);
			nsurl_unref(url);

			यदि (lerror == lwc_error_ok) {
				hint->prop = CSS_PROP_BACKGROUND_IMAGE;
				hint->दत्त.string = iurl;
				hint->स्थितिः = CSS_BACKGROUND_IMAGE_IMAGE;
				css_hint_advance(&hint);
			}
		}
	}
}

स्थिर शून्य css_hint_white_space_nowrap(
		nscss_select_ctx *ctx,
		dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_exception err;
	bool nowrap;

	err = dom_element_has_attribute(ग्रन्थिः, corestring_dom_nowrap, &nowrap);
	यदि (err == DOM_NO_ERR && nowrap == true) {
		hint->prop = CSS_PROP_WHITE_SPACE;
		hint->स्थितिः = CSS_WHITE_SPACE_NOWRAP;
		css_hint_advance(&hint);
	}
}

स्थिर शून्य css_hint_list(
		nscss_select_ctx *ctx,
		dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_exception err;
	dom_string *attr;

	err = dom_element_get_attribute(ग्रन्थिः, corestring_dom_type, &attr);
	यदि (err == DOM_NO_ERR && attr != रिक्त) {
		ध्रुव वर्ण *attr_str = dom_string_दत्त(attr);
		size_t attr_len = dom_string_byte_दीर्घता(attr);
		गणना css_list_style_type_e type = CSS_LIST_STYLE_TYPE_INHERIT;

		यदि (attr_len == 1) {
			विकल्प (attr_str[0]) {
			पक्ष 'a':
				type = CSS_LIST_STYLE_TYPE_अधस्तनम्_ALPHA;
				भङ्ग;
			पक्ष 'A':
				type = CSS_LIST_STYLE_TYPE_ऊर्ध्वस्थम्_ALPHA;
				भङ्ग;
			पक्ष 'i':
				type = CSS_LIST_STYLE_TYPE_अधस्तनम्_ROMAN;
				भङ्ग;
			पक्ष 'I':
				type = CSS_LIST_STYLE_TYPE_ऊर्ध्वस्थम्_ROMAN;
				भङ्ग;
			पक्ष '1':
				type = CSS_LIST_STYLE_TYPE_DECIMAL;
				भङ्ग;
			}
		}

		यदि (type != CSS_LIST_STYLE_TYPE_INHERIT) {
			hint->prop = CSS_PROP_LIST_STYLE_TYPE;
			hint->स्थितिः = type;
			css_hint_advance(&hint);
		}

		dom_string_unref(attr);
	}
}


/* Exported function, documented in css/hints.h */
css_error ग्रन्थिः_presentational_hint(शून्य *pw, शून्य *ग्रन्थिः,
		uint32_t *nhints, css_hint **hints)
{
	dom_exception exc;
	dom_html_element_type चिह्नकम्_type;

	यदि (nsoption_bool(author_level_css) == false) {
		*nhints = 0;
		*hints = रिक्त;
		प्रत्यागम CSS_OK;
	}

	css_hint_clean();

	exc = dom_html_element_get_चिह्नकम्_type(ग्रन्थिः, &चिह्नकम्_type);
	यदि (exc != DOM_NO_ERR) {
		चिह्नकम्_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	विकल्प (चिह्नकम्_type) {
	पक्ष DOM_HTML_ELEMENT_TYPE_TH:
	पक्ष DOM_HTML_ELEMENT_TYPE_TD:
		css_hint_विस्तारः(pw, ग्रन्थिः);
		css_hint_सारणी_cell_border_padding(pw, ग्रन्थिः);
		css_hint_white_space_nowrap(pw, ग्रन्थिः);
		fallthrough;
	पक्ष DOM_HTML_ELEMENT_TYPE_TR:
		css_hint_height(pw, ग्रन्थिः);
		fallthrough;
	पक्ष DOM_HTML_ELEMENT_TYPE_THEAD:
	पक्ष DOM_HTML_ELEMENT_TYPE_TBODY:
	पक्ष DOM_HTML_ELEMENT_TYPE_TFOOT:
		css_hint_text_align_special(pw, ग्रन्थिः);
		fallthrough;
	पक्ष DOM_HTML_ELEMENT_TYPE_COL:
		css_hint_vertical_align_सारणी_cells(pw, ग्रन्थिः);
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_APPLET:
	पक्ष DOM_HTML_ELEMENT_TYPE_IMG:
		css_hint_margin_hspace_vspace(pw, ग्रन्थिः);
		fallthrough;
	पक्ष DOM_HTML_ELEMENT_TYPE_EMBED:
	पक्ष DOM_HTML_ELEMENT_TYPE_IFRAME:
	पक्ष DOM_HTML_ELEMENT_TYPE_OBJECT:
		css_hint_height(pw, ग्रन्थिः);
		css_hint_विस्तारः(pw, ग्रन्थिः);
		css_hint_vertical_align_replaced(pw, ग्रन्थिः);
		css_hint_float(pw, ग्रन्थिः);
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_P:
	पक्ष DOM_HTML_ELEMENT_TYPE_H1:
	पक्ष DOM_HTML_ELEMENT_TYPE_H2:
	पक्ष DOM_HTML_ELEMENT_TYPE_H3:
	पक्ष DOM_HTML_ELEMENT_TYPE_H4:
	पक्ष DOM_HTML_ELEMENT_TYPE_H5:
	पक्ष DOM_HTML_ELEMENT_TYPE_H6:
		css_hint_text_align_प्रसामान्यम्(pw, ग्रन्थिः);
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_CENTER:
		css_hint_text_align_center(pw, ग्रन्थिः);
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_CAPTION:
		css_hint_caption_side(pw, ग्रन्थिः);
		fallthrough;
	पक्ष DOM_HTML_ELEMENT_TYPE_DIV:
		css_hint_text_align_special(pw, ग्रन्थिः);
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_सारणी:
		css_hint_text_align_सारणी_special(pw, ग्रन्थिः);
		css_hint_सारणी_spacing_border(pw, ग्रन्थिः);
		css_hint_float(pw, ग्रन्थिः);
		css_hint_margin_वामः_दक्षिणः_align_center(pw, ग्रन्थिः);
		css_hint_विस्तारः(pw, ग्रन्थिः);
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_HR:
		css_hint_विस्तारः(pw, ग्रन्थिः);
		css_hint_margin_वामः_दक्षिणः_hr(pw, ग्रन्थिः);
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		css_hint_height_विस्तारः_textarea(pw, ग्रन्थिः);
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_निवेशः:
		css_hint_विस्तारः_निवेशः(pw, ग्रन्थिः);
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_A:
		css_hint_anchor_color(pw, ग्रन्थिः);
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_FONT:
		css_hint_font_आकार(pw, ग्रन्थिः);
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_BODY:
		css_hint_body_color(pw, ग्रन्थिः);
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_CANVAS:
		css_hint_height_विस्तारः_canvas(pw, ग्रन्थिः);
		भङ्ग;
	पक्ष DOM_HTML_ELEMENT_TYPE_OL:
		css_hint_list(pw, ग्रन्थिः);
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	यदि (चिह्नकम्_type != DOM_HTML_ELEMENT_TYPE__UNKNOWN) {
		css_hint_color(pw, ग्रन्थिः);
		css_hint_bg_color(pw, ग्रन्थिः);
		css_hint_bg_image(pw, ग्रन्थिः);
	}

#यदि_परिभाषितम् LOG_STATS
	NSLOG(netsurf, INFO, "Properties with hints: %i", hint_ctx.len);
#यदि_अन्तः

	css_hint_get_hints(hints, nhints);

	प्रत्यागम CSS_OK;
}
