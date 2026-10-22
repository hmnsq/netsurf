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

#समावेश <string.h>
#समावेश <strings.h>

#समावेश "utils/nsoption.h"
#समावेश "utils/corestrings.h"
#समावेश "utils/log.h"
#समावेश "utils/nsurl.h"
#समावेश "utils/utils.h"

#समावेश "css/hints.h"
#समावेश "css/select.h"

#घोषणा LOG_STATS
#अघोषित LOG_STATS

/******************************************************************************
 * Utility functions                                                          *
 ******************************************************************************/

/**
 * Determine if a given character is whitespace
 *
 * \param c  Character to consider
 * \return true if character is whitespace, false otherwise
 */
अटल bool isWhitespace(अक्षर c)
{
	वापस c == ' ' || c == '\t' || c == '\f' || c == '\r' || c == '\n';
}

/**
 * Determine if a given character is a valid hex digit
 *
 * \param c  Character to consider
 * \return true if character is a valid hex digit, false otherwise
 */
अटल bool isHex(अक्षर c)
{
	वापस ('0' <= c && c <= '9') ||
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
अटल uint8_t charToHex(अक्षर c)
{
	/* 0-9 */
	c -= '0';

	/* A-F */
	अगर (c > 9)
		c -= 'A' - '9' - 1;

	/* a-f */
	अगर (c > 15)
		c -= 'a' - 'A';

	वापस c;
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
अटल bool parse_संख्या(स्थिर अक्षर *तथ्य, bool maybe_negative, bool real,
		css_fixed *मूल्य, माप_प्रकार *consumed)
{
	माप_प्रकार len;
	स्थिर uint8_t *ptr;
	int32_t intpart = 0;
	int32_t fracpart = 0;
	int32_t pwr = 1;
	पूर्णांक sign = 1;

	*consumed = 0;

	len = म_माप(तथ्य);
	ptr = (स्थिर uint8_t *) तथ्य;

	अगर (len == 0)
		वापस false;

	/* Skip leading whitespace */
	जबतक (len > 0 && isWhitespace(ptr[0])) {
		len--;
		ptr++;
	}

	अगर (len == 0)
		वापस false;

	/* Extract sign, if any */
	अगर (ptr[0] == '+') {
		len--;
		ptr++;
	} अन्यथा अगर (ptr[0] == '-' && maybe_negative) {
		sign = -1;
		len--;
		ptr++;
	}

	अगर (len == 0)
		वापस false;

	/* Must have a digit [0,9] */
	अगर ('0' > ptr[0] || ptr[0] > '9')
		वापस false;

	/* Now extract intpart, assuming base 10 */
	जबतक (len > 0) {
		/* Stop on first non-digit */
		अगर (ptr[0] < '0' || '9' < ptr[0])
			अवरोध;

		/* Prevent overflow of 'intpart'; proper clamping below */
		अगर (intpart < (1 << 22)) {
			intpart *= 10;
			intpart += ptr[0] - '0';
		}
		ptr++;
		len--;
	}

	/* And fracpart, again, assuming base 10 */
	अगर (real && len > 1 && ptr[0] == '.' &&
			('0' <= ptr[1] && ptr[1] <= '9')) {
		ptr++;
		len--;

		जबतक (len > 0) {
			अगर (ptr[0] < '0' || '9' < ptr[0])
				अवरोध;

			अगर (pwr < 1000000) {
				pwr *= 10;
				fracpart *= 10;
				fracpart += ptr[0] - '0';
			}
			ptr++;
			len--;
		}

		fracpart = ((1 << 10) * fracpart + pwr/2) / pwr;
		अगर (fracpart >= (1 << 10)) {
			intpart++;
			fracpart &= (1 << 10) - 1;
		}
	}

	अगर (sign > 0) {
		/* If the result is larger than we can represent,
		 * then clamp to the maximum value we can store. */
		अगर (intpart >= (1 << 21)) {
			intpart = (1 << 21) - 1;
			fracpart = (1 << 10) - 1;
		}
	} अन्यथा {
		/* If the negated result is smaller than we can represent
		 * then clamp to the minimum value we can store. */
		अगर (intpart >= (1 << 21)) {
			intpart = -(1 << 21);
			fracpart = 0;
		} अन्यथा {
			intpart = -intpart;
			अगर (fracpart) {
				fracpart = (1 << 10) - fracpart;
				intpart--;
			}
		}
	}

	*मूल्य = (intpart << 10) | fracpart;

	*consumed = ptr - (स्थिर uint8_t *) तथ्य;

	वापस true;
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
अटल bool parse_dimension(स्थिर अक्षर *तथ्य, bool strict, css_fixed *लंबाई,
		css_unit *unit)
{
	माप_प्रकार len;
	माप_प्रकार पढ़ो;
	css_fixed मूल्य;

	len = म_माप(तथ्य);

	अगर (parse_संख्या(तथ्य, false, true, &मूल्य, &पढ़ो) == false)
		वापस false;

	अगर (strict && मूल्य < INTTOFIX(1))
		वापस false;

	*लंबाई = मूल्य;

	अगर (len > पढ़ो && तथ्य[पढ़ो] == '%')
		*unit = CSS_UNIT_PCT;
	अन्यथा
		*unit = CSS_UNIT_PX;

	वापस true;
}

/**
 * Mapping of colour name to CSS color
 */
काष्ठा colour_map {
	स्थिर अक्षर *name;
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
अटल पूर्णांक cmp_colour_name(स्थिर व्योम *a, स्थिर व्योम *b)
{
	स्थिर अक्षर *aa = a;
	स्थिर काष्ठा colour_map *bb = b;

	वापस strcasecmp(aa, bb->name);
}

/**
 * Parse a named colour
 *
 * \param name    Name to parse
 * \param result  Pointer to location to receive css_color
 * \return true on success, false on invalid input
 */
अटल bool parse_named_colour(स्थिर अक्षर *name, css_color *परिणाम)
{
	अटल स्थिर काष्ठा colour_map named_colours[] = {
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
	स्थिर काष्ठा colour_map *entry;

	entry = द्वा_खोज(name, named_colours,
			माप(named_colours) / माप(named_colours[0]),
			माप(named_colours[0]),
			cmp_colour_name);

	अगर (entry != शून्य)
		*परिणाम = entry->color;

	वापस entry != शून्य;
}

/* exported interface documented in content/handlers/css/hints.h */
bool nscss_parse_colour(स्थिर अक्षर *तथ्य, css_color *परिणाम)
{
	माप_प्रकार len = म_माप(तथ्य);
	uint8_t r, g, b;

	/* 2 */
	अगर (len == 0)
		वापस false;

	/* 3 */
	अगर (len == SLEN("transparent") && strcasecmp(तथ्य, "transparent") == 0)
		वापस false;

	/* 4 */
	अगर (parse_named_colour(तथ्य, परिणाम))
		वापस true;

	/** \todo Implement HTML5's utterly insane legacy colour parsing */

	अगर (तथ्य[0] == '#') {
		तथ्य++;
		len--;
	}

	अगर (len == 3 && isHex(तथ्य[0]) && isHex(तथ्य[1]) && isHex(तथ्य[2])) {
		r = charToHex(तथ्य[0]);
		g = charToHex(तथ्य[1]);
		b = charToHex(तथ्य[2]);

		r |= (r << 4);
		g |= (g << 4);
		b |= (b << 4);

		*परिणाम = (0xff << 24) | (r << 16) | (g << 8) | b;

		वापस true;
	} अन्यथा अगर (len == 6 && isHex(तथ्य[0]) && isHex(तथ्य[1]) &&
			isHex(तथ्य[2]) && isHex(तथ्य[3]) && isHex(तथ्य[4]) &&
			isHex(तथ्य[5])) {
		r = (charToHex(तथ्य[0]) << 4) | charToHex(तथ्य[1]);
		g = (charToHex(तथ्य[2]) << 4) | charToHex(तथ्य[3]);
		b = (charToHex(तथ्य[4]) << 4) | charToHex(तथ्य[5]);

		*परिणाम = (0xff << 24) | (r << 16) | (g << 8) | b;

		वापस true;
	}

	वापस false;
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
अटल bool parse_font_आकार(स्थिर अक्षर *आकार, uint8_t *val,
		css_fixed *len, css_unit *unit)
{
	अटल स्थिर uint8_t आकार_map[] = {
		CSS_FONT_आकार_XX_SMALL,
		CSS_FONT_आकार_SMALL,
		CSS_FONT_आकार_MEDIUM,
		CSS_FONT_आकार_LARGE,
		CSS_FONT_आकार_X_LARGE,
		CSS_FONT_आकार_XX_LARGE,
		CSS_FONT_आकार_DIMENSION	/* xxx-large (see below) */
	};

	स्थिर अक्षर *p = आकार;
	अक्षर mode;
	पूर्णांक मूल्य = 0;

	/* Skip whitespace */
	जबतक (*p != '\0' && isWhitespace(*p))
		p++;

	mode = *p;

	/* Skip +/- */
	अगर (mode == '+' || mode == '-')
		p++;

	/* Need at least one digit */
	अगर (*p < '0' || *p > '9') {
		वापस false;
	}

	/* Consume digits, computing value */
	जबतक ('0' <= *p && *p <= '9') {
		मूल्य = मूल्य * 10 + (*p - '0');
		p++;
	}

	/* Resolve relative sizes */
	अगर (mode == '+')
		मूल्य += 3;
	अन्यथा अगर (mode == '-')
		मूल्य = 3 - मूल्य;

	/* Clamp to range [1,7] */
	अगर (मूल्य < 1)
		मूल्य = 1;
	अन्यथा अगर (मूल्य > 7)
		मूल्य = 7;

	अगर (मूल्य == 7) {
		/* Manufacture xxx-large */
	  *len = FDIV(FMUL(INTTOFIX(3), INTTOFIX(nsoption_int(font_आकार))),
				F_10);
	} अन्यथा {
		/* Len is irrelevant */
		*len = 0;
	}

	*unit = CSS_UNIT_PT;
	*val = आकार_map[मूल्य - 1];

	वापस true;
}


/******************************************************************************
 * Hint context management                                                    *
 ******************************************************************************/

#घोषणा MAX_HINTS_PER_ELEMENT 32

काष्ठा css_hint_ctx {
	काष्ठा css_hint *hints;
	uint32_t len;
};

काष्ठा css_hint_ctx hint_ctx;

nserror css_hint_init(व्योम)
{
	hint_ctx.hints = दो_स्मृति(माप(काष्ठा css_hint) *
			MAX_HINTS_PER_ELEMENT);
	अगर (hint_ctx.hints == शून्य) {
		वापस NSERROR_NOMEM;
	}

	वापस NSERROR_OK;
}

व्योम css_hint_fini(व्योम)
{
	hint_ctx.len = 0;
	मुक्त(hint_ctx.hints);
}

अटल व्योम css_hint_clean(व्योम)
{
	hint_ctx.len = 0;
}

अटल अंतरभूत व्योम css_hint_advance(काष्ठा css_hint **hint)
{
	hint_ctx.len++;
	निश्चित(hint_ctx.len < MAX_HINTS_PER_ELEMENT);

	(*hint)++;
}

अटल व्योम css_hint_get_hints(काष्ठा css_hint **hints, uint32_t *nhints)
{
	*hints = hint_ctx.hints;
	*nhints = hint_ctx.len;
}


/******************************************************************************
 * Presentational hint handlers                                               *
 ******************************************************************************/

अटल व्योम css_hint_table_cell_border_padding(
		nscss_select_ctx *ctx,
		dom_गाँठ *गाँठ)
{
	काष्ठा css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	css_qname qs;
	dom_string *attr = शून्य;
	dom_गाँठ *tablenode = शून्य;
	dom_exception exc;

	qs.ns = शून्य;
	qs.name = lwc_string_ref(corestring_lwc_table);
	अगर (named_ancestor_गाँठ(ctx, गाँठ, &qs,
			(व्योम *)&tablenode) != CSS_OK) {
		/* Didn't find, or had error */
		lwc_string_unref(qs.name);
		वापस;
	}
	lwc_string_unref(qs.name);

	अगर (tablenode == शून्य) {
		वापस;
	}
	/* No need to unref tablenode, named_ancestor_node does not
	 * return a reffed node to the CSS
	 */

	exc = dom_element_get_attribute(tablenode,
			corestring_dom_border, &attr);

	अगर (exc == DOM_NO_ERR && attr != शून्य) {
		uint32_t hint_prop;
		css_hint_लंबाई hint_लंबाई;

		अगर (parse_dimension(
				dom_string_तथ्य(attr), false,
						&hint_लंबाई.मूल्य,
						&hint_लंबाई.unit) &&
				INTTOFIX(0) != hint_लंबाई.मूल्य) {

			क्रम (hint_prop = CSS_PROP_BORDER_शीर्ष_STYLE;
			     hint_prop <= CSS_PROP_BORDER_बायाँ_STYLE;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->status = CSS_BORDER_STYLE_INSET;
				css_hint_advance(&hint);
			}

			क्रम (hint_prop = CSS_PROP_BORDER_शीर्ष_WIDTH;
			     hint_prop <= CSS_PROP_BORDER_बायाँ_WIDTH;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->तथ्य.लंबाई.मूल्य = INTTOFIX(1);
				hint->तथ्य.लंबाई.unit = CSS_UNIT_PX;
				hint->status = CSS_BORDER_WIDTH_WIDTH;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(tablenode,
			corestring_dom_bordercolor, &attr);

	अगर (exc == DOM_NO_ERR && attr != शून्य) {
		uint32_t hint_prop;
		css_color hint_color;

		अगर (nscss_parse_colour(
				(स्थिर अक्षर *)dom_string_तथ्य(attr),
				&hint_color)) {

			क्रम (hint_prop = CSS_PROP_BORDER_शीर्ष_COLOR;
			     hint_prop <= CSS_PROP_BORDER_बायाँ_COLOR;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->तथ्य.color = hint_color;
				hint->status = CSS_BORDER_COLOR_COLOR;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(tablenode,
			corestring_dom_cellpadding, &attr);

	अगर (exc == DOM_NO_ERR && attr != शून्य) {
		uint32_t hint_prop;
		css_hint_लंबाई hint_लंबाई;

		अगर (parse_dimension(
				dom_string_तथ्य(attr), false,
						&hint_लंबाई.मूल्य,
						&hint_लंबाई.unit)) {

			क्रम (hint_prop = CSS_PROP_PADDING_शीर्ष;
			     hint_prop <= CSS_PROP_PADDING_बायाँ;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->तथ्य.लंबाई.मूल्य = hint_लंबाई.मूल्य;
				hint->तथ्य.लंबाई.unit = hint_लंबाई.unit;
				hint->status = CSS_PADDING_SET;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}
}

अटल व्योम css_hint_vertical_align_table_cells(
		nscss_select_ctx *ctx,
		dom_गाँठ *गाँठ)
{
	काष्ठा css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = शून्य;
	dom_exception err;

	err = dom_element_get_attribute(गाँठ,
			corestring_dom_valign, &attr);

	अगर (err == DOM_NO_ERR && attr != शून्य) {
		hint->तथ्य.लंबाई.मूल्य = 0;
		hint->तथ्य.लंबाई.unit = CSS_UNIT_PX;
		अगर (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_शीर्ष)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->status = CSS_VERTICAL_ALIGN_शीर्ष;
			css_hint_advance(&hint);

		} अन्यथा अगर (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_मध्य)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->status = CSS_VERTICAL_ALIGN_मध्य;
			css_hint_advance(&hint);

		} अन्यथा अगर (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_bottom)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->status = CSS_VERTICAL_ALIGN_BOTTOM;
			css_hint_advance(&hint);

		} अन्यथा अगर (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_baseline)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->status = CSS_VERTICAL_ALIGN_BASELINE;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

अटल व्योम css_hint_vertical_align_replaced(
		nscss_select_ctx *ctx,
		dom_गाँठ *गाँठ)
{
	काष्ठा css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = शून्य;
	dom_exception err;

	err = dom_element_get_attribute(गाँठ,
			corestring_dom_valign, &attr);

	अगर (err == DOM_NO_ERR && attr != शून्य) {
		अगर (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_शीर्ष)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->status = CSS_VERTICAL_ALIGN_शीर्ष;
			css_hint_advance(&hint);

		} अन्यथा अगर (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_bottom) ||
			   dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_baseline)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->status = CSS_VERTICAL_ALIGN_BASELINE;
			css_hint_advance(&hint);

		} अन्यथा अगर (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_texttop)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->status = CSS_VERTICAL_ALIGN_TEXT_शीर्ष;
			css_hint_advance(&hint);

		} अन्यथा अगर (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_absmiddle) ||
			   dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_abscenter)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->status = CSS_VERTICAL_ALIGN_मध्य;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

अटल व्योम css_hint_text_align_normal(
		nscss_select_ctx *ctx,
		dom_गाँठ *गाँठ)
{
	काष्ठा css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *align = शून्य;
	dom_exception err;

	err = dom_element_get_attribute(गाँठ,
			corestring_dom_align, &align);
	अगर (err == DOM_NO_ERR && align != शून्य) {
		अगर (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_बायाँ)) {
			hint->prop = CSS_PROP_TEXT_ALIGN;
			hint->status = CSS_TEXT_ALIGN_बायाँ;
			css_hint_advance(&hint);

		} अन्यथा अगर (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_center)) {
			hint->prop = CSS_PROP_TEXT_ALIGN;
			hint->status = CSS_TEXT_ALIGN_CENTER;
			css_hint_advance(&hint);

		} अन्यथा अगर (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_दायाँ)) {
			hint->prop = CSS_PROP_TEXT_ALIGN;
			hint->status = CSS_TEXT_ALIGN_दायाँ;
			css_hint_advance(&hint);

		} अन्यथा अगर (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_justify)) {
			hint->prop = CSS_PROP_TEXT_ALIGN;
			hint->status = CSS_TEXT_ALIGN_JUSTIFY;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

अटल व्योम css_hint_text_align_center(
		nscss_select_ctx *ctx,
		dom_गाँठ *गाँठ)
{
	काष्ठा css_hint *hint = &hint_ctx.hints[hint_ctx.len];

	hint->prop = CSS_PROP_TEXT_ALIGN;
	hint->status = CSS_TEXT_ALIGN_LIBCSS_CENTER;
	css_hint_advance(&hint);
}

अटल व्योम css_hint_margin_बायाँ_दायाँ_align_center(
		nscss_select_ctx *ctx,
		dom_गाँठ *गाँठ)
{
	काष्ठा css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr;
	dom_exception exc;

	exc = dom_element_get_attribute(गाँठ,
			corestring_dom_align, &attr);

	अगर (exc == DOM_NO_ERR && attr != शून्य) {
		स_रखो(hint, 0, माप(*hint) * 2);
		अगर (dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_center) ||
				dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_abscenter) ||
				dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_मध्य) ||
				dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_absmiddle)) {
			hint->prop = CSS_PROP_MARGIN_बायाँ;
			hint->status = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_दायाँ;
			hint->status = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

अटल व्योम css_hint_text_align_special(
		nscss_select_ctx *ctx,
		dom_गाँठ *गाँठ)
{
	काष्ठा css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *align = शून्य;
	dom_exception err;

	err = dom_element_get_attribute(गाँठ,
			corestring_dom_align, &align);

	अगर (err == DOM_NO_ERR && align != शून्य) {
		अगर (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_center)) {
			hint->prop = CSS_PROP_TEXT_ALIGN;
			hint->status = CSS_TEXT_ALIGN_LIBCSS_CENTER;
			css_hint_advance(&hint);

		} अन्यथा अगर (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_बायाँ)) {
			hint->prop = CSS_PROP_TEXT_ALIGN;
			hint->status = CSS_TEXT_ALIGN_LIBCSS_बायाँ;
			css_hint_advance(&hint);

		} अन्यथा अगर (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_दायाँ)) {
			hint->prop = CSS_PROP_TEXT_ALIGN;
			hint->status = CSS_TEXT_ALIGN_LIBCSS_दायाँ;
			css_hint_advance(&hint);

		} अन्यथा अगर (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_justify)) {
			hint->prop = CSS_PROP_TEXT_ALIGN;
			hint->status = CSS_TEXT_ALIGN_JUSTIFY;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

अटल व्योम css_hint_text_align_table_special(
		nscss_select_ctx *ctx,
		dom_गाँठ *गाँठ)
{
	काष्ठा css_hint *hint = &hint_ctx.hints[hint_ctx.len];

	hint->prop = CSS_PROP_TEXT_ALIGN;
	hint->status = CSS_TEXT_ALIGN_INHERIT_IF_NON_MAGIC;
	css_hint_advance(&hint);
}

अटल व्योम css_hint_margin_hspace_vspace(
		nscss_select_ctx *ctx,
		dom_गाँठ *गाँठ)
{
	काष्ठा css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = शून्य;
	dom_exception exc;

	exc = dom_element_get_attribute(गाँठ,
			corestring_dom_vspace, &attr);

	अगर (exc == DOM_NO_ERR && attr != शून्य) {
		css_hint_लंबाई hint_लंबाई;
		अगर (parse_dimension(
				dom_string_तथ्य(attr), false,
				&hint_लंबाई.मूल्य,
				&hint_लंबाई.unit)) {
			hint->prop = CSS_PROP_MARGIN_शीर्ष;
			hint->तथ्य.लंबाई.मूल्य = hint_लंबाई.मूल्य;
			hint->तथ्य.लंबाई.unit = hint_लंबाई.unit;
			hint->status = CSS_MARGIN_SET;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_BOTTOM;
			hint->तथ्य.लंबाई.मूल्य = hint_लंबाई.मूल्य;
			hint->तथ्य.लंबाई.unit = hint_लंबाई.unit;
			hint->status = CSS_MARGIN_SET;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(गाँठ,
			corestring_dom_hspace, &attr);

	अगर (exc == DOM_NO_ERR && attr != शून्य) {
		css_hint_लंबाई hint_लंबाई;
		अगर (parse_dimension(
				dom_string_तथ्य(attr), false,
				&hint_लंबाई.मूल्य,
				&hint_लंबाई.unit)) {
			hint->prop = CSS_PROP_MARGIN_बायाँ;
			hint->तथ्य.लंबाई.मूल्य = hint_लंबाई.मूल्य;
			hint->तथ्य.लंबाई.unit = hint_लंबाई.unit;
			hint->status = CSS_MARGIN_SET;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_दायाँ;
			hint->तथ्य.लंबाई.मूल्य = hint_लंबाई.मूल्य;
			hint->तथ्य.लंबाई.unit = hint_लंबाई.unit;
			hint->status = CSS_MARGIN_SET;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

अटल व्योम css_hint_margin_बायाँ_दायाँ_hr(
		nscss_select_ctx *ctx,
		dom_गाँठ *गाँठ)
{
	काष्ठा css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr;
	dom_exception exc;

	exc = dom_element_get_attribute(गाँठ,
				corestring_dom_align, &attr);

	अगर (exc == DOM_NO_ERR && attr != शून्य) {
		स_रखो(hint, 0, माप(*hint) * 2);
		अगर (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_बायाँ)) {
			hint->prop = CSS_PROP_MARGIN_बायाँ;
			hint->तथ्य.लंबाई.मूल्य = 0;
			hint->तथ्य.लंबाई.unit = CSS_UNIT_PX;
			hint->status = CSS_MARGIN_SET;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_दायाँ;
			hint->status = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

		} अन्यथा अगर (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_center)) {
			hint->prop = CSS_PROP_MARGIN_बायाँ;
			hint->status = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_दायाँ;
			hint->status = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

		} अन्यथा अगर (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_दायाँ)) {
			hint->prop = CSS_PROP_MARGIN_बायाँ;
			hint->status = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_दायाँ;
			hint->तथ्य.लंबाई.मूल्य = 0;
			hint->तथ्य.लंबाई.unit = CSS_UNIT_PX;
			hint->status = CSS_MARGIN_SET;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

अटल व्योम css_hint_table_spacing_border(
		nscss_select_ctx *ctx,
		dom_गाँठ *गाँठ)
{
	काष्ठा css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception exc;
	dom_string *attr = शून्य;

	exc = dom_element_get_attribute(गाँठ, corestring_dom_border, &attr);

	अगर (exc == DOM_NO_ERR && attr != शून्य) {
		uint32_t hint_prop;
		css_hint_लंबाई hint_लंबाई;

		क्रम (hint_prop = CSS_PROP_BORDER_शीर्ष_STYLE;
			     hint_prop <= CSS_PROP_BORDER_बायाँ_STYLE;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->status = CSS_BORDER_STYLE_OUTSET;
				css_hint_advance(&hint);
		}

		अगर (parse_dimension(
				dom_string_तथ्य(attr), false,
						&hint_लंबाई.मूल्य,
						&hint_लंबाई.unit)) {

			क्रम (hint_prop = CSS_PROP_BORDER_शीर्ष_WIDTH;
			     hint_prop <= CSS_PROP_BORDER_बायाँ_WIDTH;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->तथ्य.लंबाई.मूल्य = hint_लंबाई.मूल्य;
				hint->तथ्य.लंबाई.unit = hint_लंबाई.unit;
				hint->status = CSS_BORDER_WIDTH_WIDTH;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(गाँठ,
			corestring_dom_bordercolor, &attr);

	अगर (exc == DOM_NO_ERR && attr != शून्य) {
		uint32_t hint_prop;
		css_color hint_color;

		अगर (nscss_parse_colour(
				(स्थिर अक्षर *)dom_string_तथ्य(attr),
				&hint_color)) {

			क्रम (hint_prop = CSS_PROP_BORDER_शीर्ष_COLOR;
			     hint_prop <= CSS_PROP_BORDER_बायाँ_COLOR;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->तथ्य.color = hint_color;
				hint->status = CSS_BORDER_COLOR_COLOR;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(गाँठ,
			corestring_dom_cellspacing, &attr);

	अगर (exc == DOM_NO_ERR && attr != शून्य) {
		अगर (parse_dimension(
				(स्थिर अक्षर *)dom_string_तथ्य(attr), false,
						&hint->तथ्य.position.h.मूल्य,
						&hint->तथ्य.position.h.unit)) {
			hint->prop = CSS_PROP_BORDER_SPACING;
			hint->तथ्य.position.v = hint->तथ्य.position.h;
			hint->status = CSS_BORDER_SPACING_SET;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

अटल व्योम css_hint_height(
		nscss_select_ctx *ctx,
		dom_गाँठ *गाँठ)
{
	काष्ठा css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = शून्य;
	dom_exception err;

	err = dom_element_get_attribute(गाँठ,
			corestring_dom_height, &attr);

	अगर (err == DOM_NO_ERR && attr != शून्य) {
		अगर (parse_dimension(
				(स्थिर अक्षर *)dom_string_तथ्य(attr), false,
				&hint->तथ्य.लंबाई.मूल्य,
				&hint->तथ्य.लंबाई.unit)) {
			hint->prop = CSS_PROP_HEIGHT;
			hint->status = CSS_HEIGHT_SET;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

अटल व्योम css_hint_width(
		nscss_select_ctx *ctx,
		dom_गाँठ *गाँठ)
{
	काष्ठा css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = शून्य;
	dom_exception err;

	err = dom_element_get_attribute(गाँठ,
			corestring_dom_width, &attr);

	अगर (err == DOM_NO_ERR && attr != शून्य) {
		अगर (parse_dimension(
				(स्थिर अक्षर *)dom_string_तथ्य(attr), false,
				&hint->तथ्य.लंबाई.मूल्य,
				&hint->तथ्य.लंबाई.unit)) {
			hint->prop = CSS_PROP_WIDTH;
			hint->status = CSS_WIDTH_SET;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

अटल व्योम css_hint_height_width_textarea(
		nscss_select_ctx *ctx,
		dom_गाँठ *गाँठ)
{
	काष्ठा css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = शून्य;
	dom_exception err;

	err = dom_element_get_attribute(गाँठ,
			corestring_dom_rows, &attr);

	अगर (err == DOM_NO_ERR && attr != शून्य) {
		अगर (parse_dimension(
				(स्थिर अक्षर *)dom_string_तथ्य(attr), false,
				&hint->तथ्य.लंबाई.मूल्य,
				&hint->तथ्य.लंबाई.unit)) {
			hint->prop = CSS_PROP_HEIGHT;
			hint->तथ्य.लंबाई.unit = CSS_UNIT_EM;
			hint->status = CSS_HEIGHT_SET;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}

	err = dom_element_get_attribute(गाँठ,
			corestring_dom_cols, &attr);

	अगर (err == DOM_NO_ERR && attr != शून्य) {
		अगर (parse_dimension(
				(स्थिर अक्षर *)dom_string_तथ्य(attr), false,
				&hint->तथ्य.लंबाई.मूल्य,
				&hint->तथ्य.लंबाई.unit)) {
			hint->prop = CSS_PROP_WIDTH;
			hint->तथ्य.लंबाई.unit = CSS_UNIT_EX;
			hint->status = CSS_WIDTH_SET;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

अटल व्योम css_hint_height_width_canvas(
		nscss_select_ctx *ctx,
		dom_गाँठ *गाँठ)
{
	काष्ठा css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = शून्य;
	dom_exception err;
	bool set_dim = false;

	err = dom_element_get_attribute(गाँठ,
			corestring_dom_height, &attr);

	अगर (err == DOM_NO_ERR && attr != शून्य) {
		अगर (parse_dimension(
				(स्थिर अक्षर *)dom_string_तथ्य(attr), true,
				&hint->तथ्य.लंबाई.मूल्य,
				&hint->तथ्य.लंबाई.unit)) {
			hint->prop = CSS_PROP_HEIGHT;
			hint->तथ्य.लंबाई.unit = CSS_UNIT_PX;
			hint->status = CSS_HEIGHT_SET;
			css_hint_advance(&hint);
			set_dim = true;
		}
		dom_string_unref(attr);
	}
	अगर (set_dim == false) {
		/* canvas defaults to 150px tall */
		hint->prop = CSS_PROP_HEIGHT;
		hint->तथ्य.लंबाई.unit = CSS_UNIT_PX;
		hint->तथ्य.लंबाई.मूल्य = INTTOFIX(150);
		hint->status = CSS_HEIGHT_SET;
		css_hint_advance(&hint);
	} अन्यथा {
		set_dim = false;
	}

	err = dom_element_get_attribute(गाँठ,
			corestring_dom_width, &attr);

	अगर (err == DOM_NO_ERR && attr != शून्य) {
		अगर (parse_dimension(
				(स्थिर अक्षर *)dom_string_तथ्य(attr), true,
				&hint->तथ्य.लंबाई.मूल्य,
				&hint->तथ्य.लंबाई.unit)) {
			hint->prop = CSS_PROP_WIDTH;
			hint->तथ्य.लंबाई.unit = CSS_UNIT_PX;
			hint->status = CSS_WIDTH_SET;
			css_hint_advance(&hint);
			set_dim = true;
		}
		dom_string_unref(attr);
	}
	अगर (set_dim == false) {
		/* canvas defaults to 300px wide */
		hint->prop = CSS_PROP_WIDTH;
		hint->तथ्य.लंबाई.unit = CSS_UNIT_PX;
		hint->तथ्य.लंबाई.मूल्य = INTTOFIX(300);
		hint->status = CSS_WIDTH_SET;
		css_hint_advance(&hint);
	}
}

अटल व्योम css_hint_width_निवेश(
		nscss_select_ctx *ctx,
		dom_गाँठ *गाँठ)
{
	काष्ठा css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_string *attr = शून्य;
	dom_exception err;

	err = dom_element_get_attribute(गाँठ,
			corestring_dom_आकार, &attr);

	अगर (err == DOM_NO_ERR && attr != शून्य) {
		अगर (parse_dimension(
				(स्थिर अक्षर *)dom_string_तथ्य(attr), false,
				&hint->तथ्य.लंबाई.मूल्य,
				&hint->तथ्य.लंबाई.unit)) {
			dom_string *attr2 = शून्य;

			err = dom_element_get_attribute(गाँठ,
					corestring_dom_type, &attr2);
			अगर (err == DOM_NO_ERR) {

				hint->prop = CSS_PROP_WIDTH;
				hint->status = CSS_WIDTH_SET;

				अगर (attr2 == शून्य ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_text) ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_खोज) ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_password) ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_file)) {
					hint->तथ्य.लंबाई.unit = CSS_UNIT_EX;
				}
				अगर (attr2 != शून्य) {
					dom_string_unref(attr2);
				}
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}
}

अटल व्योम css_hint_anchor_color(
		nscss_select_ctx *ctx,
		dom_गाँठ *गाँठ)
{
	काष्ठा css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	css_error error;
	dom_exception err;
	dom_string *color;
	dom_गाँठ *bodynode = शून्य;

	/* find body node */
	css_qname qs;
	bool is_visited;

	qs.ns = शून्य;
	qs.name = lwc_string_ref(corestring_lwc_body);
	अगर (named_ancestor_गाँठ(ctx, गाँठ, &qs,
			(व्योम *)&bodynode) != CSS_OK) {
		/* Didn't find, or had error */
		lwc_string_unref(qs.name);
		वापस ;
	}
	lwc_string_unref(qs.name);

	अगर (bodynode == शून्य) {
		वापस;
	}

	error = गाँठ_is_visited(ctx, गाँठ, &is_visited);
	अगर (error != CSS_OK)
		वापस;

	अगर (is_visited) {
		err = dom_element_get_attribute(bodynode,
				corestring_dom_vlink, &color);
	} अन्यथा {
		err = dom_element_get_attribute(bodynode,
				corestring_dom_link, &color);
	}

	अगर (err == DOM_NO_ERR && color != शून्य) {
		अगर (nscss_parse_colour(
				(स्थिर अक्षर *)dom_string_तथ्य(color),
						&hint->तथ्य.color)) {
			hint->prop = CSS_PROP_COLOR;
			hint->status = CSS_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(color);
	}
}

अटल व्योम css_hint_body_color(
		nscss_select_ctx *ctx,
		dom_गाँठ *गाँठ)
{
	काष्ठा css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *color;

	err = dom_element_get_attribute(गाँठ, corestring_dom_text, &color);

	अगर (err == DOM_NO_ERR && color != शून्य) {
		अगर (nscss_parse_colour(
				(स्थिर अक्षर *)dom_string_तथ्य(color),
						&hint->तथ्य.color)) {
			hint->prop = CSS_PROP_COLOR;
			hint->status = CSS_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(color);
	}
}

अटल व्योम css_hint_color(
		nscss_select_ctx *ctx,
		dom_गाँठ *गाँठ)
{
	काष्ठा css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *color;

	err = dom_element_get_attribute(गाँठ, corestring_dom_color, &color);

	अगर (err == DOM_NO_ERR && color != शून्य) {
		अगर (nscss_parse_colour(
				(स्थिर अक्षर *)dom_string_तथ्य(color),
						&hint->तथ्य.color)) {
			hint->prop = CSS_PROP_COLOR;
			hint->status = CSS_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(color);
	}
}

अटल व्योम css_hint_font_आकार(
		nscss_select_ctx *ctx,
		dom_गाँठ *गाँठ)
{
	काष्ठा css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *आकार;

	err = dom_element_get_attribute(गाँठ, corestring_dom_आकार, &आकार);
	अगर (err == DOM_NO_ERR && आकार != शून्य) {
		अगर (parse_font_आकार(
				(स्थिर अक्षर *)dom_string_तथ्य(आकार),
						&hint->status,
						&hint->तथ्य.लंबाई.मूल्य,
						&hint->तथ्य.लंबाई.unit)) {
			hint->prop = CSS_PROP_FONT_आकार;
			css_hint_advance(&hint);
		}
		dom_string_unref(आकार);
	}
}

अटल व्योम css_hint_float(
		nscss_select_ctx *ctx,
		dom_गाँठ *गाँठ)
{
	काष्ठा css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *align;

	err = dom_element_get_attribute(गाँठ, corestring_dom_align, &align);
	अगर (err == DOM_NO_ERR && align != शून्य) {
		अगर (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_बायाँ)) {
			hint->prop = CSS_PROP_FLOAT;
			hint->status = CSS_FLOAT_बायाँ;
			css_hint_advance(&hint);

		} अन्यथा अगर (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_दायाँ)) {
			hint->prop = CSS_PROP_FLOAT;
			hint->status = CSS_FLOAT_दायाँ;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

अटल व्योम css_hint_caption_side(
		nscss_select_ctx *ctx,
		dom_गाँठ *गाँठ)
{
	काष्ठा css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *align = शून्य;

	err = dom_element_get_attribute(गाँठ, corestring_dom_align, &align);
	अगर (err == DOM_NO_ERR && align != शून्य) {
		अगर (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_bottom)) {
			hint->prop = CSS_PROP_CAPTION_SIDE;
			hint->status = CSS_CAPTION_SIDE_BOTTOM;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

अटल व्योम css_hint_bg_color(
		nscss_select_ctx *ctx,
		dom_गाँठ *गाँठ)
{
	काष्ठा css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *bgcolor;

	err = dom_element_get_attribute(गाँठ,
			corestring_dom_bgcolor, &bgcolor);
	अगर (err == DOM_NO_ERR && bgcolor != शून्य) {
		अगर (nscss_parse_colour(
				(स्थिर अक्षर *)dom_string_तथ्य(bgcolor),
				&hint->तथ्य.color)) {
			hint->prop = CSS_PROP_BACKGROUND_COLOR;
			hint->status = CSS_BACKGROUND_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(bgcolor);
	}
}

अटल व्योम css_hint_bg_image(
		nscss_select_ctx *ctx,
		dom_गाँठ *गाँठ)
{
	काष्ठा css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_exception err;
	dom_string *attr;

	err = dom_element_get_attribute(गाँठ,
			corestring_dom_background, &attr);
	अगर (err == DOM_NO_ERR && attr != शून्य) {
		nsurl *url;
		nserror error = nsurl_join(ctx->base_url,
				(स्थिर अक्षर *)dom_string_तथ्य(attr), &url);
		dom_string_unref(attr);

		अगर (error == NSERROR_OK) {
			lwc_string *iurl;
			lwc_error lerror = lwc_intern_string(nsurl_access(url),
					nsurl_लंबाई(url), &iurl);
			nsurl_unref(url);

			अगर (lerror == lwc_error_ok) {
				hint->prop = CSS_PROP_BACKGROUND_IMAGE;
				hint->तथ्य.string = iurl;
				hint->status = CSS_BACKGROUND_IMAGE_IMAGE;
				css_hint_advance(&hint);
			}
		}
	}
}

अटल व्योम css_hint_white_space_nowrap(
		nscss_select_ctx *ctx,
		dom_गाँठ *गाँठ)
{
	काष्ठा css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_exception err;
	bool nowrap;

	err = dom_element_has_attribute(गाँठ, corestring_dom_nowrap, &nowrap);
	अगर (err == DOM_NO_ERR && nowrap == true) {
		hint->prop = CSS_PROP_WHITE_SPACE;
		hint->status = CSS_WHITE_SPACE_NOWRAP;
		css_hint_advance(&hint);
	}
}

अटल व्योम css_hint_list(
		nscss_select_ctx *ctx,
		dom_गाँठ *गाँठ)
{
	काष्ठा css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_exception err;
	dom_string *attr;

	err = dom_element_get_attribute(गाँठ, corestring_dom_type, &attr);
	अगर (err == DOM_NO_ERR && attr != शून्य) {
		स्थिर अक्षर *attr_str = dom_string_तथ्य(attr);
		माप_प्रकार attr_len = dom_string_byte_लंबाई(attr);
		क्रमागत css_list_style_type_e type = CSS_LIST_STYLE_TYPE_INHERIT;

		अगर (attr_len == 1) {
			चयन (attr_str[0]) {
			हाल 'a':
				type = CSS_LIST_STYLE_TYPE_LOWER_ALPHA;
				अवरोध;
			हाल 'A':
				type = CSS_LIST_STYLE_TYPE_UPPER_ALPHA;
				अवरोध;
			हाल 'i':
				type = CSS_LIST_STYLE_TYPE_LOWER_ROMAN;
				अवरोध;
			हाल 'I':
				type = CSS_LIST_STYLE_TYPE_UPPER_ROMAN;
				अवरोध;
			हाल '1':
				type = CSS_LIST_STYLE_TYPE_DECIMAL;
				अवरोध;
			}
		}

		अगर (type != CSS_LIST_STYLE_TYPE_INHERIT) {
			hint->prop = CSS_PROP_LIST_STYLE_TYPE;
			hint->status = type;
			css_hint_advance(&hint);
		}

		dom_string_unref(attr);
	}
}


/* Exported function, documented in css/hints.h */
css_error गाँठ_presentational_hint(व्योम *pw, व्योम *गाँठ,
		uint32_t *nhints, css_hint **hints)
{
	dom_exception exc;
	dom_html_element_type tag_type;

	अगर (nsoption_bool(author_level_css) == false) {
		*nhints = 0;
		*hints = शून्य;
		वापस CSS_OK;
	}

	css_hint_clean();

	exc = dom_html_element_get_tag_type(गाँठ, &tag_type);
	अगर (exc != DOM_NO_ERR) {
		tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	चयन (tag_type) {
	हाल DOM_HTML_ELEMENT_TYPE_TH:
	हाल DOM_HTML_ELEMENT_TYPE_TD:
		css_hint_width(pw, गाँठ);
		css_hint_table_cell_border_padding(pw, गाँठ);
		css_hint_white_space_nowrap(pw, गाँठ);
		fallthrough;
	हाल DOM_HTML_ELEMENT_TYPE_TR:
		css_hint_height(pw, गाँठ);
		fallthrough;
	हाल DOM_HTML_ELEMENT_TYPE_THEAD:
	हाल DOM_HTML_ELEMENT_TYPE_TBODY:
	हाल DOM_HTML_ELEMENT_TYPE_TFOOT:
		css_hint_text_align_special(pw, गाँठ);
		fallthrough;
	हाल DOM_HTML_ELEMENT_TYPE_COL:
		css_hint_vertical_align_table_cells(pw, गाँठ);
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_APPLET:
	हाल DOM_HTML_ELEMENT_TYPE_IMG:
		css_hint_margin_hspace_vspace(pw, गाँठ);
		fallthrough;
	हाल DOM_HTML_ELEMENT_TYPE_EMBED:
	हाल DOM_HTML_ELEMENT_TYPE_IFRAME:
	हाल DOM_HTML_ELEMENT_TYPE_OBJECT:
		css_hint_height(pw, गाँठ);
		css_hint_width(pw, गाँठ);
		css_hint_vertical_align_replaced(pw, गाँठ);
		css_hint_float(pw, गाँठ);
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_P:
	हाल DOM_HTML_ELEMENT_TYPE_H1:
	हाल DOM_HTML_ELEMENT_TYPE_H2:
	हाल DOM_HTML_ELEMENT_TYPE_H3:
	हाल DOM_HTML_ELEMENT_TYPE_H4:
	हाल DOM_HTML_ELEMENT_TYPE_H5:
	हाल DOM_HTML_ELEMENT_TYPE_H6:
		css_hint_text_align_normal(pw, गाँठ);
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_CENTER:
		css_hint_text_align_center(pw, गाँठ);
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_CAPTION:
		css_hint_caption_side(pw, गाँठ);
		fallthrough;
	हाल DOM_HTML_ELEMENT_TYPE_DIV:
		css_hint_text_align_special(pw, गाँठ);
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_TABLE:
		css_hint_text_align_table_special(pw, गाँठ);
		css_hint_table_spacing_border(pw, गाँठ);
		css_hint_float(pw, गाँठ);
		css_hint_margin_बायाँ_दायाँ_align_center(pw, गाँठ);
		css_hint_width(pw, गाँठ);
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_HR:
		css_hint_width(pw, गाँठ);
		css_hint_margin_बायाँ_दायाँ_hr(pw, गाँठ);
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		css_hint_height_width_textarea(pw, गाँठ);
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_निवेश:
		css_hint_width_निवेश(pw, गाँठ);
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_A:
		css_hint_anchor_color(pw, गाँठ);
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_FONT:
		css_hint_font_आकार(pw, गाँठ);
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_BODY:
		css_hint_body_color(pw, गाँठ);
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_CANVAS:
		css_hint_height_width_canvas(pw, गाँठ);
		अवरोध;
	हाल DOM_HTML_ELEMENT_TYPE_OL:
		css_hint_list(pw, गाँठ);
		अवरोध;
	शेष:
		अवरोध;
	}

	अगर (tag_type != DOM_HTML_ELEMENT_TYPE__UNKNOWN) {
		css_hint_color(pw, गाँठ);
		css_hint_bg_color(pw, गाँठ);
		css_hint_bg_image(pw, गाँठ);
	}

#अगर_घोषित LOG_STATS
	NSLOG(netsurf, INFO, "Properties with hints: %i", hint_ctx.len);
#पूर्ण_अगर

	css_hint_get_hints(hints, nhints);

	वापस CSS_OK;
}
