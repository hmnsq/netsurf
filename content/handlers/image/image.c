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

#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdlib.h>

#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/messages.h"
#অন্তর্ভাব "netsurf/plotters.h"
#অন্তর্ভাব "netsurf/bitmap.h"
#অন্তর্ভাব "netsurf/content.h"
#অন্তর্ভাব "desktop/gui_internal.h"
#অন্তর্ভাব "desktop/bitmap.h"

#অন্তর্ভাব "image/bmp.h"
#অন্তর্ভাব "image/gif.h"
#অন্তর্ভাব "image/ico.h"
#অন্তর্ভাব "image/jpeg.h"
#অন্তর্ভাব "image/jpegxl.h"
#অন্তর্ভাব "image/nssprite.h"
#অন্তর্ভাব "image/png.h"
#অন্তর্ভাব "image/rsvg.h"
#অন্তর্ভাব "image/svg.h"
#অন্তর্ভাব "image/webp.h"
#অন্তর্ভাব "image/image.h"

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
		ফেরত error;
#endif

#ifdef WITH_GIF
	error = nsgif_init();
	যদি (error != NSERROR_OK)
		ফেরত error;
#endif

#ifdef WITH_BMP
	error = nsico_init();
	যদি (error != NSERROR_OK)
		ফেরত error;
#endif

#ifdef WITH_JPEG
	error = nsjpeg_init();
	যদি (error != NSERROR_OK)
		ফেরত error;
#endif

#ifdef WITH_JPEGXL
	error = nsjpegxl_init();
	যদি (error != NSERROR_OK)
		ফেরত error;
#endif

#ifdef WITH_PNG
	error = nspng_init();
	যদি (error != NSERROR_OK)
		ফেরত error;
#endif

#ifdef WITH_NSSPRITE
	error = nssprite_init();
	যদি (error != NSERROR_OK)
		ফেরত error;
#endif

	/* Prefer rsvg over libsvgtiny for svgs */
#ifdef WITH_NS_SVG
	error = svg_init();
	যদি (error != NSERROR_OK)
		ফেরত error;
#endif
#ifdef WITH_RSVG
	error = nsrsvg_init();
	যদি (error != NSERROR_OK)
		ফেরত error;
#endif

#ifdef WITH_WEBP
	error = nswebp_init();
	যদি (error != NSERROR_OK)
		ফেরত error;
#endif

	ফেরত error;
}


bool image_bitmap_plot(কাঠামো bitmap *bitmap,
		       কাঠামো content_redraw_তথ্য *তথ্য,
		       ধ্রুবক কাঠামো rect *clip,
		       ধ্রুবক কাঠামো redraw_context *ctx)
{
	bitmap_flags_t flags = BITMAPF_NONE;

	পূর্ণসংখ্যা width;
	পূর্ণসংখ্যা height;
	unsigned অক্ষর *pixel;
	plot_style_t fill_style;
	কাঠামো rect area;

	width = guit->bitmap->get_width(bitmap);
	যদি (width == 1) {
		height = guit->bitmap->get_height(bitmap);
		যদি (height == 1) {
			/* optimise 1x1 bitmap plot */
			pixel = guit->bitmap->get_বাফার(bitmap);
			fill_style.fill_colour = bitmap_pixel_to_colour(pixel);

			যদি (guit->bitmap->get_opaque(bitmap) ||
			    ((fill_style.fill_colour & 0xff000000) == 0xff000000)) {

				area = *clip;

				যদি (তথ্য->repeat_x != true) {
					area.x0 = তথ্য->x;
					area.x1 = তথ্য->x + তথ্য->width;
				}

				যদি (তথ্য->repeat_y != true) {
					area.y0 = তথ্য->y;
					area.y1 = তথ্য->y + তথ্য->height;
				}

				fill_style.stroke_type = PLOT_OP_TYPE_NONE;
				fill_style.fill_type = PLOT_OP_TYPE_SOLID;

				ফেরত (ctx->plot->rectangle(ctx,
							     &fill_style,
							     &area) == NSERROR_OK);

			} অন্যথা যদি ((fill_style.fill_colour & 0xff000000) == 0) {
				/* transparent pixel used as spacer, skip it */
				ফেরত true;
			}
		}
	}

	/* do the plot */
	যদি (তথ্য->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	যদি (তথ্য->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	ফেরত (ctx->plot->bitmap(ctx,
				  bitmap,
				  তথ্য->x, তথ্য->y,
				  তথ্য->width, তথ্য->height,
				  তথ্য->background_colour,
				  flags) == NSERROR_OK);
}
