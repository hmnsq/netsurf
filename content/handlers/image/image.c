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

#ضمن <stdbool.h>
#ضمن <stdlib.h>

#ضمن "utils/utils.h"
#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "netsurf/plotters.h"
#ضمن "netsurf/bitmap.h"
#ضمن "netsurf/content.h"
#ضمن "desktop/gui_internal.h"
#ضمن "desktop/bitmap.h"

#ضمن "image/bmp.h"
#ضمن "image/gif.h"
#ضمن "image/ico.h"
#ضمن "image/jpeg.h"
#ضمن "image/jpegxl.h"
#ضمن "image/nssprite.h"
#ضمن "image/png.h"
#ضمن "image/rsvg.h"
#ضمن "image/svg.h"
#ضمن "image/webp.h"
#ضمن "image/image.h"

/**
 * Initialise image content handlers
 *
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
nserror image_init(فراغ)
{
	nserror error = NSERROR_OK;

#ifdef WITH_BMP
	error = nsbmp_init();
	𐤀𐤌 (error != NSERROR_OK)
		أرجع error;
#endif

#ifdef WITH_GIF
	error = nsgif_init();
	𐤀𐤌 (error != NSERROR_OK)
		أرجع error;
#endif

#ifdef WITH_BMP
	error = nsico_init();
	𐤀𐤌 (error != NSERROR_OK)
		أرجع error;
#endif

#ifdef WITH_JPEG
	error = nsjpeg_init();
	𐤀𐤌 (error != NSERROR_OK)
		أرجع error;
#endif

#ifdef WITH_JPEGXL
	error = nsjpegxl_init();
	𐤀𐤌 (error != NSERROR_OK)
		أرجع error;
#endif

#ifdef WITH_PNG
	error = nspng_init();
	𐤀𐤌 (error != NSERROR_OK)
		أرجع error;
#endif

#ifdef WITH_NSSPRITE
	error = nssprite_init();
	𐤀𐤌 (error != NSERROR_OK)
		أرجع error;
#endif

	/* Prefer rsvg over libsvgtiny for svgs */
#ifdef WITH_NS_SVG
	error = svg_init();
	𐤀𐤌 (error != NSERROR_OK)
		أرجع error;
#endif
#ifdef WITH_RSVG
	error = nsrsvg_init();
	𐤀𐤌 (error != NSERROR_OK)
		أرجع error;
#endif

#ifdef WITH_WEBP
	error = nswebp_init();
	𐤀𐤌 (error != NSERROR_OK)
		أرجع error;
#endif

	أرجع error;
}


bool image_bitmap_plot(بنية bitmap *bitmap,
		       بنية content_redraw_data *data,
		       ثابت بنية rect *clip,
		       ثابت بنية redraw_context *ctx)
{
	bitmap_flags_t flags = BITMAPF_NONE;

	صحيح width;
	صحيح height;
	unsigned حرف *pixel;
	plot_style_t fill_style;
	بنية rect area;

	width = guit->bitmap->get_width(bitmap);
	𐤀𐤌 (width == 1) {
		height = guit->bitmap->get_height(bitmap);
		𐤀𐤌 (height == 1) {
			/* optimise 1x1 bitmap plot */
			pixel = guit->bitmap->get_buffer(bitmap);
			fill_style.fill_colour = bitmap_pixel_to_colour(pixel);

			𐤀𐤌 (guit->bitmap->get_opaque(bitmap) ||
			    ((fill_style.fill_colour & 0xff000000) == 0xff000000)) {

				area = *clip;

				𐤀𐤌 (data->repeat_x != true) {
					area.x0 = data->x;
					area.x1 = data->x + data->width;
				}

				𐤀𐤌 (data->repeat_y != true) {
					area.y0 = data->y;
					area.y1 = data->y + data->height;
				}

				fill_style.stroke_type = PLOT_OP_TYPE_NONE;
				fill_style.fill_type = PLOT_OP_TYPE_SOLID;

				أرجع (ctx->plot->rectangle(ctx,
							     &fill_style,
							     &area) == NSERROR_OK);

			} وإلا 𐤀𐤌 ((fill_style.fill_colour & 0xff000000) == 0) {
				/* transparent pixel used as spacer, skip it */
				أرجع true;
			}
		}
	}

	/* do the plot */
	𐤀𐤌 (data->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	𐤀𐤌 (data->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	أرجع (ctx->plot->bitmap(ctx,
				  bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}
