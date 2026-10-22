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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/plotters.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/bitmap.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/content.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/gui_internal.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/bitmap.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "image/bmp.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "image/gif.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "image/ico.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "image/jpeg.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "image/jpegxl.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "image/nssprite.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "image/png.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "image/rsvg.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "image/svg.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "image/webp.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "image/image.h"

/**
 * Initialise image content handlers
 *
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
nserror image_init(ਖਾਲੀ)
{
	nserror error = NSERROR_OK;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WITH_BMP
	error = nsbmp_init();
	ਜੇ (error != NSERROR_OK)
		ਵਾਪਸ error;
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WITH_GIF
	error = nsgif_init();
	ਜੇ (error != NSERROR_OK)
		ਵਾਪਸ error;
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WITH_BMP
	error = nsico_init();
	ਜੇ (error != NSERROR_OK)
		ਵਾਪਸ error;
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WITH_JPEG
	error = nsjpeg_init();
	ਜੇ (error != NSERROR_OK)
		ਵਾਪਸ error;
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WITH_JPEGXL
	error = nsjpegxl_init();
	ਜੇ (error != NSERROR_OK)
		ਵਾਪਸ error;
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WITH_PNG
	error = nspng_init();
	ਜੇ (error != NSERROR_OK)
		ਵਾਪਸ error;
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WITH_NSSPRITE
	error = nssprite_init();
	ਜੇ (error != NSERROR_OK)
		ਵਾਪਸ error;
#ਜੇ_ਅੰਤ

	/* Prefer rsvg over libsvgtiny for svgs */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WITH_NS_SVG
	error = svg_init();
	ਜੇ (error != NSERROR_OK)
		ਵਾਪਸ error;
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WITH_RSVG
	error = nsrsvg_init();
	ਜੇ (error != NSERROR_OK)
		ਵਾਪਸ error;
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WITH_WEBP
	error = nswebp_init();
	ਜੇ (error != NSERROR_OK)
		ਵਾਪਸ error;
#ਜੇ_ਅੰਤ

	ਵਾਪਸ error;
}


bool image_bitmap_plot(ਬਣਤਰ bitmap *bitmap,
		       ਬਣਤਰ content_redraw_ਅੰਕੜੇ *ਅੰਕੜੇ,
		       ਅਚਲ ਬਣਤਰ rect *clip,
		       ਅਚਲ ਬਣਤਰ redraw_context *ctx)
{
	bitmap_flags_t flags = BITMAPF_NONE;

	ਪੂਰਨ ਚੌੜਾਈ;
	ਪੂਰਨ ਉਚਾਈ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *pixel;
	plot_style_t fill_style;
	ਬਣਤਰ rect area;

	ਚੌੜਾਈ = guit->bitmap->get_ਚੌੜਾਈ(bitmap);
	ਜੇ (ਚੌੜਾਈ == 1) {
		ਉਚਾਈ = guit->bitmap->get_ਉਚਾਈ(bitmap);
		ਜੇ (ਉਚਾਈ == 1) {
			/* optimise 1x1 bitmap plot */
			pixel = guit->bitmap->get_ਬਫਰ(bitmap);
			fill_style.fill_ਰੰਗ = bitmap_pixel_ਮੰਜ਼ਿਲ_ਰੰਗ(pixel);

			ਜੇ (guit->bitmap->get_opaque(bitmap) ||
			    ((fill_style.fill_ਰੰਗ & 0xff000000) == 0xff000000)) {

				area = *clip;

				ਜੇ (ਅੰਕੜੇ->repeat_x != true) {
					area.x0 = ਅੰਕੜੇ->x;
					area.x1 = ਅੰਕੜੇ->x + ਅੰਕੜੇ->ਚੌੜਾਈ;
				}

				ਜੇ (ਅੰਕੜੇ->repeat_y != true) {
					area.y0 = ਅੰਕੜੇ->y;
					area.y1 = ਅੰਕੜੇ->y + ਅੰਕੜੇ->ਉਚਾਈ;
				}

				fill_style.stroke_type = PLOT_OP_TYPE_NONE;
				fill_style.fill_type = PLOT_OP_TYPE_SOLID;

				ਵਾਪਸ (ctx->plot->rectangle(ctx,
							     &fill_style,
							     &area) == NSERROR_OK);

			} ਨਹੀਂਤਾਂ ਜੇ ((fill_style.fill_ਰੰਗ & 0xff000000) == 0) {
				/* transparent pixel used as spacer, skip it */
				ਵਾਪਸ true;
			}
		}
	}

	/* do the plot */
	ਜੇ (ਅੰਕੜੇ->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	ਜੇ (ਅੰਕੜੇ->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	ਵਾਪਸ (ctx->plot->bitmap(ctx,
				  bitmap,
				  ਅੰਕੜੇ->x, ਅੰਕੜੇ->y,
				  ਅੰਕੜੇ->ਚੌੜਾਈ, ਅੰਕੜੇ->ਉਚਾਈ,
				  ਅੰਕੜੇ->background_ਰੰਗ,
				  flags) == NSERROR_OK);
}
