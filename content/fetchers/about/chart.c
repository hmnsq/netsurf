/*
 * Copyright 2020 Vincent Sanders <vince@netsurf-browser.org>
 *
 * This file is part of NetSurf.
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

/**
 * \file
 * content generator for the about scheme chart page
 *
 * A chart consists of the figure area in which a chart a title and a
 * key are placed.
 *
 *
 */

#ضمن <stdbool.h>
#ضمن <stddef.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <math.h>
#ضمن <stdio.h>

#ضمن "utils/config.h"
#ضمن "netsurf/inttypes.h"
#ضمن "utils/config.h"
#ضمن "utils/utils.h"
#ضمن "utils/errors.h"
#ضمن "utils/nsurl.h"

#ضمن "private.h"
#ضمن "chart.h"

/** minimum figure dimension */
#define FIGURE_MIN_WIDTH 150
#define FIGURE_MIN_HEIGHT 100

تعداد chart_type {
		 CHART_TYPE_UNKNOWN,
		 CHART_TYPE_PIE,
};

/* type of chart key */
تعداد مبودي_type {
	       CHART_مبودي_UNSET,
	       CHART_مبودي_NONE,
	       CHART_مبودي_هغو,
	       CHART_مبودي_دام,
	       CHART_مبودي_كولي,
	       CHART_مبودي_BOT,
	       CHART_مبودي_END
};


بنية chart_label {
	حرف *title; /* label title */
	unsigned صحيح colour; /* colour */
};

بنية chart_series {
	unsigned صحيح len; /* number of values in the series */
	عائم *درج; /* array of values */
};

#define MAX_SERIES 4

بنية chart_data {
	unsigned صحيح series_len;
	بنية chart_series series[MAX_SERIES];

	unsigned صحيح label_len; /* number of labels */
	بنية chart_label *label;

};

/**
 * parameters for a chart figure
 */
بنية chart_param {
	تعداد chart_type type;
	تعداد مبودي_type مبودي; /* what type of key to use */
	unsigned صحيح width; /* width of figure */
	unsigned صحيح height; /* height of figure */
	حرف *title; /* title */
	بنية {
		unsigned صحيح x;
		unsigned صحيح y;
		unsigned صحيح width;
		unsigned صحيح height;
	} area; /* chart area within figure */
	بنية chart_data data;
};

#define DEF_COLOUR_لمب 8
/** default colour series */
ساكن unsigned صحيح colour_series[DEF_COLOUR_لمب] =
	{
	 0x00ff00, /* green */
	 0x0000ff, /* blue */
	 0xff0000, /* red */
	 0xffff00, /* yellow */
	 0x00ffff, /* cyan */
	 0xff00ff, /* pink */
	 0x777777, /* grey */
	 0x000000, /* black */
	};


/* ensures there are labels present for every value */
ساكن nserror ensure_label_كدي(بنية chart_param *chart, unsigned صحيح كدي)
{
	unsigned صحيح lidx;
	صحيح deltac;
	بنية chart_label *nlabels;

	deltac = كدي - chart->data.label_len;
	إذا (deltac <= 0) {
		/* there are enough labels */
		أرجع NSERROR_OK;
	}

	nlabels = realloc(chart->data.label,
			  كدي * حجم(بنية chart_label));
	إذا (nlabels == عدم) {
		أرجع NSERROR_NOMEM;
	}
	chart->data.label = nlabels;

	لكل (lidx = chart->data.label_len; lidx < كدي; lidx++) {
		chart->data.label[lidx].title = calloc(1, 20);
		snprintf(chart->data.label[lidx].title, 19, "item %d", lidx + 1);
		chart->data.label[lidx].colour = colour_series[lidx % DEF_COLOUR_لمب];
	}

	chart->data.label_len = كدي;

	أرجع NSERROR_OK;
}

/**
 * extract values for a series
 */
ساكن nserror
extract_series_درجوجي(بنية chart_param *chart,
		      unsigned صحيح series_لمب,
		      ثابت حرف *valstr,
		      size_t valstrlen)
{
	nserror res;
	unsigned صحيح valcur;
	size_t valstart;/* value start in valstr */
	size_t vallen; /* value end in valstr */
	بنية chart_series *series;

	series = chart->data.series + series_لمب;

	/* ensure we do not leak any data in this series */
	إذا (series->درج != عدم) {
		حرر(series->درج);
	}

	/* count how many values present */
	لكل (series->len = 1, valstart=0; valstart < valstrlen; valstart++) {
		إذا (valstr[valstart] == ',') {
			series->len++;
		}
	}

	/* allocate storage for values */
	series->درج = calloc(series->len, حجم(عائم));
	إذا (series->درج == عدم) {
		أرجع NSERROR_NOMEM;
	}

	/* extract values from query string */
	لكل (valcur = 0, vallen = 0, valstart = 0;
	     (valstart < valstrlen) && (valcur < series->len);
	     valstart += vallen, valcur++) {
		/* get query section length */
		vallen = 0;
		طالما (((valstart + vallen) < valstrlen) &&
		       (valstr[valstart + vallen] != ',')) {
			vallen++;
		}

		series->درج[valcur] = strtof(valstr + valstart, عدم);
		vallen++; /* account for , separator */
	}

	res = ensure_label_كدي(chart, series->len);

	أرجع res;
}


/**
 * extract values for next series
 */
ساكن nserror
extract_next_series_درجوجي(بنية chart_param *chart,
		      ثابت حرف *valstr,
		      size_t valstrlen)
{
	nserror res;

	إذا (chart->data.series_len >= MAX_SERIES) {
		أرجع NSERROR_NOSPACE;
	}

	res = extract_series_درجوجي(chart,
				    chart->data.series_len,
				    valstr,
				    valstrlen);
	إذا (res == NSERROR_OK) {
		chart->data.series_len++;
	}

	أرجع res;
}


/**
 * extract label title
 */
ساكن nserror
extract_series_labels(بنية chart_param *chart,
		      ثابت حرف *valstr,
		      size_t valstrlen)
{
	nserror res;
	unsigned صحيح valcount; /* count of values in valstr */
	unsigned صحيح valcur;
	size_t valstart;/* value start in valstr */
	size_t vallen; /* value end in valstr */

	لكل (valcount = 1, valstart=0; valstart < valstrlen; valstart++) {
		إذا (valstr[valstart] == ',') {
			valcount++;
		}
	}

	res = ensure_label_كدي(chart, valcount);
	إذا (res != NSERROR_OK) {
		أرجع res;
	}


	لكل (valcur = 0, vallen = 0, valstart = 0;
	     (valstart < valstrlen) && (valcur < chart->data.label_len);
	     valstart += vallen, valcur++) {
		/* get query section length */
		vallen = 0;
		طالما (((valstart + vallen) < valstrlen) &&
		       (valstr[valstart + vallen] != ',')) {
			vallen++;
		}

		chart->data.label[valcur].title = strndup(valstr + valstart, vallen);
		vallen++; /* account for , separator */
	}
	أرجع NSERROR_OK;
}


/**
 * extract labels colour
 */
ساكن nserror
extract_series_colours(بنية chart_param *chart,
		      ثابت حرف *valstr,
		      size_t valstrlen)
{
	أرجع NSERROR_OK;
}

/**
 * process a part of a query
 */
ساكن nserror
process_query_section(ثابت حرف *str, size_t len, بنية chart_param *chart)
{
	nserror res = NSERROR_OK;

	إذا ((len > 6) &&
	    (strncmp(str, "width=", 6) == 0)) {
		/* figure width */
		chart->width = strtoul(str + 6, عدم, 10);
	} وإلا إذا ((len > 7) &&
	    (strncmp(str, "height=", 7) == 0)) {
		/* figure height */
		chart->height = strtoul(str + 7, عدم, 10);
	} وإلا إذا ((len > 8) &&
	    (strncmp(str, "cawidth=", 8) == 0)) {
		/* chart area width */
		chart->area.width = strtoul(str + 8, عدم, 10);
	} وإلا إذا ((len > 9) &&
	    (strncmp(str, "caheight=", 9) == 0)) {
		/* chart area height */
		chart->area.height = strtoul(str + 9, عدم, 10);
	} وإلا إذا ((len > 4) &&
	    (strncmp(str, "key=", 4) == 0)) {
		/* figure has key */
		chart->مبودي = strtoul(str + 4, عدم, 10);
	} وإلا إذا ((len > 6) &&
	    (strncmp(str, "title=", 6) == 0)) {
		chart->title = strndup(str + 6, len - 6);
	} وإلا إذا ((len > 5) &&
	    (strncmp(str, "type=", 5) == 0)) {
		إذا (strncmp(str + 5, "pie", len - 5) == 0) {
			chart->type = CHART_TYPE_PIE;
		} وإلا {
			chart->type = CHART_TYPE_UNKNOWN;
		}
	} وإلا إذا ((len > 7) &&
	    (strncmp(str, "values=", 7) == 0)) {
		res = extract_next_series_درجوجي(chart, str + 7, len - 7);
	} وإلا إذا ((len > 7) &&
	    (strncmp(str, "labels=", 7) == 0)) {
		res = extract_series_labels(chart, str + 7, len - 7);
	} وإلا إذا ((len > 8) &&
	    (strncmp(str, "colours=", 8) == 0)) {
		res = extract_series_colours(chart, str + 8, len - 8);
	}

	أرجع res;
}



ساكن nserror
chart_from_query(بنية nsurl *url, بنية chart_param *chart)
{
	nserror res;
	حرف *querystr;
	size_t querylen;
	size_t kvstart;/* key value start */
	size_t kvlen; /* key value end */

	res = nsurl_get(url, NSURL_QUERY, &querystr, &querylen);
	إذا (res != NSERROR_OK) {
		أرجع res;
	}

	لكل (kvlen = 0, kvstart = 0; kvstart < querylen; kvstart += kvlen) {
		/* get query section length */
		kvlen = 0;
		طالما (((kvstart + kvlen) < querylen) &&
		       (querystr[kvstart + kvlen] != '&')) {
			kvlen++;
		}

		res = process_query_section(querystr + kvstart, kvlen, chart);
		إذا (res != NSERROR_OK) {
			اكسر;
		}
		kvlen++; /* account for & separator */
	}
	حرر(querystr);

	/* sanity check dimensions */
	إذا (chart->width < FIGURE_MIN_WIDTH) {
		/* bad width - check height */
		إذا (chart->height < FIGURE_MIN_HEIGHT) {
			/* both bad set to defaults */
			chart->width = FIGURE_MIN_WIDTH;
			chart->height = FIGURE_MIN_HEIGHT;
		} وإلا {
			/* base width on valid height */
			chart->width = (chart->height * 3) / 2;
		}
	} وإلا {
		/* good width check height */
		إذا (chart->height < FIGURE_MIN_HEIGHT) {
			/* base height on valid width */
			chart->height = (chart->width * 2) / 3;
		}
	}

	/* ensure legend type correct */
	إذا ((chart->مبودي == CHART_مبودي_UNSET) ||
	    (chart->مبودي >= CHART_مبودي_END )) {
		/* default to putting key on right */
		chart->مبودي = CHART_مبودي_دام;
	}

	أرجع NSERROR_OK;
}


ساكن nserror
output_pie_legend(بنية fetch_about_context *ctx, بنية chart_param *chart)
{
	nserror res;
	unsigned صحيح lblidx;
	unsigned صحيح legend_width;
	unsigned صحيح legend_height;
	unsigned صحيح vertical_spacing;

	بدل (chart->مبودي) {

	حالة CHART_مبودي_NONE:
		اكسر;
	حالة CHART_مبودي_دام:
		legend_width = chart->width - chart->area.width - chart->area.x;
		legend_width -= 10; /* margin */
		legend_height = chart->height;
		vertical_spacing = legend_height / (chart->data.label_len + 1);

		لكل(lblidx = 0; lblidx < chart->data.label_len ; lblidx++) {
			res = fetch_about_ssenddataf(ctx,
				"<rect  x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" fill=\"#%06x\" />",
				chart->width - legend_width,
				(vertical_spacing * lblidx) + (vertical_spacing/2),
				vertical_spacing * 2 / 3,
				vertical_spacing * 2 / 3,
				chart->data.label[lblidx].colour);
			إذا (res != NSERROR_OK) {
				أرجع res;
			}
			res = fetch_about_ssenddataf(ctx,
				"<text x=\"%d\" y=\"%d\" fill=\"#%06x\" >%s</text>",
				chart->width - legend_width + vertical_spacing,
				vertical_spacing * (lblidx+1),
			     chart->data.label[lblidx].colour,
				chart->data.label[lblidx].title);
			إذا (res != NSERROR_OK) {
				أرجع res;
			}
		}
		اكسر;
	افتراضي:
		اكسر;
	}

	أرجع NSERROR_OK;
}

ساكن عائم
compute_series_total(بنية chart_param *chart, unsigned صحيح series)
{
	عائم total;
	unsigned صحيح curdata;

	لكل (total = 0, curdata = 0;
	     curdata < chart->data.series[series].len;
	     curdata++) {
		total += chart->data.series[series].درج[curdata];
	}
	أرجع total;
}

/**
 * render the data as a pie chart svg
 */
ساكن bool
pie_chart(بنية fetch_about_context *ctx, بنية chart_param *chart)
{
	nserror res;
	عائم ra; /* pie a radius */
	عائم rb; /* pie b radius */
	عائم series_total;
	unsigned صحيح curdata; /* current data point index */
	عائم last_x, last_y;
	عائم end_x, end_y;
	عائم start;
	عائم extent;
	bool large;
	عائم circle_centre_x, circle_centre_y;

	/* ensure there is data to render */
	إذا ((chart->data.series_len < 1) || (chart->data.series[0].len < 2)) {
		أرجع NSERROR_BAD_PARAMETER;
	}

	/* get the first series total value */
	series_total = compute_series_total(chart, 0);
	إذا (series_total == 0) {
		/* dividing by zero is embarasing */
		أرجع NSERROR_BAD_PARAMETER;
	}

	/*
	 * need to ensure the chart area is setup correctly
	 *
	 * this is left to each chart type as different charts
	 *  have differnt requirements
	 */
	إذا ((chart->area.width == 0) || (chart->area.height == 0)) {
		/*
		 * pie chart defaults to square of smaller of figure
		 *  width and height
		 */
		إذا (chart->width > chart->height) {
			chart->area.width = chart->area.height = (chart->height - chart->area.x);
		} وإلا {
			chart->area.width = chart->area.height = (chart->width - chart->area.y);
		}
	}

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	إذا (fetch_about_send_header(ctx,
			"Content-Type: image/svg; charset=utf-8")) {
		اذهب aborted;
	}

	/* get the pie charts elipse radii */
	ra = chart->area.width / 2;
	rb = chart->area.height / 2;

	/* get the offset to the circle centre */
	circle_centre_x = chart->area.x + ra;
	circle_centre_y = chart->area.y + rb;


	/* svg header */
	res = fetch_about_ssenddataf(ctx,
			"<svg width=\"%u\" height=\"%u\" "
			"xmlns=\"http://www.w3.org/2000/svg\">\n",
			chart->width, chart->height);
	إذا (res != NSERROR_OK) {
		اذهب aborted;
	}

	/* generate the legend */
	res = output_pie_legend(ctx, chart);
	إذا (res != NSERROR_OK) {
		اذهب aborted;
	}

	/* plot the arcs */
	start = -M_PI_2;
	last_x = (ra * cos(start));
	last_y = (rb * sin(start));

	/* iterate over each data point creating a slice o pie */
	لكل (curdata=0; curdata < chart->data.series[0].len; curdata++) {
		extent = ((chart->data.series[0].درج[curdata] / series_total) * 2 * M_PI);
		end_x = (ra * cos(start + extent));
		end_y = (rb * sin(start + extent));

		إذا (extent > M_PI) {
			large = true;
		} وإلا {
			large = false;
		}

		res = fetch_about_ssenddataf(
			ctx,
			"<path d=\"M %g %g\n"
			"A %g %g 0 %d 1 %g %g\n"
			"L %g %g Z\" fill=\"#%06x\" />\n",
			circle_centre_x + last_x,
			circle_centre_y + last_y,
			ra, rb, large?1:0,
			circle_centre_x + end_x,
			circle_centre_y + end_y,
			circle_centre_x,
			circle_centre_y,
			chart->data.label[curdata].colour);
		إذا (res != NSERROR_OK) {
			اذهب aborted;
		}
		last_x = end_x;
		last_y = end_y;
		start +=extent;
	}

	res = fetch_about_ssenddataf(ctx, "</svg>\n");
	إذا (res != NSERROR_OK) {
		اذهب aborted;
	}

	fetch_about_send_finished(ctx);

	أرجع true;

 aborted:

	أرجع false;

}

/**
 * Handler to generate about scheme chart page.
 *
 * generates an svg chart
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_chart_handler(بنية fetch_about_context *ctx)
{
	nserror res;
	بنية chart_param chart;
	memset(&chart, 0, حجم(بنية chart_param));

	res = chart_from_query(fetch_about_get_url(ctx), &chart);
	إذا (res != NSERROR_OK) {
		اذهب aborted;
	}

	بدل (chart.type) {
	حالة CHART_TYPE_PIE:
		أرجع pie_chart(ctx, &chart);


	افتراضي:
		اكسر;
	}

aborted:

	أرجع false;

}
