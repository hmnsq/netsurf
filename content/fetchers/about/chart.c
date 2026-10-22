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

#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stddef.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <math.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>

#ଅନ୍ତର୍ଭାଵ "utils/config.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/inttypes.h"
#ଅନ୍ତର୍ଭାଵ "utils/config.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/errors.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsurl.h"

#ଅନ୍ତର୍ଭାଵ "private.h"
#ଅନ୍ତର୍ଭାଵ "chart.h"

/** minimum figure dimension */
#define FIGURE_MIN_WIDTH 150
#define FIGURE_MIN_HEIGHT 100

ଗଣନା chart_type {
		 CHART_TYPE_UNKNOWN,
		 CHART_TYPE_PIE,
};

/* type of chart key */
ଗଣନା ଚାବି_type {
	       CHART_ଚାବି_UNSET,
	       CHART_ଚାବି_NONE,
	       CHART_ଚାବି_ବାମ,
	       CHART_ଚାବି_ଡାହାଣ,
	       CHART_ଚାବି_ଶୀର୍ଷ,
	       CHART_ଚାବି_BOT,
	       CHART_ଚାବି_END
};


ସଂରଚନା chart_label {
	ଅକ୍ଷର *title; /* label title */
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା colour; /* colour */
};

ସଂରଚନା chart_series {
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା len; /* number of values in the series */
	ଦଶମିକ *ମୂଲ୍ୟ; /* array of values */
};

#define MAX_SERIES 4

ସଂରଚନା chart_data {
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା series_len;
	ସଂରଚନା chart_series series[MAX_SERIES];

	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା label_len; /* number of labels */
	ସଂରଚନା chart_label *label;

};

/**
 * parameters for a chart figure
 */
ସଂରଚନା chart_param {
	ଗଣନା chart_type type;
	ଗଣନା ଚାବି_type ଚାବି; /* what type of key to use */
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା width; /* width of figure */
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା height; /* height of figure */
	ଅକ୍ଷର *title; /* title */
	ସଂରଚନା {
		unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା x;
		unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା y;
		unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା width;
		unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା height;
	} area; /* chart area within figure */
	ସଂରଚନା chart_data data;
};

#define DEF_COLOUR_ଅଙ୍କ 8
/** default colour series */
ସ୍ଥିର unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା colour_series[DEF_COLOUR_ଅଙ୍କ] =
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
ସ୍ଥିର nserror ensure_label_ଗଣନା(ସଂରଚନା chart_param *chart, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା lidx;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା deltac;
	ସଂରଚନା chart_label *nlabels;

	deltac = ଗଣନା_2 - chart->data.label_len;
	ଯଦି (deltac <= 0) {
		/* there are enough labels */
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	nlabels = realloc(chart->data.label,
			  ଗଣନା_2 * ଆକାର(ସଂରଚନା chart_label));
	ଯଦି (nlabels == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}
	chart->data.label = nlabels;

	ପାଇଁ (lidx = chart->data.label_len; lidx < ଗଣନା_2; lidx++) {
		chart->data.label[lidx].title = calloc(1, 20);
		snprintf(chart->data.label[lidx].title, 19, "item %d", lidx + 1);
		chart->data.label[lidx].colour = colour_series[lidx % DEF_COLOUR_ଅଙ୍କ];
	}

	chart->data.label_len = ଗଣନା_2;

	ଫେରନ୍ତୁ NSERROR_OK;
}

/**
 * extract values for a series
 */
ସ୍ଥିର nserror
extract_series_ମୂଲ୍ୟଗୁଡ଼ିକ(ସଂରଚନା chart_param *chart,
		      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା series_ଅଙ୍କ,
		      ଅଚଳ ଅକ୍ଷର *valstr,
		      size_t valstrlen)
{
	nserror res;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା valcur;
	size_t valstart;/* value start in valstr */
	size_t vallen; /* value end in valstr */
	ସଂରଚନା chart_series *series;

	series = chart->data.series + series_ଅଙ୍କ;

	/* ensure we do not leak any data in this series */
	ଯଦି (series->ମୂଲ୍ୟ != ଖାଲି) {
		ମୁକ୍ତ(series->ମୂଲ୍ୟ);
	}

	/* count how many values present */
	ପାଇଁ (series->len = 1, valstart=0; valstart < valstrlen; valstart++) {
		ଯଦି (valstr[valstart] == ',') {
			series->len++;
		}
	}

	/* allocate storage for values */
	series->ମୂଲ୍ୟ = calloc(series->len, ଆକାର(ଦଶମିକ));
	ଯଦି (series->ମୂଲ୍ୟ == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	/* extract values from query string */
	ପାଇଁ (valcur = 0, vallen = 0, valstart = 0;
	     (valstart < valstrlen) && (valcur < series->len);
	     valstart += vallen, valcur++) {
		/* get query section length */
		vallen = 0;
		ଯେତେବେଳେ (((valstart + vallen) < valstrlen) &&
		       (valstr[valstart + vallen] != ',')) {
			vallen++;
		}

		series->ମୂଲ୍ୟ[valcur] = strtof(valstr + valstart, ଖାଲି);
		vallen++; /* account for , separator */
	}

	res = ensure_label_ଗଣନା(chart, series->len);

	ଫେରନ୍ତୁ res;
}


/**
 * extract values for next series
 */
ସ୍ଥିର nserror
extract_next_series_ମୂଲ୍ୟଗୁଡ଼ିକ(ସଂରଚନା chart_param *chart,
		      ଅଚଳ ଅକ୍ଷର *valstr,
		      size_t valstrlen)
{
	nserror res;

	ଯଦି (chart->data.series_len >= MAX_SERIES) {
		ଫେରନ୍ତୁ NSERROR_NOSPACE;
	}

	res = extract_series_ମୂଲ୍ୟଗୁଡ଼ିକ(chart,
				    chart->data.series_len,
				    valstr,
				    valstrlen);
	ଯଦି (res == NSERROR_OK) {
		chart->data.series_len++;
	}

	ଫେରନ୍ତୁ res;
}


/**
 * extract label title
 */
ସ୍ଥିର nserror
extract_series_labels(ସଂରଚନା chart_param *chart,
		      ଅଚଳ ଅକ୍ଷର *valstr,
		      size_t valstrlen)
{
	nserror res;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା valcount; /* count of values in valstr */
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା valcur;
	size_t valstart;/* value start in valstr */
	size_t vallen; /* value end in valstr */

	ପାଇଁ (valcount = 1, valstart=0; valstart < valstrlen; valstart++) {
		ଯଦି (valstr[valstart] == ',') {
			valcount++;
		}
	}

	res = ensure_label_ଗଣନା(chart, valcount);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ res;
	}


	ପାଇଁ (valcur = 0, vallen = 0, valstart = 0;
	     (valstart < valstrlen) && (valcur < chart->data.label_len);
	     valstart += vallen, valcur++) {
		/* get query section length */
		vallen = 0;
		ଯେତେବେଳେ (((valstart + vallen) < valstrlen) &&
		       (valstr[valstart + vallen] != ',')) {
			vallen++;
		}

		chart->data.label[valcur].title = strndup(valstr + valstart, vallen);
		vallen++; /* account for , separator */
	}
	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * extract labels colour
 */
ସ୍ଥିର nserror
extract_series_colours(ସଂରଚନା chart_param *chart,
		      ଅଚଳ ଅକ୍ଷର *valstr,
		      size_t valstrlen)
{
	ଫେରନ୍ତୁ NSERROR_OK;
}

/**
 * process a part of a query
 */
ସ୍ଥିର nserror
process_query_section(ଅଚଳ ଅକ୍ଷର *str, size_t len, ସଂରଚନା chart_param *chart)
{
	nserror res = NSERROR_OK;

	ଯଦି ((len > 6) &&
	    (strncmp(str, "width=", 6) == 0)) {
		/* figure width */
		chart->width = strtoul(str + 6, ଖାଲି, 10);
	} ଅନ୍ୟଥା ଯଦି ((len > 7) &&
	    (strncmp(str, "height=", 7) == 0)) {
		/* figure height */
		chart->height = strtoul(str + 7, ଖାଲି, 10);
	} ଅନ୍ୟଥା ଯଦି ((len > 8) &&
	    (strncmp(str, "cawidth=", 8) == 0)) {
		/* chart area width */
		chart->area.width = strtoul(str + 8, ଖାଲି, 10);
	} ଅନ୍ୟଥା ଯଦି ((len > 9) &&
	    (strncmp(str, "caheight=", 9) == 0)) {
		/* chart area height */
		chart->area.height = strtoul(str + 9, ଖାଲି, 10);
	} ଅନ୍ୟଥା ଯଦି ((len > 4) &&
	    (strncmp(str, "key=", 4) == 0)) {
		/* figure has key */
		chart->ଚାବି = strtoul(str + 4, ଖାଲି, 10);
	} ଅନ୍ୟଥା ଯଦି ((len > 6) &&
	    (strncmp(str, "title=", 6) == 0)) {
		chart->title = strndup(str + 6, len - 6);
	} ଅନ୍ୟଥା ଯଦି ((len > 5) &&
	    (strncmp(str, "type=", 5) == 0)) {
		ଯଦି (strncmp(str + 5, "pie", len - 5) == 0) {
			chart->type = CHART_TYPE_PIE;
		} ଅନ୍ୟଥା {
			chart->type = CHART_TYPE_UNKNOWN;
		}
	} ଅନ୍ୟଥା ଯଦି ((len > 7) &&
	    (strncmp(str, "values=", 7) == 0)) {
		res = extract_next_series_ମୂଲ୍ୟଗୁଡ଼ିକ(chart, str + 7, len - 7);
	} ଅନ୍ୟଥା ଯଦି ((len > 7) &&
	    (strncmp(str, "labels=", 7) == 0)) {
		res = extract_series_labels(chart, str + 7, len - 7);
	} ଅନ୍ୟଥା ଯଦି ((len > 8) &&
	    (strncmp(str, "colours=", 8) == 0)) {
		res = extract_series_colours(chart, str + 8, len - 8);
	}

	ଫେରନ୍ତୁ res;
}



ସ୍ଥିର nserror
chart_from_query(ସଂରଚନା nsurl *url, ସଂରଚନା chart_param *chart)
{
	nserror res;
	ଅକ୍ଷର *querystr;
	size_t querylen;
	size_t kvstart;/* key value start */
	size_t kvlen; /* key value end */

	res = nsurl_get(url, NSURL_QUERY, &querystr, &querylen);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ res;
	}

	ପାଇଁ (kvlen = 0, kvstart = 0; kvstart < querylen; kvstart += kvlen) {
		/* get query section length */
		kvlen = 0;
		ଯେତେବେଳେ (((kvstart + kvlen) < querylen) &&
		       (querystr[kvstart + kvlen] != '&')) {
			kvlen++;
		}

		res = process_query_section(querystr + kvstart, kvlen, chart);
		ଯଦି (res != NSERROR_OK) {
			ଭାଙ୍ଗ;
		}
		kvlen++; /* account for & separator */
	}
	ମୁକ୍ତ(querystr);

	/* sanity check dimensions */
	ଯଦି (chart->width < FIGURE_MIN_WIDTH) {
		/* bad width - check height */
		ଯଦି (chart->height < FIGURE_MIN_HEIGHT) {
			/* both bad set to defaults */
			chart->width = FIGURE_MIN_WIDTH;
			chart->height = FIGURE_MIN_HEIGHT;
		} ଅନ୍ୟଥା {
			/* base width on valid height */
			chart->width = (chart->height * 3) / 2;
		}
	} ଅନ୍ୟଥା {
		/* good width check height */
		ଯଦି (chart->height < FIGURE_MIN_HEIGHT) {
			/* base height on valid width */
			chart->height = (chart->width * 2) / 3;
		}
	}

	/* ensure legend type correct */
	ଯଦି ((chart->ଚାବି == CHART_ଚାବି_UNSET) ||
	    (chart->ଚାବି >= CHART_ଚାବି_END )) {
		/* default to putting key on right */
		chart->ଚାବି = CHART_ଚାବି_ଡାହାଣ;
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}


ସ୍ଥିର nserror
output_pie_legend(ସଂରଚନା fetch_about_context *ctx, ସଂରଚନା chart_param *chart)
{
	nserror res;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା lblidx;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା legend_width;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା legend_height;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା vertical_spacing;

	ବାଛ (chart->ଚାବି) {

	ସ୍ଥିତି CHART_ଚାବି_NONE:
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CHART_ଚାବି_ଡାହାଣ:
		legend_width = chart->width - chart->area.width - chart->area.x;
		legend_width -= 10; /* margin */
		legend_height = chart->height;
		vertical_spacing = legend_height / (chart->data.label_len + 1);

		ପାଇଁ(lblidx = 0; lblidx < chart->data.label_len ; lblidx++) {
			res = fetch_about_ssenddataf(ctx,
				"<rect  x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" fill=\"#%06x\" />",
				chart->width - legend_width,
				(vertical_spacing * lblidx) + (vertical_spacing/2),
				vertical_spacing * 2 / 3,
				vertical_spacing * 2 / 3,
				chart->data.label[lblidx].colour);
			ଯଦି (res != NSERROR_OK) {
				ଫେରନ୍ତୁ res;
			}
			res = fetch_about_ssenddataf(ctx,
				"<text x=\"%d\" y=\"%d\" fill=\"#%06x\" >%s</text>",
				chart->width - legend_width + vertical_spacing,
				vertical_spacing * (lblidx+1),
			     chart->data.label[lblidx].colour,
				chart->data.label[lblidx].title);
			ଯଦି (res != NSERROR_OK) {
				ଫେରନ୍ତୁ res;
			}
		}
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}

ସ୍ଥିର ଦଶମିକ
compute_series_total(ସଂରଚନା chart_param *chart, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା series)
{
	ଦଶମିକ total;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା curdata;

	ପାଇଁ (total = 0, curdata = 0;
	     curdata < chart->data.series[series].len;
	     curdata++) {
		total += chart->data.series[series].ମୂଲ୍ୟ[curdata];
	}
	ଫେରନ୍ତୁ total;
}

/**
 * render the data as a pie chart svg
 */
ସ୍ଥିର bool
pie_chart(ସଂରଚନା fetch_about_context *ctx, ସଂରଚନା chart_param *chart)
{
	nserror res;
	ଦଶମିକ ra; /* pie a radius */
	ଦଶମିକ rb; /* pie b radius */
	ଦଶମିକ series_total;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା curdata; /* current data point index */
	ଦଶମିକ last_x, last_y;
	ଦଶମିକ end_x, end_y;
	ଦଶମିକ start;
	ଦଶମିକ extent;
	bool large;
	ଦଶମିକ circle_centre_x, circle_centre_y;

	/* ensure there is data to render */
	ଯଦି ((chart->data.series_len < 1) || (chart->data.series[0].len < 2)) {
		ଫେରନ୍ତୁ NSERROR_BAD_PARAMETER;
	}

	/* get the first series total value */
	series_total = compute_series_total(chart, 0);
	ଯଦି (series_total == 0) {
		/* dividing by zero is embarasing */
		ଫେରନ୍ତୁ NSERROR_BAD_PARAMETER;
	}

	/*
	 * need to ensure the chart area is setup correctly
	 *
	 * this is left to each chart type as different charts
	 *  have differnt requirements
	 */
	ଯଦି ((chart->area.width == 0) || (chart->area.height == 0)) {
		/*
		 * pie chart defaults to square of smaller of figure
		 *  width and height
		 */
		ଯଦି (chart->width > chart->height) {
			chart->area.width = chart->area.height = (chart->height - chart->area.x);
		} ଅନ୍ୟଥା {
			chart->area.width = chart->area.height = (chart->width - chart->area.y);
		}
	}

	/* content is going to return ok */
	fetch_about_set_http_code(ctx, 200);

	/* content type */
	ଯଦି (fetch_about_send_header(ctx,
			"Content-Type: image/svg; charset=utf-8")) {
		ଯାଅ aborted;
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
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ aborted;
	}

	/* generate the legend */
	res = output_pie_legend(ctx, chart);
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ aborted;
	}

	/* plot the arcs */
	start = -M_PI_2;
	last_x = (ra * cos(start));
	last_y = (rb * sin(start));

	/* iterate over each data point creating a slice o pie */
	ପାଇଁ (curdata=0; curdata < chart->data.series[0].len; curdata++) {
		extent = ((chart->data.series[0].ମୂଲ୍ୟ[curdata] / series_total) * 2 * M_PI);
		end_x = (ra * cos(start + extent));
		end_y = (rb * sin(start + extent));

		ଯଦି (extent > M_PI) {
			large = true;
		} ଅନ୍ୟଥା {
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
		ଯଦି (res != NSERROR_OK) {
			ଯାଅ aborted;
		}
		last_x = end_x;
		last_y = end_y;
		start +=extent;
	}

	res = fetch_about_ssenddataf(ctx, "</svg>\n");
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ aborted;
	}

	fetch_about_send_finished(ctx);

	ଫେରନ୍ତୁ true;

 aborted:

	ଫେରନ୍ତୁ false;

}

/**
 * Handler to generate about scheme chart page.
 *
 * generates an svg chart
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_chart_handler(ସଂରଚନା fetch_about_context *ctx)
{
	nserror res;
	ସଂରଚନା chart_param chart;
	memset(&chart, 0, ଆକାର(ସଂରଚନା chart_param));

	res = chart_from_query(fetch_about_get_url(ctx), &chart);
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ aborted;
	}

	ବାଛ (chart.type) {
	ସ୍ଥିତି CHART_TYPE_PIE:
		ଫେରନ୍ତୁ pie_chart(ctx, &chart);


	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

aborted:

	ଫେରନ୍ତୁ false;

}
