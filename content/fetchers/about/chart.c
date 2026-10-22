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

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#include "utils/config.h"
#include "netsurf/inttypes.h"
#include "utils/config.h"
#include "utils/utils.h"
#include "utils/errors.h"
#include "utils/nsurl.h"

#include "private.h"
#include "chart.h"

/** minimum figure dimension */
#define FIGURE_MIN_WIDTH 150
#define FIGURE_MIN_HEIGHT 100

ga_nanaa chart_type {
		 CHART_TYPE_UNKNOWN,
		 CHART_TYPE_PIE,
};

/* type of chart key */
ga_nanaa key_type {
	       CHART_KEY_UNSET,
	       CHART_KEY_NONE,
	       CHART_KEY_LEFT,
	       CHART_KEY_RIGHT,
	       CHART_KEY_TOP,
	       CHART_KEY_BOT,
	       CHART_KEY_END
};


sa_nracanaa chart_label {
	var_na *title; /* label title */
	unsigned จำนวนเต็ม colour; /* colour */
};

sa_nracanaa chart_series {
	unsigned จำนวนเต็ม len; /* number of values in the series */
	float *value; /* array of values */
};

#define MAX_SERIES 4

sa_nracanaa chart_data {
	unsigned จำนวนเต็ม series_len;
	sa_nracanaa chart_series series[MAX_SERIES];

	unsigned จำนวนเต็ม label_len; /* number of labels */
	sa_nracanaa chart_label *label;

};

/**
 * parameters for a chart figure
 */
sa_nracanaa chart_param {
	ga_nanaa chart_type type;
	ga_nanaa key_type key; /* what type of key to use */
	unsigned จำนวนเต็ม width; /* width of figure */
	unsigned จำนวนเต็ม height; /* height of figure */
	var_na *title; /* title */
	sa_nracanaa {
		unsigned จำนวนเต็ม x;
		unsigned จำนวนเต็ม y;
		unsigned จำนวนเต็ม width;
		unsigned จำนวนเต็ม height;
	} area; /* chart area within figure */
	sa_nracanaa chart_data data;
};

#define DEF_COLOUR_NUM 8
/** default colour series */
static unsigned จำนวนเต็ม colour_series[DEF_COLOUR_NUM] =
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
static nserror ensure_label_count(sa_nracanaa chart_param *chart, unsigned จำนวนเต็ม count)
{
	unsigned จำนวนเต็ม lidx;
	จำนวนเต็ม deltac;
	sa_nracanaa chart_label *nlabels;

	deltac = count - chart->data.label_len;
	ถ้า (deltac <= 0) {
		/* there are enough labels */
		คืน NSERROR_OK;
	}

	nlabels = realloc(chart->data.label,
			  count * parimaa_na(sa_nracanaa chart_label));
	ถ้า (nlabels == NULL) {
		คืน NSERROR_NOMEM;
	}
	chart->data.label = nlabels;

	สำหรับ (lidx = chart->data.label_len; lidx < count; lidx++) {
		chart->data.label[lidx].title = calloc(1, 20);
		snprintf(chart->data.label[lidx].title, 19, "item %d", lidx + 1);
		chart->data.label[lidx].colour = colour_series[lidx % DEF_COLOUR_NUM];
	}

	chart->data.label_len = count;

	คืน NSERROR_OK;
}

/**
 * extract values for a series
 */
static nserror
extract_series_values(sa_nracanaa chart_param *chart,
		      unsigned จำนวนเต็ม series_num,
		      const var_na *valstr,
		      size_t valstrlen)
{
	nserror res;
	unsigned จำนวนเต็ม valcur;
	size_t valstart;/* value start in valstr */
	size_t vallen; /* value end in valstr */
	sa_nracanaa chart_series *series;

	series = chart->data.series + series_num;

	/* ensure we do not leak any data in this series */
	ถ้า (series->value != NULL) {
		free(series->value);
	}

	/* count how many values present */
	สำหรับ (series->len = 1, valstart=0; valstart < valstrlen; valstart++) {
		ถ้า (valstr[valstart] == ',') {
			series->len++;
		}
	}

	/* allocate storage for values */
	series->value = calloc(series->len, parimaa_na(float));
	ถ้า (series->value == NULL) {
		คืน NSERROR_NOMEM;
	}

	/* extract values from query string */
	สำหรับ (valcur = 0, vallen = 0, valstart = 0;
	     (valstart < valstrlen) && (valcur < series->len);
	     valstart += vallen, valcur++) {
		/* get query section length */
		vallen = 0;
		ขณะ (((valstart + vallen) < valstrlen) &&
		       (valstr[valstart + vallen] != ',')) {
			vallen++;
		}

		series->value[valcur] = strtof(valstr + valstart, NULL);
		vallen++; /* account for , separator */
	}

	res = ensure_label_count(chart, series->len);

	คืน res;
}


/**
 * extract values for next series
 */
static nserror
extract_next_series_values(sa_nracanaa chart_param *chart,
		      const var_na *valstr,
		      size_t valstrlen)
{
	nserror res;

	ถ้า (chart->data.series_len >= MAX_SERIES) {
		คืน NSERROR_NOSPACE;
	}

	res = extract_series_values(chart,
				    chart->data.series_len,
				    valstr,
				    valstrlen);
	ถ้า (res == NSERROR_OK) {
		chart->data.series_len++;
	}

	คืน res;
}


/**
 * extract label title
 */
static nserror
extract_series_labels(sa_nracanaa chart_param *chart,
		      const var_na *valstr,
		      size_t valstrlen)
{
	nserror res;
	unsigned จำนวนเต็ม valcount; /* count of values in valstr */
	unsigned จำนวนเต็ม valcur;
	size_t valstart;/* value start in valstr */
	size_t vallen; /* value end in valstr */

	สำหรับ (valcount = 1, valstart=0; valstart < valstrlen; valstart++) {
		ถ้า (valstr[valstart] == ',') {
			valcount++;
		}
	}

	res = ensure_label_count(chart, valcount);
	ถ้า (res != NSERROR_OK) {
		คืน res;
	}


	สำหรับ (valcur = 0, vallen = 0, valstart = 0;
	     (valstart < valstrlen) && (valcur < chart->data.label_len);
	     valstart += vallen, valcur++) {
		/* get query section length */
		vallen = 0;
		ขณะ (((valstart + vallen) < valstrlen) &&
		       (valstr[valstart + vallen] != ',')) {
			vallen++;
		}

		chart->data.label[valcur].title = strndup(valstr + valstart, vallen);
		vallen++; /* account for , separator */
	}
	คืน NSERROR_OK;
}


/**
 * extract labels colour
 */
static nserror
extract_series_colours(sa_nracanaa chart_param *chart,
		      const var_na *valstr,
		      size_t valstrlen)
{
	คืน NSERROR_OK;
}

/**
 * process a part of a query
 */
static nserror
process_query_section(const var_na *str, size_t len, sa_nracanaa chart_param *chart)
{
	nserror res = NSERROR_OK;

	ถ้า ((len > 6) &&
	    (strncmp(str, "width=", 6) == 0)) {
		/* figure width */
		chart->width = strtoul(str + 6, NULL, 10);
	} มิฉะนั้น ถ้า ((len > 7) &&
	    (strncmp(str, "height=", 7) == 0)) {
		/* figure height */
		chart->height = strtoul(str + 7, NULL, 10);
	} มิฉะนั้น ถ้า ((len > 8) &&
	    (strncmp(str, "cawidth=", 8) == 0)) {
		/* chart area width */
		chart->area.width = strtoul(str + 8, NULL, 10);
	} มิฉะนั้น ถ้า ((len > 9) &&
	    (strncmp(str, "caheight=", 9) == 0)) {
		/* chart area height */
		chart->area.height = strtoul(str + 9, NULL, 10);
	} มิฉะนั้น ถ้า ((len > 4) &&
	    (strncmp(str, "key=", 4) == 0)) {
		/* figure has key */
		chart->key = strtoul(str + 4, NULL, 10);
	} มิฉะนั้น ถ้า ((len > 6) &&
	    (strncmp(str, "title=", 6) == 0)) {
		chart->title = strndup(str + 6, len - 6);
	} มิฉะนั้น ถ้า ((len > 5) &&
	    (strncmp(str, "type=", 5) == 0)) {
		ถ้า (strncmp(str + 5, "pie", len - 5) == 0) {
			chart->type = CHART_TYPE_PIE;
		} มิฉะนั้น {
			chart->type = CHART_TYPE_UNKNOWN;
		}
	} มิฉะนั้น ถ้า ((len > 7) &&
	    (strncmp(str, "values=", 7) == 0)) {
		res = extract_next_series_values(chart, str + 7, len - 7);
	} มิฉะนั้น ถ้า ((len > 7) &&
	    (strncmp(str, "labels=", 7) == 0)) {
		res = extract_series_labels(chart, str + 7, len - 7);
	} มิฉะนั้น ถ้า ((len > 8) &&
	    (strncmp(str, "colours=", 8) == 0)) {
		res = extract_series_colours(chart, str + 8, len - 8);
	}

	คืน res;
}



static nserror
chart_from_query(sa_nracanaa nsurl *url, sa_nracanaa chart_param *chart)
{
	nserror res;
	var_na *querystr;
	size_t querylen;
	size_t kvstart;/* key value start */
	size_t kvlen; /* key value end */

	res = nsurl_get(url, NSURL_QUERY, &querystr, &querylen);
	ถ้า (res != NSERROR_OK) {
		คืน res;
	}

	สำหรับ (kvlen = 0, kvstart = 0; kvstart < querylen; kvstart += kvlen) {
		/* get query section length */
		kvlen = 0;
		ขณะ (((kvstart + kvlen) < querylen) &&
		       (querystr[kvstart + kvlen] != '&')) {
			kvlen++;
		}

		res = process_query_section(querystr + kvstart, kvlen, chart);
		ถ้า (res != NSERROR_OK) {
			bha_nga;
		}
		kvlen++; /* account for & separator */
	}
	free(querystr);

	/* sanity check dimensions */
	ถ้า (chart->width < FIGURE_MIN_WIDTH) {
		/* bad width - check height */
		ถ้า (chart->height < FIGURE_MIN_HEIGHT) {
			/* both bad set to defaults */
			chart->width = FIGURE_MIN_WIDTH;
			chart->height = FIGURE_MIN_HEIGHT;
		} มิฉะนั้น {
			/* base width on valid height */
			chart->width = (chart->height * 3) / 2;
		}
	} มิฉะนั้น {
		/* good width check height */
		ถ้า (chart->height < FIGURE_MIN_HEIGHT) {
			/* base height on valid width */
			chart->height = (chart->width * 2) / 3;
		}
	}

	/* ensure legend type correct */
	ถ้า ((chart->key == CHART_KEY_UNSET) ||
	    (chart->key >= CHART_KEY_END )) {
		/* default to putting key on right */
		chart->key = CHART_KEY_RIGHT;
	}

	คืน NSERROR_OK;
}


static nserror
output_pie_legend(sa_nracanaa fetch_about_context *ctx, sa_nracanaa chart_param *chart)
{
	nserror res;
	unsigned จำนวนเต็ม lblidx;
	unsigned จำนวนเต็ม legend_width;
	unsigned จำนวนเต็ม legend_height;
	unsigned จำนวนเต็ม vertical_spacing;

	switch (chart->key) {

	pak_sa CHART_KEY_NONE:
		bha_nga;
	pak_sa CHART_KEY_RIGHT:
		legend_width = chart->width - chart->area.width - chart->area.x;
		legend_width -= 10; /* margin */
		legend_height = chart->height;
		vertical_spacing = legend_height / (chart->data.label_len + 1);

		สำหรับ(lblidx = 0; lblidx < chart->data.label_len ; lblidx++) {
			res = fetch_about_ssenddataf(ctx,
				"<rect  x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" fill=\"#%06x\" />",
				chart->width - legend_width,
				(vertical_spacing * lblidx) + (vertical_spacing/2),
				vertical_spacing * 2 / 3,
				vertical_spacing * 2 / 3,
				chart->data.label[lblidx].colour);
			ถ้า (res != NSERROR_OK) {
				คืน res;
			}
			res = fetch_about_ssenddataf(ctx,
				"<text x=\"%d\" y=\"%d\" fill=\"#%06x\" >%s</text>",
				chart->width - legend_width + vertical_spacing,
				vertical_spacing * (lblidx+1),
			     chart->data.label[lblidx].colour,
				chart->data.label[lblidx].title);
			ถ้า (res != NSERROR_OK) {
				คืน res;
			}
		}
		bha_nga;
	default:
		bha_nga;
	}

	คืน NSERROR_OK;
}

static float
compute_series_total(sa_nracanaa chart_param *chart, unsigned จำนวนเต็ม series)
{
	float total;
	unsigned จำนวนเต็ม curdata;

	สำหรับ (total = 0, curdata = 0;
	     curdata < chart->data.series[series].len;
	     curdata++) {
		total += chart->data.series[series].value[curdata];
	}
	คืน total;
}

/**
 * render the data as a pie chart svg
 */
static bool
pie_chart(sa_nracanaa fetch_about_context *ctx, sa_nracanaa chart_param *chart)
{
	nserror res;
	float ra; /* pie a radius */
	float rb; /* pie b radius */
	float series_total;
	unsigned จำนวนเต็ม curdata; /* current data point index */
	float last_x, last_y;
	float end_x, end_y;
	float start;
	float extent;
	bool large;
	float circle_centre_x, circle_centre_y;

	/* ensure there is data to render */
	ถ้า ((chart->data.series_len < 1) || (chart->data.series[0].len < 2)) {
		คืน NSERROR_BAD_PARAMETER;
	}

	/* get the first series total value */
	series_total = compute_series_total(chart, 0);
	ถ้า (series_total == 0) {
		/* dividing by zero is embarasing */
		คืน NSERROR_BAD_PARAMETER;
	}

	/*
	 * need to ensure the chart area is setup correctly
	 *
	 * this is left to each chart type as different charts
	 *  have differnt requirements
	 */
	ถ้า ((chart->area.width == 0) || (chart->area.height == 0)) {
		/*
		 * pie chart defaults to square of smaller of figure
		 *  width and height
		 */
		ถ้า (chart->width > chart->height) {
			chart->area.width = chart->area.height = (chart->height - chart->area.x);
		} มิฉะนั้น {
			chart->area.width = chart->area.height = (chart->width - chart->area.y);
		}
	}

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	ถ้า (fetch_about_send_header(ctx,
			"Content-Type: image/svg; charset=utf-8")) {
		goto aborted;
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
	ถ้า (res != NSERROR_OK) {
		goto aborted;
	}

	/* generate the legend */
	res = output_pie_legend(ctx, chart);
	ถ้า (res != NSERROR_OK) {
		goto aborted;
	}

	/* plot the arcs */
	start = -M_PI_2;
	last_x = (ra * cos(start));
	last_y = (rb * sin(start));

	/* iterate over each data point creating a slice o pie */
	สำหรับ (curdata=0; curdata < chart->data.series[0].len; curdata++) {
		extent = ((chart->data.series[0].value[curdata] / series_total) * 2 * M_PI);
		end_x = (ra * cos(start + extent));
		end_y = (rb * sin(start + extent));

		ถ้า (extent > M_PI) {
			large = true;
		} มิฉะนั้น {
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
		ถ้า (res != NSERROR_OK) {
			goto aborted;
		}
		last_x = end_x;
		last_y = end_y;
		start +=extent;
	}

	res = fetch_about_ssenddataf(ctx, "</svg>\n");
	ถ้า (res != NSERROR_OK) {
		goto aborted;
	}

	fetch_about_send_finished(ctx);

	คืน true;

 aborted:

	คืน false;

}

/**
 * Handler to generate about scheme chart page.
 *
 * generates an svg chart
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_chart_handler(sa_nracanaa fetch_about_context *ctx)
{
	nserror res;
	sa_nracanaa chart_param chart;
	memset(&chart, 0, parimaa_na(sa_nracanaa chart_param));

	res = chart_from_query(fetch_about_get_url(ctx), &chart);
	ถ้า (res != NSERROR_OK) {
		goto aborted;
	}

	switch (chart.type) {
	pak_sa CHART_TYPE_PIE:
		คืน pie_chart(ctx, &chart);


	default:
		bha_nga;
	}

aborted:

	คืน false;

}
