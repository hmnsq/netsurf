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

#यदि_परिभाषितम् WITH_BMP
	error = nsbmp_init();
	यदि (error != NSERROR_OK)
		प्रत्यागम error;
#यदि_अन्तः

#यदि_परिभाषितम् WITH_GIF
	error = nsgif_init();
	यदि (error != NSERROR_OK)
		प्रत्यागम error;
#यदि_अन्तः

#यदि_परिभाषितम् WITH_BMP
	error = nsico_init();
	यदि (error != NSERROR_OK)
		प्रत्यागम error;
#यदि_अन्तः

#यदि_परिभाषितम् WITH_JPEG
	error = nsjpeg_init();
	यदि (error != NSERROR_OK)
		प्रत्यागम error;
#यदि_अन्तः

#यदि_परिभाषितम् WITH_JPEGXL
	error = nsjpegxl_init();
	यदि (error != NSERROR_OK)
		प्रत्यागम error;
#यदि_अन्तः

#यदि_परिभाषितम् WITH_PNG
	error = nspng_init();
	यदि (error != NSERROR_OK)
		प्रत्यागम error;
#यदि_अन्तः

#यदि_परिभाषितम् WITH_NSSPRITE
	error = nssprite_init();
	यदि (error != NSERROR_OK)
		प्रत्यागम error;
#यदि_अन्तः

	/* Prefer rsvg over libsvgtiny for svgs */
#यदि_परिभाषितम् WITH_NS_SVG
	error = svg_init();
	यदि (error != NSERROR_OK)
		प्रत्यागम error;
#यदि_अन्तः
#यदि_परिभाषितम् WITH_RSVG
	error = nsrsvg_init();
	यदि (error != NSERROR_OK)
		प्रत्यागम error;
#यदि_अन्तः

#यदि_परिभाषितम् WITH_WEBP
	error = nswebp_init();
	यदि (error != NSERROR_OK)
		प्रत्यागम error;
#यदि_अन्तः

	प्रत्यागम error;
}


bool image_bitmap_plot(संरचना bitmap *bitmap,
		       संरचना content_redraw_दत्त *दत्त,
		       ध्रुव संरचना rect *clip,
		       ध्रुव संरचना redraw_context *ctx)
{
	bitmap_flags_t flags = BITMAPF_NONE;

	पूर्णाङ्क विस्तारः;
	पूर्णाङ्क height;
	अचिह्नित वर्ण *pixel;
	plot_style_t fill_style;
	संरचना rect area;

	विस्तारः = guit->bitmap->get_विस्तारः(bitmap);
	यदि (विस्तारः == 1) {
		height = guit->bitmap->get_height(bitmap);
		यदि (height == 1) {
			/* optimise 1x1 bitmap plot */
			pixel = guit->bitmap->get_बफर(bitmap);
			fill_style.fill_रङ्गः = bitmap_pixel_गन्तव्यम्_रङ्गः(pixel);

			यदि (guit->bitmap->get_opaque(bitmap) ||
			    ((fill_style.fill_रङ्गः & 0xff000000) == 0xff000000)) {

				area = *clip;

				यदि (दत्त->repeat_x != true) {
					area.x0 = दत्त->x;
					area.x1 = दत्त->x + दत्त->विस्तारः;
				}

				यदि (दत्त->repeat_y != true) {
					area.y0 = दत्त->y;
					area.y1 = दत्त->y + दत्त->height;
				}

				fill_style.stroke_type = PLOT_OP_TYPE_NONE;
				fill_style.fill_type = PLOT_OP_TYPE_SOLID;

				प्रत्यागम (ctx->plot->rectangle(ctx,
							     &fill_style,
							     &area) == NSERROR_OK);

			} अन्यथा यदि ((fill_style.fill_रङ्गः & 0xff000000) == 0) {
				/* transparent pixel used as spacer, skip it */
				प्रत्यागम true;
			}
		}
	}

	/* do the plot */
	यदि (दत्त->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	यदि (दत्त->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	प्रत्यागम (ctx->plot->bitmap(ctx,
				  bitmap,
				  दत्त->x, दत्त->y,
				  दत्त->विस्तारः, दत्त->height,
				  दत्त->background_रङ्गः,
				  flags) == NSERROR_OK);
}
