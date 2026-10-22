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

#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdlib.h>

#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "netsurf/plotters.h"
#अन्तर्भाव "netsurf/bitmap.h"
#अन्तर्भाव "netsurf/content.h"
#अन्तर्भाव "desktop/gui_internal.h"
#अन्तर्भाव "desktop/bitmap.h"

#अन्तर्भाव "image/bmp.h"
#अन्तर्भाव "image/gif.h"
#अन्तर्भाव "image/ico.h"
#अन्तर्भाव "image/jpeg.h"
#अन्तर्भाव "image/jpegxl.h"
#अन्तर्भाव "image/nssprite.h"
#अन्तर्भाव "image/png.h"
#अन्तर्भाव "image/rsvg.h"
#अन्तर्भाव "image/svg.h"
#अन्तर्भाव "image/webp.h"
#अन्तर्भाव "image/image.h"

/**
 * Initialise image content handlers
 *
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
nserror image_init(शून्य)
{
	nserror error = NSERROR_OK;

#ifdef WITH_BMP
	error = nsbmp_init();
	सचे (error != NSERROR_OK)
		पटिनिवत्त error;
#endif

#ifdef WITH_GIF
	error = nsgif_init();
	सचे (error != NSERROR_OK)
		पटिनिवत्त error;
#endif

#ifdef WITH_BMP
	error = nsico_init();
	सचे (error != NSERROR_OK)
		पटिनिवत्त error;
#endif

#ifdef WITH_JPEG
	error = nsjpeg_init();
	सचे (error != NSERROR_OK)
		पटिनिवत्त error;
#endif

#ifdef WITH_JPEGXL
	error = nsjpegxl_init();
	सचे (error != NSERROR_OK)
		पटिनिवत्त error;
#endif

#ifdef WITH_PNG
	error = nspng_init();
	सचे (error != NSERROR_OK)
		पटिनिवत्त error;
#endif

#ifdef WITH_NSSPRITE
	error = nssprite_init();
	सचे (error != NSERROR_OK)
		पटिनिवत्त error;
#endif

	/* Prefer rsvg over libsvgtiny for svgs */
#ifdef WITH_NS_SVG
	error = svg_init();
	सचे (error != NSERROR_OK)
		पटिनिवत्त error;
#endif
#ifdef WITH_RSVG
	error = nsrsvg_init();
	सचे (error != NSERROR_OK)
		पटिनिवत्त error;
#endif

#ifdef WITH_WEBP
	error = nswebp_init();
	सचे (error != NSERROR_OK)
		पटिनिवत्त error;
#endif

	पटिनिवत्त error;
}


bool image_bitmap_plot(संरचना bitmap *bitmap,
		       संरचना content_redraw_data *data,
		       ध्रुव संरचना rect *clip,
		       ध्रुव संरचना redraw_context *ctx)
{
	bitmap_flags_t flags = BITMAPF_NONE;

	पूर्णाङ्क width;
	पूर्णाङ्क height;
	unsigned वर्ण *pixel;
	plot_style_t fill_style;
	संरचना rect area;

	width = guit->bitmap->get_width(bitmap);
	सचे (width == 1) {
		height = guit->bitmap->get_height(bitmap);
		सचे (height == 1) {
			/* optimise 1x1 bitmap plot */
			pixel = guit->bitmap->get_buffer(bitmap);
			fill_style.fill_colour = bitmap_pixel_to_colour(pixel);

			सचे (guit->bitmap->get_opaque(bitmap) ||
			    ((fill_style.fill_colour & 0xff000000) == 0xff000000)) {

				area = *clip;

				सचे (data->repeat_x != true) {
					area.x0 = data->x;
					area.x1 = data->x + data->width;
				}

				सचे (data->repeat_y != true) {
					area.y0 = data->y;
					area.y1 = data->y + data->height;
				}

				fill_style.stroke_type = PLOT_OP_TYPE_NONE;
				fill_style.fill_type = PLOT_OP_TYPE_SOLID;

				पटिनिवत्त (ctx->plot->rectangle(ctx,
							     &fill_style,
							     &area) == NSERROR_OK);

			} अञ्ञथा सचे ((fill_style.fill_colour & 0xff000000) == 0) {
				/* transparent pixel used as spacer, skip it */
				पटिनिवत्त true;
			}
		}
	}

	/* do the plot */
	सचे (data->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	सचे (data->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	पटिनिवत्त (ctx->plot->bitmap(ctx,
				  bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}
