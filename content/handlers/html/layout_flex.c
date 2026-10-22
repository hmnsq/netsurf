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

#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "html/box.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/html.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/private.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box_inspect.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/layout_internal.h"

/**
 * Flex item data
 */
ਬਣਤਰ flex_ਵਸਤੂ_ਅੰਕੜੇ {
	ਗਿਣਤੀ css_flex_basis_e basis;
	css_fixed basis_ਲੰਮਾਈ;
	css_unit basis_unit;
	ਬਣਤਰ box *box;

	css_fixed shrink;
	css_fixed grow;

	ਪੂਰਨ min_main;
	ਪੂਰਨ max_main;
	ਪੂਰਨ min_cross;
	ਪੂਰਨ max_cross;

	ਪੂਰਨ ਨਿਸ਼ਾਨਾ_main_ਮਾਪ;
	ਪੂਰਨ ਅਧਾਰ_ਮਾਪ;
	ਪੂਰਨ main_ਮਾਪ;
	size_t ਸਤਰ;

	bool freeze;
	bool min_violation;
	bool max_violation;
};

/**
 * Flex line data
 */
ਬਣਤਰ flex_ਸਤਰ_ਅੰਕੜੇ {
	ਪੂਰਨ main_ਮਾਪ;
	ਪੂਰਨ cross_ਮਾਪ;

	ਪੂਰਨ ਵਰਤਿਆ_main_ਮਾਪ;
	ਪੂਰਨ main_auto_margin_ਗਿਣਤੀ;

	ਪੂਰਨ pos;

	size_t ਪਹਿਲਾ;
	size_t ਗਿਣਤੀ_2;
	size_t frozen;
};

/**
 * Flex layout context
 */
ਬਣਤਰ flex_ctx {
	html_content *content;
	ਅਚਲ ਬਣਤਰ box *flex;
	ਅਚਲ css_unit_ctx *unit_len_ctx;

	ਪੂਰਨ main_ਮਾਪ;
	ਪੂਰਨ cross_ਮਾਪ;

	ਪੂਰਨ ਉਪਲਬਧ_main;
	ਪੂਰਨ ਉਪਲਬਧ_cross;

	bool horizontal;
	bool main_reversed;
	ਗਿਣਤੀ css_flex_wrap_e wrap;

	ਬਣਤਰ flex_ਚੀਜ਼ਾਂ {
		size_t ਗਿਣਤੀ_2;
		ਬਣਤਰ flex_ਵਸਤੂ_ਅੰਕੜੇ *ਅੰਕੜੇ;
	} ਵਸਤੂ;

	ਬਣਤਰ flex_ਸਤਰਾਂ {
		size_t ਗਿਣਤੀ_2;
		size_t alloc;
		ਬਣਤਰ flex_ਸਤਰ_ਅੰਕੜੇ *ਅੰਕੜੇ;
	} ਸਤਰ;
};

/**
 * Destroy a flex layout context
 *
 * \param[in] ctx  Flex layout context
 */
ਸਥਿਰ ਖਾਲੀ layout_flex_ctx__destroy(ਬਣਤਰ flex_ctx *ctx)
{
	ਜੇ (ctx != ਨਿਰਾ) {
		ਮੁਕਤ(ctx->ਵਸਤੂ.ਅੰਕੜੇ);
		ਮੁਕਤ(ctx->ਸਤਰ.ਅੰਕੜੇ);
		ਮੁਕਤ(ctx);
	}
}

/**
 * Create a flex layout context
 *
 * \param[in] content  HTML content containing flex box
 * \param[in] flex     Box to create layout context for
 * \return flex layout context or NULL on error
 */
ਸਥਿਰ ਬਣਤਰ flex_ctx *layout_flex_ctx__create(
		html_content *content,
		ਅਚਲ ਬਣਤਰ box *flex)
{
	ਬਣਤਰ flex_ctx *ctx;

	ctx = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(*ctx));
	ਜੇ (ctx == ਨਿਰਾ) {
		ਵਾਪਸ ਨਿਰਾ;
	}
	ctx->ਸਤਰ.alloc = 1;

	ctx->ਵਸਤੂ.ਗਿਣਤੀ_2 = box_ਗਿਣਤੀ_children(flex);
	ctx->ਵਸਤੂ.ਅੰਕੜੇ = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(ctx->ਵਸਤੂ.ਗਿਣਤੀ_2, ਆਕਾਰ(*ctx->ਵਸਤੂ.ਅੰਕੜੇ));
	ਜੇ (ctx->ਵਸਤੂ.ਅੰਕੜੇ == ਨਿਰਾ) {
		layout_flex_ctx__destroy(ctx);
		ਵਾਪਸ ਨਿਰਾ;
	}

	ctx->ਸਤਰ.alloc = 1;
	ctx->ਸਤਰ.ਅੰਕੜੇ = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(ctx->ਸਤਰ.alloc, ਆਕਾਰ(*ctx->ਸਤਰ.ਅੰਕੜੇ));
	ਜੇ (ctx->ਸਤਰ.ਅੰਕੜੇ == ਨਿਰਾ) {
		layout_flex_ctx__destroy(ctx);
		ਵਾਪਸ ਨਿਰਾ;
	}

	ctx->flex = flex;
	ctx->content = content;
	ctx->unit_len_ctx = &content->unit_len_ctx;

	ctx->wrap = css_computed_flex_wrap(flex->style);
	ctx->horizontal = lh__flex_main_is_horizontal(flex);
	ctx->main_reversed = lh__flex_direction_reversed(flex);

	ਵਾਪਸ ctx;
}

/**
 * Find box side representing the start of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the start side.
 */
ਸਥਿਰ ਗਿਣਤੀ box_side layout_flex_main_ਸ਼ੁਰੂ_side(
		ਅਚਲ ਬਣਤਰ flex_ctx *ctx)
{
	ਜੇ (ctx->horizontal) {
		ਵਾਪਸ (ctx->main_reversed) ? ਸੱਜਾ_2 : ਖੱਬਾ_2;
	} ਨਹੀਂਤਾਂ {
		ਵਾਪਸ (ctx->main_reversed) ? BOTTOM : ਸਿਖਰ_2;
	}
}

/**
 * Find box side representing the end of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the end side.
 */
ਸਥਿਰ ਗਿਣਤੀ box_side layout_flex_main_ਸਿਰਾ_side(
		ਅਚਲ ਬਣਤਰ flex_ctx *ctx)
{
	ਜੇ (ctx->horizontal) {
		ਵਾਪਸ (ctx->main_reversed) ? ਖੱਬਾ_2 : ਸੱਜਾ_2;
	} ਨਹੀਂਤਾਂ {
		ਵਾਪਸ (ctx->main_reversed) ? ਸਿਖਰ_2 : BOTTOM;
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
ਸਥਿਰ bool layout_flex_ਵਸਤੂ(
		ਅਚਲ ਬਣਤਰ flex_ctx *ctx,
		ਅਚਲ ਬਣਤਰ flex_ਵਸਤੂ_ਅੰਕੜੇ *ਵਸਤੂ,
		ਪੂਰਨ ਉਪਲਬਧ_ਚੌੜਾਈ)
{
	bool success;
	ਬਣਤਰ box *b = ਵਸਤੂ->box;

	ਚੋਣ (b->type) {
	ਹਾਲਤ BOX_ਖੰਡ:
		success = layout_ਖੰਡ_context(b, -1, ctx->content);
		ਰੋਕੋ;
	ਹਾਲਤ BOX_ਸਾਰਣੀ:
		b->float_container = b->ਮਾਪਾ;
		success = layout_ਸਾਰਣੀ(b, ਉਪਲਬਧ_ਚੌੜਾਈ, ctx->content);
		b->float_container = ਨਿਰਾ;
		ਰੋਕੋ;
	ਹਾਲਤ BOX_FLEX:
		b->float_container = b->ਮਾਪਾ;
		success = layout_flex(b, ਉਪਲਬਧ_ਚੌੜਾਈ, ctx->content);
		b->float_container = ਨਿਰਾ;
		ਰੋਕੋ;
	ਮੂਲ:
		ਪ੍ਰਤਿਜ੍ਞਾ(0 && "Bad flex item back type");
		success = false;
		ਰੋਕੋ;
	}

	ਜੇ (!success) {
		NSLOG(flex, ERROR, "box %p: layout failed", b);
	}

	ਵਾਪਸ success;
}

/**
 * Calculate an item's base and target main sizes.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] item             Item to get sizes of
 * \param[in] available_width  Available width in pixels
 * \return true on success false on failure
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool layout_flex_ਅਧਾਰ_and_main_sizes(
		ਅਚਲ ਬਣਤਰ flex_ctx *ctx,
		ਬਣਤਰ flex_ਵਸਤੂ_ਅੰਕੜੇ *ਵਸਤੂ,
		ਪੂਰਨ ਉਪਲਬਧ_ਚੌੜਾਈ)
{
	ਬਣਤਰ box *b = ਵਸਤੂ->box;
	ਪੂਰਨ content_min_ਚੌੜਾਈ = b->min_ਚੌੜਾਈ;
	ਪੂਰਨ content_max_ਚੌੜਾਈ = b->max_ਚੌੜਾਈ;
	ਪੂਰਨ delta_outer_main = lh__delta_outer_main(ctx->flex, b);

	NSLOG(flex, DEEPDEBUG, "box %p: delta_outer_main: %i",
			b, delta_outer_main);

	ਜੇ (ਵਸਤੂ->basis == CSS_FLEX_BASIS_ਸੈੱਟ) {
		ਜੇ (ਵਸਤੂ->basis_unit == CSS_UNIT_PCT) {
			ਵਸਤੂ->ਅਧਾਰ_ਮਾਪ = FPCT_ਦਾ_INT_TOINT(
					ਵਸਤੂ->basis_ਲੰਮਾਈ,
					ਉਪਲਬਧ_ਚੌੜਾਈ);
		} ਨਹੀਂਤਾਂ {
			ਵਸਤੂ->ਅਧਾਰ_ਮਾਪ = FIXTOINT(css_unit_len2device_px(
					b->style, ctx->unit_len_ctx,
					ਵਸਤੂ->basis_ਲੰਮਾਈ,
					ਵਸਤੂ->basis_unit));
		}

	} ਨਹੀਂਤਾਂ ਜੇ (ਵਸਤੂ->basis == CSS_FLEX_BASIS_AUTO) {
		ਵਸਤੂ->ਅਧਾਰ_ਮਾਪ = ctx->horizontal ? b->ਚੌੜਾਈ : b->ਉਚਾਈ;
	} ਨਹੀਂਤਾਂ {
		ਵਸਤੂ->ਅਧਾਰ_ਮਾਪ = AUTO;
	}

	ਜੇ (ctx->horizontal == false) {
		ਜੇ (b->ਚੌੜਾਈ == AUTO) {
			b->ਚੌੜਾਈ = min(max(content_min_ਚੌੜਾਈ, ਉਪਲਬਧ_ਚੌੜਾਈ),
					content_max_ਚੌੜਾਈ);
			b->ਚੌੜਾਈ -= lh_delta_outer_ਚੌੜਾਈ(b);
		}

		ਜੇ (!layout_flex_ਵਸਤੂ(ctx, ਵਸਤੂ, b->ਚੌੜਾਈ)) {
			ਵਾਪਸ false;
		}
	}

	ਜੇ (ਵਸਤੂ->ਅਧਾਰ_ਮਾਪ == AUTO) {
		ਜੇ (ctx->horizontal == false) {
			ਵਸਤੂ->ਅਧਾਰ_ਮਾਪ = b->ਉਚਾਈ;
		} ਨਹੀਂਤਾਂ {
			ਵਸਤੂ->ਅਧਾਰ_ਮਾਪ = content_max_ਚੌੜਾਈ - delta_outer_main;
		}
	}

	ਵਸਤੂ->ਅਧਾਰ_ਮਾਪ += delta_outer_main;

	ਜੇ (ctx->horizontal) {
		ਵਸਤੂ->ਅਧਾਰ_ਮਾਪ = min(ਵਸਤੂ->ਅਧਾਰ_ਮਾਪ, ਉਪਲਬਧ_ਚੌੜਾਈ);
		ਵਸਤੂ->ਅਧਾਰ_ਮਾਪ = max(ਵਸਤੂ->ਅਧਾਰ_ਮਾਪ, content_min_ਚੌੜਾਈ);
	}

	ਵਸਤੂ->ਨਿਸ਼ਾਨਾ_main_ਮਾਪ = ਵਸਤੂ->ਅਧਾਰ_ਮਾਪ;
	ਵਸਤੂ->main_ਮਾਪ = ਵਸਤੂ->ਅਧਾਰ_ਮਾਪ;

	ਜੇ (ਵਸਤੂ->max_main > 0 &&
	    ਵਸਤੂ->main_ਮਾਪ > ਵਸਤੂ->max_main + delta_outer_main) {
		ਵਸਤੂ->main_ਮਾਪ = ਵਸਤੂ->max_main + delta_outer_main;
	}

	ਜੇ (ਵਸਤੂ->main_ਮਾਪ < ਵਸਤੂ->min_main + delta_outer_main) {
		ਵਸਤੂ->main_ਮਾਪ = ਵਸਤੂ->min_main + delta_outer_main;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: base_size: %i, main_size %i",
			b, ਵਸਤੂ->ਅਧਾਰ_ਮਾਪ, ਵਸਤੂ->main_ਮਾਪ);

	ਵਾਪਸ true;
}

/**
 * Fill out all item's data in a flex container.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] flex             Flex box
 * \param[in] available_width  Available width in pixels
 */
ਸਥਿਰ ਖਾਲੀ layout_flex_ctx_populate_ਵਸਤੂ_ਅੰਕੜੇ(
		ਅਚਲ ਬਣਤਰ flex_ctx *ctx,
		ਅਚਲ ਬਣਤਰ box *flex,
		ਪੂਰਨ ਉਪਲਬਧ_ਚੌੜਾਈ)
{
	size_t i = 0;
	bool horizontal = ctx->horizontal;

	ਲਈ (ਬਣਤਰ box *b = flex->children; b != ਨਿਰਾ; b = b->ਅਗਲਾ) {
		ਬਣਤਰ flex_ਵਸਤੂ_ਅੰਕੜੇ *ਵਸਤੂ = &ctx->ਵਸਤੂ.ਅੰਕੜੇ[i++];

		b->float_container = b->ਮਾਪਾ;
		layout_ਲੱਭੋ_dimensions(ctx->unit_len_ctx, ਉਪਲਬਧ_ਚੌੜਾਈ, -1,
				b, b->style, &b->ਚੌੜਾਈ, &b->ਉਚਾਈ,
				horizontal ? &ਵਸਤੂ->max_main : &ਵਸਤੂ->max_cross,
				horizontal ? &ਵਸਤੂ->min_main : &ਵਸਤੂ->min_cross,
				horizontal ? &ਵਸਤੂ->max_cross : &ਵਸਤੂ->max_main,
				horizontal ? &ਵਸਤੂ->min_cross : &ਵਸਤੂ->min_main,
				b->margin, b->padding, b->border);
		b->float_container = ਨਿਰਾ;

		NSLOG(flex, DEEPDEBUG, "flex-item box: %p: width: %i",
				b, b->ਚੌੜਾਈ);

		ਵਸਤੂ->box = b;
		ਵਸਤੂ->basis = css_computed_flex_basis(b->style,
				&ਵਸਤੂ->basis_ਲੰਮਾਈ, &ਵਸਤੂ->basis_unit);

		css_computed_flex_shrink(b->style, &ਵਸਤੂ->shrink);
		css_computed_flex_grow(b->style, &ਵਸਤੂ->grow);

		layout_flex_ਅਧਾਰ_and_main_sizes(ctx, ਵਸਤੂ, ਉਪਲਬਧ_ਚੌੜਾਈ);
	}
}

/**
 * Ensure context's lines array has a free space
 *
 * \param[in] ctx  Flex layout context
 * \return true on success false on out of memory
 */
ਸਥਿਰ bool layout_flex_ctx_ensure_ਸਤਰ(ਬਣਤਰ flex_ctx *ctx)
{
	ਬਣਤਰ flex_ਸਤਰ_ਅੰਕੜੇ *ਆਰਜ਼ੀ;
	size_t ਸਤਰ_alloc = ctx->ਸਤਰ.alloc * 2;

	ਜੇ (ctx->ਸਤਰ.alloc > ctx->ਸਤਰ.ਗਿਣਤੀ_2) {
		ਵਾਪਸ true;
	}

	ਆਰਜ਼ੀ = realloc(ctx->ਸਤਰ.ਅੰਕੜੇ, ਆਕਾਰ(*ctx->ਸਤਰ.ਅੰਕੜੇ) * ਸਤਰ_alloc);
	ਜੇ (ਆਰਜ਼ੀ == ਨਿਰਾ) {
		ਵਾਪਸ false;
	}
	ctx->ਸਤਰ.ਅੰਕੜੇ = ਆਰਜ਼ੀ;

	memset(ctx->ਸਤਰ.ਅੰਕੜੇ + ctx->ਸਤਰ.alloc, 0,
	       ਆਕਾਰ(*ctx->ਸਤਰ.ਅੰਕੜੇ) * (ਸਤਰ_alloc - ctx->ਸਤਰ.alloc));
	ctx->ਸਤਰ.alloc = ਸਤਰ_alloc;

	ਵਾਪਸ true;
}

/**
 * Assigns flex items to the line and returns the line
 *
 * \param[in] ctx         Flex layout context
 * \param[in] item_index  Index to first item to assign to this line
 * \return Pointer to the new line, or NULL on error.
 */
ਸਥਿਰ ਬਣਤਰ flex_ਸਤਰ_ਅੰਕੜੇ *layout_flex_build_ਸਤਰ(ਬਣਤਰ flex_ctx *ctx,
		size_t ਵਸਤੂ_ਸੂਚਕਾਂਕ)
{
	ਗਿਣਤੀ box_side ਸ਼ੁਰੂ_side = layout_flex_main_ਸ਼ੁਰੂ_side(ctx);
	ਗਿਣਤੀ box_side ਸਿਰਾ_side = layout_flex_main_ਸਿਰਾ_side(ctx);
	ਬਣਤਰ flex_ਸਤਰ_ਅੰਕੜੇ *ਸਤਰ;
	ਪੂਰਨ ਵਰਤਿਆ_main = 0;

	ਜੇ (!layout_flex_ctx_ensure_ਸਤਰ(ctx)) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਸਤਰ = &ctx->ਸਤਰ.ਅੰਕੜੇ[ctx->ਸਤਰ.ਗਿਣਤੀ_2];
	ਸਤਰ->ਪਹਿਲਾ = ਵਸਤੂ_ਸੂਚਕਾਂਕ;

	NSLOG(flex, DEEPDEBUG, "flex container %p: available main: %i",
			ctx->flex, ctx->ਉਪਲਬਧ_main);

	ਜਦੋਂ (ਵਸਤੂ_ਸੂਚਕਾਂਕ < ctx->ਵਸਤੂ.ਗਿਣਤੀ_2) {
		ਬਣਤਰ flex_ਵਸਤੂ_ਅੰਕੜੇ *ਵਸਤੂ = &ctx->ਵਸਤੂ.ਅੰਕੜੇ[ਵਸਤੂ_ਸੂਚਕਾਂਕ];
		ਬਣਤਰ box *b = ਵਸਤੂ->box;
		ਪੂਰਨ pos_main;

		pos_main = ctx->horizontal ?
				ਵਸਤੂ->main_ਮਾਪ :
				b->ਉਚਾਈ + lh__delta_outer_main(ctx->flex, b);

		ਜੇ (ctx->wrap == CSS_FLEX_WRAP_NOWRAP ||
		    pos_main + ਵਰਤਿਆ_main <= ctx->ਉਪਲਬਧ_main ||
		    lh__box_is_absolute(ਵਸਤੂ->box) ||
		    ctx->ਉਪਲਬਧ_main == AUTO ||
		    ਸਤਰ->ਗਿਣਤੀ_2 == 0 ||
		    pos_main == 0) {
			ਜੇ (lh__box_is_absolute(ਵਸਤੂ->box) == false) {
				ਸਤਰ->main_ਮਾਪ += ਵਸਤੂ->main_ਮਾਪ;
				ਵਰਤਿਆ_main += pos_main;

				ਜੇ (b->margin[ਸ਼ੁਰੂ_side] == AUTO) {
					ਸਤਰ->main_auto_margin_ਗਿਣਤੀ++;
				}
				ਜੇ (b->margin[ਸਿਰਾ_side] == AUTO) {
					ਸਤਰ->main_auto_margin_ਗਿਣਤੀ++;
				}
			}
			ਵਸਤੂ->ਸਤਰ = ctx->ਸਤਰ.ਗਿਣਤੀ_2;
			ਸਤਰ->ਗਿਣਤੀ_2++;
			ਵਸਤੂ_ਸੂਚਕਾਂਕ++;
		} ਨਹੀਂਤਾਂ {
			ਰੋਕੋ;
		}
	}

	ਜੇ (ਸਤਰ->ਗਿਣਤੀ_2 > 0) {
		ctx->ਸਤਰ.ਗਿਣਤੀ_2++;
	} ਨਹੀਂਤਾਂ {
		NSLOG(layout, ERROR, "Failed to fit any flex items");
	}

	ਵਾਪਸ ਸਤਰ;
}

/**
 * Freeze an item on a line
 *
 * \param[in] line  Line to containing item
 * \param[in] item  Item to freeze
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ layout_flex_ਵਸਤੂ_freeze(
		ਬਣਤਰ flex_ਸਤਰ_ਅੰਕੜੇ *ਸਤਰ,
		ਬਣਤਰ flex_ਵਸਤੂ_ਅੰਕੜੇ *ਵਸਤੂ)
{
	ਵਸਤੂ->freeze = true;
	ਸਤਰ->frozen++;

	ਜੇ (!lh__box_is_absolute(ਵਸਤੂ->box)){
		ਸਤਰ->ਵਰਤਿਆ_main_ਮਾਪ += ਵਸਤੂ->ਨਿਸ਼ਾਨਾ_main_ਮਾਪ;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: "
			"Frozen at target_main_size: %i",
			ਵਸਤੂ->box, ਵਸਤੂ->ਨਿਸ਼ਾਨਾ_main_ਮਾਪ);
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
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ layout_flex_ਬਚਿਆ_free_main(
		ਬਣਤਰ flex_ctx *ctx,
		ਬਣਤਰ flex_ਸਤਰ_ਅੰਕੜੇ *ਸਤਰ,
		css_fixed *unfrozen_ਗੁਣਕ_ਜੋੜ,
		ਪੂਰਨ initial_free_main,
		ਪੂਰਨ ਉਪਲਬਧ_main,
		bool grow)
{
	ਪੂਰਨ ਬਚਿਆ_free_main = ਉਪਲਬਧ_main;
	size_t ਵਸਤੂ_ਗਿਣਤੀ = ਸਤਰ->ਪਹਿਲਾ + ਸਤਰ->ਗਿਣਤੀ_2;

	*unfrozen_ਗੁਣਕ_ਜੋੜ = 0;

	ਲਈ (size_t i = ਸਤਰ->ਪਹਿਲਾ; i < ਵਸਤੂ_ਗਿਣਤੀ; i++) {
		ਬਣਤਰ flex_ਵਸਤੂ_ਅੰਕੜੇ *ਵਸਤੂ = &ctx->ਵਸਤੂ.ਅੰਕੜੇ[i];

		ਜੇ (ਵਸਤੂ->freeze) {
			ਬਚਿਆ_free_main -= ਵਸਤੂ->ਨਿਸ਼ਾਨਾ_main_ਮਾਪ;
		} ਨਹੀਂਤਾਂ {
			ਬਚਿਆ_free_main -= ਵਸਤੂ->ਅਧਾਰ_ਮਾਪ;

			*unfrozen_ਗੁਣਕ_ਜੋੜ += grow ?
					ਵਸਤੂ->grow : ਵਸਤੂ->shrink;
		}
	}

	ਜੇ (*unfrozen_ਗੁਣਕ_ਜੋੜ < F_1) {
		ਪੂਰਨ free_space = FIXTOINT(FMUL(INTTOFIX(initial_free_main),
				*unfrozen_ਗੁਣਕ_ਜੋੜ));

		ਜੇ (free_space < ਬਚਿਆ_free_main) {
			ਬਚਿਆ_free_main = free_space;
		}
	}

	NSLOG(flex, DEEPDEBUG, "Remaining free space: %i",
			ਬਚਿਆ_free_main);

	ਵਾਪਸ ਬਚਿਆ_free_main;
}

/**
 * Clamp flex item target main size and get min/max violations
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to align items on
 * return total violation in pixels
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ layout_flex__get_min_max_violations(
		ਬਣਤਰ flex_ctx *ctx,
		ਬਣਤਰ flex_ਸਤਰ_ਅੰਕੜੇ *ਸਤਰ)
{

	ਪੂਰਨ ਕੁੱਲ_violation = 0;
	size_t ਵਸਤੂ_ਗਿਣਤੀ = ਸਤਰ->ਪਹਿਲਾ + ਸਤਰ->ਗਿਣਤੀ_2;

	ਲਈ (size_t i = ਸਤਰ->ਪਹਿਲਾ; i < ਵਸਤੂ_ਗਿਣਤੀ; i++) {
		ਬਣਤਰ flex_ਵਸਤੂ_ਅੰਕੜੇ *ਵਸਤੂ = &ctx->ਵਸਤੂ.ਅੰਕੜੇ[i];
		ਪੂਰਨ ਨਿਸ਼ਾਨਾ_main_ਮਾਪ = ਵਸਤੂ->ਨਿਸ਼ਾਨਾ_main_ਮਾਪ;

		NSLOG(flex, DEEPDEBUG, "item %p: target_main_size: %i",
					ਵਸਤੂ->box, ਨਿਸ਼ਾਨਾ_main_ਮਾਪ);

		ਜੇ (ਵਸਤੂ->freeze) {
			ਜਾਰੀ;
		}

		ਜੇ (ਵਸਤੂ->max_main > 0 &&
		    ਨਿਸ਼ਾਨਾ_main_ਮਾਪ > ਵਸਤੂ->max_main) {
			ਨਿਸ਼ਾਨਾ_main_ਮਾਪ = ਵਸਤੂ->max_main;
			ਵਸਤੂ->max_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: max_main: %i",
					ਵਸਤੂ->max_main);
		}

		ਜੇ (ਨਿਸ਼ਾਨਾ_main_ਮਾਪ < ਵਸਤੂ->min_main) {
			ਨਿਸ਼ਾਨਾ_main_ਮਾਪ = ਵਸਤੂ->min_main;
			ਵਸਤੂ->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: min_main: %i",
					ਵਸਤੂ->min_main);
		}

		ਜੇ (ਨਿਸ਼ਾਨਾ_main_ਮਾਪ < ਵਸਤੂ->box->min_ਚੌੜਾਈ) {
			ਨਿਸ਼ਾਨਾ_main_ਮਾਪ = ਵਸਤੂ->box->min_ਚੌੜਾਈ;
			ਵਸਤੂ->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: box min_width: %i",
					ਵਸਤੂ->box->min_ਚੌੜਾਈ);
		}

		ਜੇ (ਨਿਸ਼ਾਨਾ_main_ਮਾਪ < 0) {
			ਨਿਸ਼ਾਨਾ_main_ਮਾਪ = 0;
			ਵਸਤੂ->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: less than 0");
		}

		ਕੁੱਲ_violation += ਨਿਸ਼ਾਨਾ_main_ਮਾਪ - ਵਸਤੂ->ਨਿਸ਼ਾਨਾ_main_ਮਾਪ;
		ਵਸਤੂ->ਨਿਸ਼ਾਨਾ_main_ਮਾਪ = ਨਿਸ਼ਾਨਾ_main_ਮਾਪ;
	}

	NSLOG(flex, DEEPDEBUG, "Total violation: %i", ਕੁੱਲ_violation);

	ਵਾਪਸ ਕੁੱਲ_violation;
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
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ layout_flex__distribute_free_main(
		ਬਣਤਰ flex_ctx *ctx,
		ਬਣਤਰ flex_ਸਤਰ_ਅੰਕੜੇ *ਸਤਰ,
		css_fixed unfrozen_ਗੁਣਕ_ਜੋੜ,
		ਪੂਰਨ ਬਚਿਆ_free_main,
		bool grow)
{
	size_t ਵਸਤੂ_ਗਿਣਤੀ = ਸਤਰ->ਪਹਿਲਾ + ਸਤਰ->ਗਿਣਤੀ_2;

	ਜੇ (grow) {
		css_fixed remainder = 0;
		ਲਈ (size_t i = ਸਤਰ->ਪਹਿਲਾ; i < ਵਸਤੂ_ਗਿਣਤੀ; i++) {
			ਬਣਤਰ flex_ਵਸਤੂ_ਅੰਕੜੇ *ਵਸਤੂ = &ctx->ਵਸਤੂ.ਅੰਕੜੇ[i];
			css_fixed ਨਤੀਜਾ;
			css_fixed ਅਨੁਪਾਤ;

			ਜੇ (ਵਸਤੂ->freeze) {
				ਜਾਰੀ;
			}

			ਅਨੁਪਾਤ = FDIV(ਵਸਤੂ->grow, unfrozen_ਗੁਣਕ_ਜੋੜ);
			ਨਤੀਜਾ = FMUL(INTTOFIX(ਬਚਿਆ_free_main), ਅਨੁਪਾਤ) +
					remainder;

			ਵਸਤੂ->ਨਿਸ਼ਾਨਾ_main_ਮਾਪ = ਵਸਤੂ->ਅਧਾਰ_ਮਾਪ +
					FIXTOINT(ਨਤੀਜਾ);
			remainder = FIXFRAC(ਨਤੀਜਾ);
		}
	} ਨਹੀਂਤਾਂ {
		css_fixed scaled_shrink_ਗੁਣਕ_ਜੋੜ = 0;
		css_fixed remainder = 0;

		ਲਈ (size_t i = ਸਤਰ->ਪਹਿਲਾ; i < ਵਸਤੂ_ਗਿਣਤੀ; i++) {
			ਬਣਤਰ flex_ਵਸਤੂ_ਅੰਕੜੇ *ਵਸਤੂ = &ctx->ਵਸਤੂ.ਅੰਕੜੇ[i];
			css_fixed scaled_shrink_ਗੁਣਕ;

			ਜੇ (ਵਸਤੂ->freeze) {
				ਜਾਰੀ;
			}

			scaled_shrink_ਗੁਣਕ = FMUL(
					ਵਸਤੂ->shrink,
					INTTOFIX(ਵਸਤੂ->ਅਧਾਰ_ਮਾਪ));
			scaled_shrink_ਗੁਣਕ_ਜੋੜ += scaled_shrink_ਗੁਣਕ;
		}

		ਲਈ (size_t i = ਸਤਰ->ਪਹਿਲਾ; i < ਵਸਤੂ_ਗਿਣਤੀ; i++) {
			ਬਣਤਰ flex_ਵਸਤੂ_ਅੰਕੜੇ *ਵਸਤੂ = &ctx->ਵਸਤੂ.ਅੰਕੜੇ[i];
			css_fixed scaled_shrink_ਗੁਣਕ;
			css_fixed ਨਤੀਜਾ;
			css_fixed ਅਨੁਪਾਤ;

			ਜੇ (ਵਸਤੂ->freeze) {
				ਜਾਰੀ;
			} ਨਹੀਂਤਾਂ ਜੇ (scaled_shrink_ਗੁਣਕ_ਜੋੜ == 0) {
				ਵਸਤੂ->ਨਿਸ਼ਾਨਾ_main_ਮਾਪ = ਵਸਤੂ->main_ਮਾਪ;
				layout_flex_ਵਸਤੂ_freeze(ਸਤਰ, ਵਸਤੂ);
				ਜਾਰੀ;
			}

			scaled_shrink_ਗੁਣਕ = FMUL(
					ਵਸਤੂ->shrink,
					INTTOFIX(ਵਸਤੂ->ਅਧਾਰ_ਮਾਪ));
			ਅਨੁਪਾਤ = FDIV(scaled_shrink_ਗੁਣਕ,
			             scaled_shrink_ਗੁਣਕ_ਜੋੜ);
			ਨਤੀਜਾ = FMUL(INTTOFIX(ਨਿਰਪੇਖ(ਬਚਿਆ_free_main)),
			              ਅਨੁਪਾਤ) + remainder;

			ਵਸਤੂ->ਨਿਸ਼ਾਨਾ_main_ਮਾਪ = ਵਸਤੂ->ਅਧਾਰ_ਮਾਪ -
					FIXTOINT(ਨਤੀਜਾ);
			remainder = FIXFRAC(ਨਤੀਜਾ);
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
ਸਥਿਰ bool layout_flex_resolve_ਸਤਰ(
		ਬਣਤਰ flex_ctx *ctx,
		ਬਣਤਰ flex_ਸਤਰ_ਅੰਕੜੇ *ਸਤਰ)
{
	size_t ਵਸਤੂ_ਗਿਣਤੀ = ਸਤਰ->ਪਹਿਲਾ + ਸਤਰ->ਗਿਣਤੀ_2;
	ਪੂਰਨ ਉਪਲਬਧ_main = ctx->ਉਪਲਬਧ_main;
	ਪੂਰਨ initial_free_main;
	bool grow;

	ਜੇ (ਉਪਲਬਧ_main == AUTO) {
		ਉਪਲਬਧ_main = ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ;
	}

	grow = (ਸਤਰ->main_ਮਾਪ < ਉਪਲਬਧ_main);
	initial_free_main = ਉਪਲਬਧ_main;

	NSLOG(flex, DEEPDEBUG, "box %p: line %zu: first: %zu, count: %zu",
			ctx->flex, ਸਤਰ - ctx->ਸਤਰ.ਅੰਕੜੇ,
			ਸਤਰ->ਪਹਿਲਾ, ਸਤਰ->ਗਿਣਤੀ_2);
	NSLOG(flex, DEEPDEBUG, "Line main_size: %i, available_main: %i",
			ਸਤਰ->main_ਮਾਪ, ਉਪਲਬਧ_main);

	ਲਈ (size_t i = ਸਤਰ->ਪਹਿਲਾ; i < ਵਸਤੂ_ਗਿਣਤੀ; i++) {
		ਬਣਤਰ flex_ਵਸਤੂ_ਅੰਕੜੇ *ਵਸਤੂ = &ctx->ਵਸਤੂ.ਅੰਕੜੇ[i];

		/* 3. Size inflexible items */
		ਜੇ (grow) {
			ਜੇ (ਵਸਤੂ->grow == 0 ||
			    ਵਸਤੂ->ਅਧਾਰ_ਮਾਪ > ਵਸਤੂ->main_ਮਾਪ) {
				ਵਸਤੂ->ਨਿਸ਼ਾਨਾ_main_ਮਾਪ = ਵਸਤੂ->main_ਮਾਪ;
				layout_flex_ਵਸਤੂ_freeze(ਸਤਰ, ਵਸਤੂ);
			}
		} ਨਹੀਂਤਾਂ {
			ਜੇ (ਵਸਤੂ->shrink == 0 ||
			    ਵਸਤੂ->ਅਧਾਰ_ਮਾਪ < ਵਸਤੂ->main_ਮਾਪ) {
				ਵਸਤੂ->ਨਿਸ਼ਾਨਾ_main_ਮਾਪ = ਵਸਤੂ->main_ਮਾਪ;
				layout_flex_ਵਸਤੂ_freeze(ਸਤਰ, ਵਸਤੂ);
			}
		}

		/* 4. Calculate initial free space */
		ਜੇ (ਵਸਤੂ->freeze) {
			initial_free_main -= ਵਸਤੂ->ਨਿਸ਼ਾਨਾ_main_ਮਾਪ;
		} ਨਹੀਂਤਾਂ {
			initial_free_main -= ਵਸਤੂ->ਅਧਾਰ_ਮਾਪ;
		}
	}

	/* 5. Loop */
	ਜਦੋਂ (ਸਤਰ->frozen < ਸਤਰ->ਗਿਣਤੀ_2) {
		css_fixed unfrozen_ਗੁਣਕ_ਜੋੜ;
		ਪੂਰਨ ਬਚਿਆ_free_main;
		ਪੂਰਨ ਕੁੱਲ_violation;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: Resolver pass",
				ctx->flex);

		/* b */
		ਬਚਿਆ_free_main = layout_flex_ਬਚਿਆ_free_main(ctx,
				ਸਤਰ, &unfrozen_ਗੁਣਕ_ਜੋੜ, initial_free_main,
				ਉਪਲਬਧ_main, grow);

		/* c */
		ਜੇ (ਬਚਿਆ_free_main != 0) {
			layout_flex__distribute_free_main(ctx,
					ਸਤਰ, unfrozen_ਗੁਣਕ_ਜੋੜ,
					ਬਚਿਆ_free_main, grow);
		}

		/* d */
		ਕੁੱਲ_violation = layout_flex__get_min_max_violations(
				ctx, ਸਤਰ);

		/* e */
		ਲਈ (size_t i = ਸਤਰ->ਪਹਿਲਾ; i < ਵਸਤੂ_ਗਿਣਤੀ; i++) {
			ਬਣਤਰ flex_ਵਸਤੂ_ਅੰਕੜੇ *ਵਸਤੂ = &ctx->ਵਸਤੂ.ਅੰਕੜੇ[i];

			ਜੇ (ਵਸਤੂ->freeze) {
				ਜਾਰੀ;
			}

			ਜੇ (ਕੁੱਲ_violation == 0 ||
			    (ਕੁੱਲ_violation > 0 && ਵਸਤੂ->min_violation) ||
			    (ਕੁੱਲ_violation < 0 && ਵਸਤੂ->max_violation)) {
				layout_flex_ਵਸਤੂ_freeze(ਸਤਰ, ਵਸਤੂ);
			}
		}
	}

	ਵਾਪਸ true;
}

/**
 * Position items along a line
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to resolve
 * \return true on success, false on failure.
 */
ਸਥਿਰ bool layout_flex_ਰੱਖੋ_ਸਤਰ_ਚੀਜ਼ਾਂ_main(
		ਬਣਤਰ flex_ctx *ctx,
		ਬਣਤਰ flex_ਸਤਰ_ਅੰਕੜੇ *ਸਤਰ)
{
	ਪੂਰਨ main_pos = ctx->flex->padding[layout_flex_main_ਸ਼ੁਰੂ_side(ctx)];
	ਪੂਰਨ post_multiplier = ctx->main_reversed ? 0 : 1;
	ਪੂਰਨ pre_multiplier = ctx->main_reversed ? -1 : 0;
	size_t ਵਸਤੂ_ਗਿਣਤੀ = ਸਤਰ->ਪਹਿਲਾ + ਸਤਰ->ਗਿਣਤੀ_2;
	ਪੂਰਨ extra_remainder = 0;
	ਪੂਰਨ extra = 0;

	ਜੇ (ctx->main_reversed) {
		main_pos = lh_box_ਮਾਪ_main(ctx->horizontal, ctx->flex) -
				main_pos;
	}

	ਜੇ (ctx->ਉਪਲਬਧ_main != AUTO &&
	    ctx->ਉਪਲਬਧ_main != UNKNOWN_ਚੌੜਾਈ &&
	    ctx->ਉਪਲਬਧ_main > ਸਤਰ->ਵਰਤਿਆ_main_ਮਾਪ) {
		ਜੇ (ਸਤਰ->main_auto_margin_ਗਿਣਤੀ > 0) {
			extra = ctx->ਉਪਲਬਧ_main - ਸਤਰ->ਵਰਤਿਆ_main_ਮਾਪ;

			extra_remainder = extra % ਸਤਰ->main_auto_margin_ਗਿਣਤੀ;
			extra /= ਸਤਰ->main_auto_margin_ਗਿਣਤੀ;
		}
	}

	ਲਈ (size_t i = ਸਤਰ->ਪਹਿਲਾ; i < ਵਸਤੂ_ਗਿਣਤੀ; i++) {
		ਗਿਣਤੀ box_side main_ਸਿਰਾ = ctx->horizontal ? ਸੱਜਾ_2 : BOTTOM;
		ਗਿਣਤੀ box_side main_ਸ਼ੁਰੂ = ctx->horizontal ? ਖੱਬਾ_2 : ਸਿਖਰ_2;
		ਬਣਤਰ flex_ਵਸਤੂ_ਅੰਕੜੇ *ਵਸਤੂ = &ctx->ਵਸਤੂ.ਅੰਕੜੇ[i];
		ਬਣਤਰ box *b = ਵਸਤੂ->box;
		ਪੂਰਨ extra_ਕੁੱਲ = 0;
		ਪੂਰਨ extra_post = 0;
		ਪੂਰਨ extra_pre = 0;
		ਪੂਰਨ box_ਮਾਪ_main;
		ਪੂਰਨ *box_pos_main;

		ਜੇ (ctx->horizontal) {
			b->ਚੌੜਾਈ = ਵਸਤੂ->ਨਿਸ਼ਾਨਾ_main_ਮਾਪ -
					lh_delta_outer_ਚੌੜਾਈ(b);

			ਜੇ (!layout_flex_ਵਸਤੂ(ctx, ਵਸਤੂ, b->ਚੌੜਾਈ)) {
				ਵਾਪਸ false;
			}
		}

		box_ਮਾਪ_main = lh_box_ਮਾਪ_main(ctx->horizontal, b);
		box_pos_main = ctx->horizontal ? &b->x : &b->y;

		ਜੇ (!lh__box_is_absolute(b)) {
			ਜੇ (b->margin[main_ਸ਼ੁਰੂ] == AUTO) {
				extra_pre = extra + extra_remainder;
			}
			ਜੇ (b->margin[main_ਸਿਰਾ] == AUTO) {
				extra_post = extra + extra_remainder;
			}
			extra_ਕੁੱਲ = extra_pre + extra_post;

			main_pos += pre_multiplier *
					(extra_ਕੁੱਲ + box_ਮਾਪ_main +
					 lh__delta_outer_main(ctx->flex, b));
		}

		*box_pos_main = main_pos + lh__non_auto_margin(b, main_ਸ਼ੁਰੂ) +
				extra_pre + b->border[main_ਸ਼ੁਰੂ].ਚੌੜਾਈ;

		ਜੇ (!lh__box_is_absolute(b)) {
			ਪੂਰਨ cross_ਮਾਪ;
			ਪੂਰਨ box_ਮਾਪ_cross = lh_box_ਮਾਪ_cross(
					ctx->horizontal, b);

			main_pos += post_multiplier *
					(extra_ਕੁੱਲ + box_ਮਾਪ_main +
					 lh__delta_outer_main(ctx->flex, b));

			cross_ਮਾਪ = box_ਮਾਪ_cross + lh__delta_outer_cross(
					ctx->flex, b);
			ਜੇ (ਸਤਰ->cross_ਮਾਪ < cross_ਮਾਪ) {
				ਸਤਰ->cross_ਮਾਪ = cross_ਮਾਪ;
			}
		}
	}

	ਵਾਪਸ true;
}

/**
 * Collect items onto lines and place items along the lines
 *
 * \param[in] ctx   Flex layout context
 * \return true on success, false on failure.
 */
ਸਥਿਰ bool layout_flex_collect_ਚੀਜ਼ਾਂ_into_ਸਤਰਾਂ(
		ਬਣਤਰ flex_ctx *ctx)
{
	size_t pos = 0;

	ਜਦੋਂ (pos < ctx->ਵਸਤੂ.ਗਿਣਤੀ_2) {
		ਬਣਤਰ flex_ਸਤਰ_ਅੰਕੜੇ *ਸਤਰ;

		ਸਤਰ = layout_flex_build_ਸਤਰ(ctx, pos);
		ਜੇ (ਸਤਰ == ਨਿਰਾ) {
			ਵਾਪਸ false;
		}

		pos += ਸਤਰ->ਗਿਣਤੀ_2;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: "
				"fitted: %zu (total: %zu/%zu)",
				ctx->flex, ਸਤਰ->ਗਿਣਤੀ_2,
				pos, ctx->ਵਸਤੂ.ਗਿਣਤੀ_2);

		ਜੇ (!layout_flex_resolve_ਸਤਰ(ctx, ਸਤਰ)) {
			ਵਾਪਸ false;
		}

		ਜੇ (!layout_flex_ਰੱਖੋ_ਸਤਰ_ਚੀਜ਼ਾਂ_main(ctx, ਸਤਰ)) {
			ਵਾਪਸ false;
		}

		ctx->cross_ਮਾਪ += ਸਤਰ->cross_ਮਾਪ;
		ਜੇ (ctx->main_ਮਾਪ < ਸਤਰ->main_ਮਾਪ) {
			ctx->main_ਮਾਪ = ਸਤਰ->main_ਮਾਪ;
		}
	}

	ਵਾਪਸ true;
}

/**
 * Align items on a line.
 *
 * \param[in] ctx    Flex layout context
 * \param[in] line   Line to align items on
 * \param[in] extra  Extra line width in pixels
 */
ਸਥਿਰ ਖਾਲੀ layout_flex_ਰੱਖੋ_ਸਤਰ_ਚੀਜ਼ਾਂ_cross(ਬਣਤਰ flex_ctx *ctx,
		ਬਣਤਰ flex_ਸਤਰ_ਅੰਕੜੇ *ਸਤਰ, ਪੂਰਨ extra)
{
	ਗਿਣਤੀ box_side cross_ਸ਼ੁਰੂ = ctx->horizontal ? ਸਿਖਰ_2 : ਖੱਬਾ_2;
	size_t ਵਸਤੂ_ਗਿਣਤੀ = ਸਤਰ->ਪਹਿਲਾ + ਸਤਰ->ਗਿਣਤੀ_2;

	ਲਈ (size_t i = ਸਤਰ->ਪਹਿਲਾ; i < ਵਸਤੂ_ਗਿਣਤੀ; i++) {
		ਬਣਤਰ flex_ਵਸਤੂ_ਅੰਕੜੇ *ਵਸਤੂ = &ctx->ਵਸਤੂ.ਅੰਕੜੇ[i];
		ਬਣਤਰ box *b = ਵਸਤੂ->box;
		ਪੂਰਨ cross_free_space;
		ਪੂਰਨ *box_ਮਾਪ_cross;
		ਪੂਰਨ *box_pos_cross;

		box_pos_cross = ctx->horizontal ? &b->y : &b->x;
		box_ਮਾਪ_cross = lh_box_ਮਾਪ_cross_ptr(ctx->horizontal, b);

		cross_free_space = ਸਤਰ->cross_ਮਾਪ + extra - *box_ਮਾਪ_cross -
				lh__delta_outer_cross(ctx->flex, b);

		ਚੋਣ (lh__box_align_self(ctx->flex, b)) {
		ਮੂਲ:
		ਹਾਲਤ CSS_ALIGN_SELF_STRETCH:
			ਜੇ (lh_box_ਮਾਪ_cross_is_auto(ctx->horizontal, b)) {
				*box_ਮਾਪ_cross += cross_free_space;

				/* Relayout children for stretch. */
				ਜੇ (!layout_flex_ਵਸਤੂ(ctx, ਵਸਤੂ, b->ਚੌੜਾਈ)) {
					ਵਾਪਸ;
				}
			}
			fallthrough;
		ਹਾਲਤ CSS_ALIGN_SELF_FLEX_ਸ਼ੁਰੂ:
			*box_pos_cross = ctx->flex->padding[cross_ਸ਼ੁਰੂ] +
					ਸਤਰ->pos +
					lh__non_auto_margin(b, cross_ਸ਼ੁਰੂ) +
					b->border[cross_ਸ਼ੁਰੂ].ਚੌੜਾਈ;
			ਰੋਕੋ;

		ਹਾਲਤ CSS_ALIGN_SELF_FLEX_ਸਿਰਾ:
			*box_pos_cross = ctx->flex->padding[cross_ਸ਼ੁਰੂ] +
					ਸਤਰ->pos + cross_free_space +
					lh__non_auto_margin(b, cross_ਸ਼ੁਰੂ) +
					b->border[cross_ਸ਼ੁਰੂ].ਚੌੜਾਈ;
			ਰੋਕੋ;

		ਹਾਲਤ CSS_ALIGN_SELF_BASELINE:
		ਹਾਲਤ CSS_ALIGN_SELF_CENTER:
			*box_pos_cross = ctx->flex->padding[cross_ਸ਼ੁਰੂ] +
					ਸਤਰ->pos + cross_free_space / 2 +
					lh__non_auto_margin(b, cross_ਸ਼ੁਰੂ) +
					b->border[cross_ਸ਼ੁਰੂ].ਚੌੜਾਈ;
			ਰੋਕੋ;
		}
	}
}

/**
 * Place the lines and align the items on the line.
 *
 * \param[in] ctx  Flex layout context
 */
ਸਥਿਰ ਖਾਲੀ layout_flex_ਰੱਖੋ_ਸਤਰਾਂ(ਬਣਤਰ flex_ctx *ctx)
{
	bool reversed = ctx->wrap == CSS_FLEX_WRAP_WRAP_ਉਲਟ;
	ਪੂਰਨ ਸਤਰ_pos = reversed ? ctx->cross_ਮਾਪ : 0;
	ਪੂਰਨ post_multiplier = reversed ? 0 : 1;
	ਪੂਰਨ pre_multiplier = reversed ? -1 : 0;
	ਪੂਰਨ extra_remainder = 0;
	ਪੂਰਨ extra = 0;

	ਜੇ (ctx->ਉਪਲਬਧ_cross != AUTO &&
	    ctx->ਉਪਲਬਧ_cross > ctx->cross_ਮਾਪ &&
	    ctx->ਸਤਰ.ਗਿਣਤੀ_2 > 0) {
		extra = ctx->ਉਪਲਬਧ_cross - ctx->cross_ਮਾਪ;

		extra_remainder = extra % ctx->ਸਤਰ.ਗਿਣਤੀ_2;
		extra /= ctx->ਸਤਰ.ਗਿਣਤੀ_2;
	}

	ਲਈ (size_t i = 0; i < ctx->ਸਤਰ.ਗਿਣਤੀ_2; i++) {
		ਬਣਤਰ flex_ਸਤਰ_ਅੰਕੜੇ *ਸਤਰ = &ctx->ਸਤਰ.ਅੰਕੜੇ[i];

		ਸਤਰ_pos += pre_multiplier * ਸਤਰ->cross_ਮਾਪ;
		ਸਤਰ->pos = ਸਤਰ_pos;
		ਸਤਰ_pos += post_multiplier * ਸਤਰ->cross_ਮਾਪ +
				extra + extra_remainder;

		layout_flex_ਰੱਖੋ_ਸਤਰ_ਚੀਜ਼ਾਂ_cross(ctx, ਸਤਰ,
				extra + extra_remainder);

		ਜੇ (extra_remainder > 0) {
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
bool layout_flex(ਬਣਤਰ box *flex, ਪੂਰਨ ਉਪਲਬਧ_ਚੌੜਾਈ,
		html_content *content)
{
	ਪੂਰਨ max_ਉਚਾਈ, min_ਉਚਾਈ;
	ਬਣਤਰ flex_ctx *ctx;
	bool success = false;

	ctx = layout_flex_ctx__create(content, flex);
	ਜੇ (ctx == ਨਿਰਾ) {
		ਵਾਪਸ false;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: %s, available_width %i, width: %i",
			flex, ctx->horizontal ? "horizontal" : "vertical",
			ਉਪਲਬਧ_ਚੌੜਾਈ, flex->ਚੌੜਾਈ);

	layout_ਲੱਭੋ_dimensions(
			ctx->unit_len_ctx, ਉਪਲਬਧ_ਚੌੜਾਈ, -1,
			flex, flex->style, ਨਿਰਾ, &flex->ਉਚਾਈ,
			ਨਿਰਾ, ਨਿਰਾ, &max_ਉਚਾਈ, &min_ਉਚਾਈ,
			flex->margin, flex->padding, flex->border);

	ਉਪਲਬਧ_ਚੌੜਾਈ = min(ਉਪਲਬਧ_ਚੌੜਾਈ, flex->ਚੌੜਾਈ);

	ਜੇ (ctx->horizontal) {
		ctx->ਉਪਲਬਧ_main = ਉਪਲਬਧ_ਚੌੜਾਈ;
		ctx->ਉਪਲਬਧ_cross = ctx->flex->ਉਚਾਈ;
	} ਨਹੀਂਤਾਂ {
		ctx->ਉਪਲਬਧ_main = ctx->flex->ਉਚਾਈ;
		ctx->ਉਪਲਬਧ_cross = ਉਪਲਬਧ_ਚੌੜਾਈ;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: available_main: %i",
			flex, ctx->ਉਪਲਬਧ_main);
	NSLOG(flex, DEEPDEBUG, "box %p: available_cross: %i",
			flex, ctx->ਉਪਲਬਧ_cross);

	layout_flex_ctx_populate_ਵਸਤੂ_ਅੰਕੜੇ(ctx, flex, ਉਪਲਬਧ_ਚੌੜਾਈ);

	/* Place items onto lines. */
	success = layout_flex_collect_ਚੀਜ਼ਾਂ_into_ਸਤਰਾਂ(ctx);
	ਜੇ (!success) {
		ਜਾਓ cleanup;
	}

	layout_flex_ਰੱਖੋ_ਸਤਰਾਂ(ctx);

	ਜੇ (flex->ਉਚਾਈ == AUTO) {
		flex->ਉਚਾਈ = ctx->horizontal ?
				ctx->cross_ਮਾਪ :
				ctx->main_ਮਾਪ;
	}

	ਜੇ (flex->ਉਚਾਈ != AUTO) {
		ਜੇ (max_ਉਚਾਈ >= 0 && flex->ਉਚਾਈ > max_ਉਚਾਈ) {
			flex->ਉਚਾਈ = max_ਉਚਾਈ;
		}
		ਜੇ (min_ਉਚਾਈ >  0 && flex->ਉਚਾਈ < min_ਉਚਾਈ) {
			flex->ਉਚਾਈ = min_ਉਚਾਈ;
		}
	}

	success = true;

cleanup:
	layout_flex_ctx__destroy(ctx);

	NSLOG(flex, DEEPDEBUG, "box %p: %s: w: %i, h: %i", flex,
			success ? "success" : "failure",
			flex->ਚੌੜਾਈ, flex->ਉਚਾਈ);
	ਵਾਪਸ success;
}
