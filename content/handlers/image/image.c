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

#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdlib.h>

#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/messages.h"
#અન્તર્ભાવ "netsurf/plotters.h"
#અન્તર્ભાવ "netsurf/bitmap.h"
#અન્તર્ભાવ "netsurf/content.h"
#અન્તર્ભાવ "desktop/gui_internal.h"
#અન્તર્ભાવ "desktop/bitmap.h"

#અન્તર્ભાવ "image/bmp.h"
#અન્તર્ભાવ "image/gif.h"
#અન્તર્ભાવ "image/ico.h"
#અન્તર્ભાવ "image/jpeg.h"
#અન્તર્ભાવ "image/jpegxl.h"
#અન્તર્ભાવ "image/nssprite.h"
#અન્તર્ભાવ "image/png.h"
#અન્તર્ભાવ "image/rsvg.h"
#અન્તર્ભાવ "image/svg.h"
#અન્તર્ભાવ "image/webp.h"
#અન્તર્ભાવ "image/image.h"

/**
 * Initialise image content handlers
 *
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
nserror image_init(શૂન્ય)
{
	nserror error = NSERROR_OK;

#ifdef WITH_BMP
	error = nsbmp_init();
	જો (error != NSERROR_OK)
		પાછા error;
#endif

#ifdef WITH_GIF
	error = nsgif_init();
	જો (error != NSERROR_OK)
		પાછા error;
#endif

#ifdef WITH_BMP
	error = nsico_init();
	જો (error != NSERROR_OK)
		પાછા error;
#endif

#ifdef WITH_JPEG
	error = nsjpeg_init();
	જો (error != NSERROR_OK)
		પાછા error;
#endif

#ifdef WITH_JPEGXL
	error = nsjpegxl_init();
	જો (error != NSERROR_OK)
		પાછા error;
#endif

#ifdef WITH_PNG
	error = nspng_init();
	જો (error != NSERROR_OK)
		પાછા error;
#endif

#ifdef WITH_NSSPRITE
	error = nssprite_init();
	જો (error != NSERROR_OK)
		પાછા error;
#endif

	/* Prefer rsvg over libsvgtiny for svgs */
#ifdef WITH_NS_SVG
	error = svg_init();
	જો (error != NSERROR_OK)
		પાછા error;
#endif
#ifdef WITH_RSVG
	error = nsrsvg_init();
	જો (error != NSERROR_OK)
		પાછા error;
#endif

#ifdef WITH_WEBP
	error = nswebp_init();
	જો (error != NSERROR_OK)
		પાછા error;
#endif

	પાછા error;
}


bool image_bitmap_plot(સંરચના bitmap *bitmap,
		       સંરચના content_redraw_data *data,
		       અચલ સંરચના rect *clip,
		       અચલ સંરચના redraw_context *ctx)
{
	bitmap_flags_t flags = BITMAPF_NONE;

	પૂર્ણાંક width;
	પૂર્ણાંક height;
	unsigned અક્ષર *pixel;
	plot_style_t fill_style;
	સંરચના rect area;

	width = guit->bitmap->get_width(bitmap);
	જો (width == 1) {
		height = guit->bitmap->get_height(bitmap);
		જો (height == 1) {
			/* optimise 1x1 bitmap plot */
			pixel = guit->bitmap->get_buffer(bitmap);
			fill_style.fill_colour = bitmap_pixel_to_colour(pixel);

			જો (guit->bitmap->get_opaque(bitmap) ||
			    ((fill_style.fill_colour & 0xff000000) == 0xff000000)) {

				area = *clip;

				જો (data->repeat_x != true) {
					area.x0 = data->x;
					area.x1 = data->x + data->width;
				}

				જો (data->repeat_y != true) {
					area.y0 = data->y;
					area.y1 = data->y + data->height;
				}

				fill_style.stroke_type = PLOT_OP_TYPE_NONE;
				fill_style.fill_type = PLOT_OP_TYPE_SOLID;

				પાછા (ctx->plot->rectangle(ctx,
							     &fill_style,
							     &area) == NSERROR_OK);

			} નહીંતર જો ((fill_style.fill_colour & 0xff000000) == 0) {
				/* transparent pixel used as spacer, skip it */
				પાછા true;
			}
		}
	}

	/* do the plot */
	જો (data->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	જો (data->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	પાછા (ctx->plot->bitmap(ctx,
				  bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}
