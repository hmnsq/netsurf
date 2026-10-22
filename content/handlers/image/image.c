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

#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>

#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/plotters.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/bitmap.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/content.h"
#ଅନ୍ତର୍ଭାଵ "desktop/gui_internal.h"
#ଅନ୍ତର୍ଭାଵ "desktop/bitmap.h"

#ଅନ୍ତର୍ଭାଵ "image/bmp.h"
#ଅନ୍ତର୍ଭାଵ "image/gif.h"
#ଅନ୍ତର୍ଭାଵ "image/ico.h"
#ଅନ୍ତର୍ଭାଵ "image/jpeg.h"
#ଅନ୍ତର୍ଭାଵ "image/jpegxl.h"
#ଅନ୍ତର୍ଭାଵ "image/nssprite.h"
#ଅନ୍ତର୍ଭାଵ "image/png.h"
#ଅନ୍ତର୍ଭାଵ "image/rsvg.h"
#ଅନ୍ତର୍ଭାଵ "image/svg.h"
#ଅନ୍ତର୍ଭାଵ "image/webp.h"
#ଅନ୍ତର୍ଭାଵ "image/image.h"

/**
 * Initialise image content handlers
 *
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
nserror image_init(ଶୂନ୍ୟ)
{
	nserror error = NSERROR_OK;

#ifdef WITH_BMP
	error = nsbmp_init();
	ଯଦି (error != NSERROR_OK)
		ଫେରନ୍ତୁ error;
#endif

#ifdef WITH_GIF
	error = nsgif_init();
	ଯଦି (error != NSERROR_OK)
		ଫେରନ୍ତୁ error;
#endif

#ifdef WITH_BMP
	error = nsico_init();
	ଯଦି (error != NSERROR_OK)
		ଫେରନ୍ତୁ error;
#endif

#ifdef WITH_JPEG
	error = nsjpeg_init();
	ଯଦି (error != NSERROR_OK)
		ଫେରନ୍ତୁ error;
#endif

#ifdef WITH_JPEGXL
	error = nsjpegxl_init();
	ଯଦି (error != NSERROR_OK)
		ଫେରନ୍ତୁ error;
#endif

#ifdef WITH_PNG
	error = nspng_init();
	ଯଦି (error != NSERROR_OK)
		ଫେରନ୍ତୁ error;
#endif

#ifdef WITH_NSSPRITE
	error = nssprite_init();
	ଯଦି (error != NSERROR_OK)
		ଫେରନ୍ତୁ error;
#endif

	/* Prefer rsvg over libsvgtiny for svgs */
#ifdef WITH_NS_SVG
	error = svg_init();
	ଯଦି (error != NSERROR_OK)
		ଫେରନ୍ତୁ error;
#endif
#ifdef WITH_RSVG
	error = nsrsvg_init();
	ଯଦି (error != NSERROR_OK)
		ଫେରନ୍ତୁ error;
#endif

#ifdef WITH_WEBP
	error = nswebp_init();
	ଯଦି (error != NSERROR_OK)
		ଫେରନ୍ତୁ error;
#endif

	ଫେରନ୍ତୁ error;
}


bool image_bitmap_plot(ସଂରଚନା bitmap *bitmap,
		       ସଂରଚନା content_redraw_data *data,
		       ଅଚଳ ସଂରଚନା rect *clip,
		       ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	bitmap_flags_t flags = BITMAPF_NONE;

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା width;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା height;
	unsigned ଅକ୍ଷର *pixel;
	plot_style_t fill_style;
	ସଂରଚନା rect area;

	width = guit->bitmap->get_width(bitmap);
	ଯଦି (width == 1) {
		height = guit->bitmap->get_height(bitmap);
		ଯଦି (height == 1) {
			/* optimise 1x1 bitmap plot */
			pixel = guit->bitmap->get_buffer(bitmap);
			fill_style.fill_colour = bitmap_pixel_to_colour(pixel);

			ଯଦି (guit->bitmap->get_opaque(bitmap) ||
			    ((fill_style.fill_colour & 0xff000000) == 0xff000000)) {

				area = *clip;

				ଯଦି (data->repeat_x != true) {
					area.x0 = data->x;
					area.x1 = data->x + data->width;
				}

				ଯଦି (data->repeat_y != true) {
					area.y0 = data->y;
					area.y1 = data->y + data->height;
				}

				fill_style.stroke_type = PLOT_OP_TYPE_NONE;
				fill_style.fill_type = PLOT_OP_TYPE_SOLID;

				ଫେରନ୍ତୁ (ctx->plot->rectangle(ctx,
							     &fill_style,
							     &area) == NSERROR_OK);

			} ଅନ୍ୟଥା ଯଦି ((fill_style.fill_colour & 0xff000000) == 0) {
				/* transparent pixel used as spacer, skip it */
				ଫେରନ୍ତୁ true;
			}
		}
	}

	/* do the plot */
	ଯଦି (data->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	ଯଦି (data->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	ଫେରନ୍ତୁ (ctx->plot->bitmap(ctx,
				  bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}
