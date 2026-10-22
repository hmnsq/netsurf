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

#అన్తర్భావ <string.h>
#అన్తర్భావ <strings.h>

#అన్తర్భావ "utils/nsoption.h"
#అన్తర్భావ "utils/corestrings.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/nsurl.h"
#అన్తర్భావ "utils/utils.h"

#అన్తర్భావ "css/hints.h"
#అన్తర్భావ "css/select.h"

#నిర్వచించు LOG_STATS
#నిర్వచనం_రద్దు LOG_STATS

/******************************************************************************
 * Utility functions                                                          *
 ******************************************************************************/

/**
 * Determine if a given character is whitespace
 *
 * \param c  Character to consider
 * \return true if character is whitespace, false otherwise
 */
స్థిర bool isWhitespace(అక్షరం c)
{
	తిరిగి c == ' ' || c == '\t' || c == '\f' || c == '\r' || c == '\n';
}

/**
 * Determine if a given character is a valid hex digit
 *
 * \param c  Character to consider
 * \return true if character is a valid hex digit, false otherwise
 */
స్థిర bool isHex(అక్షరం c)
{
	తిరిగి ('0' <= c && c <= '9') ||
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
స్థిర uint8_t char_గమ్యం_Hex(అక్షరం c)
{
	/* 0-9 */
	c -= '0';

	/* A-F */
	ఒకవేళ (c > 9)
		c -= 'A' - '9' - 1;

	/* a-f */
	ఒకవేళ (c > 15)
		c -= 'a' - 'A';

	తిరిగి c;
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
స్థిర bool parse_సంఖ్య(స్థిరరాశి అక్షరం *దత్తాంశం, bool maybe_negative, bool real,
		css_fixed *విలువ, size_t *consumed)
{
	size_t len;
	స్థిరరాశి uint8_t *ptr;
	int32_t intpart = 0;
	int32_t fracpart = 0;
	int32_t pwr = 1;
	పూర్ణాంకం sign = 1;

	*consumed = 0;

	len = అక్షరశ్రేణి_పొడవు(దత్తాంశం);
	ptr = (స్థిరరాశి uint8_t *) దత్తాంశం;

	ఒకవేళ (len == 0)
		తిరిగి false;

	/* Skip leading whitespace */
	అయితే (len > 0 && isWhitespace(ptr[0])) {
		len--;
		ptr++;
	}

	ఒకవేళ (len == 0)
		తిరిగి false;

	/* Extract sign, if any */
	ఒకవేళ (ptr[0] == '+') {
		len--;
		ptr++;
	} లేకపోతే ఒకవేళ (ptr[0] == '-' && maybe_negative) {
		sign = -1;
		len--;
		ptr++;
	}

	ఒకవేళ (len == 0)
		తిరిగి false;

	/* Must have a digit [0,9] */
	ఒకవేళ ('0' > ptr[0] || ptr[0] > '9')
		తిరిగి false;

	/* Now extract intpart, assuming base 10 */
	అయితే (len > 0) {
		/* Stop on first non-digit */
		ఒకవేళ (ptr[0] < '0' || '9' < ptr[0])
			ఆపు;

		/* Prevent overflow of 'intpart'; proper clamping below */
		ఒకవేళ (intpart < (1 << 22)) {
			intpart *= 10;
			intpart += ptr[0] - '0';
		}
		ptr++;
		len--;
	}

	/* And fracpart, again, assuming base 10 */
	ఒకవేళ (real && len > 1 && ptr[0] == '.' &&
			('0' <= ptr[1] && ptr[1] <= '9')) {
		ptr++;
		len--;

		అయితే (len > 0) {
			ఒకవేళ (ptr[0] < '0' || '9' < ptr[0])
				ఆపు;

			ఒకవేళ (pwr < 1000000) {
				pwr *= 10;
				fracpart *= 10;
				fracpart += ptr[0] - '0';
			}
			ptr++;
			len--;
		}

		fracpart = ((1 << 10) * fracpart + pwr/2) / pwr;
		ఒకవేళ (fracpart >= (1 << 10)) {
			intpart++;
			fracpart &= (1 << 10) - 1;
		}
	}

	ఒకవేళ (sign > 0) {
		/* If the result is larger than we can represent,
		 * then clamp to the maximum value we can store. */
		ఒకవేళ (intpart >= (1 << 21)) {
			intpart = (1 << 21) - 1;
			fracpart = (1 << 10) - 1;
		}
	} లేకపోతే {
		/* If the negated result is smaller than we can represent
		 * then clamp to the minimum value we can store. */
		ఒకవేళ (intpart >= (1 << 21)) {
			intpart = -(1 << 21);
			fracpart = 0;
		} లేకపోతే {
			intpart = -intpart;
			ఒకవేళ (fracpart) {
				fracpart = (1 << 10) - fracpart;
				intpart--;
			}
		}
	}

	*విలువ = (intpart << 10) | fracpart;

	*consumed = ptr - (స్థిరరాశి uint8_t *) దత్తాంశం;

	తిరిగి true;
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
స్థిర bool parse_dimension(స్థిరరాశి అక్షరం *దత్తాంశం, bool strict, css_fixed *నిడివి,
		css_unit *unit)
{
	size_t len;
	size_t పఠ;
	css_fixed విలువ;

	len = అక్షరశ్రేణి_పొడవు(దత్తాంశం);

	ఒకవేళ (parse_సంఖ్య(దత్తాంశం, false, true, &విలువ, &పఠ) == false)
		తిరిగి false;

	ఒకవేళ (strict && విలువ < INTTOFIX(1))
		తిరిగి false;

	*నిడివి = విలువ;

	ఒకవేళ (len > పఠ && దత్తాంశం[పఠ] == '%')
		*unit = CSS_UNIT_PCT;
	లేకపోతే
		*unit = CSS_UNIT_PX;

	తిరిగి true;
}

/**
 * Mapping of colour name to CSS color
 */
నిర్మాణం రంగు_map {
	స్థిరరాశి అక్షరం *name;
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
స్థిర పూర్ణాంకం cmp_రంగు_name(స్థిరరాశి శూన్య *a, స్థిరరాశి శూన్య *b)
{
	స్థిరరాశి అక్షరం *aa = a;
	స్థిరరాశి నిర్మాణం రంగు_map *bb = b;

	తిరిగి strcasecmp(aa, bb->name);
}

/**
 * Parse a named colour
 *
 * \param name    Name to parse
 * \param result  Pointer to location to receive css_color
 * \return true on success, false on invalid input
 */
స్థిర bool parse_named_రంగు(స్థిరరాశి అక్షరం *name, css_color *ఫలితం)
{
	స్థిర స్థిరరాశి నిర్మాణం రంగు_map named_colours[] = {
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
	స్థిరరాశి నిర్మాణం రంగు_map *నమోదు_2;

	నమోదు_2 = bsearch(name, named_colours,
			పరిమాణం(named_colours) / పరిమాణం(named_colours[0]),
			పరిమాణం(named_colours[0]),
			cmp_రంగు_name);

	ఒకవేళ (నమోదు_2 != ఖాళీ)
		*ఫలితం = నమోదు_2->color;

	తిరిగి నమోదు_2 != ఖాళీ;
}

/* exported interface documented in content/handlers/css/hints.h */
bool nscss_parse_రంగు(స్థిరరాశి అక్షరం *దత్తాంశం, css_color *ఫలితం)
{
	size_t len = అక్షరశ్రేణి_పొడవు(దత్తాంశం);
	uint8_t r, g, b;

	/* 2 */
	ఒకవేళ (len == 0)
		తిరిగి false;

	/* 3 */
	ఒకవేళ (len == SLEN("transparent") && strcasecmp(దత్తాంశం, "transparent") == 0)
		తిరిగి false;

	/* 4 */
	ఒకవేళ (parse_named_రంగు(దత్తాంశం, ఫలితం))
		తిరిగి true;

	/** \todo Implement HTML5's utterly insane legacy colour parsing */

	ఒకవేళ (దత్తాంశం[0] == '#') {
		దత్తాంశం++;
		len--;
	}

	ఒకవేళ (len == 3 && isHex(దత్తాంశం[0]) && isHex(దత్తాంశం[1]) && isHex(దత్తాంశం[2])) {
		r = char_గమ్యం_Hex(దత్తాంశం[0]);
		g = char_గమ్యం_Hex(దత్తాంశం[1]);
		b = char_గమ్యం_Hex(దత్తాంశం[2]);

		r |= (r << 4);
		g |= (g << 4);
		b |= (b << 4);

		*ఫలితం = (0xff << 24) | (r << 16) | (g << 8) | b;

		తిరిగి true;
	} లేకపోతే ఒకవేళ (len == 6 && isHex(దత్తాంశం[0]) && isHex(దత్తాంశం[1]) &&
			isHex(దత్తాంశం[2]) && isHex(దత్తాంశం[3]) && isHex(దత్తాంశం[4]) &&
			isHex(దత్తాంశం[5])) {
		r = (char_గమ్యం_Hex(దత్తాంశం[0]) << 4) | char_గమ్యం_Hex(దత్తాంశం[1]);
		g = (char_గమ్యం_Hex(దత్తాంశం[2]) << 4) | char_గమ్యం_Hex(దత్తాంశం[3]);
		b = (char_గమ్యం_Hex(దత్తాంశం[4]) << 4) | char_గమ్యం_Hex(దత్తాంశం[5]);

		*ఫలితం = (0xff << 24) | (r << 16) | (g << 8) | b;

		తిరిగి true;
	}

	తిరిగి false;
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
స్థిర bool parse_font_కొలత(స్థిరరాశి అక్షరం *కొలత, uint8_t *val,
		css_fixed *len, css_unit *unit)
{
	స్థిర స్థిరరాశి uint8_t కొలత_map[] = {
		CSS_FONT_కొలత_XX_SMALL,
		CSS_FONT_కొలత_SMALL,
		CSS_FONT_కొలత_MEDIUM,
		CSS_FONT_కొలత_LARGE,
		CSS_FONT_కొలత_X_LARGE,
		CSS_FONT_కొలత_XX_LARGE,
		CSS_FONT_కొలత_DIMENSION	/* xxx-large (see below) */
	};

	స్థిరరాశి అక్షరం *p = కొలత;
	అక్షరం బహుళకం;
	పూర్ణాంకం విలువ = 0;

	/* Skip whitespace */
	అయితే (*p != '\0' && isWhitespace(*p))
		p++;

	బహుళకం = *p;

	/* Skip +/- */
	ఒకవేళ (బహుళకం == '+' || బహుళకం == '-')
		p++;

	/* Need at least one digit */
	ఒకవేళ (*p < '0' || *p > '9') {
		తిరిగి false;
	}

	/* Consume digits, computing value */
	అయితే ('0' <= *p && *p <= '9') {
		విలువ = విలువ * 10 + (*p - '0');
		p++;
	}

	/* Resolve relative sizes */
	ఒకవేళ (బహుళకం == '+')
		విలువ += 3;
	లేకపోతే ఒకవేళ (బహుళకం == '-')
		విలువ = 3 - విలువ;

	/* Clamp to range [1,7] */
	ఒకవేళ (విలువ < 1)
		విలువ = 1;
	లేకపోతే ఒకవేళ (విలువ > 7)
		విలువ = 7;

	ఒకవేళ (విలువ == 7) {
		/* Manufacture xxx-large */
	  *len = FDIV(FMUL(INTTOFIX(3), INTTOFIX(nsoption_int(font_కొలత))),
				F_10);
	} లేకపోతే {
		/* Len is irrelevant */
		*len = 0;
	}

	*unit = CSS_UNIT_PT;
	*val = కొలత_map[విలువ - 1];

	తిరిగి true;
}


/******************************************************************************
 * Hint context management                                                    *
 ******************************************************************************/

#నిర్వచించు MAX_HINTS_చొప్పున_ELEMENT 32

నిర్మాణం css_hint_ctx {
	నిర్మాణం css_hint *hints;
	uint32_t len;
};

నిర్మాణం css_hint_ctx hint_ctx;

nserror css_hint_init(శూన్య)
{
	hint_ctx.hints = జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం css_hint) *
			MAX_HINTS_చొప్పున_ELEMENT);
	ఒకవేళ (hint_ctx.hints == ఖాళీ) {
		తిరిగి NSERROR_NOMEM;
	}

	తిరిగి NSERROR_OK;
}

శూన్య css_hint_fini(శూన్య)
{
	hint_ctx.len = 0;
	విడుదల(hint_ctx.hints);
}

స్థిర శూన్య css_hint_clean(శూన్య)
{
	hint_ctx.len = 0;
}

స్థిర అంతఃవరుస శూన్య css_hint_advance(నిర్మాణం css_hint **hint)
{
	hint_ctx.len++;
	ప్రతిజ్ఞా(hint_ctx.len < MAX_HINTS_చొప్పున_ELEMENT);

	(*hint)++;
}

స్థిర శూన్య css_hint_get_hints(నిర్మాణం css_hint **hints, uint32_t *nhints)
{
	*hints = hint_ctx.hints;
	*nhints = hint_ctx.len;
}


/******************************************************************************
 * Presentational hint handlers                                               *
 ******************************************************************************/

స్థిర శూన్య css_hint_పట్టిక_cell_border_padding(
		nscss_select_ctx *ctx,
		dom_కణుపు *కణుపు)
{
	నిర్మాణం css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	css_qname qs;
	dom_string *attr = ఖాళీ;
	dom_కణుపు *tablenode = ఖాళీ;
	dom_exception exc;

	qs.ns = ఖాళీ;
	qs.name = lwc_string_ref(corestring_lwc_పట్టిక);
	ఒకవేళ (named_ancestor_కణుపు(ctx, కణుపు, &qs,
			(శూన్య *)&tablenode) != CSS_OK) {
		/* Didn't find, or had error */
		lwc_string_unref(qs.name);
		తిరిగి;
	}
	lwc_string_unref(qs.name);

	ఒకవేళ (tablenode == ఖాళీ) {
		తిరిగి;
	}
	/* No need to unref tablenode, named_ancestor_node does not
	 * return a reffed node to the CSS
	 */

	exc = dom_element_get_attribute(tablenode,
			corestring_dom_border, &attr);

	ఒకవేళ (exc == DOM_NO_ERR && attr != ఖాళీ) {
		uint32_t hint_prop;
		css_hint_నిడివి hint_నిడివి;

		ఒకవేళ (parse_dimension(
				dom_string_దత్తాంశం(attr), false,
						&hint_నిడివి.విలువ,
						&hint_నిడివి.unit) &&
				INTTOFIX(0) != hint_నిడివి.విలువ) {

			కొరకు (hint_prop = CSS_PROP_BORDER_శిఖరం_STYLE;
			     hint_prop <= CSS_PROP_BORDER_ఎడమ_STYLE;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->స్థితి_2 = CSS_BORDER_STYLE_INSET;
				css_hint_advance(&hint);
			}

			కొరకు (hint_prop = CSS_PROP_BORDER_శిఖరం_వెడల్పు;
			     hint_prop <= CSS_PROP_BORDER_ఎడమ_వెడల్పు;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->దత్తాంశం.నిడివి.విలువ = INTTOFIX(1);
				hint->దత్తాంశం.నిడివి.unit = CSS_UNIT_PX;
				hint->స్థితి_2 = CSS_BORDER_వెడల్పు_వెడల్పు;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(tablenode,
			corestring_dom_bordercolor, &attr);

	ఒకవేళ (exc == DOM_NO_ERR && attr != ఖాళీ) {
		uint32_t hint_prop;
		css_color hint_color;

		ఒకవేళ (nscss_parse_రంగు(
				(స్థిరరాశి అక్షరం *)dom_string_దత్తాంశం(attr),
				&hint_color)) {

			కొరకు (hint_prop = CSS_PROP_BORDER_శిఖరం_COLOR;
			     hint_prop <= CSS_PROP_BORDER_ఎడమ_COLOR;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->దత్తాంశం.color = hint_color;
				hint->స్థితి_2 = CSS_BORDER_COLOR_COLOR;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(tablenode,
			corestring_dom_cellpadding, &attr);

	ఒకవేళ (exc == DOM_NO_ERR && attr != ఖాళీ) {
		uint32_t hint_prop;
		css_hint_నిడివి hint_నిడివి;

		ఒకవేళ (parse_dimension(
				dom_string_దత్తాంశం(attr), false,
						&hint_నిడివి.విలువ,
						&hint_నిడివి.unit)) {

			కొరకు (hint_prop = CSS_PROP_PADDING_శిఖరం;
			     hint_prop <= CSS_PROP_PADDING_ఎడమ;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->దత్తాంశం.నిడివి.విలువ = hint_నిడివి.విలువ;
				hint->దత్తాంశం.నిడివి.unit = hint_నిడివి.unit;
				hint->స్థితి_2 = CSS_PADDING_సమితి;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}
}

స్థిర శూన్య css_hint_vertical_align_పట్టిక_cells(
		nscss_select_ctx *ctx,
		dom_కణుపు *కణుపు)
{
	నిర్మాణం css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = ఖాళీ;
	dom_exception err;

	err = dom_element_get_attribute(కణుపు,
			corestring_dom_valign, &attr);

	ఒకవేళ (err == DOM_NO_ERR && attr != ఖాళీ) {
		hint->దత్తాంశం.నిడివి.విలువ = 0;
		hint->దత్తాంశం.నిడివి.unit = CSS_UNIT_PX;
		ఒకవేళ (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_శిఖరం)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->స్థితి_2 = CSS_VERTICAL_ALIGN_శిఖరం;
			css_hint_advance(&hint);

		} లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_మధ్య)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->స్థితి_2 = CSS_VERTICAL_ALIGN_మధ్య;
			css_hint_advance(&hint);

		} లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_bottom)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->స్థితి_2 = CSS_VERTICAL_ALIGN_BOTTOM;
			css_hint_advance(&hint);

		} లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_baseline)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->స్థితి_2 = CSS_VERTICAL_ALIGN_BASELINE;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

స్థిర శూన్య css_hint_vertical_align_replaced(
		nscss_select_ctx *ctx,
		dom_కణుపు *కణుపు)
{
	నిర్మాణం css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = ఖాళీ;
	dom_exception err;

	err = dom_element_get_attribute(కణుపు,
			corestring_dom_valign, &attr);

	ఒకవేళ (err == DOM_NO_ERR && attr != ఖాళీ) {
		ఒకవేళ (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_శిఖరం)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->స్థితి_2 = CSS_VERTICAL_ALIGN_శిఖరం;
			css_hint_advance(&hint);

		} లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_bottom) ||
			   dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_baseline)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->స్థితి_2 = CSS_VERTICAL_ALIGN_BASELINE;
			css_hint_advance(&hint);

		} లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_texttop)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->స్థితి_2 = CSS_VERTICAL_ALIGN_పాఠ్యం_శిఖరం;
			css_hint_advance(&hint);

		} లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_absmiddle) ||
			   dom_string_caseless_lwc_isequal(attr,
					corestring_lwc_abscenter)) {
			hint->prop = CSS_PROP_VERTICAL_ALIGN;
			hint->స్థితి_2 = CSS_VERTICAL_ALIGN_మధ్య;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

స్థిర శూన్య css_hint_పాఠ్యం_align_సాధారణ(
		nscss_select_ctx *ctx,
		dom_కణుపు *కణుపు)
{
	నిర్మాణం css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *align = ఖాళీ;
	dom_exception err;

	err = dom_element_get_attribute(కణుపు,
			corestring_dom_align, &align);
	ఒకవేళ (err == DOM_NO_ERR && align != ఖాళీ) {
		ఒకవేళ (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_ఎడమ)) {
			hint->prop = CSS_PROP_పాఠ్యం_ALIGN;
			hint->స్థితి_2 = CSS_పాఠ్యం_ALIGN_ఎడమ;
			css_hint_advance(&hint);

		} లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_center)) {
			hint->prop = CSS_PROP_పాఠ్యం_ALIGN;
			hint->స్థితి_2 = CSS_పాఠ్యం_ALIGN_CENTER;
			css_hint_advance(&hint);

		} లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_కుడి)) {
			hint->prop = CSS_PROP_పాఠ్యం_ALIGN;
			hint->స్థితి_2 = CSS_పాఠ్యం_ALIGN_కుడి;
			css_hint_advance(&hint);

		} లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_justify)) {
			hint->prop = CSS_PROP_పాఠ్యం_ALIGN;
			hint->స్థితి_2 = CSS_పాఠ్యం_ALIGN_JUSTIFY;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

స్థిర శూన్య css_hint_పాఠ్యం_align_center(
		nscss_select_ctx *ctx,
		dom_కణుపు *కణుపు)
{
	నిర్మాణం css_hint *hint = &hint_ctx.hints[hint_ctx.len];

	hint->prop = CSS_PROP_పాఠ్యం_ALIGN;
	hint->స్థితి_2 = CSS_పాఠ్యం_ALIGN_LIBCSS_CENTER;
	css_hint_advance(&hint);
}

స్థిర శూన్య css_hint_margin_ఎడమ_కుడి_align_center(
		nscss_select_ctx *ctx,
		dom_కణుపు *కణుపు)
{
	నిర్మాణం css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr;
	dom_exception exc;

	exc = dom_element_get_attribute(కణుపు,
			corestring_dom_align, &attr);

	ఒకవేళ (exc == DOM_NO_ERR && attr != ఖాళీ) {
		memset(hint, 0, పరిమాణం(*hint) * 2);
		ఒకవేళ (dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_center) ||
				dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_abscenter) ||
				dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_మధ్య) ||
				dom_string_caseless_lwc_isequal(attr,
						corestring_lwc_absmiddle)) {
			hint->prop = CSS_PROP_MARGIN_ఎడమ;
			hint->స్థితి_2 = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_కుడి;
			hint->స్థితి_2 = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

స్థిర శూన్య css_hint_పాఠ్యం_align_special(
		nscss_select_ctx *ctx,
		dom_కణుపు *కణుపు)
{
	నిర్మాణం css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *align = ఖాళీ;
	dom_exception err;

	err = dom_element_get_attribute(కణుపు,
			corestring_dom_align, &align);

	ఒకవేళ (err == DOM_NO_ERR && align != ఖాళీ) {
		ఒకవేళ (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_center)) {
			hint->prop = CSS_PROP_పాఠ్యం_ALIGN;
			hint->స్థితి_2 = CSS_పాఠ్యం_ALIGN_LIBCSS_CENTER;
			css_hint_advance(&hint);

		} లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_ఎడమ)) {
			hint->prop = CSS_PROP_పాఠ్యం_ALIGN;
			hint->స్థితి_2 = CSS_పాఠ్యం_ALIGN_LIBCSS_ఎడమ;
			css_hint_advance(&hint);

		} లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_కుడి)) {
			hint->prop = CSS_PROP_పాఠ్యం_ALIGN;
			hint->స్థితి_2 = CSS_పాఠ్యం_ALIGN_LIBCSS_కుడి;
			css_hint_advance(&hint);

		} లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(align,
					corestring_lwc_justify)) {
			hint->prop = CSS_PROP_పాఠ్యం_ALIGN;
			hint->స్థితి_2 = CSS_పాఠ్యం_ALIGN_JUSTIFY;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

స్థిర శూన్య css_hint_పాఠ్యం_align_పట్టిక_special(
		nscss_select_ctx *ctx,
		dom_కణుపు *కణుపు)
{
	నిర్మాణం css_hint *hint = &hint_ctx.hints[hint_ctx.len];

	hint->prop = CSS_PROP_పాఠ్యం_ALIGN;
	hint->స్థితి_2 = CSS_పాఠ్యం_ALIGN_INHERIT_IF_NON_MAGIC;
	css_hint_advance(&hint);
}

స్థిర శూన్య css_hint_margin_hspace_vspace(
		nscss_select_ctx *ctx,
		dom_కణుపు *కణుపు)
{
	నిర్మాణం css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = ఖాళీ;
	dom_exception exc;

	exc = dom_element_get_attribute(కణుపు,
			corestring_dom_vspace, &attr);

	ఒకవేళ (exc == DOM_NO_ERR && attr != ఖాళీ) {
		css_hint_నిడివి hint_నిడివి;
		ఒకవేళ (parse_dimension(
				dom_string_దత్తాంశం(attr), false,
				&hint_నిడివి.విలువ,
				&hint_నిడివి.unit)) {
			hint->prop = CSS_PROP_MARGIN_శిఖరం;
			hint->దత్తాంశం.నిడివి.విలువ = hint_నిడివి.విలువ;
			hint->దత్తాంశం.నిడివి.unit = hint_నిడివి.unit;
			hint->స్థితి_2 = CSS_MARGIN_సమితి;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_BOTTOM;
			hint->దత్తాంశం.నిడివి.విలువ = hint_నిడివి.విలువ;
			hint->దత్తాంశం.నిడివి.unit = hint_నిడివి.unit;
			hint->స్థితి_2 = CSS_MARGIN_సమితి;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(కణుపు,
			corestring_dom_hspace, &attr);

	ఒకవేళ (exc == DOM_NO_ERR && attr != ఖాళీ) {
		css_hint_నిడివి hint_నిడివి;
		ఒకవేళ (parse_dimension(
				dom_string_దత్తాంశం(attr), false,
				&hint_నిడివి.విలువ,
				&hint_నిడివి.unit)) {
			hint->prop = CSS_PROP_MARGIN_ఎడమ;
			hint->దత్తాంశం.నిడివి.విలువ = hint_నిడివి.విలువ;
			hint->దత్తాంశం.నిడివి.unit = hint_నిడివి.unit;
			hint->స్థితి_2 = CSS_MARGIN_సమితి;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_కుడి;
			hint->దత్తాంశం.నిడివి.విలువ = hint_నిడివి.విలువ;
			hint->దత్తాంశం.నిడివి.unit = hint_నిడివి.unit;
			hint->స్థితి_2 = CSS_MARGIN_సమితి;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

స్థిర శూన్య css_hint_margin_ఎడమ_కుడి_hr(
		nscss_select_ctx *ctx,
		dom_కణుపు *కణుపు)
{
	నిర్మాణం css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr;
	dom_exception exc;

	exc = dom_element_get_attribute(కణుపు,
				corestring_dom_align, &attr);

	ఒకవేళ (exc == DOM_NO_ERR && attr != ఖాళీ) {
		memset(hint, 0, పరిమాణం(*hint) * 2);
		ఒకవేళ (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_ఎడమ)) {
			hint->prop = CSS_PROP_MARGIN_ఎడమ;
			hint->దత్తాంశం.నిడివి.విలువ = 0;
			hint->దత్తాంశం.నిడివి.unit = CSS_UNIT_PX;
			hint->స్థితి_2 = CSS_MARGIN_సమితి;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_కుడి;
			hint->స్థితి_2 = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

		} లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_center)) {
			hint->prop = CSS_PROP_MARGIN_ఎడమ;
			hint->స్థితి_2 = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_కుడి;
			hint->స్థితి_2 = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

		} లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(attr,
				corestring_lwc_కుడి)) {
			hint->prop = CSS_PROP_MARGIN_ఎడమ;
			hint->స్థితి_2 = CSS_MARGIN_AUTO;
			css_hint_advance(&hint);

			hint->prop = CSS_PROP_MARGIN_కుడి;
			hint->దత్తాంశం.నిడివి.విలువ = 0;
			hint->దత్తాంశం.నిడివి.unit = CSS_UNIT_PX;
			hint->స్థితి_2 = CSS_MARGIN_సమితి;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

స్థిర శూన్య css_hint_పట్టిక_spacing_border(
		nscss_select_ctx *ctx,
		dom_కణుపు *కణుపు)
{
	నిర్మాణం css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception exc;
	dom_string *attr = ఖాళీ;

	exc = dom_element_get_attribute(కణుపు, corestring_dom_border, &attr);

	ఒకవేళ (exc == DOM_NO_ERR && attr != ఖాళీ) {
		uint32_t hint_prop;
		css_hint_నిడివి hint_నిడివి;

		కొరకు (hint_prop = CSS_PROP_BORDER_శిఖరం_STYLE;
			     hint_prop <= CSS_PROP_BORDER_ఎడమ_STYLE;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->స్థితి_2 = CSS_BORDER_STYLE_OUTSET;
				css_hint_advance(&hint);
		}

		ఒకవేళ (parse_dimension(
				dom_string_దత్తాంశం(attr), false,
						&hint_నిడివి.విలువ,
						&hint_నిడివి.unit)) {

			కొరకు (hint_prop = CSS_PROP_BORDER_శిఖరం_వెడల్పు;
			     hint_prop <= CSS_PROP_BORDER_ఎడమ_వెడల్పు;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->దత్తాంశం.నిడివి.విలువ = hint_నిడివి.విలువ;
				hint->దత్తాంశం.నిడివి.unit = hint_నిడివి.unit;
				hint->స్థితి_2 = CSS_BORDER_వెడల్పు_వెడల్పు;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(కణుపు,
			corestring_dom_bordercolor, &attr);

	ఒకవేళ (exc == DOM_NO_ERR && attr != ఖాళీ) {
		uint32_t hint_prop;
		css_color hint_color;

		ఒకవేళ (nscss_parse_రంగు(
				(స్థిరరాశి అక్షరం *)dom_string_దత్తాంశం(attr),
				&hint_color)) {

			కొరకు (hint_prop = CSS_PROP_BORDER_శిఖరం_COLOR;
			     hint_prop <= CSS_PROP_BORDER_ఎడమ_COLOR;
			     hint_prop++) {
				hint->prop = hint_prop;
				hint->దత్తాంశం.color = hint_color;
				hint->స్థితి_2 = CSS_BORDER_COLOR_COLOR;
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}

	exc = dom_element_get_attribute(కణుపు,
			corestring_dom_cellspacing, &attr);

	ఒకవేళ (exc == DOM_NO_ERR && attr != ఖాళీ) {
		ఒకవేళ (parse_dimension(
				(స్థిరరాశి అక్షరం *)dom_string_దత్తాంశం(attr), false,
						&hint->దత్తాంశం.position.h.విలువ,
						&hint->దత్తాంశం.position.h.unit)) {
			hint->prop = CSS_PROP_BORDER_SPACING;
			hint->దత్తాంశం.position.v = hint->దత్తాంశం.position.h;
			hint->స్థితి_2 = CSS_BORDER_SPACING_సమితి;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

స్థిర శూన్య css_hint_ఎత్తు(
		nscss_select_ctx *ctx,
		dom_కణుపు *కణుపు)
{
	నిర్మాణం css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = ఖాళీ;
	dom_exception err;

	err = dom_element_get_attribute(కణుపు,
			corestring_dom_ఎత్తు, &attr);

	ఒకవేళ (err == DOM_NO_ERR && attr != ఖాళీ) {
		ఒకవేళ (parse_dimension(
				(స్థిరరాశి అక్షరం *)dom_string_దత్తాంశం(attr), false,
				&hint->దత్తాంశం.నిడివి.విలువ,
				&hint->దత్తాంశం.నిడివి.unit)) {
			hint->prop = CSS_PROP_ఎత్తు;
			hint->స్థితి_2 = CSS_ఎత్తు_సమితి;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

స్థిర శూన్య css_hint_వెడల్పు(
		nscss_select_ctx *ctx,
		dom_కణుపు *కణుపు)
{
	నిర్మాణం css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = ఖాళీ;
	dom_exception err;

	err = dom_element_get_attribute(కణుపు,
			corestring_dom_వెడల్పు, &attr);

	ఒకవేళ (err == DOM_NO_ERR && attr != ఖాళీ) {
		ఒకవేళ (parse_dimension(
				(స్థిరరాశి అక్షరం *)dom_string_దత్తాంశం(attr), false,
				&hint->దత్తాంశం.నిడివి.విలువ,
				&hint->దత్తాంశం.నిడివి.unit)) {
			hint->prop = CSS_PROP_వెడల్పు;
			hint->స్థితి_2 = CSS_వెడల్పు_సమితి;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

స్థిర శూన్య css_hint_ఎత్తు_వెడల్పు_textarea(
		nscss_select_ctx *ctx,
		dom_కణుపు *కణుపు)
{
	నిర్మాణం css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = ఖాళీ;
	dom_exception err;

	err = dom_element_get_attribute(కణుపు,
			corestring_dom_అడ్డువరుసలు, &attr);

	ఒకవేళ (err == DOM_NO_ERR && attr != ఖాళీ) {
		ఒకవేళ (parse_dimension(
				(స్థిరరాశి అక్షరం *)dom_string_దత్తాంశం(attr), false,
				&hint->దత్తాంశం.నిడివి.విలువ,
				&hint->దత్తాంశం.నిడివి.unit)) {
			hint->prop = CSS_PROP_ఎత్తు;
			hint->దత్తాంశం.నిడివి.unit = CSS_UNIT_EM;
			hint->స్థితి_2 = CSS_ఎత్తు_సమితి;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}

	err = dom_element_get_attribute(కణుపు,
			corestring_dom_నిలువుల_సంఖ్య, &attr);

	ఒకవేళ (err == DOM_NO_ERR && attr != ఖాళీ) {
		ఒకవేళ (parse_dimension(
				(స్థిరరాశి అక్షరం *)dom_string_దత్తాంశం(attr), false,
				&hint->దత్తాంశం.నిడివి.విలువ,
				&hint->దత్తాంశం.నిడివి.unit)) {
			hint->prop = CSS_PROP_వెడల్పు;
			hint->దత్తాంశం.నిడివి.unit = CSS_UNIT_EX;
			hint->స్థితి_2 = CSS_వెడల్పు_సమితి;
			css_hint_advance(&hint);
		}
		dom_string_unref(attr);
	}
}

స్థిర శూన్య css_hint_ఎత్తు_వెడల్పు_canvas(
		nscss_select_ctx *ctx,
		dom_కణుపు *కణుపు)
{
	నిర్మాణం css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_string *attr = ఖాళీ;
	dom_exception err;
	bool సమితి_dim = false;

	err = dom_element_get_attribute(కణుపు,
			corestring_dom_ఎత్తు, &attr);

	ఒకవేళ (err == DOM_NO_ERR && attr != ఖాళీ) {
		ఒకవేళ (parse_dimension(
				(స్థిరరాశి అక్షరం *)dom_string_దత్తాంశం(attr), true,
				&hint->దత్తాంశం.నిడివి.విలువ,
				&hint->దత్తాంశం.నిడివి.unit)) {
			hint->prop = CSS_PROP_ఎత్తు;
			hint->దత్తాంశం.నిడివి.unit = CSS_UNIT_PX;
			hint->స్థితి_2 = CSS_ఎత్తు_సమితి;
			css_hint_advance(&hint);
			సమితి_dim = true;
		}
		dom_string_unref(attr);
	}
	ఒకవేళ (సమితి_dim == false) {
		/* canvas defaults to 150px tall */
		hint->prop = CSS_PROP_ఎత్తు;
		hint->దత్తాంశం.నిడివి.unit = CSS_UNIT_PX;
		hint->దత్తాంశం.నిడివి.విలువ = INTTOFIX(150);
		hint->స్థితి_2 = CSS_ఎత్తు_సమితి;
		css_hint_advance(&hint);
	} లేకపోతే {
		సమితి_dim = false;
	}

	err = dom_element_get_attribute(కణుపు,
			corestring_dom_వెడల్పు, &attr);

	ఒకవేళ (err == DOM_NO_ERR && attr != ఖాళీ) {
		ఒకవేళ (parse_dimension(
				(స్థిరరాశి అక్షరం *)dom_string_దత్తాంశం(attr), true,
				&hint->దత్తాంశం.నిడివి.విలువ,
				&hint->దత్తాంశం.నిడివి.unit)) {
			hint->prop = CSS_PROP_వెడల్పు;
			hint->దత్తాంశం.నిడివి.unit = CSS_UNIT_PX;
			hint->స్థితి_2 = CSS_వెడల్పు_సమితి;
			css_hint_advance(&hint);
			సమితి_dim = true;
		}
		dom_string_unref(attr);
	}
	ఒకవేళ (సమితి_dim == false) {
		/* canvas defaults to 300px wide */
		hint->prop = CSS_PROP_వెడల్పు;
		hint->దత్తాంశం.నిడివి.unit = CSS_UNIT_PX;
		hint->దత్తాంశం.నిడివి.విలువ = INTTOFIX(300);
		hint->స్థితి_2 = CSS_వెడల్పు_సమితి;
		css_hint_advance(&hint);
	}
}

స్థిర శూన్య css_hint_వెడల్పు_ఆదానం(
		nscss_select_ctx *ctx,
		dom_కణుపు *కణుపు)
{
	నిర్మాణం css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_string *attr = ఖాళీ;
	dom_exception err;

	err = dom_element_get_attribute(కణుపు,
			corestring_dom_కొలత, &attr);

	ఒకవేళ (err == DOM_NO_ERR && attr != ఖాళీ) {
		ఒకవేళ (parse_dimension(
				(స్థిరరాశి అక్షరం *)dom_string_దత్తాంశం(attr), false,
				&hint->దత్తాంశం.నిడివి.విలువ,
				&hint->దత్తాంశం.నిడివి.unit)) {
			dom_string *attr2 = ఖాళీ;

			err = dom_element_get_attribute(కణుపు,
					corestring_dom_type, &attr2);
			ఒకవేళ (err == DOM_NO_ERR) {

				hint->prop = CSS_PROP_వెడల్పు;
				hint->స్థితి_2 = CSS_వెడల్పు_సమితి;

				ఒకవేళ (attr2 == ఖాళీ ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_పాఠ్యం) ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_శోధన) ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_password) ||
						dom_string_caseless_lwc_isequal(
						attr2,
						corestring_lwc_file)) {
					hint->దత్తాంశం.నిడివి.unit = CSS_UNIT_EX;
				}
				ఒకవేళ (attr2 != ఖాళీ) {
					dom_string_unref(attr2);
				}
				css_hint_advance(&hint);
			}
		}
		dom_string_unref(attr);
	}
}

స్థిర శూన్య css_hint_anchor_color(
		nscss_select_ctx *ctx,
		dom_కణుపు *కణుపు)
{
	నిర్మాణం css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	css_error error;
	dom_exception err;
	dom_string *color;
	dom_కణుపు *bodynode = ఖాళీ;

	/* find body node */
	css_qname qs;
	bool is_సందర్శించిన;

	qs.ns = ఖాళీ;
	qs.name = lwc_string_ref(corestring_lwc_body);
	ఒకవేళ (named_ancestor_కణుపు(ctx, కణుపు, &qs,
			(శూన్య *)&bodynode) != CSS_OK) {
		/* Didn't find, or had error */
		lwc_string_unref(qs.name);
		తిరిగి ;
	}
	lwc_string_unref(qs.name);

	ఒకవేళ (bodynode == ఖాళీ) {
		తిరిగి;
	}

	error = కణుపు_is_సందర్శించిన(ctx, కణుపు, &is_సందర్శించిన);
	ఒకవేళ (error != CSS_OK)
		తిరిగి;

	ఒకవేళ (is_సందర్శించిన) {
		err = dom_element_get_attribute(bodynode,
				corestring_dom_vlink, &color);
	} లేకపోతే {
		err = dom_element_get_attribute(bodynode,
				corestring_dom_link, &color);
	}

	ఒకవేళ (err == DOM_NO_ERR && color != ఖాళీ) {
		ఒకవేళ (nscss_parse_రంగు(
				(స్థిరరాశి అక్షరం *)dom_string_దత్తాంశం(color),
						&hint->దత్తాంశం.color)) {
			hint->prop = CSS_PROP_COLOR;
			hint->స్థితి_2 = CSS_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(color);
	}
}

స్థిర శూన్య css_hint_body_color(
		nscss_select_ctx *ctx,
		dom_కణుపు *కణుపు)
{
	నిర్మాణం css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *color;

	err = dom_element_get_attribute(కణుపు, corestring_dom_పాఠ్యం, &color);

	ఒకవేళ (err == DOM_NO_ERR && color != ఖాళీ) {
		ఒకవేళ (nscss_parse_రంగు(
				(స్థిరరాశి అక్షరం *)dom_string_దత్తాంశం(color),
						&hint->దత్తాంశం.color)) {
			hint->prop = CSS_PROP_COLOR;
			hint->స్థితి_2 = CSS_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(color);
	}
}

స్థిర శూన్య css_hint_color(
		nscss_select_ctx *ctx,
		dom_కణుపు *కణుపు)
{
	నిర్మాణం css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *color;

	err = dom_element_get_attribute(కణుపు, corestring_dom_color, &color);

	ఒకవేళ (err == DOM_NO_ERR && color != ఖాళీ) {
		ఒకవేళ (nscss_parse_రంగు(
				(స్థిరరాశి అక్షరం *)dom_string_దత్తాంశం(color),
						&hint->దత్తాంశం.color)) {
			hint->prop = CSS_PROP_COLOR;
			hint->స్థితి_2 = CSS_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(color);
	}
}

స్థిర శూన్య css_hint_font_కొలత(
		nscss_select_ctx *ctx,
		dom_కణుపు *కణుపు)
{
	నిర్మాణం css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *కొలత;

	err = dom_element_get_attribute(కణుపు, corestring_dom_కొలత, &కొలత);
	ఒకవేళ (err == DOM_NO_ERR && కొలత != ఖాళీ) {
		ఒకవేళ (parse_font_కొలత(
				(స్థిరరాశి అక్షరం *)dom_string_దత్తాంశం(కొలత),
						&hint->స్థితి_2,
						&hint->దత్తాంశం.నిడివి.విలువ,
						&hint->దత్తాంశం.నిడివి.unit)) {
			hint->prop = CSS_PROP_FONT_కొలత;
			css_hint_advance(&hint);
		}
		dom_string_unref(కొలత);
	}
}

స్థిర శూన్య css_hint_float(
		nscss_select_ctx *ctx,
		dom_కణుపు *కణుపు)
{
	నిర్మాణం css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *align;

	err = dom_element_get_attribute(కణుపు, corestring_dom_align, &align);
	ఒకవేళ (err == DOM_NO_ERR && align != ఖాళీ) {
		ఒకవేళ (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_ఎడమ)) {
			hint->prop = CSS_PROP_FLOAT;
			hint->స్థితి_2 = CSS_FLOAT_ఎడమ;
			css_hint_advance(&hint);

		} లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_కుడి)) {
			hint->prop = CSS_PROP_FLOAT;
			hint->స్థితి_2 = CSS_FLOAT_కుడి;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

స్థిర శూన్య css_hint_caption_side(
		nscss_select_ctx *ctx,
		dom_కణుపు *కణుపు)
{
	నిర్మాణం css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *align = ఖాళీ;

	err = dom_element_get_attribute(కణుపు, corestring_dom_align, &align);
	ఒకవేళ (err == DOM_NO_ERR && align != ఖాళీ) {
		ఒకవేళ (dom_string_caseless_lwc_isequal(align,
				corestring_lwc_bottom)) {
			hint->prop = CSS_PROP_CAPTION_SIDE;
			hint->స్థితి_2 = CSS_CAPTION_SIDE_BOTTOM;
			css_hint_advance(&hint);
		}
		dom_string_unref(align);
	}
}

స్థిర శూన్య css_hint_bg_color(
		nscss_select_ctx *ctx,
		dom_కణుపు *కణుపు)
{
	నిర్మాణం css_hint *hint = &hint_ctx.hints[hint_ctx.len];
	dom_exception err;
	dom_string *bgcolor;

	err = dom_element_get_attribute(కణుపు,
			corestring_dom_bgcolor, &bgcolor);
	ఒకవేళ (err == DOM_NO_ERR && bgcolor != ఖాళీ) {
		ఒకవేళ (nscss_parse_రంగు(
				(స్థిరరాశి అక్షరం *)dom_string_దత్తాంశం(bgcolor),
				&hint->దత్తాంశం.color)) {
			hint->prop = CSS_PROP_BACKGROUND_COLOR;
			hint->స్థితి_2 = CSS_BACKGROUND_COLOR_COLOR;
			css_hint_advance(&hint);
		}
		dom_string_unref(bgcolor);
	}
}

స్థిర శూన్య css_hint_bg_image(
		nscss_select_ctx *ctx,
		dom_కణుపు *కణుపు)
{
	నిర్మాణం css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_exception err;
	dom_string *attr;

	err = dom_element_get_attribute(కణుపు,
			corestring_dom_background, &attr);
	ఒకవేళ (err == DOM_NO_ERR && attr != ఖాళీ) {
		nsurl *url;
		nserror error = nsurl_join(ctx->పునాది_url,
				(స్థిరరాశి అక్షరం *)dom_string_దత్తాంశం(attr), &url);
		dom_string_unref(attr);

		ఒకవేళ (error == NSERROR_OK) {
			lwc_string *iurl;
			lwc_error lerror = lwc_intern_string(nsurl_access(url),
					nsurl_నిడివి(url), &iurl);
			nsurl_unref(url);

			ఒకవేళ (lerror == lwc_error_ok) {
				hint->prop = CSS_PROP_BACKGROUND_IMAGE;
				hint->దత్తాంశం.string = iurl;
				hint->స్థితి_2 = CSS_BACKGROUND_IMAGE_IMAGE;
				css_hint_advance(&hint);
			}
		}
	}
}

స్థిర శూన్య css_hint_white_space_nowrap(
		nscss_select_ctx *ctx,
		dom_కణుపు *కణుపు)
{
	నిర్మాణం css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_exception err;
	bool nowrap;

	err = dom_element_has_attribute(కణుపు, corestring_dom_nowrap, &nowrap);
	ఒకవేళ (err == DOM_NO_ERR && nowrap == true) {
		hint->prop = CSS_PROP_WHITE_SPACE;
		hint->స్థితి_2 = CSS_WHITE_SPACE_NOWRAP;
		css_hint_advance(&hint);
	}
}

స్థిర శూన్య css_hint_list(
		nscss_select_ctx *ctx,
		dom_కణుపు *కణుపు)
{
	నిర్మాణం css_hint *hint = &(hint_ctx.hints[hint_ctx.len]);
	dom_exception err;
	dom_string *attr;

	err = dom_element_get_attribute(కణుపు, corestring_dom_type, &attr);
	ఒకవేళ (err == DOM_NO_ERR && attr != ఖాళీ) {
		స్థిరరాశి అక్షరం *attr_str = dom_string_దత్తాంశం(attr);
		size_t attr_len = dom_string_byte_నిడివి(attr);
		గణన css_list_style_type_e type = CSS_LIST_STYLE_TYPE_INHERIT;

		ఒకవేళ (attr_len == 1) {
			ఎంపిక (attr_str[0]) {
			స్థితి 'a':
				type = CSS_LIST_STYLE_TYPE_క్రింది_ALPHA;
				ఆపు;
			స్థితి 'A':
				type = CSS_LIST_STYLE_TYPE_పై_ALPHA;
				ఆపు;
			స్థితి 'i':
				type = CSS_LIST_STYLE_TYPE_క్రింది_ROMAN;
				ఆపు;
			స్థితి 'I':
				type = CSS_LIST_STYLE_TYPE_పై_ROMAN;
				ఆపు;
			స్థితి '1':
				type = CSS_LIST_STYLE_TYPE_DECIMAL;
				ఆపు;
			}
		}

		ఒకవేళ (type != CSS_LIST_STYLE_TYPE_INHERIT) {
			hint->prop = CSS_PROP_LIST_STYLE_TYPE;
			hint->స్థితి_2 = type;
			css_hint_advance(&hint);
		}

		dom_string_unref(attr);
	}
}


/* Exported function, documented in css/hints.h */
css_error కణుపు_presentational_hint(శూన్య *pw, శూన్య *కణుపు,
		uint32_t *nhints, css_hint **hints)
{
	dom_exception exc;
	dom_html_element_type ట్యాగ్_type;

	ఒకవేళ (nsoption_bool(author_level_css) == false) {
		*nhints = 0;
		*hints = ఖాళీ;
		తిరిగి CSS_OK;
	}

	css_hint_clean();

	exc = dom_html_element_get_ట్యాగ్_type(కణుపు, &ట్యాగ్_type);
	ఒకవేళ (exc != DOM_NO_ERR) {
		ట్యాగ్_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	ఎంపిక (ట్యాగ్_type) {
	స్థితి DOM_HTML_ELEMENT_TYPE_TH:
	స్థితి DOM_HTML_ELEMENT_TYPE_TD:
		css_hint_వెడల్పు(pw, కణుపు);
		css_hint_పట్టిక_cell_border_padding(pw, కణుపు);
		css_hint_white_space_nowrap(pw, కణుపు);
		fallthrough;
	స్థితి DOM_HTML_ELEMENT_TYPE_TR:
		css_hint_ఎత్తు(pw, కణుపు);
		fallthrough;
	స్థితి DOM_HTML_ELEMENT_TYPE_THEAD:
	స్థితి DOM_HTML_ELEMENT_TYPE_TBODY:
	స్థితి DOM_HTML_ELEMENT_TYPE_TFOOT:
		css_hint_పాఠ్యం_align_special(pw, కణుపు);
		fallthrough;
	స్థితి DOM_HTML_ELEMENT_TYPE_నిలువు:
		css_hint_vertical_align_పట్టిక_cells(pw, కణుపు);
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_APPLET:
	స్థితి DOM_HTML_ELEMENT_TYPE_IMG:
		css_hint_margin_hspace_vspace(pw, కణుపు);
		fallthrough;
	స్థితి DOM_HTML_ELEMENT_TYPE_EMBED:
	స్థితి DOM_HTML_ELEMENT_TYPE_IFRAME:
	స్థితి DOM_HTML_ELEMENT_TYPE_OBJECT:
		css_hint_ఎత్తు(pw, కణుపు);
		css_hint_వెడల్పు(pw, కణుపు);
		css_hint_vertical_align_replaced(pw, కణుపు);
		css_hint_float(pw, కణుపు);
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_P:
	స్థితి DOM_HTML_ELEMENT_TYPE_H1:
	స్థితి DOM_HTML_ELEMENT_TYPE_H2:
	స్థితి DOM_HTML_ELEMENT_TYPE_H3:
	స్థితి DOM_HTML_ELEMENT_TYPE_H4:
	స్థితి DOM_HTML_ELEMENT_TYPE_H5:
	స్థితి DOM_HTML_ELEMENT_TYPE_H6:
		css_hint_పాఠ్యం_align_సాధారణ(pw, కణుపు);
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_CENTER:
		css_hint_పాఠ్యం_align_center(pw, కణుపు);
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_CAPTION:
		css_hint_caption_side(pw, కణుపు);
		fallthrough;
	స్థితి DOM_HTML_ELEMENT_TYPE_DIV:
		css_hint_పాఠ్యం_align_special(pw, కణుపు);
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_పట్టిక:
		css_hint_పాఠ్యం_align_పట్టిక_special(pw, కణుపు);
		css_hint_పట్టిక_spacing_border(pw, కణుపు);
		css_hint_float(pw, కణుపు);
		css_hint_margin_ఎడమ_కుడి_align_center(pw, కణుపు);
		css_hint_వెడల్పు(pw, కణుపు);
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_HR:
		css_hint_వెడల్పు(pw, కణుపు);
		css_hint_margin_ఎడమ_కుడి_hr(pw, కణుపు);
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		css_hint_ఎత్తు_వెడల్పు_textarea(pw, కణుపు);
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_ఆదానం:
		css_hint_వెడల్పు_ఆదానం(pw, కణుపు);
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_A:
		css_hint_anchor_color(pw, కణుపు);
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_FONT:
		css_hint_font_కొలత(pw, కణుపు);
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_BODY:
		css_hint_body_color(pw, కణుపు);
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_CANVAS:
		css_hint_ఎత్తు_వెడల్పు_canvas(pw, కణుపు);
		ఆపు;
	స్థితి DOM_HTML_ELEMENT_TYPE_OL:
		css_hint_list(pw, కణుపు);
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	ఒకవేళ (ట్యాగ్_type != DOM_HTML_ELEMENT_TYPE__UNKNOWN) {
		css_hint_color(pw, కణుపు);
		css_hint_bg_color(pw, కణుపు);
		css_hint_bg_image(pw, కణుపు);
	}

#నిర్వచితమైతే LOG_STATS
	NSLOG(netsurf, INFO, "Properties with hints: %i", hint_ctx.len);
#అయితే_ముగింపు

	css_hint_get_hints(hints, nhints);

	తిరిగి CSS_OK;
}
