/*
 * Copyright 2022 Michael Drake <tlsa@netsurf-browser.org>
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
 * HTML layout implementation: display: flex.
 *
 * Layout is carried out in two stages:
 *
 * 1. + calculation of minimum / maximum box widths, and
 *    + determination of whether block level boxes will have >zero height
 *
 * 2. + layout (position and dimensions)
 *
 * In most cases the functions for the two stages are a corresponding pair
 * layout_minmax_X() and layout_X().
 */

#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"

#ಅನ್ತರ್ಭಾವ "html/box.h"
#ಅನ್ತರ್ಭಾವ "html/html.h"
#ಅನ್ತರ್ಭಾವ "html/private.h"
#ಅನ್ತರ್ಭಾವ "html/box_inspect.h"
#ಅನ್ತರ್ಭಾವ "html/layout_internal.h"

/**
 * Flex item data
 */
ರಚನೆ flex_ಅಂಶ_ದತ್ತಾಂಶ {
	ಎಣಿಕೆ css_flex_basis_e basis;
	css_fixed basis_ಉದ್ದಳತೆ;
	css_unit basis_unit;
	ರಚನೆ box *box;

	css_fixed shrink;
	css_fixed grow;

	ಪೂರ್ಣಾಂಕ min_main;
	ಪೂರ್ಣಾಂಕ max_main;
	ಪೂರ್ಣಾಂಕ min_cross;
	ಪೂರ್ಣಾಂಕ max_cross;

	ಪೂರ್ಣಾಂಕ ಗುರಿ_main_ಗಾತ್ರ;
	ಪೂರ್ಣಾಂಕ ತಳ_ಗಾತ್ರ;
	ಪೂರ್ಣಾಂಕ main_ಗಾತ್ರ;
	size_t ಸಾಲು;

	bool freeze;
	bool min_violation;
	bool max_violation;
};

/**
 * Flex line data
 */
ರಚನೆ flex_ಸಾಲು_ದತ್ತಾಂಶ {
	ಪೂರ್ಣಾಂಕ main_ಗಾತ್ರ;
	ಪೂರ್ಣಾಂಕ cross_ಗಾತ್ರ;

	ಪೂರ್ಣಾಂಕ ಬಳಸಿದ_main_ಗಾತ್ರ;
	ಪೂರ್ಣಾಂಕ main_auto_margin_ಎಣಿಕೆ;

	ಪೂರ್ಣಾಂಕ pos;

	size_t ಮೊದಲ;
	size_t ಎಣಿಕೆ_2;
	size_t frozen;
};

/**
 * Flex layout context
 */
ರಚನೆ flex_ctx {
	html_content *content;
	ಸ್ಥಿರಾಂಕ ರಚನೆ box *flex;
	ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx;

	ಪೂರ್ಣಾಂಕ main_ಗಾತ್ರ;
	ಪೂರ್ಣಾಂಕ cross_ಗಾತ್ರ;

	ಪೂರ್ಣಾಂಕ ಲಭ್ಯ_main;
	ಪೂರ್ಣಾಂಕ ಲಭ್ಯ_cross;

	bool horizontal;
	bool main_reversed;
	ಎಣಿಕೆ css_flex_wrap_e wrap;

	ರಚನೆ flex_ವಸ್ತುಗಳು {
		size_t ಎಣಿಕೆ_2;
		ರಚನೆ flex_ಅಂಶ_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ;
	} ಅಂಶ;

	ರಚನೆ flex_ಸಾಲುಗಳು {
		size_t ಎಣಿಕೆ_2;
		size_t alloc;
		ರಚನೆ flex_ಸಾಲು_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ;
	} ಸಾಲು;
};

/**
 * Destroy a flex layout context
 *
 * \param[in] ctx  Flex layout context
 */
ಸ್ಥಿರ ಶೂನ್ಯ layout_flex_ctx__destroy(ರಚನೆ flex_ctx *ctx)
{
	ಆದರೆ (ctx != ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(ctx->ಅಂಶ.ದತ್ತಾಂಶ);
		ಬಿಡುಗಡೆ(ctx->ಸಾಲು.ದತ್ತಾಂಶ);
		ಬಿಡುಗಡೆ(ctx);
	}
}

/**
 * Create a flex layout context
 *
 * \param[in] content  HTML content containing flex box
 * \param[in] flex     Box to create layout context for
 * \return flex layout context or NULL on error
 */
ಸ್ಥಿರ ರಚನೆ flex_ctx *layout_flex_ctx__create(
		html_content *content,
		ಸ್ಥಿರಾಂಕ ರಚನೆ box *flex)
{
	ರಚನೆ flex_ctx *ctx;

	ctx = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(*ctx));
	ಆದರೆ (ctx == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}
	ctx->ಸಾಲು.alloc = 1;

	ctx->ಅಂಶ.ಎಣಿಕೆ_2 = box_ಎಣಿಕೆ_children(flex);
	ctx->ಅಂಶ.ದತ್ತಾಂಶ = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(ctx->ಅಂಶ.ಎಣಿಕೆ_2, ಗಾತ್ರ(*ctx->ಅಂಶ.ದತ್ತಾಂಶ));
	ಆದರೆ (ctx->ಅಂಶ.ದತ್ತಾಂಶ == ಖಾಲಿ) {
		layout_flex_ctx__destroy(ctx);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ctx->ಸಾಲು.alloc = 1;
	ctx->ಸಾಲು.ದತ್ತಾಂಶ = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(ctx->ಸಾಲು.alloc, ಗಾತ್ರ(*ctx->ಸಾಲು.ದತ್ತಾಂಶ));
	ಆದರೆ (ctx->ಸಾಲು.ದತ್ತಾಂಶ == ಖಾಲಿ) {
		layout_flex_ctx__destroy(ctx);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ctx->flex = flex;
	ctx->content = content;
	ctx->unit_len_ctx = &content->unit_len_ctx;

	ctx->wrap = css_computed_flex_wrap(flex->style);
	ctx->horizontal = lh__flex_main_is_horizontal(flex);
	ctx->main_reversed = lh__flex_direction_reversed(flex);

	ಹಿಂತಿರುಗಿ ctx;
}

/**
 * Find box side representing the start of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the start side.
 */
ಸ್ಥಿರ ಎಣಿಕೆ box_side layout_flex_main_ಆರಂಭ_side(
		ಸ್ಥಿರಾಂಕ ರಚನೆ flex_ctx *ctx)
{
	ಆದರೆ (ctx->horizontal) {
		ಹಿಂತಿರುಗಿ (ctx->main_reversed) ? ಬಲ_2 : ಎಡ_2;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಹಿಂತಿರುಗಿ (ctx->main_reversed) ? BOTTOM : ತುದಿ_2;
	}
}

/**
 * Find box side representing the end of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the end side.
 */
ಸ್ಥಿರ ಎಣಿಕೆ box_side layout_flex_main_ಕೊನೆ_side(
		ಸ್ಥಿರಾಂಕ ರಚನೆ flex_ctx *ctx)
{
	ಆದರೆ (ctx->horizontal) {
		ಹಿಂತಿರುಗಿ (ctx->main_reversed) ? ಎಡ_2 : ಬಲ_2;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಹಿಂತಿರುಗಿ (ctx->main_reversed) ? ತುದಿ_2 : BOTTOM;
	}
}

/**
 * Perform layout on a flex item
 *
 * \param[in] ctx              Flex layout context
 * \param[in] item             Item to lay out
 * \param[in] available_width  Available width for item in pixels
 * \return true on success false on failure
 */
ಸ್ಥಿರ bool layout_flex_ಅಂಶ(
		ಸ್ಥಿರಾಂಕ ರಚನೆ flex_ctx *ctx,
		ಸ್ಥಿರಾಂಕ ರಚನೆ flex_ಅಂಶ_ದತ್ತಾಂಶ *ಅಂಶ,
		ಪೂರ್ಣಾಂಕ ಲಭ್ಯ_ಅಗಲ)
{
	bool success;
	ರಚನೆ box *b = ಅಂಶ->box;

	ಆಯ್ಕೆ (b->type) {
	ಸ್ಥಿತಿ BOX_ಖಂಡ:
		success = layout_ಖಂಡ_context(b, -1, ctx->content);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ:
		b->float_container = b->ಪೋಷಕ;
		success = layout_ಕೋಷ್ಟಕ(b, ಲಭ್ಯ_ಅಗಲ, ctx->content);
		b->float_container = ಖಾಲಿ;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ BOX_FLEX:
		b->float_container = b->ಪೋಷಕ;
		success = layout_flex(b, ಲಭ್ಯ_ಅಗಲ, ctx->content);
		b->float_container = ಖಾಲಿ;
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಪ್ರತಿಜ್ಞಾ(0 && "Bad flex item back type");
		success = false;
		ನಿಲ್ಲಿಸು;
	}

	ಆದರೆ (!success) {
		NSLOG(flex, ERROR, "box %p: layout failed", b);
	}

	ಹಿಂತಿರುಗಿ success;
}

/**
 * Calculate an item's base and target main sizes.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] item             Item to get sizes of
 * \param[in] available_width  Available width in pixels
 * \return true on success false on failure
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool layout_flex_ತಳ_and_main_sizes(
		ಸ್ಥಿರಾಂಕ ರಚನೆ flex_ctx *ctx,
		ರಚನೆ flex_ಅಂಶ_ದತ್ತಾಂಶ *ಅಂಶ,
		ಪೂರ್ಣಾಂಕ ಲಭ್ಯ_ಅಗಲ)
{
	ರಚನೆ box *b = ಅಂಶ->box;
	ಪೂರ್ಣಾಂಕ content_min_ಅಗಲ = b->min_ಅಗಲ;
	ಪೂರ್ಣಾಂಕ content_max_ಅಗಲ = b->max_ಅಗಲ;
	ಪೂರ್ಣಾಂಕ delta_outer_main = lh__delta_outer_main(ctx->flex, b);

	NSLOG(flex, DEEPDEBUG, "box %p: delta_outer_main: %i",
			b, delta_outer_main);

	ಆದರೆ (ಅಂಶ->basis == CSS_FLEX_BASIS_ಗಣ) {
		ಆದರೆ (ಅಂಶ->basis_unit == CSS_UNIT_PCT) {
			ಅಂಶ->ತಳ_ಗಾತ್ರ = FPCT_ದ_INT_TOINT(
					ಅಂಶ->basis_ಉದ್ದಳತೆ,
					ಲಭ್ಯ_ಅಗಲ);
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಅಂಶ->ತಳ_ಗಾತ್ರ = FIXTOINT(css_unit_len2device_px(
					b->style, ctx->unit_len_ctx,
					ಅಂಶ->basis_ಉದ್ದಳತೆ,
					ಅಂಶ->basis_unit));
		}

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಅಂಶ->basis == CSS_FLEX_BASIS_AUTO) {
		ಅಂಶ->ತಳ_ಗಾತ್ರ = ctx->horizontal ? b->ಅಗಲ : b->ಎತ್ತರ;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಅಂಶ->ತಳ_ಗಾತ್ರ = AUTO;
	}

	ಆದರೆ (ctx->horizontal == false) {
		ಆದರೆ (b->ಅಗಲ == AUTO) {
			b->ಅಗಲ = min(max(content_min_ಅಗಲ, ಲಭ್ಯ_ಅಗಲ),
					content_max_ಅಗಲ);
			b->ಅಗಲ -= lh_delta_outer_ಅಗಲ(b);
		}

		ಆದರೆ (!layout_flex_ಅಂಶ(ctx, ಅಂಶ, b->ಅಗಲ)) {
			ಹಿಂತಿರುಗಿ false;
		}
	}

	ಆದರೆ (ಅಂಶ->ತಳ_ಗಾತ್ರ == AUTO) {
		ಆದರೆ (ctx->horizontal == false) {
			ಅಂಶ->ತಳ_ಗಾತ್ರ = b->ಎತ್ತರ;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಅಂಶ->ತಳ_ಗಾತ್ರ = content_max_ಅಗಲ - delta_outer_main;
		}
	}

	ಅಂಶ->ತಳ_ಗಾತ್ರ += delta_outer_main;

	ಆದರೆ (ctx->horizontal) {
		ಅಂಶ->ತಳ_ಗಾತ್ರ = min(ಅಂಶ->ತಳ_ಗಾತ್ರ, ಲಭ್ಯ_ಅಗಲ);
		ಅಂಶ->ತಳ_ಗಾತ್ರ = max(ಅಂಶ->ತಳ_ಗಾತ್ರ, content_min_ಅಗಲ);
	}

	ಅಂಶ->ಗುರಿ_main_ಗಾತ್ರ = ಅಂಶ->ತಳ_ಗಾತ್ರ;
	ಅಂಶ->main_ಗಾತ್ರ = ಅಂಶ->ತಳ_ಗಾತ್ರ;

	ಆದರೆ (ಅಂಶ->max_main > 0 &&
	    ಅಂಶ->main_ಗಾತ್ರ > ಅಂಶ->max_main + delta_outer_main) {
		ಅಂಶ->main_ಗಾತ್ರ = ಅಂಶ->max_main + delta_outer_main;
	}

	ಆದರೆ (ಅಂಶ->main_ಗಾತ್ರ < ಅಂಶ->min_main + delta_outer_main) {
		ಅಂಶ->main_ಗಾತ್ರ = ಅಂಶ->min_main + delta_outer_main;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: base_size: %i, main_size %i",
			b, ಅಂಶ->ತಳ_ಗಾತ್ರ, ಅಂಶ->main_ಗಾತ್ರ);

	ಹಿಂತಿರುಗಿ true;
}

/**
 * Fill out all item's data in a flex container.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] flex             Flex box
 * \param[in] available_width  Available width in pixels
 */
ಸ್ಥಿರ ಶೂನ್ಯ layout_flex_ctx_populate_ಅಂಶ_ದತ್ತಾಂಶ(
		ಸ್ಥಿರಾಂಕ ರಚನೆ flex_ctx *ctx,
		ಸ್ಥಿರಾಂಕ ರಚನೆ box *flex,
		ಪೂರ್ಣಾಂಕ ಲಭ್ಯ_ಅಗಲ)
{
	size_t i = 0;
	bool horizontal = ctx->horizontal;

	ಗಾಗಿ (ರಚನೆ box *b = flex->children; b != ಖಾಲಿ; b = b->ಮುಂದಿನ) {
		ರಚನೆ flex_ಅಂಶ_ದತ್ತಾಂಶ *ಅಂಶ = &ctx->ಅಂಶ.ದತ್ತಾಂಶ[i++];

		b->float_container = b->ಪೋಷಕ;
		layout_ಕಂಡುಹಿಡಿ_dimensions(ctx->unit_len_ctx, ಲಭ್ಯ_ಅಗಲ, -1,
				b, b->style, &b->ಅಗಲ, &b->ಎತ್ತರ,
				horizontal ? &ಅಂಶ->max_main : &ಅಂಶ->max_cross,
				horizontal ? &ಅಂಶ->min_main : &ಅಂಶ->min_cross,
				horizontal ? &ಅಂಶ->max_cross : &ಅಂಶ->max_main,
				horizontal ? &ಅಂಶ->min_cross : &ಅಂಶ->min_main,
				b->margin, b->padding, b->border);
		b->float_container = ಖಾಲಿ;

		NSLOG(flex, DEEPDEBUG, "flex-item box: %p: width: %i",
				b, b->ಅಗಲ);

		ಅಂಶ->box = b;
		ಅಂಶ->basis = css_computed_flex_basis(b->style,
				&ಅಂಶ->basis_ಉದ್ದಳತೆ, &ಅಂಶ->basis_unit);

		css_computed_flex_shrink(b->style, &ಅಂಶ->shrink);
		css_computed_flex_grow(b->style, &ಅಂಶ->grow);

		layout_flex_ತಳ_and_main_sizes(ctx, ಅಂಶ, ಲಭ್ಯ_ಅಗಲ);
	}
}

/**
 * Ensure context's lines array has a free space
 *
 * \param[in] ctx  Flex layout context
 * \return true on success false on out of memory
 */
ಸ್ಥಿರ bool layout_flex_ctx_ensure_ಸಾಲು(ರಚನೆ flex_ctx *ctx)
{
	ರಚನೆ flex_ಸಾಲು_ದತ್ತಾಂಶ *ತಾತ್ಕಾಲಿಕ;
	size_t ಸಾಲು_alloc = ctx->ಸಾಲು.alloc * 2;

	ಆದರೆ (ctx->ಸಾಲು.alloc > ctx->ಸಾಲು.ಎಣಿಕೆ_2) {
		ಹಿಂತಿರುಗಿ true;
	}

	ತಾತ್ಕಾಲಿಕ = realloc(ctx->ಸಾಲು.ದತ್ತಾಂಶ, ಗಾತ್ರ(*ctx->ಸಾಲು.ದತ್ತಾಂಶ) * ಸಾಲು_alloc);
	ಆದರೆ (ತಾತ್ಕಾಲಿಕ == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ false;
	}
	ctx->ಸಾಲು.ದತ್ತಾಂಶ = ತಾತ್ಕಾಲಿಕ;

	memset(ctx->ಸಾಲು.ದತ್ತಾಂಶ + ctx->ಸಾಲು.alloc, 0,
	       ಗಾತ್ರ(*ctx->ಸಾಲು.ದತ್ತಾಂಶ) * (ಸಾಲು_alloc - ctx->ಸಾಲು.alloc));
	ctx->ಸಾಲು.alloc = ಸಾಲು_alloc;

	ಹಿಂತಿರುಗಿ true;
}

/**
 * Assigns flex items to the line and returns the line
 *
 * \param[in] ctx         Flex layout context
 * \param[in] item_index  Index to first item to assign to this line
 * \return Pointer to the new line, or NULL on error.
 */
ಸ್ಥಿರ ರಚನೆ flex_ಸಾಲು_ದತ್ತಾಂಶ *layout_flex_build_ಸಾಲು(ರಚನೆ flex_ctx *ctx,
		size_t ಅಂಶ_ಸೂಚ್ಯಂಕ)
{
	ಎಣಿಕೆ box_side ಆರಂಭ_side = layout_flex_main_ಆರಂಭ_side(ctx);
	ಎಣಿಕೆ box_side ಕೊನೆ_side = layout_flex_main_ಕೊನೆ_side(ctx);
	ರಚನೆ flex_ಸಾಲು_ದತ್ತಾಂಶ *ಸಾಲು;
	ಪೂರ್ಣಾಂಕ ಬಳಸಿದ_main = 0;

	ಆದರೆ (!layout_flex_ctx_ensure_ಸಾಲು(ctx)) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಸಾಲು = &ctx->ಸಾಲು.ದತ್ತಾಂಶ[ctx->ಸಾಲು.ಎಣಿಕೆ_2];
	ಸಾಲು->ಮೊದಲ = ಅಂಶ_ಸೂಚ್ಯಂಕ;

	NSLOG(flex, DEEPDEBUG, "flex container %p: available main: %i",
			ctx->flex, ctx->ಲಭ್ಯ_main);

	ಆಗ (ಅಂಶ_ಸೂಚ್ಯಂಕ < ctx->ಅಂಶ.ಎಣಿಕೆ_2) {
		ರಚನೆ flex_ಅಂಶ_ದತ್ತಾಂಶ *ಅಂಶ = &ctx->ಅಂಶ.ದತ್ತಾಂಶ[ಅಂಶ_ಸೂಚ್ಯಂಕ];
		ರಚನೆ box *b = ಅಂಶ->box;
		ಪೂರ್ಣಾಂಕ pos_main;

		pos_main = ctx->horizontal ?
				ಅಂಶ->main_ಗಾತ್ರ :
				b->ಎತ್ತರ + lh__delta_outer_main(ctx->flex, b);

		ಆದರೆ (ctx->wrap == CSS_FLEX_WRAP_NOWRAP ||
		    pos_main + ಬಳಸಿದ_main <= ctx->ಲಭ್ಯ_main ||
		    lh__box_is_absolute(ಅಂಶ->box) ||
		    ctx->ಲಭ್ಯ_main == AUTO ||
		    ಸಾಲು->ಎಣಿಕೆ_2 == 0 ||
		    pos_main == 0) {
			ಆದರೆ (lh__box_is_absolute(ಅಂಶ->box) == false) {
				ಸಾಲು->main_ಗಾತ್ರ += ಅಂಶ->main_ಗಾತ್ರ;
				ಬಳಸಿದ_main += pos_main;

				ಆದರೆ (b->margin[ಆರಂಭ_side] == AUTO) {
					ಸಾಲು->main_auto_margin_ಎಣಿಕೆ++;
				}
				ಆದರೆ (b->margin[ಕೊನೆ_side] == AUTO) {
					ಸಾಲು->main_auto_margin_ಎಣಿಕೆ++;
				}
			}
			ಅಂಶ->ಸಾಲು = ctx->ಸಾಲು.ಎಣಿಕೆ_2;
			ಸಾಲು->ಎಣಿಕೆ_2++;
			ಅಂಶ_ಸೂಚ್ಯಂಕ++;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ನಿಲ್ಲಿಸು;
		}
	}

	ಆದರೆ (ಸಾಲು->ಎಣಿಕೆ_2 > 0) {
		ctx->ಸಾಲು.ಎಣಿಕೆ_2++;
	} ಇಲ್ಲದಿದ್ದರೆ {
		NSLOG(layout, ERROR, "Failed to fit any flex items");
	}

	ಹಿಂತಿರುಗಿ ಸಾಲು;
}

/**
 * Freeze an item on a line
 *
 * \param[in] line  Line to containing item
 * \param[in] item  Item to freeze
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ layout_flex_ಅಂಶ_freeze(
		ರಚನೆ flex_ಸಾಲು_ದತ್ತಾಂಶ *ಸಾಲು,
		ರಚನೆ flex_ಅಂಶ_ದತ್ತಾಂಶ *ಅಂಶ)
{
	ಅಂಶ->freeze = true;
	ಸಾಲು->frozen++;

	ಆದರೆ (!lh__box_is_absolute(ಅಂಶ->box)){
		ಸಾಲು->ಬಳಸಿದ_main_ಗಾತ್ರ += ಅಂಶ->ಗುರಿ_main_ಗಾತ್ರ;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: "
			"Frozen at target_main_size: %i",
			ಅಂಶ->box, ಅಂಶ->ಗುರಿ_main_ಗಾತ್ರ);
}

/**
 * Calculate remaining free space and unfrozen item factor sum
 *
 * \param[in]  ctx                  Flex layout context
 * \param[in]  line                 Line to calculate free space on
 * \param[out] unfrozen_factor_sum  Returns sum of unfrozen item's flex factors
 * \param[in]  initial_free_main    Initial free space in main direction
 * \param[in]  available_main       Available space in main direction
 * \param[in]  grow                 Whether to grow or shrink
 * return remaining free space on line
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ layout_flex_ಉಳಿದ_free_main(
		ರಚನೆ flex_ctx *ctx,
		ರಚನೆ flex_ಸಾಲು_ದತ್ತಾಂಶ *ಸಾಲು,
		css_fixed *unfrozen_ಗುಣಕ_ಮೊತ್ತ,
		ಪೂರ್ಣಾಂಕ initial_free_main,
		ಪೂರ್ಣಾಂಕ ಲಭ್ಯ_main,
		bool grow)
{
	ಪೂರ್ಣಾಂಕ ಉಳಿದ_free_main = ಲಭ್ಯ_main;
	size_t ಅಂಶ_ಎಣಿಕೆ = ಸಾಲು->ಮೊದಲ + ಸಾಲು->ಎಣಿಕೆ_2;

	*unfrozen_ಗುಣಕ_ಮೊತ್ತ = 0;

	ಗಾಗಿ (size_t i = ಸಾಲು->ಮೊದಲ; i < ಅಂಶ_ಎಣಿಕೆ; i++) {
		ರಚನೆ flex_ಅಂಶ_ದತ್ತಾಂಶ *ಅಂಶ = &ctx->ಅಂಶ.ದತ್ತಾಂಶ[i];

		ಆದರೆ (ಅಂಶ->freeze) {
			ಉಳಿದ_free_main -= ಅಂಶ->ಗುರಿ_main_ಗಾತ್ರ;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಉಳಿದ_free_main -= ಅಂಶ->ತಳ_ಗಾತ್ರ;

			*unfrozen_ಗುಣಕ_ಮೊತ್ತ += grow ?
					ಅಂಶ->grow : ಅಂಶ->shrink;
		}
	}

	ಆದರೆ (*unfrozen_ಗುಣಕ_ಮೊತ್ತ < F_1) {
		ಪೂರ್ಣಾಂಕ free_space = FIXTOINT(FMUL(INTTOFIX(initial_free_main),
				*unfrozen_ಗುಣಕ_ಮೊತ್ತ));

		ಆದರೆ (free_space < ಉಳಿದ_free_main) {
			ಉಳಿದ_free_main = free_space;
		}
	}

	NSLOG(flex, DEEPDEBUG, "Remaining free space: %i",
			ಉಳಿದ_free_main);

	ಹಿಂತಿರುಗಿ ಉಳಿದ_free_main;
}

/**
 * Clamp flex item target main size and get min/max violations
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to align items on
 * return total violation in pixels
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ layout_flex__get_min_max_violations(
		ರಚನೆ flex_ctx *ctx,
		ರಚನೆ flex_ಸಾಲು_ದತ್ತಾಂಶ *ಸಾಲು)
{

	ಪೂರ್ಣಾಂಕ ಒಟ್ಟು_violation = 0;
	size_t ಅಂಶ_ಎಣಿಕೆ = ಸಾಲು->ಮೊದಲ + ಸಾಲು->ಎಣಿಕೆ_2;

	ಗಾಗಿ (size_t i = ಸಾಲು->ಮೊದಲ; i < ಅಂಶ_ಎಣಿಕೆ; i++) {
		ರಚನೆ flex_ಅಂಶ_ದತ್ತಾಂಶ *ಅಂಶ = &ctx->ಅಂಶ.ದತ್ತಾಂಶ[i];
		ಪೂರ್ಣಾಂಕ ಗುರಿ_main_ಗಾತ್ರ = ಅಂಶ->ಗುರಿ_main_ಗಾತ್ರ;

		NSLOG(flex, DEEPDEBUG, "item %p: target_main_size: %i",
					ಅಂಶ->box, ಗುರಿ_main_ಗಾತ್ರ);

		ಆದರೆ (ಅಂಶ->freeze) {
			ಮುಂದುವರಿ;
		}

		ಆದರೆ (ಅಂಶ->max_main > 0 &&
		    ಗುರಿ_main_ಗಾತ್ರ > ಅಂಶ->max_main) {
			ಗುರಿ_main_ಗಾತ್ರ = ಅಂಶ->max_main;
			ಅಂಶ->max_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: max_main: %i",
					ಅಂಶ->max_main);
		}

		ಆದರೆ (ಗುರಿ_main_ಗಾತ್ರ < ಅಂಶ->min_main) {
			ಗುರಿ_main_ಗಾತ್ರ = ಅಂಶ->min_main;
			ಅಂಶ->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: min_main: %i",
					ಅಂಶ->min_main);
		}

		ಆದರೆ (ಗುರಿ_main_ಗಾತ್ರ < ಅಂಶ->box->min_ಅಗಲ) {
			ಗುರಿ_main_ಗಾತ್ರ = ಅಂಶ->box->min_ಅಗಲ;
			ಅಂಶ->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: box min_width: %i",
					ಅಂಶ->box->min_ಅಗಲ);
		}

		ಆದರೆ (ಗುರಿ_main_ಗಾತ್ರ < 0) {
			ಗುರಿ_main_ಗಾತ್ರ = 0;
			ಅಂಶ->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: less than 0");
		}

		ಒಟ್ಟು_violation += ಗುರಿ_main_ಗಾತ್ರ - ಅಂಶ->ಗುರಿ_main_ಗಾತ್ರ;
		ಅಂಶ->ಗುರಿ_main_ಗಾತ್ರ = ಗುರಿ_main_ಗಾತ್ರ;
	}

	NSLOG(flex, DEEPDEBUG, "Total violation: %i", ಒಟ್ಟು_violation);

	ಹಿಂತಿರುಗಿ ಒಟ್ಟು_violation;
}

/**
 * Distribute remaining free space proportional to the flex factors.
 *
 * Remaining free space may be negative.
 *
 * \param[in] ctx                  Flex layout context
 * \param[in] line                 Line to distribute free space on
 * \param[in] unfrozen_factor_sum  Sum of unfrozen item's flex factors
 * \param[in] remaining_free_main  Remaining free space in main direction
 * \param[in] grow                 Whether to grow or shrink
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ layout_flex__distribute_free_main(
		ರಚನೆ flex_ctx *ctx,
		ರಚನೆ flex_ಸಾಲು_ದತ್ತಾಂಶ *ಸಾಲು,
		css_fixed unfrozen_ಗುಣಕ_ಮೊತ್ತ,
		ಪೂರ್ಣಾಂಕ ಉಳಿದ_free_main,
		bool grow)
{
	size_t ಅಂಶ_ಎಣಿಕೆ = ಸಾಲು->ಮೊದಲ + ಸಾಲು->ಎಣಿಕೆ_2;

	ಆದರೆ (grow) {
		css_fixed remainder = 0;
		ಗಾಗಿ (size_t i = ಸಾಲು->ಮೊದಲ; i < ಅಂಶ_ಎಣಿಕೆ; i++) {
			ರಚನೆ flex_ಅಂಶ_ದತ್ತಾಂಶ *ಅಂಶ = &ctx->ಅಂಶ.ದತ್ತಾಂಶ[i];
			css_fixed ಫಲಿತಾಂಶ;
			css_fixed ಅನುಪಾತ;

			ಆದರೆ (ಅಂಶ->freeze) {
				ಮುಂದುವರಿ;
			}

			ಅನುಪಾತ = FDIV(ಅಂಶ->grow, unfrozen_ಗುಣಕ_ಮೊತ್ತ);
			ಫಲಿತಾಂಶ = FMUL(INTTOFIX(ಉಳಿದ_free_main), ಅನುಪಾತ) +
					remainder;

			ಅಂಶ->ಗುರಿ_main_ಗಾತ್ರ = ಅಂಶ->ತಳ_ಗಾತ್ರ +
					FIXTOINT(ಫಲಿತಾಂಶ);
			remainder = FIXFRAC(ಫಲಿತಾಂಶ);
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		css_fixed scaled_shrink_ಗುಣಕ_ಮೊತ್ತ = 0;
		css_fixed remainder = 0;

		ಗಾಗಿ (size_t i = ಸಾಲು->ಮೊದಲ; i < ಅಂಶ_ಎಣಿಕೆ; i++) {
			ರಚನೆ flex_ಅಂಶ_ದತ್ತಾಂಶ *ಅಂಶ = &ctx->ಅಂಶ.ದತ್ತಾಂಶ[i];
			css_fixed scaled_shrink_ಗುಣಕ;

			ಆದರೆ (ಅಂಶ->freeze) {
				ಮುಂದುವರಿ;
			}

			scaled_shrink_ಗುಣಕ = FMUL(
					ಅಂಶ->shrink,
					INTTOFIX(ಅಂಶ->ತಳ_ಗಾತ್ರ));
			scaled_shrink_ಗುಣಕ_ಮೊತ್ತ += scaled_shrink_ಗುಣಕ;
		}

		ಗಾಗಿ (size_t i = ಸಾಲು->ಮೊದಲ; i < ಅಂಶ_ಎಣಿಕೆ; i++) {
			ರಚನೆ flex_ಅಂಶ_ದತ್ತಾಂಶ *ಅಂಶ = &ctx->ಅಂಶ.ದತ್ತಾಂಶ[i];
			css_fixed scaled_shrink_ಗುಣಕ;
			css_fixed ಫಲಿತಾಂಶ;
			css_fixed ಅನುಪಾತ;

			ಆದರೆ (ಅಂಶ->freeze) {
				ಮುಂದುವರಿ;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (scaled_shrink_ಗುಣಕ_ಮೊತ್ತ == 0) {
				ಅಂಶ->ಗುರಿ_main_ಗಾತ್ರ = ಅಂಶ->main_ಗಾತ್ರ;
				layout_flex_ಅಂಶ_freeze(ಸಾಲು, ಅಂಶ);
				ಮುಂದುವರಿ;
			}

			scaled_shrink_ಗುಣಕ = FMUL(
					ಅಂಶ->shrink,
					INTTOFIX(ಅಂಶ->ತಳ_ಗಾತ್ರ));
			ಅನುಪಾತ = FDIV(scaled_shrink_ಗುಣಕ,
			             scaled_shrink_ಗುಣಕ_ಮೊತ್ತ);
			ಫಲಿತಾಂಶ = FMUL(INTTOFIX(ನಿರಪೇಕ್ಷ(ಉಳಿದ_free_main)),
			              ಅನುಪಾತ) + remainder;

			ಅಂಶ->ಗುರಿ_main_ಗಾತ್ರ = ಅಂಶ->ತಳ_ಗಾತ್ರ -
					FIXTOINT(ಫಲಿತಾಂಶ);
			remainder = FIXFRAC(ಫಲಿತಾಂಶ);
		}
	}
}

/**
 * Resolve flexible item lengths along a line.
 *
 * See 9.7 of Tests CSS Flexible Box Layout Module Level 1.
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to resolve
 * \return true on success, false on failure.
 */
ಸ್ಥಿರ bool layout_flex_resolve_ಸಾಲು(
		ರಚನೆ flex_ctx *ctx,
		ರಚನೆ flex_ಸಾಲು_ದತ್ತಾಂಶ *ಸಾಲು)
{
	size_t ಅಂಶ_ಎಣಿಕೆ = ಸಾಲು->ಮೊದಲ + ಸಾಲು->ಎಣಿಕೆ_2;
	ಪೂರ್ಣಾಂಕ ಲಭ್ಯ_main = ctx->ಲಭ್ಯ_main;
	ಪೂರ್ಣಾಂಕ initial_free_main;
	bool grow;

	ಆದರೆ (ಲಭ್ಯ_main == AUTO) {
		ಲಭ್ಯ_main = ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ;
	}

	grow = (ಸಾಲು->main_ಗಾತ್ರ < ಲಭ್ಯ_main);
	initial_free_main = ಲಭ್ಯ_main;

	NSLOG(flex, DEEPDEBUG, "box %p: line %zu: first: %zu, count: %zu",
			ctx->flex, ಸಾಲು - ctx->ಸಾಲು.ದತ್ತಾಂಶ,
			ಸಾಲು->ಮೊದಲ, ಸಾಲು->ಎಣಿಕೆ_2);
	NSLOG(flex, DEEPDEBUG, "Line main_size: %i, available_main: %i",
			ಸಾಲು->main_ಗಾತ್ರ, ಲಭ್ಯ_main);

	ಗಾಗಿ (size_t i = ಸಾಲು->ಮೊದಲ; i < ಅಂಶ_ಎಣಿಕೆ; i++) {
		ರಚನೆ flex_ಅಂಶ_ದತ್ತಾಂಶ *ಅಂಶ = &ctx->ಅಂಶ.ದತ್ತಾಂಶ[i];

		/* 3. Size inflexible items */
		ಆದರೆ (grow) {
			ಆದರೆ (ಅಂಶ->grow == 0 ||
			    ಅಂಶ->ತಳ_ಗಾತ್ರ > ಅಂಶ->main_ಗಾತ್ರ) {
				ಅಂಶ->ಗುರಿ_main_ಗಾತ್ರ = ಅಂಶ->main_ಗಾತ್ರ;
				layout_flex_ಅಂಶ_freeze(ಸಾಲು, ಅಂಶ);
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (ಅಂಶ->shrink == 0 ||
			    ಅಂಶ->ತಳ_ಗಾತ್ರ < ಅಂಶ->main_ಗಾತ್ರ) {
				ಅಂಶ->ಗುರಿ_main_ಗಾತ್ರ = ಅಂಶ->main_ಗಾತ್ರ;
				layout_flex_ಅಂಶ_freeze(ಸಾಲು, ಅಂಶ);
			}
		}

		/* 4. Calculate initial free space */
		ಆದರೆ (ಅಂಶ->freeze) {
			initial_free_main -= ಅಂಶ->ಗುರಿ_main_ಗಾತ್ರ;
		} ಇಲ್ಲದಿದ್ದರೆ {
			initial_free_main -= ಅಂಶ->ತಳ_ಗಾತ್ರ;
		}
	}

	/* 5. Loop */
	ಆಗ (ಸಾಲು->frozen < ಸಾಲು->ಎಣಿಕೆ_2) {
		css_fixed unfrozen_ಗುಣಕ_ಮೊತ್ತ;
		ಪೂರ್ಣಾಂಕ ಉಳಿದ_free_main;
		ಪೂರ್ಣಾಂಕ ಒಟ್ಟು_violation;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: Resolver pass",
				ctx->flex);

		/* b */
		ಉಳಿದ_free_main = layout_flex_ಉಳಿದ_free_main(ctx,
				ಸಾಲು, &unfrozen_ಗುಣಕ_ಮೊತ್ತ, initial_free_main,
				ಲಭ್ಯ_main, grow);

		/* c */
		ಆದರೆ (ಉಳಿದ_free_main != 0) {
			layout_flex__distribute_free_main(ctx,
					ಸಾಲು, unfrozen_ಗುಣಕ_ಮೊತ್ತ,
					ಉಳಿದ_free_main, grow);
		}

		/* d */
		ಒಟ್ಟು_violation = layout_flex__get_min_max_violations(
				ctx, ಸಾಲು);

		/* e */
		ಗಾಗಿ (size_t i = ಸಾಲು->ಮೊದಲ; i < ಅಂಶ_ಎಣಿಕೆ; i++) {
			ರಚನೆ flex_ಅಂಶ_ದತ್ತಾಂಶ *ಅಂಶ = &ctx->ಅಂಶ.ದತ್ತಾಂಶ[i];

			ಆದರೆ (ಅಂಶ->freeze) {
				ಮುಂದುವರಿ;
			}

			ಆದರೆ (ಒಟ್ಟು_violation == 0 ||
			    (ಒಟ್ಟು_violation > 0 && ಅಂಶ->min_violation) ||
			    (ಒಟ್ಟು_violation < 0 && ಅಂಶ->max_violation)) {
				layout_flex_ಅಂಶ_freeze(ಸಾಲು, ಅಂಶ);
			}
		}
	}

	ಹಿಂತಿರುಗಿ true;
}

/**
 * Position items along a line
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to resolve
 * \return true on success, false on failure.
 */
ಸ್ಥಿರ bool layout_flex_ಇರಿಸು_ಸಾಲು_ವಸ್ತುಗಳು_main(
		ರಚನೆ flex_ctx *ctx,
		ರಚನೆ flex_ಸಾಲು_ದತ್ತಾಂಶ *ಸಾಲು)
{
	ಪೂರ್ಣಾಂಕ main_pos = ctx->flex->padding[layout_flex_main_ಆರಂಭ_side(ctx)];
	ಪೂರ್ಣಾಂಕ post_multiplier = ctx->main_reversed ? 0 : 1;
	ಪೂರ್ಣಾಂಕ pre_multiplier = ctx->main_reversed ? -1 : 0;
	size_t ಅಂಶ_ಎಣಿಕೆ = ಸಾಲು->ಮೊದಲ + ಸಾಲು->ಎಣಿಕೆ_2;
	ಪೂರ್ಣಾಂಕ extra_remainder = 0;
	ಪೂರ್ಣಾಂಕ extra = 0;

	ಆದರೆ (ctx->main_reversed) {
		main_pos = lh_box_ಗಾತ್ರ_main(ctx->horizontal, ctx->flex) -
				main_pos;
	}

	ಆದರೆ (ctx->ಲಭ್ಯ_main != AUTO &&
	    ctx->ಲಭ್ಯ_main != UNKNOWN_ಅಗಲ &&
	    ctx->ಲಭ್ಯ_main > ಸಾಲು->ಬಳಸಿದ_main_ಗಾತ್ರ) {
		ಆದರೆ (ಸಾಲು->main_auto_margin_ಎಣಿಕೆ > 0) {
			extra = ctx->ಲಭ್ಯ_main - ಸಾಲು->ಬಳಸಿದ_main_ಗಾತ್ರ;

			extra_remainder = extra % ಸಾಲು->main_auto_margin_ಎಣಿಕೆ;
			extra /= ಸಾಲು->main_auto_margin_ಎಣಿಕೆ;
		}
	}

	ಗಾಗಿ (size_t i = ಸಾಲು->ಮೊದಲ; i < ಅಂಶ_ಎಣಿಕೆ; i++) {
		ಎಣಿಕೆ box_side main_ಕೊನೆ = ctx->horizontal ? ಬಲ_2 : BOTTOM;
		ಎಣಿಕೆ box_side main_ಆರಂಭ = ctx->horizontal ? ಎಡ_2 : ತುದಿ_2;
		ರಚನೆ flex_ಅಂಶ_ದತ್ತಾಂಶ *ಅಂಶ = &ctx->ಅಂಶ.ದತ್ತಾಂಶ[i];
		ರಚನೆ box *b = ಅಂಶ->box;
		ಪೂರ್ಣಾಂಕ extra_ಒಟ್ಟು = 0;
		ಪೂರ್ಣಾಂಕ extra_post = 0;
		ಪೂರ್ಣಾಂಕ extra_pre = 0;
		ಪೂರ್ಣಾಂಕ box_ಗಾತ್ರ_main;
		ಪೂರ್ಣಾಂಕ *box_pos_main;

		ಆದರೆ (ctx->horizontal) {
			b->ಅಗಲ = ಅಂಶ->ಗುರಿ_main_ಗಾತ್ರ -
					lh_delta_outer_ಅಗಲ(b);

			ಆದರೆ (!layout_flex_ಅಂಶ(ctx, ಅಂಶ, b->ಅಗಲ)) {
				ಹಿಂತಿರುಗಿ false;
			}
		}

		box_ಗಾತ್ರ_main = lh_box_ಗಾತ್ರ_main(ctx->horizontal, b);
		box_pos_main = ctx->horizontal ? &b->x : &b->y;

		ಆದರೆ (!lh__box_is_absolute(b)) {
			ಆದರೆ (b->margin[main_ಆರಂಭ] == AUTO) {
				extra_pre = extra + extra_remainder;
			}
			ಆದರೆ (b->margin[main_ಕೊನೆ] == AUTO) {
				extra_post = extra + extra_remainder;
			}
			extra_ಒಟ್ಟು = extra_pre + extra_post;

			main_pos += pre_multiplier *
					(extra_ಒಟ್ಟು + box_ಗಾತ್ರ_main +
					 lh__delta_outer_main(ctx->flex, b));
		}

		*box_pos_main = main_pos + lh__non_auto_margin(b, main_ಆರಂಭ) +
				extra_pre + b->border[main_ಆರಂಭ].ಅಗಲ;

		ಆದರೆ (!lh__box_is_absolute(b)) {
			ಪೂರ್ಣಾಂಕ cross_ಗಾತ್ರ;
			ಪೂರ್ಣಾಂಕ box_ಗಾತ್ರ_cross = lh_box_ಗಾತ್ರ_cross(
					ctx->horizontal, b);

			main_pos += post_multiplier *
					(extra_ಒಟ್ಟು + box_ಗಾತ್ರ_main +
					 lh__delta_outer_main(ctx->flex, b));

			cross_ಗಾತ್ರ = box_ಗಾತ್ರ_cross + lh__delta_outer_cross(
					ctx->flex, b);
			ಆದರೆ (ಸಾಲು->cross_ಗಾತ್ರ < cross_ಗಾತ್ರ) {
				ಸಾಲು->cross_ಗಾತ್ರ = cross_ಗಾತ್ರ;
			}
		}
	}

	ಹಿಂತಿರುಗಿ true;
}

/**
 * Collect items onto lines and place items along the lines
 *
 * \param[in] ctx   Flex layout context
 * \return true on success, false on failure.
 */
ಸ್ಥಿರ bool layout_flex_collect_ವಸ್ತುಗಳು_into_ಸಾಲುಗಳು(
		ರಚನೆ flex_ctx *ctx)
{
	size_t pos = 0;

	ಆಗ (pos < ctx->ಅಂಶ.ಎಣಿಕೆ_2) {
		ರಚನೆ flex_ಸಾಲು_ದತ್ತಾಂಶ *ಸಾಲು;

		ಸಾಲು = layout_flex_build_ಸಾಲು(ctx, pos);
		ಆದರೆ (ಸಾಲು == ಖಾಲಿ) {
			ಹಿಂತಿರುಗಿ false;
		}

		pos += ಸಾಲು->ಎಣಿಕೆ_2;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: "
				"fitted: %zu (total: %zu/%zu)",
				ctx->flex, ಸಾಲು->ಎಣಿಕೆ_2,
				pos, ctx->ಅಂಶ.ಎಣಿಕೆ_2);

		ಆದರೆ (!layout_flex_resolve_ಸಾಲು(ctx, ಸಾಲು)) {
			ಹಿಂತಿರುಗಿ false;
		}

		ಆದರೆ (!layout_flex_ಇರಿಸು_ಸಾಲು_ವಸ್ತುಗಳು_main(ctx, ಸಾಲು)) {
			ಹಿಂತಿರುಗಿ false;
		}

		ctx->cross_ಗಾತ್ರ += ಸಾಲು->cross_ಗಾತ್ರ;
		ಆದರೆ (ctx->main_ಗಾತ್ರ < ಸಾಲು->main_ಗಾತ್ರ) {
			ctx->main_ಗಾತ್ರ = ಸಾಲು->main_ಗಾತ್ರ;
		}
	}

	ಹಿಂತಿರುಗಿ true;
}

/**
 * Align items on a line.
 *
 * \param[in] ctx    Flex layout context
 * \param[in] line   Line to align items on
 * \param[in] extra  Extra line width in pixels
 */
ಸ್ಥಿರ ಶೂನ್ಯ layout_flex_ಇರಿಸು_ಸಾಲು_ವಸ್ತುಗಳು_cross(ರಚನೆ flex_ctx *ctx,
		ರಚನೆ flex_ಸಾಲು_ದತ್ತಾಂಶ *ಸಾಲು, ಪೂರ್ಣಾಂಕ extra)
{
	ಎಣಿಕೆ box_side cross_ಆರಂಭ = ctx->horizontal ? ತುದಿ_2 : ಎಡ_2;
	size_t ಅಂಶ_ಎಣಿಕೆ = ಸಾಲು->ಮೊದಲ + ಸಾಲು->ಎಣಿಕೆ_2;

	ಗಾಗಿ (size_t i = ಸಾಲು->ಮೊದಲ; i < ಅಂಶ_ಎಣಿಕೆ; i++) {
		ರಚನೆ flex_ಅಂಶ_ದತ್ತಾಂಶ *ಅಂಶ = &ctx->ಅಂಶ.ದತ್ತಾಂಶ[i];
		ರಚನೆ box *b = ಅಂಶ->box;
		ಪೂರ್ಣಾಂಕ cross_free_space;
		ಪೂರ್ಣಾಂಕ *box_ಗಾತ್ರ_cross;
		ಪೂರ್ಣಾಂಕ *box_pos_cross;

		box_pos_cross = ctx->horizontal ? &b->y : &b->x;
		box_ಗಾತ್ರ_cross = lh_box_ಗಾತ್ರ_cross_ptr(ctx->horizontal, b);

		cross_free_space = ಸಾಲು->cross_ಗಾತ್ರ + extra - *box_ಗಾತ್ರ_cross -
				lh__delta_outer_cross(ctx->flex, b);

		ಆಯ್ಕೆ (lh__box_align_self(ctx->flex, b)) {
		ಪೂರ್ವನಿಯೋಜಿತ:
		ಸ್ಥಿತಿ CSS_ALIGN_SELF_STRETCH:
			ಆದರೆ (lh_box_ಗಾತ್ರ_cross_is_auto(ctx->horizontal, b)) {
				*box_ಗಾತ್ರ_cross += cross_free_space;

				/* Relayout children for stretch. */
				ಆದರೆ (!layout_flex_ಅಂಶ(ctx, ಅಂಶ, b->ಅಗಲ)) {
					ಹಿಂತಿರುಗಿ;
				}
			}
			fallthrough;
		ಸ್ಥಿತಿ CSS_ALIGN_SELF_FLEX_ಆರಂಭ:
			*box_pos_cross = ctx->flex->padding[cross_ಆರಂಭ] +
					ಸಾಲು->pos +
					lh__non_auto_margin(b, cross_ಆರಂಭ) +
					b->border[cross_ಆರಂಭ].ಅಗಲ;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ CSS_ALIGN_SELF_FLEX_ಕೊನೆ:
			*box_pos_cross = ctx->flex->padding[cross_ಆರಂಭ] +
					ಸಾಲು->pos + cross_free_space +
					lh__non_auto_margin(b, cross_ಆರಂಭ) +
					b->border[cross_ಆರಂಭ].ಅಗಲ;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ CSS_ALIGN_SELF_BASELINE:
		ಸ್ಥಿತಿ CSS_ALIGN_SELF_CENTER:
			*box_pos_cross = ctx->flex->padding[cross_ಆರಂಭ] +
					ಸಾಲು->pos + cross_free_space / 2 +
					lh__non_auto_margin(b, cross_ಆರಂಭ) +
					b->border[cross_ಆರಂಭ].ಅಗಲ;
			ನಿಲ್ಲಿಸು;
		}
	}
}

/**
 * Place the lines and align the items on the line.
 *
 * \param[in] ctx  Flex layout context
 */
ಸ್ಥಿರ ಶೂನ್ಯ layout_flex_ಇರಿಸು_ಸಾಲುಗಳು(ರಚನೆ flex_ctx *ctx)
{
	bool reversed = ctx->wrap == CSS_FLEX_WRAP_WRAP_ಹಿಮ್ಮುಖ;
	ಪೂರ್ಣಾಂಕ ಸಾಲು_pos = reversed ? ctx->cross_ಗಾತ್ರ : 0;
	ಪೂರ್ಣಾಂಕ post_multiplier = reversed ? 0 : 1;
	ಪೂರ್ಣಾಂಕ pre_multiplier = reversed ? -1 : 0;
	ಪೂರ್ಣಾಂಕ extra_remainder = 0;
	ಪೂರ್ಣಾಂಕ extra = 0;

	ಆದರೆ (ctx->ಲಭ್ಯ_cross != AUTO &&
	    ctx->ಲಭ್ಯ_cross > ctx->cross_ಗಾತ್ರ &&
	    ctx->ಸಾಲು.ಎಣಿಕೆ_2 > 0) {
		extra = ctx->ಲಭ್ಯ_cross - ctx->cross_ಗಾತ್ರ;

		extra_remainder = extra % ctx->ಸಾಲು.ಎಣಿಕೆ_2;
		extra /= ctx->ಸಾಲು.ಎಣಿಕೆ_2;
	}

	ಗಾಗಿ (size_t i = 0; i < ctx->ಸಾಲು.ಎಣಿಕೆ_2; i++) {
		ರಚನೆ flex_ಸಾಲು_ದತ್ತಾಂಶ *ಸಾಲು = &ctx->ಸಾಲು.ದತ್ತಾಂಶ[i];

		ಸಾಲು_pos += pre_multiplier * ಸಾಲು->cross_ಗಾತ್ರ;
		ಸಾಲು->pos = ಸಾಲು_pos;
		ಸಾಲು_pos += post_multiplier * ಸಾಲು->cross_ಗಾತ್ರ +
				extra + extra_remainder;

		layout_flex_ಇರಿಸು_ಸಾಲು_ವಸ್ತುಗಳು_cross(ctx, ಸಾಲು,
				extra + extra_remainder);

		ಆದರೆ (extra_remainder > 0) {
			extra_remainder--;
		}
	}
}

/**
 * Layout a flex container.
 *
 * \param[in] flex             table to layout
 * \param[in] available_width  width of containing block
 * \param[in] content          memory pool for any new boxes
 * \return  true on success, false on memory exhaustion
 */
bool layout_flex(ರಚನೆ box *flex, ಪೂರ್ಣಾಂಕ ಲಭ್ಯ_ಅಗಲ,
		html_content *content)
{
	ಪೂರ್ಣಾಂಕ max_ಎತ್ತರ, min_ಎತ್ತರ;
	ರಚನೆ flex_ctx *ctx;
	bool success = false;

	ctx = layout_flex_ctx__create(content, flex);
	ಆದರೆ (ctx == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ false;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: %s, available_width %i, width: %i",
			flex, ctx->horizontal ? "horizontal" : "vertical",
			ಲಭ್ಯ_ಅಗಲ, flex->ಅಗಲ);

	layout_ಕಂಡುಹಿಡಿ_dimensions(
			ctx->unit_len_ctx, ಲಭ್ಯ_ಅಗಲ, -1,
			flex, flex->style, ಖಾಲಿ, &flex->ಎತ್ತರ,
			ಖಾಲಿ, ಖಾಲಿ, &max_ಎತ್ತರ, &min_ಎತ್ತರ,
			flex->margin, flex->padding, flex->border);

	ಲಭ್ಯ_ಅಗಲ = min(ಲಭ್ಯ_ಅಗಲ, flex->ಅಗಲ);

	ಆದರೆ (ctx->horizontal) {
		ctx->ಲಭ್ಯ_main = ಲಭ್ಯ_ಅಗಲ;
		ctx->ಲಭ್ಯ_cross = ctx->flex->ಎತ್ತರ;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ctx->ಲಭ್ಯ_main = ctx->flex->ಎತ್ತರ;
		ctx->ಲಭ್ಯ_cross = ಲಭ್ಯ_ಅಗಲ;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: available_main: %i",
			flex, ctx->ಲಭ್ಯ_main);
	NSLOG(flex, DEEPDEBUG, "box %p: available_cross: %i",
			flex, ctx->ಲಭ್ಯ_cross);

	layout_flex_ctx_populate_ಅಂಶ_ದತ್ತಾಂಶ(ctx, flex, ಲಭ್ಯ_ಅಗಲ);

	/* Place items onto lines. */
	success = layout_flex_collect_ವಸ್ತುಗಳು_into_ಸಾಲುಗಳು(ctx);
	ಆದರೆ (!success) {
		ಹೋಗು cleanup;
	}

	layout_flex_ಇರಿಸು_ಸಾಲುಗಳು(ctx);

	ಆದರೆ (flex->ಎತ್ತರ == AUTO) {
		flex->ಎತ್ತರ = ctx->horizontal ?
				ctx->cross_ಗಾತ್ರ :
				ctx->main_ಗಾತ್ರ;
	}

	ಆದರೆ (flex->ಎತ್ತರ != AUTO) {
		ಆದರೆ (max_ಎತ್ತರ >= 0 && flex->ಎತ್ತರ > max_ಎತ್ತರ) {
			flex->ಎತ್ತರ = max_ಎತ್ತರ;
		}
		ಆದರೆ (min_ಎತ್ತರ >  0 && flex->ಎತ್ತರ < min_ಎತ್ತರ) {
			flex->ಎತ್ತರ = min_ಎತ್ತರ;
		}
	}

	success = true;

cleanup:
	layout_flex_ctx__destroy(ctx);

	NSLOG(flex, DEEPDEBUG, "box %p: %s: w: %i, h: %i", flex,
			success ? "success" : "failure",
			flex->ಅಗಲ, flex->ಎತ್ತರ);
	ಹಿಂತಿರುಗಿ success;
}
