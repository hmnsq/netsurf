/*
 * Copyright 2004 John M Bell <jmb202@ecs.soton.ac.uk>
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

/**
 * \file
 * Interface to HTML imagemap
 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NETSURF_HTML_IMAGEMAP_H
#ವ್ಯಾಖ್ಯಾನಿಸು NETSURF_HTML_IMAGEMAP_H

#ಅನ್ತರ್ಭಾವ <dom/dom.h>

ರಚನೆ html_content;
ರಚನೆ hlcache_handle;
ರಚನೆ nsurl;

ಶೂನ್ಯ imagemap_destroy(ರಚನೆ html_content *c);
ಶೂನ್ಯ imagemap_dump(ರಚನೆ html_content *c);
nserror imagemap_ಹೊರತೆಗೆ(ರಚನೆ html_content *c);

ರಚನೆ nsurl *imagemap_get(ರಚನೆ html_content *c, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೀಲಿ,
		ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ x, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ y,
		ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ click_x, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ click_y,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **ಗುರಿ);

#ಆದರೆ_ಅಂತ್ಯ
