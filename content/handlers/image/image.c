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

#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdlib.h>

#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/messages.h"
#அந்தர்பாவ "netsurf/plotters.h"
#அந்தர்பாவ "netsurf/bitmap.h"
#அந்தர்பாவ "netsurf/content.h"
#அந்தர்பாவ "desktop/gui_internal.h"
#அந்தர்பாவ "desktop/bitmap.h"

#அந்தர்பாவ "image/bmp.h"
#அந்தர்பாவ "image/gif.h"
#அந்தர்பாவ "image/ico.h"
#அந்தர்பாவ "image/jpeg.h"
#அந்தர்பாவ "image/jpegxl.h"
#அந்தர்பாவ "image/nssprite.h"
#அந்தர்பாவ "image/png.h"
#அந்தர்பாவ "image/rsvg.h"
#அந்தர்பாவ "image/svg.h"
#அந்தர்பாவ "image/webp.h"
#அந்தர்பாவ "image/image.h"

/**
 * Initialise image content handlers
 *
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
nserror image_init(வெறுமை)
{
	nserror error = NSERROR_OK;

#ifdef WITH_BMP
	error = nsbmp_init();
	எனில் (error != NSERROR_OK)
		திரும்பு error;
#endif

#ifdef WITH_GIF
	error = nsgif_init();
	எனில் (error != NSERROR_OK)
		திரும்பு error;
#endif

#ifdef WITH_BMP
	error = nsico_init();
	எனில் (error != NSERROR_OK)
		திரும்பு error;
#endif

#ifdef WITH_JPEG
	error = nsjpeg_init();
	எனில் (error != NSERROR_OK)
		திரும்பு error;
#endif

#ifdef WITH_JPEGXL
	error = nsjpegxl_init();
	எனில் (error != NSERROR_OK)
		திரும்பு error;
#endif

#ifdef WITH_PNG
	error = nspng_init();
	எனில் (error != NSERROR_OK)
		திரும்பு error;
#endif

#ifdef WITH_NSSPRITE
	error = nssprite_init();
	எனில் (error != NSERROR_OK)
		திரும்பு error;
#endif

	/* Prefer rsvg over libsvgtiny for svgs */
#ifdef WITH_NS_SVG
	error = svg_init();
	எனில் (error != NSERROR_OK)
		திரும்பு error;
#endif
#ifdef WITH_RSVG
	error = nsrsvg_init();
	எனில் (error != NSERROR_OK)
		திரும்பு error;
#endif

#ifdef WITH_WEBP
	error = nswebp_init();
	எனில் (error != NSERROR_OK)
		திரும்பு error;
#endif

	திரும்பு error;
}


bool image_bitmap_plot(கட்டமைப்பு bitmap *bitmap,
		       கட்டமைப்பு content_redraw_data *data,
		       மாறிலி கட்டமைப்பு rect *clip,
		       மாறிலி கட்டமைப்பு redraw_context *ctx)
{
	bitmap_flags_t flags = BITMAPF_NONE;

	முழு width;
	முழு height;
	unsigned எழுத்து *pixel;
	plot_style_t fill_style;
	கட்டமைப்பு rect area;

	width = guit->bitmap->get_width(bitmap);
	எனில் (width == 1) {
		height = guit->bitmap->get_height(bitmap);
		எனில் (height == 1) {
			/* optimise 1x1 bitmap plot */
			pixel = guit->bitmap->get_buffer(bitmap);
			fill_style.fill_colour = bitmap_pixel_to_colour(pixel);

			எனில் (guit->bitmap->get_opaque(bitmap) ||
			    ((fill_style.fill_colour & 0xff000000) == 0xff000000)) {

				area = *clip;

				எனில் (data->repeat_x != true) {
					area.x0 = data->x;
					area.x1 = data->x + data->width;
				}

				எனில் (data->repeat_y != true) {
					area.y0 = data->y;
					area.y1 = data->y + data->height;
				}

				fill_style.stroke_type = PLOT_OP_TYPE_NONE;
				fill_style.fill_type = PLOT_OP_TYPE_SOLID;

				திரும்பு (ctx->plot->rectangle(ctx,
							     &fill_style,
							     &area) == NSERROR_OK);

			} இல்லையெனில் எனில் ((fill_style.fill_colour & 0xff000000) == 0) {
				/* transparent pixel used as spacer, skip it */
				திரும்பு true;
			}
		}
	}

	/* do the plot */
	எனில் (data->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	எனில் (data->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	திரும்பு (ctx->plot->bitmap(ctx,
				  bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}
