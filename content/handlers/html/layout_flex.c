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
ಸಂರಚನಾ flex_item_data {
	ಗಣನಾ css_flex_basis_e basis;
	css_fixed basis_length;
	css_unit basis_unit;
	ಸಂರಚನಾ box *box;

	css_fixed shrink;
	css_fixed grow;

	ಪೂರ್ಣಾಙ್ಕ min_main;
	ಪೂರ್ಣಾಙ್ಕ max_main;
	ಪೂರ್ಣಾಙ್ಕ min_cross;
	ಪೂರ್ಣಾಙ್ಕ max_cross;

	ಪೂರ್ಣಾಙ್ಕ target_main_size;
	ಪೂರ್ಣಾಙ್ಕ base_size;
	ಪೂರ್ಣಾಙ್ಕ main_size;
	size_t ಸಾಲು;

	bool freeze;
	bool min_violation;
	bool max_violation;
};

/**
 * Flex line data
 */
ಸಂರಚನಾ flex_ಸಾಲು_data {
	ಪೂರ್ಣಾಙ್ಕ main_size;
	ಪೂರ್ಣಾಙ್ಕ cross_size;

	ಪೂರ್ಣಾಙ್ಕ used_main_size;
	ಪೂರ್ಣಾಙ್ಕ main_auto_margin_ಎಣಿಕೆ;

	ಪೂರ್ಣಾಙ್ಕ pos;

	size_t first;
	size_t ಎಣಿಕೆ;
	size_t frozen;
};

/**
 * Flex layout context
 */
ಸಂರಚನಾ flex_ctx {
	html_content *content;
	ಧ್ರುವ ಸಂರಚನಾ box *flex;
	ಧ್ರುವ css_unit_ctx *unit_len_ctx;

	ಪೂರ್ಣಾಙ್ಕ main_size;
	ಪೂರ್ಣಾಙ್ಕ cross_size;

	ಪೂರ್ಣಾಙ್ಕ available_main;
	ಪೂರ್ಣಾಙ್ಕ available_cross;

	bool horizontal;
	bool main_reversed;
	ಗಣನಾ css_flex_wrap_e wrap;

	ಸಂರಚನಾ flex_ವಸ್ತುಗಳು {
		size_t ಎಣಿಕೆ;
		ಸಂರಚನಾ flex_item_data *data;
	} item;

	ಸಂರಚನಾ flex_ಸಾಲುಗಳು {
		size_t ಎಣಿಕೆ;
		size_t alloc;
		ಸಂರಚನಾ flex_ಸಾಲು_data *data;
	} ಸಾಲು;
};

/**
 * Destroy a flex layout context
 *
 * \param[in] ctx  Flex layout context
 */
ಸ್ಥಿರ ಶೂನ್ಯ layout_flex_ctx__destroy(ಸಂರಚನಾ flex_ctx *ctx)
{
	ಯದಿ (ctx != ರಿಕ್ತ) {
		ಮುಕ್ತ(ctx->item.data);
		ಮುಕ್ತ(ctx->ಸಾಲು.data);
		ಮುಕ್ತ(ctx);
	}
}

/**
 * Create a flex layout context
 *
 * \param[in] content  HTML content containing flex box
 * \param[in] flex     Box to create layout context for
 * \return flex layout context or NULL on error
 */
ಸ್ಥಿರ ಸಂರಚನಾ flex_ctx *layout_flex_ctx__create(
		html_content *content,
		ಧ್ರುವ ಸಂರಚನಾ box *flex)
{
	ಸಂರಚನಾ flex_ctx *ctx;

	ctx = calloc(1, ಪರಿಮಾಣ(*ctx));
	ಯದಿ (ctx == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}
	ctx->ಸಾಲು.alloc = 1;

	ctx->item.ಎಣಿಕೆ = box_ಎಣಿಕೆ_children(flex);
	ctx->item.data = calloc(ctx->item.ಎಣಿಕೆ, ಪರಿಮಾಣ(*ctx->item.data));
	ಯದಿ (ctx->item.data == ರಿಕ್ತ) {
		layout_flex_ctx__destroy(ctx);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ctx->ಸಾಲು.alloc = 1;
	ctx->ಸಾಲು.data = calloc(ctx->ಸಾಲು.alloc, ಪರಿಮಾಣ(*ctx->ಸಾಲು.data));
	ಯದಿ (ctx->ಸಾಲು.data == ರಿಕ್ತ) {
		layout_flex_ctx__destroy(ctx);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ctx->flex = flex;
	ctx->content = content;
	ctx->unit_len_ctx = &content->unit_len_ctx;

	ctx->wrap = css_computed_flex_wrap(flex->style);
	ctx->horizontal = lh__flex_main_is_horizontal(flex);
	ctx->main_reversed = lh__flex_direction_reversed(flex);

	ಪ್ರತ್ಯಾಗಮ ctx;
}

/**
 * Find box side representing the start of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the start side.
 */
ಸ್ಥಿರ ಗಣನಾ box_side layout_flex__main_start_side(
		ಧ್ರುವ ಸಂರಚನಾ flex_ctx *ctx)
{
	ಯದಿ (ctx->horizontal) {
		ಪ್ರತ್ಯಾಗಮ (ctx->main_reversed) ? ಬಲ_2 : ಎಡ_2;
	} ಅನ್ಯಥಾ {
		ಪ್ರತ್ಯಾಗಮ (ctx->main_reversed) ? BOTTOM : ತುದಿ_2;
	}
}

/**
 * Find box side representing the end of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the end side.
 */
ಸ್ಥಿರ ಗಣನಾ box_side layout_flex__main_end_side(
		ಧ್ರುವ ಸಂರಚನಾ flex_ctx *ctx)
{
	ಯದಿ (ctx->horizontal) {
		ಪ್ರತ್ಯಾಗಮ (ctx->main_reversed) ? ಎಡ_2 : ಬಲ_2;
	} ಅನ್ಯಥಾ {
		ಪ್ರತ್ಯಾಗಮ (ctx->main_reversed) ? ತುದಿ_2 : BOTTOM;
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
ಸ್ಥಿರ bool layout_flex_item(
		ಧ್ರುವ ಸಂರಚನಾ flex_ctx *ctx,
		ಧ್ರುವ ಸಂರಚನಾ flex_item_data *item,
		ಪೂರ್ಣಾಙ್ಕ available_width)
{
	bool success;
	ಸಂರಚನಾ box *b = item->box;

	ವಿಕಲ್ಪ (b->type) {
	ಪಕ್ಷ BOX_BLOCK:
		success = layout_block_context(b, -1, ctx->content);
		ಭಙ್ಗ;
	ಪಕ್ಷ BOX_TABLE:
		b->float_container = b->parent;
		success = layout_table(b, available_width, ctx->content);
		b->float_container = ರಿಕ್ತ;
		ಭಙ್ಗ;
	ಪಕ್ಷ BOX_FLEX:
		b->float_container = b->parent;
		success = layout_flex(b, available_width, ctx->content);
		b->float_container = ರಿಕ್ತ;
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಪ್ರತಿಜ್ಞಾ(0 && "Bad flex item back type");
		success = false;
		ಭಙ್ಗ;
	}

	ಯದಿ (!success) {
		NSLOG(flex, ERROR, "box %p: layout failed", b);
	}

	ಪ್ರತ್ಯಾಗಮ success;
}

/**
 * Calculate an item's base and target main sizes.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] item             Item to get sizes of
 * \param[in] available_width  Available width in pixels
 * \return true on success false on failure
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool layout_flex__base_and_main_sizes(
		ಧ್ರುವ ಸಂರಚನಾ flex_ctx *ctx,
		ಸಂರಚನಾ flex_item_data *item,
		ಪೂರ್ಣಾಙ್ಕ available_width)
{
	ಸಂರಚನಾ box *b = item->box;
	ಪೂರ್ಣಾಙ್ಕ content_min_width = b->min_width;
	ಪೂರ್ಣಾಙ್ಕ content_max_width = b->max_width;
	ಪೂರ್ಣಾಙ್ಕ delta_outer_main = lh__delta_outer_main(ctx->flex, b);

	NSLOG(flex, DEEPDEBUG, "box %p: delta_outer_main: %i",
			b, delta_outer_main);

	ಯದಿ (item->basis == CSS_FLEX_BASIS_SET) {
		ಯದಿ (item->basis_unit == CSS_UNIT_PCT) {
			item->base_size = FPCT_OF_INT_TOINT(
					item->basis_length,
					available_width);
		} ಅನ್ಯಥಾ {
			item->base_size = FIXTOINT(css_unit_len2device_px(
					b->style, ctx->unit_len_ctx,
					item->basis_length,
					item->basis_unit));
		}

	} ಅನ್ಯಥಾ ಯದಿ (item->basis == CSS_FLEX_BASIS_AUTO) {
		item->base_size = ctx->horizontal ? b->width : b->height;
	} ಅನ್ಯಥಾ {
		item->base_size = AUTO;
	}

	ಯದಿ (ctx->horizontal == false) {
		ಯದಿ (b->width == AUTO) {
			b->width = min(max(content_min_width, available_width),
					content_max_width);
			b->width -= lh__delta_outer_width(b);
		}

		ಯದಿ (!layout_flex_item(ctx, item, b->width)) {
			ಪ್ರತ್ಯಾಗಮ false;
		}
	}

	ಯದಿ (item->base_size == AUTO) {
		ಯದಿ (ctx->horizontal == false) {
			item->base_size = b->height;
		} ಅನ್ಯಥಾ {
			item->base_size = content_max_width - delta_outer_main;
		}
	}

	item->base_size += delta_outer_main;

	ಯದಿ (ctx->horizontal) {
		item->base_size = min(item->base_size, available_width);
		item->base_size = max(item->base_size, content_min_width);
	}

	item->target_main_size = item->base_size;
	item->main_size = item->base_size;

	ಯದಿ (item->max_main > 0 &&
	    item->main_size > item->max_main + delta_outer_main) {
		item->main_size = item->max_main + delta_outer_main;
	}

	ಯದಿ (item->main_size < item->min_main + delta_outer_main) {
		item->main_size = item->min_main + delta_outer_main;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: base_size: %i, main_size %i",
			b, item->base_size, item->main_size);

	ಪ್ರತ್ಯಾಗಮ true;
}

/**
 * Fill out all item's data in a flex container.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] flex             Flex box
 * \param[in] available_width  Available width in pixels
 */
ಸ್ಥಿರ ಶೂನ್ಯ layout_flex_ctx__populate_item_data(
		ಧ್ರುವ ಸಂರಚನಾ flex_ctx *ctx,
		ಧ್ರುವ ಸಂರಚನಾ box *flex,
		ಪೂರ್ಣಾಙ್ಕ available_width)
{
	size_t i = 0;
	bool horizontal = ctx->horizontal;

	ಪ್ರತಿ (ಸಂರಚನಾ box *b = flex->children; b != ರಿಕ್ತ; b = b->next) {
		ಸಂರಚನಾ flex_item_data *item = &ctx->item.data[i++];

		b->float_container = b->parent;
		layout_find_dimensions(ctx->unit_len_ctx, available_width, -1,
				b, b->style, &b->width, &b->height,
				horizontal ? &item->max_main : &item->max_cross,
				horizontal ? &item->min_main : &item->min_cross,
				horizontal ? &item->max_cross : &item->max_main,
				horizontal ? &item->min_cross : &item->min_main,
				b->margin, b->padding, b->border);
		b->float_container = ರಿಕ್ತ;

		NSLOG(flex, DEEPDEBUG, "flex-item box: %p: width: %i",
				b, b->width);

		item->box = b;
		item->basis = css_computed_flex_basis(b->style,
				&item->basis_length, &item->basis_unit);

		css_computed_flex_shrink(b->style, &item->shrink);
		css_computed_flex_grow(b->style, &item->grow);

		layout_flex__base_and_main_sizes(ctx, item, available_width);
	}
}

/**
 * Ensure context's lines array has a free space
 *
 * \param[in] ctx  Flex layout context
 * \return true on success false on out of memory
 */
ಸ್ಥಿರ bool layout_flex_ctx_ensure_ಸಾಲು(ಸಂರಚನಾ flex_ctx *ctx)
{
	ಸಂರಚನಾ flex_ಸಾಲು_data *temp;
	size_t ಸಾಲು_alloc = ctx->ಸಾಲು.alloc * 2;

	ಯದಿ (ctx->ಸಾಲು.alloc > ctx->ಸಾಲು.ಎಣಿಕೆ) {
		ಪ್ರತ್ಯಾಗಮ true;
	}

	temp = realloc(ctx->ಸಾಲು.data, ಪರಿಮಾಣ(*ctx->ಸಾಲು.data) * ಸಾಲು_alloc);
	ಯದಿ (temp == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ false;
	}
	ctx->ಸಾಲು.data = temp;

	memset(ctx->ಸಾಲು.data + ctx->ಸಾಲು.alloc, 0,
	       ಪರಿಮಾಣ(*ctx->ಸಾಲು.data) * (ಸಾಲು_alloc - ctx->ಸಾಲು.alloc));
	ctx->ಸಾಲು.alloc = ಸಾಲು_alloc;

	ಪ್ರತ್ಯಾಗಮ true;
}

/**
 * Assigns flex items to the line and returns the line
 *
 * \param[in] ctx         Flex layout context
 * \param[in] item_index  Index to first item to assign to this line
 * \return Pointer to the new line, or NULL on error.
 */
ಸ್ಥಿರ ಸಂರಚನಾ flex_ಸಾಲು_data *layout_flex_build_ಸಾಲು(ಸಂರಚನಾ flex_ctx *ctx,
		size_t item_index)
{
	ಗಣನಾ box_side start_side = layout_flex__main_start_side(ctx);
	ಗಣನಾ box_side end_side = layout_flex__main_end_side(ctx);
	ಸಂರಚನಾ flex_ಸಾಲು_data *ಸಾಲು;
	ಪೂರ್ಣಾಙ್ಕ used_main = 0;

	ಯದಿ (!layout_flex_ctx_ensure_ಸಾಲು(ctx)) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಸಾಲು = &ctx->ಸಾಲು.data[ctx->ಸಾಲು.ಎಣಿಕೆ];
	ಸಾಲು->first = item_index;

	NSLOG(flex, DEEPDEBUG, "flex container %p: available main: %i",
			ctx->flex, ctx->available_main);

	ಯಾವತ್ (item_index < ctx->item.ಎಣಿಕೆ) {
		ಸಂರಚನಾ flex_item_data *item = &ctx->item.data[item_index];
		ಸಂರಚನಾ box *b = item->box;
		ಪೂರ್ಣಾಙ್ಕ pos_main;

		pos_main = ctx->horizontal ?
				item->main_size :
				b->height + lh__delta_outer_main(ctx->flex, b);

		ಯದಿ (ctx->wrap == CSS_FLEX_WRAP_NOWRAP ||
		    pos_main + used_main <= ctx->available_main ||
		    lh__box_is_absolute(item->box) ||
		    ctx->available_main == AUTO ||
		    ಸಾಲು->ಎಣಿಕೆ == 0 ||
		    pos_main == 0) {
			ಯದಿ (lh__box_is_absolute(item->box) == false) {
				ಸಾಲು->main_size += item->main_size;
				used_main += pos_main;

				ಯದಿ (b->margin[start_side] == AUTO) {
					ಸಾಲು->main_auto_margin_ಎಣಿಕೆ++;
				}
				ಯದಿ (b->margin[end_side] == AUTO) {
					ಸಾಲು->main_auto_margin_ಎಣಿಕೆ++;
				}
			}
			item->ಸಾಲು = ctx->ಸಾಲು.ಎಣಿಕೆ;
			ಸಾಲು->ಎಣಿಕೆ++;
			item_index++;
		} ಅನ್ಯಥಾ {
			ಭಙ್ಗ;
		}
	}

	ಯದಿ (ಸಾಲು->ಎಣಿಕೆ > 0) {
		ctx->ಸಾಲು.ಎಣಿಕೆ++;
	} ಅನ್ಯಥಾ {
		NSLOG(layout, ERROR, "Failed to fit any flex items");
	}

	ಪ್ರತ್ಯಾಗಮ ಸಾಲು;
}

/**
 * Freeze an item on a line
 *
 * \param[in] line  Line to containing item
 * \param[in] item  Item to freeze
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ layout_flex__item_freeze(
		ಸಂರಚನಾ flex_ಸಾಲು_data *ಸಾಲು,
		ಸಂರಚನಾ flex_item_data *item)
{
	item->freeze = true;
	ಸಾಲು->frozen++;

	ಯದಿ (!lh__box_is_absolute(item->box)){
		ಸಾಲು->used_main_size += item->target_main_size;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: "
			"Frozen at target_main_size: %i",
			item->box, item->target_main_size);
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
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ layout_flex__remaining_free_main(
		ಸಂರಚನಾ flex_ctx *ctx,
		ಸಂರಚನಾ flex_ಸಾಲು_data *ಸಾಲು,
		css_fixed *unfrozen_factor_sum,
		ಪೂರ್ಣಾಙ್ಕ initial_free_main,
		ಪೂರ್ಣಾಙ್ಕ available_main,
		bool grow)
{
	ಪೂರ್ಣಾಙ್ಕ remaining_free_main = available_main;
	size_t item_ಎಣಿಕೆ = ಸಾಲು->first + ಸಾಲು->ಎಣಿಕೆ;

	*unfrozen_factor_sum = 0;

	ಪ್ರತಿ (size_t i = ಸಾಲು->first; i < item_ಎಣಿಕೆ; i++) {
		ಸಂರಚನಾ flex_item_data *item = &ctx->item.data[i];

		ಯದಿ (item->freeze) {
			remaining_free_main -= item->target_main_size;
		} ಅನ್ಯಥಾ {
			remaining_free_main -= item->base_size;

			*unfrozen_factor_sum += grow ?
					item->grow : item->shrink;
		}
	}

	ಯದಿ (*unfrozen_factor_sum < F_1) {
		ಪೂರ್ಣಾಙ್ಕ free_space = FIXTOINT(FMUL(INTTOFIX(initial_free_main),
				*unfrozen_factor_sum));

		ಯದಿ (free_space < remaining_free_main) {
			remaining_free_main = free_space;
		}
	}

	NSLOG(flex, DEEPDEBUG, "Remaining free space: %i",
			remaining_free_main);

	ಪ್ರತ್ಯಾಗಮ remaining_free_main;
}

/**
 * Clamp flex item target main size and get min/max violations
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to align items on
 * return total violation in pixels
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ layout_flex__get_min_max_violations(
		ಸಂರಚನಾ flex_ctx *ctx,
		ಸಂರಚನಾ flex_ಸಾಲು_data *ಸಾಲು)
{

	ಪೂರ್ಣಾಙ್ಕ total_violation = 0;
	size_t item_ಎಣಿಕೆ = ಸಾಲು->first + ಸಾಲು->ಎಣಿಕೆ;

	ಪ್ರತಿ (size_t i = ಸಾಲು->first; i < item_ಎಣಿಕೆ; i++) {
		ಸಂರಚನಾ flex_item_data *item = &ctx->item.data[i];
		ಪೂರ್ಣಾಙ್ಕ target_main_size = item->target_main_size;

		NSLOG(flex, DEEPDEBUG, "item %p: target_main_size: %i",
					item->box, target_main_size);

		ಯದಿ (item->freeze) {
			ಅನುವರ್ತ;
		}

		ಯದಿ (item->max_main > 0 &&
		    target_main_size > item->max_main) {
			target_main_size = item->max_main;
			item->max_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: max_main: %i",
					item->max_main);
		}

		ಯದಿ (target_main_size < item->min_main) {
			target_main_size = item->min_main;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: min_main: %i",
					item->min_main);
		}

		ಯದಿ (target_main_size < item->box->min_width) {
			target_main_size = item->box->min_width;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: box min_width: %i",
					item->box->min_width);
		}

		ಯದಿ (target_main_size < 0) {
			target_main_size = 0;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: less than 0");
		}

		total_violation += target_main_size - item->target_main_size;
		item->target_main_size = target_main_size;
	}

	NSLOG(flex, DEEPDEBUG, "Total violation: %i", total_violation);

	ಪ್ರತ್ಯಾಗಮ total_violation;
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
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ layout_flex__distribute_free_main(
		ಸಂರಚನಾ flex_ctx *ctx,
		ಸಂರಚನಾ flex_ಸಾಲು_data *ಸಾಲು,
		css_fixed unfrozen_factor_sum,
		ಪೂರ್ಣಾಙ್ಕ remaining_free_main,
		bool grow)
{
	size_t item_ಎಣಿಕೆ = ಸಾಲು->first + ಸಾಲು->ಎಣಿಕೆ;

	ಯದಿ (grow) {
		css_fixed remainder = 0;
		ಪ್ರತಿ (size_t i = ಸಾಲು->first; i < item_ಎಣಿಕೆ; i++) {
			ಸಂರಚನಾ flex_item_data *item = &ctx->item.data[i];
			css_fixed result;
			css_fixed ratio;

			ಯದಿ (item->freeze) {
				ಅನುವರ್ತ;
			}

			ratio = FDIV(item->grow, unfrozen_factor_sum);
			result = FMUL(INTTOFIX(remaining_free_main), ratio) +
					remainder;

			item->target_main_size = item->base_size +
					FIXTOINT(result);
			remainder = FIXFRAC(result);
		}
	} ಅನ್ಯಥಾ {
		css_fixed scaled_shrink_factor_sum = 0;
		css_fixed remainder = 0;

		ಪ್ರತಿ (size_t i = ಸಾಲು->first; i < item_ಎಣಿಕೆ; i++) {
			ಸಂರಚನಾ flex_item_data *item = &ctx->item.data[i];
			css_fixed scaled_shrink_factor;

			ಯದಿ (item->freeze) {
				ಅನುವರ್ತ;
			}

			scaled_shrink_factor = FMUL(
					item->shrink,
					INTTOFIX(item->base_size));
			scaled_shrink_factor_sum += scaled_shrink_factor;
		}

		ಪ್ರತಿ (size_t i = ಸಾಲು->first; i < item_ಎಣಿಕೆ; i++) {
			ಸಂರಚನಾ flex_item_data *item = &ctx->item.data[i];
			css_fixed scaled_shrink_factor;
			css_fixed result;
			css_fixed ratio;

			ಯದಿ (item->freeze) {
				ಅನುವರ್ತ;
			} ಅನ್ಯಥಾ ಯದಿ (scaled_shrink_factor_sum == 0) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(ಸಾಲು, item);
				ಅನುವರ್ತ;
			}

			scaled_shrink_factor = FMUL(
					item->shrink,
					INTTOFIX(item->base_size));
			ratio = FDIV(scaled_shrink_factor,
			             scaled_shrink_factor_sum);
			result = FMUL(INTTOFIX(abs(remaining_free_main)),
			              ratio) + remainder;

			item->target_main_size = item->base_size -
					FIXTOINT(result);
			remainder = FIXFRAC(result);
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
		ಸಂರಚನಾ flex_ctx *ctx,
		ಸಂರಚನಾ flex_ಸಾಲು_data *ಸಾಲು)
{
	size_t item_ಎಣಿಕೆ = ಸಾಲು->first + ಸಾಲು->ಎಣಿಕೆ;
	ಪೂರ್ಣಾಙ್ಕ available_main = ctx->available_main;
	ಪೂರ್ಣಾಙ್ಕ initial_free_main;
	bool grow;

	ಯದಿ (available_main == AUTO) {
		available_main = INT_MAX;
	}

	grow = (ಸಾಲು->main_size < available_main);
	initial_free_main = available_main;

	NSLOG(flex, DEEPDEBUG, "box %p: line %zu: first: %zu, count: %zu",
			ctx->flex, ಸಾಲು - ctx->ಸಾಲು.data,
			ಸಾಲು->first, ಸಾಲು->ಎಣಿಕೆ);
	NSLOG(flex, DEEPDEBUG, "Line main_size: %i, available_main: %i",
			ಸಾಲು->main_size, available_main);

	ಪ್ರತಿ (size_t i = ಸಾಲು->first; i < item_ಎಣಿಕೆ; i++) {
		ಸಂರಚನಾ flex_item_data *item = &ctx->item.data[i];

		/* 3. Size inflexible items */
		ಯದಿ (grow) {
			ಯದಿ (item->grow == 0 ||
			    item->base_size > item->main_size) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(ಸಾಲು, item);
			}
		} ಅನ್ಯಥಾ {
			ಯದಿ (item->shrink == 0 ||
			    item->base_size < item->main_size) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(ಸಾಲು, item);
			}
		}

		/* 4. Calculate initial free space */
		ಯದಿ (item->freeze) {
			initial_free_main -= item->target_main_size;
		} ಅನ್ಯಥಾ {
			initial_free_main -= item->base_size;
		}
	}

	/* 5. Loop */
	ಯಾವತ್ (ಸಾಲು->frozen < ಸಾಲು->ಎಣಿಕೆ) {
		css_fixed unfrozen_factor_sum;
		ಪೂರ್ಣಾಙ್ಕ remaining_free_main;
		ಪೂರ್ಣಾಙ್ಕ total_violation;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: Resolver pass",
				ctx->flex);

		/* b */
		remaining_free_main = layout_flex__remaining_free_main(ctx,
				ಸಾಲು, &unfrozen_factor_sum, initial_free_main,
				available_main, grow);

		/* c */
		ಯದಿ (remaining_free_main != 0) {
			layout_flex__distribute_free_main(ctx,
					ಸಾಲು, unfrozen_factor_sum,
					remaining_free_main, grow);
		}

		/* d */
		total_violation = layout_flex__get_min_max_violations(
				ctx, ಸಾಲು);

		/* e */
		ಪ್ರತಿ (size_t i = ಸಾಲು->first; i < item_ಎಣಿಕೆ; i++) {
			ಸಂರಚನಾ flex_item_data *item = &ctx->item.data[i];

			ಯದಿ (item->freeze) {
				ಅನುವರ್ತ;
			}

			ಯದಿ (total_violation == 0 ||
			    (total_violation > 0 && item->min_violation) ||
			    (total_violation < 0 && item->max_violation)) {
				layout_flex__item_freeze(ಸಾಲು, item);
			}
		}
	}

	ಪ್ರತ್ಯಾಗಮ true;
}

/**
 * Position items along a line
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to resolve
 * \return true on success, false on failure.
 */
ಸ್ಥಿರ bool layout_flex_place_ಸಾಲು_ವಸ್ತುಗಳು_main(
		ಸಂರಚನಾ flex_ctx *ctx,
		ಸಂರಚನಾ flex_ಸಾಲು_data *ಸಾಲು)
{
	ಪೂರ್ಣಾಙ್ಕ main_pos = ctx->flex->padding[layout_flex__main_start_side(ctx)];
	ಪೂರ್ಣಾಙ್ಕ post_multiplier = ctx->main_reversed ? 0 : 1;
	ಪೂರ್ಣಾಙ್ಕ pre_multiplier = ctx->main_reversed ? -1 : 0;
	size_t item_ಎಣಿಕೆ = ಸಾಲು->first + ಸಾಲು->ಎಣಿಕೆ;
	ಪೂರ್ಣಾಙ್ಕ extra_remainder = 0;
	ಪೂರ್ಣಾಙ್ಕ extra = 0;

	ಯದಿ (ctx->main_reversed) {
		main_pos = lh__box_size_main(ctx->horizontal, ctx->flex) -
				main_pos;
	}

	ಯದಿ (ctx->available_main != AUTO &&
	    ctx->available_main != UNKNOWN_WIDTH &&
	    ctx->available_main > ಸಾಲು->used_main_size) {
		ಯದಿ (ಸಾಲು->main_auto_margin_ಎಣಿಕೆ > 0) {
			extra = ctx->available_main - ಸಾಲು->used_main_size;

			extra_remainder = extra % ಸಾಲು->main_auto_margin_ಎಣಿಕೆ;
			extra /= ಸಾಲು->main_auto_margin_ಎಣಿಕೆ;
		}
	}

	ಪ್ರತಿ (size_t i = ಸಾಲು->first; i < item_ಎಣಿಕೆ; i++) {
		ಗಣನಾ box_side main_end = ctx->horizontal ? ಬಲ_2 : BOTTOM;
		ಗಣನಾ box_side main_start = ctx->horizontal ? ಎಡ_2 : ತುದಿ_2;
		ಸಂರಚನಾ flex_item_data *item = &ctx->item.data[i];
		ಸಂರಚನಾ box *b = item->box;
		ಪೂರ್ಣಾಙ್ಕ extra_total = 0;
		ಪೂರ್ಣಾಙ್ಕ extra_post = 0;
		ಪೂರ್ಣಾಙ್ಕ extra_pre = 0;
		ಪೂರ್ಣಾಙ್ಕ box_size_main;
		ಪೂರ್ಣಾಙ್ಕ *box_pos_main;

		ಯದಿ (ctx->horizontal) {
			b->width = item->target_main_size -
					lh__delta_outer_width(b);

			ಯದಿ (!layout_flex_item(ctx, item, b->width)) {
				ಪ್ರತ್ಯಾಗಮ false;
			}
		}

		box_size_main = lh__box_size_main(ctx->horizontal, b);
		box_pos_main = ctx->horizontal ? &b->x : &b->y;

		ಯದಿ (!lh__box_is_absolute(b)) {
			ಯದಿ (b->margin[main_start] == AUTO) {
				extra_pre = extra + extra_remainder;
			}
			ಯದಿ (b->margin[main_end] == AUTO) {
				extra_post = extra + extra_remainder;
			}
			extra_total = extra_pre + extra_post;

			main_pos += pre_multiplier *
					(extra_total + box_size_main +
					 lh__delta_outer_main(ctx->flex, b));
		}

		*box_pos_main = main_pos + lh__non_auto_margin(b, main_start) +
				extra_pre + b->border[main_start].width;

		ಯದಿ (!lh__box_is_absolute(b)) {
			ಪೂರ್ಣಾಙ್ಕ cross_size;
			ಪೂರ್ಣಾಙ್ಕ box_size_cross = lh__box_size_cross(
					ctx->horizontal, b);

			main_pos += post_multiplier *
					(extra_total + box_size_main +
					 lh__delta_outer_main(ctx->flex, b));

			cross_size = box_size_cross + lh__delta_outer_cross(
					ctx->flex, b);
			ಯದಿ (ಸಾಲು->cross_size < cross_size) {
				ಸಾಲು->cross_size = cross_size;
			}
		}
	}

	ಪ್ರತ್ಯಾಗಮ true;
}

/**
 * Collect items onto lines and place items along the lines
 *
 * \param[in] ctx   Flex layout context
 * \return true on success, false on failure.
 */
ಸ್ಥಿರ bool layout_flex_collect_ವಸ್ತುಗಳು_into_ಸಾಲುಗಳು(
		ಸಂರಚನಾ flex_ctx *ctx)
{
	size_t pos = 0;

	ಯಾವತ್ (pos < ctx->item.ಎಣಿಕೆ) {
		ಸಂರಚನಾ flex_ಸಾಲು_data *ಸಾಲು;

		ಸಾಲು = layout_flex_build_ಸಾಲು(ctx, pos);
		ಯದಿ (ಸಾಲು == ರಿಕ್ತ) {
			ಪ್ರತ್ಯಾಗಮ false;
		}

		pos += ಸಾಲು->ಎಣಿಕೆ;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: "
				"fitted: %zu (total: %zu/%zu)",
				ctx->flex, ಸಾಲು->ಎಣಿಕೆ,
				pos, ctx->item.ಎಣಿಕೆ);

		ಯದಿ (!layout_flex_resolve_ಸಾಲು(ctx, ಸಾಲು)) {
			ಪ್ರತ್ಯಾಗಮ false;
		}

		ಯದಿ (!layout_flex_place_ಸಾಲು_ವಸ್ತುಗಳು_main(ctx, ಸಾಲು)) {
			ಪ್ರತ್ಯಾಗಮ false;
		}

		ctx->cross_size += ಸಾಲು->cross_size;
		ಯದಿ (ctx->main_size < ಸಾಲು->main_size) {
			ctx->main_size = ಸಾಲು->main_size;
		}
	}

	ಪ್ರತ್ಯಾಗಮ true;
}

/**
 * Align items on a line.
 *
 * \param[in] ctx    Flex layout context
 * \param[in] line   Line to align items on
 * \param[in] extra  Extra line width in pixels
 */
ಸ್ಥಿರ ಶೂನ್ಯ layout_flex_place_ಸಾಲು_ವಸ್ತುಗಳು_cross(ಸಂರಚನಾ flex_ctx *ctx,
		ಸಂರಚನಾ flex_ಸಾಲು_data *ಸಾಲು, ಪೂರ್ಣಾಙ್ಕ extra)
{
	ಗಣನಾ box_side cross_start = ctx->horizontal ? ತುದಿ_2 : ಎಡ_2;
	size_t item_ಎಣಿಕೆ = ಸಾಲು->first + ಸಾಲು->ಎಣಿಕೆ;

	ಪ್ರತಿ (size_t i = ಸಾಲು->first; i < item_ಎಣಿಕೆ; i++) {
		ಸಂರಚನಾ flex_item_data *item = &ctx->item.data[i];
		ಸಂರಚನಾ box *b = item->box;
		ಪೂರ್ಣಾಙ್ಕ cross_free_space;
		ಪೂರ್ಣಾಙ್ಕ *box_size_cross;
		ಪೂರ್ಣಾಙ್ಕ *box_pos_cross;

		box_pos_cross = ctx->horizontal ? &b->y : &b->x;
		box_size_cross = lh__box_size_cross_ptr(ctx->horizontal, b);

		cross_free_space = ಸಾಲು->cross_size + extra - *box_size_cross -
				lh__delta_outer_cross(ctx->flex, b);

		ವಿಕಲ್ಪ (lh__box_align_self(ctx->flex, b)) {
		ಸ್ವಭಾವ:
		ಪಕ್ಷ CSS_ALIGN_SELF_STRETCH:
			ಯದಿ (lh__box_size_cross_is_auto(ctx->horizontal, b)) {
				*box_size_cross += cross_free_space;

				/* Relayout children for stretch. */
				ಯದಿ (!layout_flex_item(ctx, item, b->width)) {
					ಪ್ರತ್ಯಾಗಮ;
				}
			}
			fallthrough;
		ಪಕ್ಷ CSS_ALIGN_SELF_FLEX_START:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					ಸಾಲು->pos +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			ಭಙ್ಗ;

		ಪಕ್ಷ CSS_ALIGN_SELF_FLEX_END:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					ಸಾಲು->pos + cross_free_space +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			ಭಙ್ಗ;

		ಪಕ್ಷ CSS_ALIGN_SELF_BASELINE:
		ಪಕ್ಷ CSS_ALIGN_SELF_CENTER:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					ಸಾಲು->pos + cross_free_space / 2 +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			ಭಙ್ಗ;
		}
	}
}

/**
 * Place the lines and align the items on the line.
 *
 * \param[in] ctx  Flex layout context
 */
ಸ್ಥಿರ ಶೂನ್ಯ layout_flex_place_ಸಾಲುಗಳು(ಸಂರಚನಾ flex_ctx *ctx)
{
	bool reversed = ctx->wrap == CSS_FLEX_WRAP_WRAP_REVERSE;
	ಪೂರ್ಣಾಙ್ಕ ಸಾಲು_pos = reversed ? ctx->cross_size : 0;
	ಪೂರ್ಣಾಙ್ಕ post_multiplier = reversed ? 0 : 1;
	ಪೂರ್ಣಾಙ್ಕ pre_multiplier = reversed ? -1 : 0;
	ಪೂರ್ಣಾಙ್ಕ extra_remainder = 0;
	ಪೂರ್ಣಾಙ್ಕ extra = 0;

	ಯದಿ (ctx->available_cross != AUTO &&
	    ctx->available_cross > ctx->cross_size &&
	    ctx->ಸಾಲು.ಎಣಿಕೆ > 0) {
		extra = ctx->available_cross - ctx->cross_size;

		extra_remainder = extra % ctx->ಸಾಲು.ಎಣಿಕೆ;
		extra /= ctx->ಸಾಲು.ಎಣಿಕೆ;
	}

	ಪ್ರತಿ (size_t i = 0; i < ctx->ಸಾಲು.ಎಣಿಕೆ; i++) {
		ಸಂರಚನಾ flex_ಸಾಲು_data *ಸಾಲು = &ctx->ಸಾಲು.data[i];

		ಸಾಲು_pos += pre_multiplier * ಸಾಲು->cross_size;
		ಸಾಲು->pos = ಸಾಲು_pos;
		ಸಾಲು_pos += post_multiplier * ಸಾಲು->cross_size +
				extra + extra_remainder;

		layout_flex_place_ಸಾಲು_ವಸ್ತುಗಳು_cross(ctx, ಸಾಲು,
				extra + extra_remainder);

		ಯದಿ (extra_remainder > 0) {
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
bool layout_flex(ಸಂರಚನಾ box *flex, ಪೂರ್ಣಾಙ್ಕ available_width,
		html_content *content)
{
	ಪೂರ್ಣಾಙ್ಕ max_height, min_height;
	ಸಂರಚನಾ flex_ctx *ctx;
	bool success = false;

	ctx = layout_flex_ctx__create(content, flex);
	ಯದಿ (ctx == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: %s, available_width %i, width: %i",
			flex, ctx->horizontal ? "horizontal" : "vertical",
			available_width, flex->width);

	layout_find_dimensions(
			ctx->unit_len_ctx, available_width, -1,
			flex, flex->style, ರಿಕ್ತ, &flex->height,
			ರಿಕ್ತ, ರಿಕ್ತ, &max_height, &min_height,
			flex->margin, flex->padding, flex->border);

	available_width = min(available_width, flex->width);

	ಯದಿ (ctx->horizontal) {
		ctx->available_main = available_width;
		ctx->available_cross = ctx->flex->height;
	} ಅನ್ಯಥಾ {
		ctx->available_main = ctx->flex->height;
		ctx->available_cross = available_width;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: available_main: %i",
			flex, ctx->available_main);
	NSLOG(flex, DEEPDEBUG, "box %p: available_cross: %i",
			flex, ctx->available_cross);

	layout_flex_ctx__populate_item_data(ctx, flex, available_width);

	/* Place items onto lines. */
	success = layout_flex_collect_ವಸ್ತುಗಳು_into_ಸಾಲುಗಳು(ctx);
	ಯದಿ (!success) {
		ಗಚ್ಛ cleanup;
	}

	layout_flex_place_ಸಾಲುಗಳು(ctx);

	ಯದಿ (flex->height == AUTO) {
		flex->height = ctx->horizontal ?
				ctx->cross_size :
				ctx->main_size;
	}

	ಯದಿ (flex->height != AUTO) {
		ಯದಿ (max_height >= 0 && flex->height > max_height) {
			flex->height = max_height;
		}
		ಯದಿ (min_height >  0 && flex->height < min_height) {
			flex->height = min_height;
		}
	}

	success = true;

cleanup:
	layout_flex_ctx__destroy(ctx);

	NSLOG(flex, DEEPDEBUG, "box %p: %s: w: %i, h: %i", flex,
			success ? "success" : "failure",
			flex->width, flex->height);
	ಪ್ರತ್ಯಾಗಮ success;
}
