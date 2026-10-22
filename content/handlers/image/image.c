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

#جے_تعریف_شدہ WITH_BMP
	error = nsbmp_init();
	إذا (error != NSERROR_OK)
		أرجع error;
#جے_انت

#جے_تعریف_شدہ WITH_GIF
	error = nsgif_init();
	إذا (error != NSERROR_OK)
		أرجع error;
#جے_انت

#جے_تعریف_شدہ WITH_BMP
	error = nsico_init();
	إذا (error != NSERROR_OK)
		أرجع error;
#جے_انت

#جے_تعریف_شدہ WITH_JPEG
	error = nsjpeg_init();
	إذا (error != NSERROR_OK)
		أرجع error;
#جے_انت

#جے_تعریف_شدہ WITH_JPEGXL
	error = nsjpegxl_init();
	إذا (error != NSERROR_OK)
		أرجع error;
#جے_انت

#جے_تعریف_شدہ WITH_PNG
	error = nspng_init();
	إذا (error != NSERROR_OK)
		أرجع error;
#جے_انت

#جے_تعریف_شدہ WITH_NSSPRITE
	error = nssprite_init();
	إذا (error != NSERROR_OK)
		أرجع error;
#جے_انت

	/* Prefer rsvg over libsvgtiny for svgs */
#جے_تعریف_شدہ WITH_NS_SVG
	error = svg_init();
	إذا (error != NSERROR_OK)
		أرجع error;
#جے_انت
#جے_تعریف_شدہ WITH_RSVG
	error = nsrsvg_init();
	إذا (error != NSERROR_OK)
		أرجع error;
#جے_انت

#جے_تعریف_شدہ WITH_WEBP
	error = nswebp_init();
	إذا (error != NSERROR_OK)
		أرجع error;
#جے_انت

	أرجع error;
}


bool image_bitmap_plot(بنية bitmap *bitmap,
		       بنية content_redraw_انکڑے *انکڑے,
		       ثابت بنية rect *clip,
		       ثابت بنية redraw_context *ctx)
{
	bitmap_flags_t flags = BITMAPF_NONE;

	صحيح چوڑائی;
	صحيح اچائی;
	بنانشانی حرف *pixel;
	plot_style_t fill_style;
	بنية rect area;

	چوڑائی = guit->bitmap->get_چوڑائی(bitmap);
	إذا (چوڑائی == 1) {
		اچائی = guit->bitmap->get_اچائی(bitmap);
		إذا (اچائی == 1) {
			/* optimise 1x1 bitmap plot */
			pixel = guit->bitmap->get_بفر(bitmap);
			fill_style.fill_رنگ = bitmap_pixel_منزل_رنگ(pixel);

			إذا (guit->bitmap->get_opaque(bitmap) ||
			    ((fill_style.fill_رنگ & 0xff000000) == 0xff000000)) {

				area = *clip;

				إذا (انکڑے->repeat_x != true) {
					area.x0 = انکڑے->x;
					area.x1 = انکڑے->x + انکڑے->چوڑائی;
				}

				إذا (انکڑے->repeat_y != true) {
					area.y0 = انکڑے->y;
					area.y1 = انکڑے->y + انکڑے->اچائی;
				}

				fill_style.stroke_type = PLOT_OP_TYPE_NONE;
				fill_style.fill_type = PLOT_OP_TYPE_SOLID;

				أرجع (ctx->plot->rectangle(ctx,
							     &fill_style,
							     &area) == NSERROR_OK);

			} وإلا إذا ((fill_style.fill_رنگ & 0xff000000) == 0) {
				/* transparent pixel used as spacer, skip it */
				أرجع true;
			}
		}
	}

	/* do the plot */
	إذا (انکڑے->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	إذا (انکڑے->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	أرجع (ctx->plot->bitmap(ctx,
				  bitmap,
				  انکڑے->x, انکڑے->y,
				  انکڑے->چوڑائی, انکڑے->اچائی,
				  انکڑے->background_رنگ,
				  flags) == NSERROR_OK);
}
