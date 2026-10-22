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

#اگر_تعریف_شده WITH_BMP
	error = nsbmp_init();
	إذا (error != NSERROR_OK)
		أرجع error;
#پایان_اگر

#اگر_تعریف_شده WITH_GIF
	error = nsgif_init();
	إذا (error != NSERROR_OK)
		أرجع error;
#پایان_اگر

#اگر_تعریف_شده WITH_BMP
	error = nsico_init();
	إذا (error != NSERROR_OK)
		أرجع error;
#پایان_اگر

#اگر_تعریف_شده WITH_JPEG
	error = nsjpeg_init();
	إذا (error != NSERROR_OK)
		أرجع error;
#پایان_اگر

#اگر_تعریف_شده WITH_JPEGXL
	error = nsjpegxl_init();
	إذا (error != NSERROR_OK)
		أرجع error;
#پایان_اگر

#اگر_تعریف_شده WITH_PNG
	error = nspng_init();
	إذا (error != NSERROR_OK)
		أرجع error;
#پایان_اگر

#اگر_تعریف_شده WITH_NSSPRITE
	error = nssprite_init();
	إذا (error != NSERROR_OK)
		أرجع error;
#پایان_اگر

	/* Prefer rsvg over libsvgtiny for svgs */
#اگر_تعریف_شده WITH_NS_SVG
	error = svg_init();
	إذا (error != NSERROR_OK)
		أرجع error;
#پایان_اگر
#اگر_تعریف_شده WITH_RSVG
	error = nsrsvg_init();
	إذا (error != NSERROR_OK)
		أرجع error;
#پایان_اگر

#اگر_تعریف_شده WITH_WEBP
	error = nswebp_init();
	إذا (error != NSERROR_OK)
		أرجع error;
#پایان_اگر

	أرجع error;
}


bool image_bitmap_plot(بنية bitmap *bitmap,
		       بنية content_redraw_داده *داده,
		       ثابت بنية rect *clip,
		       ثابت بنية redraw_context *ctx)
{
	bitmap_flags_t flags = BITMAPF_NONE;

	صحيح عرض;
	صحيح ارتفاع;
	بیعلامت حرف *pixel;
	plot_style_t fill_style;
	بنية rect area;

	عرض = guit->bitmap->get_عرض(bitmap);
	إذا (عرض == 1) {
		ارتفاع = guit->bitmap->get_ارتفاع(bitmap);
		إذا (ارتفاع == 1) {
			/* optimise 1x1 bitmap plot */
			pixel = guit->bitmap->get_میانگیر(bitmap);
			fill_style.fill_رنگ = bitmap_pixel_مقصد_رنگ(pixel);

			إذا (guit->bitmap->get_opaque(bitmap) ||
			    ((fill_style.fill_رنگ & 0xff000000) == 0xff000000)) {

				area = *clip;

				إذا (داده->repeat_x != true) {
					area.x0 = داده->x;
					area.x1 = داده->x + داده->عرض;
				}

				إذا (داده->repeat_y != true) {
					area.y0 = داده->y;
					area.y1 = داده->y + داده->ارتفاع;
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
	إذا (داده->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	إذا (داده->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	أرجع (ctx->plot->bitmap(ctx,
				  bitmap,
				  داده->x, داده->y,
				  داده->عرض, داده->ارتفاع,
				  داده->background_رنگ,
				  flags) == NSERROR_OK);
}
