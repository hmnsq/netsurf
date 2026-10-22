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

#include <stdbool.h>
#include <stdlib.h>

#include "utils/utils.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "netsurf/plotters.h"
#include "netsurf/bitmap.h"
#include "netsurf/content.h"
#include "desktop/gui_internal.h"
#include "desktop/bitmap.h"

#include "image/bmp.h"
#include "image/gif.h"
#include "image/ico.h"
#include "image/jpeg.h"
#include "image/jpegxl.h"
#include "image/nssprite.h"
#include "image/png.h"
#include "image/rsvg.h"
#include "image/svg.h"
#include "image/webp.h"
#include "image/image.h"

/**
 * Initialise image content handlers
 *
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
nserror image_init(void)
{
	nserror error = NSERROR_OK;

#ifdef WITH_BMP
	error = nsbmp_init();
	ᯀᯮᯝ (error != NSERROR_OK)
		ᯀᯮᯞ error;
#endif

#ifdef WITH_GIF
	error = nsgif_init();
	ᯀᯮᯝ (error != NSERROR_OK)
		ᯀᯮᯞ error;
#endif

#ifdef WITH_BMP
	error = nsico_init();
	ᯀᯮᯝ (error != NSERROR_OK)
		ᯀᯮᯞ error;
#endif

#ifdef WITH_JPEG
	error = nsjpeg_init();
	ᯀᯮᯝ (error != NSERROR_OK)
		ᯀᯮᯞ error;
#endif

#ifdef WITH_JPEGXL
	error = nsjpegxl_init();
	ᯀᯮᯝ (error != NSERROR_OK)
		ᯀᯮᯞ error;
#endif

#ifdef WITH_PNG
	error = nspng_init();
	ᯀᯮᯝ (error != NSERROR_OK)
		ᯀᯮᯞ error;
#endif

#ifdef WITH_NSSPRITE
	error = nssprite_init();
	ᯀᯮᯝ (error != NSERROR_OK)
		ᯀᯮᯞ error;
#endif

	/* Prefer rsvg over libsvgtiny for svgs */
#ifdef WITH_NS_SVG
	error = svg_init();
	ᯀᯮᯝ (error != NSERROR_OK)
		ᯀᯮᯞ error;
#endif
#ifdef WITH_RSVG
	error = nsrsvg_init();
	ᯀᯮᯝ (error != NSERROR_OK)
		ᯀᯮᯞ error;
#endif

#ifdef WITH_WEBP
	error = nswebp_init();
	ᯀᯮᯝ (error != NSERROR_OK)
		ᯀᯮᯞ error;
#endif

	ᯀᯮᯞ error;
}


bool image_bitmap_plot(sa_nracanaa bitmap *bitmap,
		       sa_nracanaa content_redraw_data *data,
		       const sa_nracanaa rect *clip,
		       const sa_nracanaa redraw_context *ctx)
{
	bitmap_flags_t flags = BITMAPF_NONE;

	puur_naa_nka width;
	puur_naa_nka height;
	unsigned var_na *pixel;
	plot_style_t fill_style;
	sa_nracanaa rect area;

	width = guit->bitmap->get_width(bitmap);
	ᯀᯮᯝ (width == 1) {
		height = guit->bitmap->get_height(bitmap);
		ᯀᯮᯝ (height == 1) {
			/* optimise 1x1 bitmap plot */
			pixel = guit->bitmap->get_buffer(bitmap);
			fill_style.fill_colour = bitmap_pixel_to_colour(pixel);

			ᯀᯮᯝ (guit->bitmap->get_opaque(bitmap) ||
			    ((fill_style.fill_colour & 0xff000000) == 0xff000000)) {

				area = *clip;

				ᯀᯮᯝ (data->repeat_x != true) {
					area.x0 = data->x;
					area.x1 = data->x + data->width;
				}

				ᯀᯮᯝ (data->repeat_y != true) {
					area.y0 = data->y;
					area.y1 = data->y + data->height;
				}

				fill_style.stroke_type = PLOT_OP_TYPE_NONE;
				fill_style.fill_type = PLOT_OP_TYPE_SOLID;

				ᯀᯮᯞ (ctx->plot->rectangle(ctx,
							     &fill_style,
							     &area) == NSERROR_OK);

			} ᯀᯮᯝᯂᯪ ᯀᯮᯝ ((fill_style.fill_colour & 0xff000000) == 0) {
				/* transparent pixel used as spacer, skip it */
				ᯀᯮᯞ true;
			}
		}
	}

	/* do the plot */
	ᯀᯮᯝ (data->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	ᯀᯮᯝ (data->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	ᯀᯮᯞ (ctx->plot->bitmap(ctx,
				  bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}
