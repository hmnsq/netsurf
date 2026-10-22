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

#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <stdlib.h>

#অন্তর্ভাৱ "utils/utils.h"
#অন্তর্ভাৱ "utils/log.h"
#অন্তর্ভাৱ "utils/messages.h"
#অন্তর্ভাৱ "netsurf/plotters.h"
#অন্তর্ভাৱ "netsurf/bitmap.h"
#অন্তর্ভাৱ "netsurf/content.h"
#অন্তর্ভাৱ "desktop/gui_internal.h"
#অন্তর্ভাৱ "desktop/bitmap.h"

#অন্তর্ভাৱ "image/bmp.h"
#অন্তর্ভাৱ "image/gif.h"
#অন্তর্ভাৱ "image/ico.h"
#অন্তর্ভাৱ "image/jpeg.h"
#অন্তর্ভাৱ "image/jpegxl.h"
#অন্তর্ভাৱ "image/nssprite.h"
#অন্তর্ভাৱ "image/png.h"
#অন্তর্ভাৱ "image/rsvg.h"
#অন্তর্ভাৱ "image/svg.h"
#অন্তর্ভাৱ "image/webp.h"
#অন্তর্ভাৱ "image/image.h"

/**
 * Initialise image content handlers
 *
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
nserror image_init(শূন্য)
{
	nserror error = NSERROR_OK;

#ifdef WITH_BMP
	error = nsbmp_init();
	যদি (error != NSERROR_OK)
		ঘূৰাই error;
#endif

#ifdef WITH_GIF
	error = nsgif_init();
	যদি (error != NSERROR_OK)
		ঘূৰাই error;
#endif

#ifdef WITH_BMP
	error = nsico_init();
	যদি (error != NSERROR_OK)
		ঘূৰাই error;
#endif

#ifdef WITH_JPEG
	error = nsjpeg_init();
	যদি (error != NSERROR_OK)
		ঘূৰাই error;
#endif

#ifdef WITH_JPEGXL
	error = nsjpegxl_init();
	যদি (error != NSERROR_OK)
		ঘূৰাই error;
#endif

#ifdef WITH_PNG
	error = nspng_init();
	যদি (error != NSERROR_OK)
		ঘূৰাই error;
#endif

#ifdef WITH_NSSPRITE
	error = nssprite_init();
	যদি (error != NSERROR_OK)
		ঘূৰাই error;
#endif

	/* Prefer rsvg over libsvgtiny for svgs */
#ifdef WITH_NS_SVG
	error = svg_init();
	যদি (error != NSERROR_OK)
		ঘূৰাই error;
#endif
#ifdef WITH_RSVG
	error = nsrsvg_init();
	যদি (error != NSERROR_OK)
		ঘূৰাই error;
#endif

#ifdef WITH_WEBP
	error = nswebp_init();
	যদি (error != NSERROR_OK)
		ঘূৰাই error;
#endif

	ঘূৰাই error;
}


bool image_bitmap_plot(সংরচনা bitmap *bitmap,
		       সংরচনা content_redraw_data *data,
		       ধ্রুৱ সংরচনা rect *clip,
		       ধ্রুৱ সংরচনা redraw_context *ctx)
{
	bitmap_flags_t flags = BITMAPF_NONE;

	পূর্ণাঙ্ক width;
	পূর্ণাঙ্ক height;
	unsigned ৱর্ণ *pixel;
	plot_style_t fill_style;
	সংরচনা rect area;

	width = guit->bitmap->get_width(bitmap);
	যদি (width == 1) {
		height = guit->bitmap->get_height(bitmap);
		যদি (height == 1) {
			/* optimise 1x1 bitmap plot */
			pixel = guit->bitmap->get_buffer(bitmap);
			fill_style.fill_colour = bitmap_pixel_to_colour(pixel);

			যদি (guit->bitmap->get_opaque(bitmap) ||
			    ((fill_style.fill_colour & 0xff000000) == 0xff000000)) {

				area = *clip;

				যদি (data->repeat_x != true) {
					area.x0 = data->x;
					area.x1 = data->x + data->width;
				}

				যদি (data->repeat_y != true) {
					area.y0 = data->y;
					area.y1 = data->y + data->height;
				}

				fill_style.stroke_type = PLOT_OP_TYPE_NONE;
				fill_style.fill_type = PLOT_OP_TYPE_SOLID;

				ঘূৰাই (ctx->plot->rectangle(ctx,
							     &fill_style,
							     &area) == NSERROR_OK);

			} অন্যথা যদি ((fill_style.fill_colour & 0xff000000) == 0) {
				/* transparent pixel used as spacer, skip it */
				ঘূৰাই true;
			}
		}
	}

	/* do the plot */
	যদি (data->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	যদি (data->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	ঘূৰাই (ctx->plot->bitmap(ctx,
				  bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}
