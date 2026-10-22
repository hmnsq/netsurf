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

#شامل <stdbool.h>
#شامل <stdlib.h>

#شامل "utils/utils.h"
#شامل "utils/log.h"
#شامل "utils/messages.h"
#شامل "netsurf/plotters.h"
#شامل "netsurf/bitmap.h"
#شامل "netsurf/content.h"
#شامل "desktop/gui_internal.h"
#شامل "desktop/bitmap.h"

#شامل "image/bmp.h"
#شامل "image/gif.h"
#شامل "image/ico.h"
#شامل "image/jpeg.h"
#شامل "image/jpegxl.h"
#شامل "image/nssprite.h"
#شامل "image/png.h"
#شامل "image/rsvg.h"
#شامل "image/svg.h"
#شامل "image/webp.h"
#شامل "image/image.h"

/**
 * Initialise image content handlers
 *
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
nserror image_init(خالی)
{
	nserror error = NSERROR_OK;

#اگر_تعریف_شدہ WITH_BMP
	error = nsbmp_init();
	اگر (error != NSERROR_OK)
		واپس error;
#اگر_ختم

#اگر_تعریف_شدہ WITH_GIF
	error = nsgif_init();
	اگر (error != NSERROR_OK)
		واپس error;
#اگر_ختم

#اگر_تعریف_شدہ WITH_BMP
	error = nsico_init();
	اگر (error != NSERROR_OK)
		واپس error;
#اگر_ختم

#اگر_تعریف_شدہ WITH_JPEG
	error = nsjpeg_init();
	اگر (error != NSERROR_OK)
		واپس error;
#اگر_ختم

#اگر_تعریف_شدہ WITH_JPEGXL
	error = nsjpegxl_init();
	اگر (error != NSERROR_OK)
		واپس error;
#اگر_ختم

#اگر_تعریف_شدہ WITH_PNG
	error = nspng_init();
	اگر (error != NSERROR_OK)
		واپس error;
#اگر_ختم

#اگر_تعریف_شدہ WITH_NSSPRITE
	error = nssprite_init();
	اگر (error != NSERROR_OK)
		واپس error;
#اگر_ختم

	/* Prefer rsvg over libsvgtiny for svgs */
#اگر_تعریف_شدہ WITH_NS_SVG
	error = svg_init();
	اگر (error != NSERROR_OK)
		واپس error;
#اگر_ختم
#اگر_تعریف_شدہ WITH_RSVG
	error = nsrsvg_init();
	اگر (error != NSERROR_OK)
		واپس error;
#اگر_ختم

#اگر_تعریف_شدہ WITH_WEBP
	error = nswebp_init();
	اگر (error != NSERROR_OK)
		واپس error;
#اگر_ختم

	واپس error;
}


bool image_bitmap_plot(ساخت bitmap *bitmap,
		       ساخت content_redraw_ڈیٹا *ڈیٹا,
		       ثابت ساخت rect *clip,
		       ثابت ساخت redraw_context *ctx)
{
	bitmap_flags_t flags = BITMAPF_NONE;

	صحیح چوڑائی;
	صحیح اونچائی;
	غیرعلامتی حرف *pixel;
	plot_style_t fill_style;
	ساخت rect area;

	چوڑائی = guit->bitmap->get_چوڑائی(bitmap);
	اگر (چوڑائی == 1) {
		اونچائی = guit->bitmap->get_اونچائی(bitmap);
		اگر (اونچائی == 1) {
			/* optimise 1x1 bitmap plot */
			pixel = guit->bitmap->get_بفر(bitmap);
			fill_style.fill_رنگ = bitmap_pixel_مقصد_رنگ(pixel);

			اگر (guit->bitmap->get_opaque(bitmap) ||
			    ((fill_style.fill_رنگ & 0xff000000) == 0xff000000)) {

				area = *clip;

				اگر (ڈیٹا->repeat_x != true) {
					area.x0 = ڈیٹا->x;
					area.x1 = ڈیٹا->x + ڈیٹا->چوڑائی;
				}

				اگر (ڈیٹا->repeat_y != true) {
					area.y0 = ڈیٹا->y;
					area.y1 = ڈیٹا->y + ڈیٹا->اونچائی;
				}

				fill_style.stroke_type = PLOT_OP_TYPE_NONE;
				fill_style.fill_type = PLOT_OP_TYPE_SOLID;

				واپس (ctx->plot->rectangle(ctx,
							     &fill_style,
							     &area) == NSERROR_OK);

			} ورنہ اگر ((fill_style.fill_رنگ & 0xff000000) == 0) {
				/* transparent pixel used as spacer, skip it */
				واپس true;
			}
		}
	}

	/* do the plot */
	اگر (ڈیٹا->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	اگر (ڈیٹا->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	واپس (ctx->plot->bitmap(ctx,
				  bitmap,
				  ڈیٹا->x, ڈیٹا->y,
				  ڈیٹا->چوڑائی, ڈیٹا->اونچائی,
				  ڈیٹا->background_رنگ,
				  flags) == NSERROR_OK);
}
