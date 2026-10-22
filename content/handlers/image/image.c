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

#شامل <stdbool.h>
#شامل <stdlib.h>

#شامل "utils/utils.h"
#شامل "utils/log.h"
#شامل "utils/messages.h"
#شامل "netsurf/plotters.h"
#شامل "netsurf/bitmap.h"
#شامل "netsurf/content.h"
#شامل "desktop/gui_internal.h"
#شامل "desktop/bitmap.h"

#شامل "image/bmp.h"
#شامل "image/gif.h"
#شامل "image/ico.h"
#شامل "image/jpeg.h"
#شامل "image/jpegxl.h"
#شامل "image/nssprite.h"
#شامل "image/png.h"
#شامل "image/rsvg.h"
#شامل "image/svg.h"
#شامل "image/webp.h"
#شامل "image/image.h"

/**
 * Initialise image content handlers
 *
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
nserror image_init(تهی)
{
	nserror error = NSERROR_OK;

#ifdef WITH_BMP
	error = nsbmp_init();
	اگر (error != NSERROR_OK)
		بازگشت error;
#endif

#ifdef WITH_GIF
	error = nsgif_init();
	اگر (error != NSERROR_OK)
		بازگشت error;
#endif

#ifdef WITH_BMP
	error = nsico_init();
	اگر (error != NSERROR_OK)
		بازگشت error;
#endif

#ifdef WITH_JPEG
	error = nsjpeg_init();
	اگر (error != NSERROR_OK)
		بازگشت error;
#endif

#ifdef WITH_JPEGXL
	error = nsjpegxl_init();
	اگر (error != NSERROR_OK)
		بازگشت error;
#endif

#ifdef WITH_PNG
	error = nspng_init();
	اگر (error != NSERROR_OK)
		بازگشت error;
#endif

#ifdef WITH_NSSPRITE
	error = nssprite_init();
	اگر (error != NSERROR_OK)
		بازگشت error;
#endif

	/* Prefer rsvg over libsvgtiny for svgs */
#ifdef WITH_NS_SVG
	error = svg_init();
	اگر (error != NSERROR_OK)
		بازگشت error;
#endif
#ifdef WITH_RSVG
	error = nsrsvg_init();
	اگر (error != NSERROR_OK)
		بازگشت error;
#endif

#ifdef WITH_WEBP
	error = nswebp_init();
	اگر (error != NSERROR_OK)
		بازگشت error;
#endif

	بازگشت error;
}


bool image_bitmap_plot(ساختار bitmap *bitmap,
		       ساختار content_redraw_data *data,
		       ثابت ساختار rect *clip,
		       ثابت ساختار redraw_context *ctx)
{
	bitmap_flags_t flags = BITMAPF_NONE;

	صحیح width;
	صحیح height;
	unsigned نویسه *pixel;
	plot_style_t fill_style;
	ساختار rect area;

	width = guit->bitmap->get_width(bitmap);
	اگر (width == 1) {
		height = guit->bitmap->get_height(bitmap);
		اگر (height == 1) {
			/* optimise 1x1 bitmap plot */
			pixel = guit->bitmap->get_buffer(bitmap);
			fill_style.fill_colour = bitmap_pixel_to_colour(pixel);

			اگر (guit->bitmap->get_opaque(bitmap) ||
			    ((fill_style.fill_colour & 0xff000000) == 0xff000000)) {

				area = *clip;

				اگر (data->repeat_x != true) {
					area.x0 = data->x;
					area.x1 = data->x + data->width;
				}

				اگر (data->repeat_y != true) {
					area.y0 = data->y;
					area.y1 = data->y + data->height;
				}

				fill_style.stroke_type = PLOT_OP_TYPE_NONE;
				fill_style.fill_type = PLOT_OP_TYPE_SOLID;

				بازگشت (ctx->plot->rectangle(ctx,
							     &fill_style,
							     &area) == NSERROR_OK);

			} وگرنه اگر ((fill_style.fill_colour & 0xff000000) == 0) {
				/* transparent pixel used as spacer, skip it */
				بازگشت true;
			}
		}
	}

	/* do the plot */
	اگر (data->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	اگر (data->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	بازگشت (ctx->plot->bitmap(ctx,
				  bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}
