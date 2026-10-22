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

/** \file
 * Initialisation/finalisation of image handlers.
 */

#اگر_غیر_تعریف_شدہ NETSURF_IMAGE_IMAGE_H_
#تعریف_کرو NETSURF_IMAGE_IMAGE_H_

#شامل "utils/errors.h"

ساخت content_redraw_ڈیٹا;

/** Initialise the content handlers for image types.
 */
nserror image_init(خالی);

/** Common image content handler bitmap plot call.
 *
 * This plots the specified bitmap controlled by the redraw context
 * and specific content redraw data. It is a helper specifically
 * provided for image content handlers redraw callback.
 */
bool image_bitmap_plot(ساخت bitmap *bitmap,
		       ساخت content_redraw_ڈیٹا *ڈیٹا, 
		       ثابت ساخت rect *clip,
		       ثابت ساخت redraw_context *ctx);

#اگر_ختم
