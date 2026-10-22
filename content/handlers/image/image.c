/*
 * Copyright 2011 John-Mark Bell <jmb@netsurf-browser.org>
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

#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdlib.h>

#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/messages.h"
#అన్తర్భావ "netsurf/plotters.h"
#అన్తర్భావ "netsurf/bitmap.h"
#అన్తర్భావ "netsurf/content.h"
#అన్తర్భావ "desktop/gui_internal.h"
#అన్తర్భావ "desktop/bitmap.h"

#అన్తర్భావ "image/bmp.h"
#అన్తర్భావ "image/gif.h"
#అన్తర్భావ "image/ico.h"
#అన్తర్భావ "image/jpeg.h"
#అన్తర్భావ "image/jpegxl.h"
#అన్తర్భావ "image/nssprite.h"
#అన్తర్భావ "image/png.h"
#అన్తర్భావ "image/rsvg.h"
#అన్తర్భావ "image/svg.h"
#అన్తర్భావ "image/webp.h"
#అన్తర్భావ "image/image.h"

/**
 * Initialise image content handlers
 *
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
nserror image_init(శూన్య)
{
	nserror error = NSERROR_OK;

#నిర్వచితమైతే WITH_BMP
	error = nsbmp_init();
	ఒకవేళ (error != NSERROR_OK)
		తిరిగి error;
#అయితే_ముగింపు

#నిర్వచితమైతే WITH_GIF
	error = nsgif_init();
	ఒకవేళ (error != NSERROR_OK)
		తిరిగి error;
#అయితే_ముగింపు

#నిర్వచితమైతే WITH_BMP
	error = nsico_init();
	ఒకవేళ (error != NSERROR_OK)
		తిరిగి error;
#అయితే_ముగింపు

#నిర్వచితమైతే WITH_JPEG
	error = nsjpeg_init();
	ఒకవేళ (error != NSERROR_OK)
		తిరిగి error;
#అయితే_ముగింపు

#నిర్వచితమైతే WITH_JPEGXL
	error = nsjpegxl_init();
	ఒకవేళ (error != NSERROR_OK)
		తిరిగి error;
#అయితే_ముగింపు

#నిర్వచితమైతే WITH_PNG
	error = nspng_init();
	ఒకవేళ (error != NSERROR_OK)
		తిరిగి error;
#అయితే_ముగింపు

#నిర్వచితమైతే WITH_NSSPRITE
	error = nssprite_init();
	ఒకవేళ (error != NSERROR_OK)
		తిరిగి error;
#అయితే_ముగింపు

	/* Prefer rsvg over libsvgtiny for svgs */
#నిర్వచితమైతే WITH_NS_SVG
	error = svg_init();
	ఒకవేళ (error != NSERROR_OK)
		తిరిగి error;
#అయితే_ముగింపు
#నిర్వచితమైతే WITH_RSVG
	error = nsrsvg_init();
	ఒకవేళ (error != NSERROR_OK)
		తిరిగి error;
#అయితే_ముగింపు

#నిర్వచితమైతే WITH_WEBP
	error = nswebp_init();
	ఒకవేళ (error != NSERROR_OK)
		తిరిగి error;
#అయితే_ముగింపు

	తిరిగి error;
}


bool image_bitmap_plot(నిర్మాణం bitmap *bitmap,
		       నిర్మాణం content_redraw_దత్తాంశం *దత్తాంశం,
		       స్థిరరాశి నిర్మాణం rect *clip,
		       స్థిరరాశి నిర్మాణం redraw_context *ctx)
{
	bitmap_flags_t flags = BITMAPF_NONE;

	పూర్ణాంకం వెడల్పు;
	పూర్ణాంకం ఎత్తు;
	గుర్తులేని అక్షరం *pixel;
	plot_style_t fill_style;
	నిర్మాణం rect area;

	వెడల్పు = guit->bitmap->get_వెడల్పు(bitmap);
	ఒకవేళ (వెడల్పు == 1) {
		ఎత్తు = guit->bitmap->get_ఎత్తు(bitmap);
		ఒకవేళ (ఎత్తు == 1) {
			/* optimise 1x1 bitmap plot */
			pixel = guit->bitmap->get_బఫరు(bitmap);
			fill_style.fill_రంగు = bitmap_pixel_గమ్యం_రంగు(pixel);

			ఒకవేళ (guit->bitmap->get_opaque(bitmap) ||
			    ((fill_style.fill_రంగు & 0xff000000) == 0xff000000)) {

				area = *clip;

				ఒకవేళ (దత్తాంశం->repeat_x != true) {
					area.x0 = దత్తాంశం->x;
					area.x1 = దత్తాంశం->x + దత్తాంశం->వెడల్పు;
				}

				ఒకవేళ (దత్తాంశం->repeat_y != true) {
					area.y0 = దత్తాంశం->y;
					area.y1 = దత్తాంశం->y + దత్తాంశం->ఎత్తు;
				}

				fill_style.stroke_type = PLOT_OP_TYPE_NONE;
				fill_style.fill_type = PLOT_OP_TYPE_SOLID;

				తిరిగి (ctx->plot->rectangle(ctx,
							     &fill_style,
							     &area) == NSERROR_OK);

			} లేకపోతే ఒకవేళ ((fill_style.fill_రంగు & 0xff000000) == 0) {
				/* transparent pixel used as spacer, skip it */
				తిరిగి true;
			}
		}
	}

	/* do the plot */
	ఒకవేళ (దత్తాంశం->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	ఒకవేళ (దత్తాంశం->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	తిరిగి (ctx->plot->bitmap(ctx,
				  bitmap,
				  దత్తాంశం->x, దత్తాంశం->y,
				  దత్తాంశం->వెడల్పు, దత్తాంశం->ఎత్తు,
				  దత్తాంశం->background_రంగు,
				  flags) == NSERROR_OK);
}
