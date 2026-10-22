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

#समावेश <stdbool.h>
#समावेश <stdlib.h>

#समावेश "utils/utils.h"
#समावेश "utils/log.h"
#समावेश "utils/messages.h"
#समावेश "netsurf/plotters.h"
#समावेश "netsurf/bitmap.h"
#समावेश "netsurf/content.h"
#समावेश "desktop/gui_internal.h"
#समावेश "desktop/bitmap.h"

#समावेश "image/bmp.h"
#समावेश "image/gif.h"
#समावेश "image/ico.h"
#समावेश "image/jpeg.h"
#समावेश "image/jpegxl.h"
#समावेश "image/nssprite.h"
#समावेश "image/png.h"
#समावेश "image/rsvg.h"
#समावेश "image/svg.h"
#समावेश "image/webp.h"
#समावेश "image/image.h"

/**
 * Initialise image content handlers
 *
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
nserror image_init(व्योम)
{
	nserror error = NSERROR_OK;

#अगर_घोषित WITH_BMP
	error = nsbmp_init();
	अगर (error != NSERROR_OK)
		वापस error;
#पूर्ण_अगर

#अगर_घोषित WITH_GIF
	error = nsgif_init();
	अगर (error != NSERROR_OK)
		वापस error;
#पूर्ण_अगर

#अगर_घोषित WITH_BMP
	error = nsico_init();
	अगर (error != NSERROR_OK)
		वापस error;
#पूर्ण_अगर

#अगर_घोषित WITH_JPEG
	error = nsjpeg_init();
	अगर (error != NSERROR_OK)
		वापस error;
#पूर्ण_अगर

#अगर_घोषित WITH_JPEGXL
	error = nsjpegxl_init();
	अगर (error != NSERROR_OK)
		वापस error;
#पूर्ण_अगर

#अगर_घोषित WITH_PNG
	error = nspng_init();
	अगर (error != NSERROR_OK)
		वापस error;
#पूर्ण_अगर

#अगर_घोषित WITH_NSSPRITE
	error = nssprite_init();
	अगर (error != NSERROR_OK)
		वापस error;
#पूर्ण_अगर

	/* Prefer rsvg over libsvgtiny for svgs */
#अगर_घोषित WITH_NS_SVG
	error = svg_init();
	अगर (error != NSERROR_OK)
		वापस error;
#पूर्ण_अगर
#अगर_घोषित WITH_RSVG
	error = nsrsvg_init();
	अगर (error != NSERROR_OK)
		वापस error;
#पूर्ण_अगर

#अगर_घोषित WITH_WEBP
	error = nswebp_init();
	अगर (error != NSERROR_OK)
		वापस error;
#पूर्ण_अगर

	वापस error;
}


bool image_bitmap_plot(काष्ठा bitmap *bitmap,
		       काष्ठा content_redraw_तथ्य *तथ्य,
		       स्थिर काष्ठा rect *clip,
		       स्थिर काष्ठा redraw_context *ctx)
{
	bitmap_flags_t flags = BITMAPF_NONE;

	पूर्णांक width;
	पूर्णांक height;
	अचिन्हित अक्षर *pixel;
	plot_style_t fill_style;
	काष्ठा rect area;

	width = guit->bitmap->get_width(bitmap);
	अगर (width == 1) {
		height = guit->bitmap->get_height(bitmap);
		अगर (height == 1) {
			/* optimise 1x1 bitmap plot */
			pixel = guit->bitmap->get_बफर(bitmap);
			fill_style.fill_colour = bitmap_pixel_to_colour(pixel);

			अगर (guit->bitmap->get_opaque(bitmap) ||
			    ((fill_style.fill_colour & 0xff000000) == 0xff000000)) {

				area = *clip;

				अगर (तथ्य->repeat_x != true) {
					area.x0 = तथ्य->x;
					area.x1 = तथ्य->x + तथ्य->width;
				}

				अगर (तथ्य->repeat_y != true) {
					area.y0 = तथ्य->y;
					area.y1 = तथ्य->y + तथ्य->height;
				}

				fill_style.stroke_type = PLOT_OP_TYPE_NONE;
				fill_style.fill_type = PLOT_OP_TYPE_SOLID;

				वापस (ctx->plot->rectangle(ctx,
							     &fill_style,
							     &area) == NSERROR_OK);

			} अन्यथा अगर ((fill_style.fill_colour & 0xff000000) == 0) {
				/* transparent pixel used as spacer, skip it */
				वापस true;
			}
		}
	}

	/* do the plot */
	अगर (तथ्य->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	अगर (तथ्य->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	वापस (ctx->plot->bitmap(ctx,
				  bitmap,
				  तथ्य->x, तथ्य->y,
				  तथ्य->width, तथ्य->height,
				  तथ्य->background_colour,
				  flags) == NSERROR_OK);
}
