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

#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdlib.h>

#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/messages.h"
#അന്തര്ഭാവ "netsurf/plotters.h"
#അന്തര്ഭാവ "netsurf/bitmap.h"
#അന്തര്ഭാവ "netsurf/content.h"
#അന്തര്ഭാവ "desktop/gui_internal.h"
#അന്തര്ഭാവ "desktop/bitmap.h"

#അന്തര്ഭാവ "image/bmp.h"
#അന്തര്ഭാവ "image/gif.h"
#അന്തര്ഭാവ "image/ico.h"
#അന്തര്ഭാവ "image/jpeg.h"
#അന്തര്ഭാവ "image/jpegxl.h"
#അന്തര്ഭാവ "image/nssprite.h"
#അന്തര്ഭാവ "image/png.h"
#അന്തര്ഭാവ "image/rsvg.h"
#അന്തര്ഭാവ "image/svg.h"
#അന്തര്ഭാവ "image/webp.h"
#അന്തര്ഭാവ "image/image.h"

/**
 * Initialise image content handlers
 *
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
nserror image_init(ശൂന്യം)
{
	nserror error = NSERROR_OK;

#ifdef WITH_BMP
	error = nsbmp_init();
	എങ്കിൽ (error != NSERROR_OK)
		തിരികെ error;
#endif

#ifdef WITH_GIF
	error = nsgif_init();
	എങ്കിൽ (error != NSERROR_OK)
		തിരികെ error;
#endif

#ifdef WITH_BMP
	error = nsico_init();
	എങ്കിൽ (error != NSERROR_OK)
		തിരികെ error;
#endif

#ifdef WITH_JPEG
	error = nsjpeg_init();
	എങ്കിൽ (error != NSERROR_OK)
		തിരികെ error;
#endif

#ifdef WITH_JPEGXL
	error = nsjpegxl_init();
	എങ്കിൽ (error != NSERROR_OK)
		തിരികെ error;
#endif

#ifdef WITH_PNG
	error = nspng_init();
	എങ്കിൽ (error != NSERROR_OK)
		തിരികെ error;
#endif

#ifdef WITH_NSSPRITE
	error = nssprite_init();
	എങ്കിൽ (error != NSERROR_OK)
		തിരികെ error;
#endif

	/* Prefer rsvg over libsvgtiny for svgs */
#ifdef WITH_NS_SVG
	error = svg_init();
	എങ്കിൽ (error != NSERROR_OK)
		തിരികെ error;
#endif
#ifdef WITH_RSVG
	error = nsrsvg_init();
	എങ്കിൽ (error != NSERROR_OK)
		തിരികെ error;
#endif

#ifdef WITH_WEBP
	error = nswebp_init();
	എങ്കിൽ (error != NSERROR_OK)
		തിരികെ error;
#endif

	തിരികെ error;
}


bool image_bitmap_plot(ഘടന bitmap *bitmap,
		       ഘടന content_redraw_data *data,
		       സ്ഥിരം ഘടന rect *clip,
		       സ്ഥിരം ഘടന redraw_context *ctx)
{
	bitmap_flags_t flags = BITMAPF_NONE;

	പൂർണ്ണസംഖ്യ width;
	പൂർണ്ണസംഖ്യ height;
	unsigned അക്ഷരം *pixel;
	plot_style_t fill_style;
	ഘടന rect area;

	width = guit->bitmap->get_width(bitmap);
	എങ്കിൽ (width == 1) {
		height = guit->bitmap->get_height(bitmap);
		എങ്കിൽ (height == 1) {
			/* optimise 1x1 bitmap plot */
			pixel = guit->bitmap->get_buffer(bitmap);
			fill_style.fill_colour = bitmap_pixel_to_colour(pixel);

			എങ്കിൽ (guit->bitmap->get_opaque(bitmap) ||
			    ((fill_style.fill_colour & 0xff000000) == 0xff000000)) {

				area = *clip;

				എങ്കിൽ (data->repeat_x != true) {
					area.x0 = data->x;
					area.x1 = data->x + data->width;
				}

				എങ്കിൽ (data->repeat_y != true) {
					area.y0 = data->y;
					area.y1 = data->y + data->height;
				}

				fill_style.stroke_type = PLOT_OP_TYPE_NONE;
				fill_style.fill_type = PLOT_OP_TYPE_SOLID;

				തിരികെ (ctx->plot->rectangle(ctx,
							     &fill_style,
							     &area) == NSERROR_OK);

			} അല്ലെങ്കിൽ എങ്കിൽ ((fill_style.fill_colour & 0xff000000) == 0) {
				/* transparent pixel used as spacer, skip it */
				തിരികെ true;
			}
		}
	}

	/* do the plot */
	എങ്കിൽ (data->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	എങ്കിൽ (data->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	തിരികെ (ctx->plot->bitmap(ctx,
				  bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}
