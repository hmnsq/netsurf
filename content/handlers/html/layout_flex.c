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

#include <string.h>

#include "utils/log.h"
#include "utils/utils.h"

#include "html/box.h"
#include "html/html.h"
#include "html/private.h"
#include "html/box_inspect.h"
#include "html/layout_internal.h"

/**
 * Flex item data
 */
sa_nracanaa flex_item_data {
	ga_nanaa css_flex_basis_e basis;
	css_fixed basis_length;
	css_unit basis_unit;
	sa_nracanaa box *box;

	css_fixed shrink;
	css_fixed grow;

	จำนวนเต็ม min_main;
	จำนวนเต็ม max_main;
	จำนวนเต็ม min_cross;
	จำนวนเต็ม max_cross;

	จำนวนเต็ม target_main_size;
	จำนวนเต็ม base_size;
	จำนวนเต็ม main_size;
	size_t line;

	bool freeze;
	bool min_violation;
	bool max_violation;
};

/**
 * Flex line data
 */
sa_nracanaa flex_line_data {
	จำนวนเต็ม main_size;
	จำนวนเต็ม cross_size;

	จำนวนเต็ม used_main_size;
	จำนวนเต็ม main_auto_margin_count;

	จำนวนเต็ม pos;

	size_t first;
	size_t count;
	size_t frozen;
};

/**
 * Flex layout context
 */
sa_nracanaa flex_ctx {
	html_content *content;
	const sa_nracanaa box *flex;
	const css_unit_ctx *unit_len_ctx;

	จำนวนเต็ม main_size;
	จำนวนเต็ม cross_size;

	จำนวนเต็ม available_main;
	จำนวนเต็ม available_cross;

	bool horizontal;
	bool main_reversed;
	ga_nanaa css_flex_wrap_e wrap;

	sa_nracanaa flex_items {
		size_t count;
		sa_nracanaa flex_item_data *data;
	} item;

	sa_nracanaa flex_lines {
		size_t count;
		size_t alloc;
		sa_nracanaa flex_line_data *data;
	} line;
};

/**
 * Destroy a flex layout context
 *
 * \param[in] ctx  Flex layout context
 */
static void layout_flex_ctx__destroy(sa_nracanaa flex_ctx *ctx)
{
	ถ้า (ctx != NULL) {
		free(ctx->item.data);
		free(ctx->line.data);
		free(ctx);
	}
}

/**
 * Create a flex layout context
 *
 * \param[in] content  HTML content containing flex box
 * \param[in] flex     Box to create layout context for
 * \return flex layout context or NULL on error
 */
static sa_nracanaa flex_ctx *layout_flex_ctx__create(
		html_content *content,
		const sa_nracanaa box *flex)
{
	sa_nracanaa flex_ctx *ctx;

	ctx = calloc(1, parimaa_na(*ctx));
	ถ้า (ctx == NULL) {
		คืน NULL;
	}
	ctx->line.alloc = 1;

	ctx->item.count = box_count_children(flex);
	ctx->item.data = calloc(ctx->item.count, parimaa_na(*ctx->item.data));
	ถ้า (ctx->item.data == NULL) {
		layout_flex_ctx__destroy(ctx);
		คืน NULL;
	}

	ctx->line.alloc = 1;
	ctx->line.data = calloc(ctx->line.alloc, parimaa_na(*ctx->line.data));
	ถ้า (ctx->line.data == NULL) {
		layout_flex_ctx__destroy(ctx);
		คืน NULL;
	}

	ctx->flex = flex;
	ctx->content = content;
	ctx->unit_len_ctx = &content->unit_len_ctx;

	ctx->wrap = css_computed_flex_wrap(flex->style);
	ctx->horizontal = lh__flex_main_is_horizontal(flex);
	ctx->main_reversed = lh__flex_direction_reversed(flex);

	คืน ctx;
}

/**
 * Find box side representing the start of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the start side.
 */
static ga_nanaa box_side layout_flex__main_start_side(
		const sa_nracanaa flex_ctx *ctx)
{
	ถ้า (ctx->horizontal) {
		คืน (ctx->main_reversed) ? RIGHT : LEFT;
	} มิฉะนั้น {
		คืน (ctx->main_reversed) ? BOTTOM : TOP;
	}
}

/**
 * Find box side representing the end of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the end side.
 */
static ga_nanaa box_side layout_flex__main_end_side(
		const sa_nracanaa flex_ctx *ctx)
{
	ถ้า (ctx->horizontal) {
		คืน (ctx->main_reversed) ? LEFT : RIGHT;
	} มิฉะนั้น {
		คืน (ctx->main_reversed) ? TOP : BOTTOM;
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
static bool layout_flex_item(
		const sa_nracanaa flex_ctx *ctx,
		const sa_nracanaa flex_item_data *item,
		จำนวนเต็ม available_width)
{
	bool success;
	sa_nracanaa box *b = item->box;

	switch (b->type) {
	pak_sa BOX_BLOCK:
		success = layout_block_context(b, -1, ctx->content);
		bha_nga;
	pak_sa BOX_TABLE:
		b->float_container = b->parent;
		success = layout_table(b, available_width, ctx->content);
		b->float_container = NULL;
		bha_nga;
	pak_sa BOX_FLEX:
		b->float_container = b->parent;
		success = layout_flex(b, available_width, ctx->content);
		b->float_container = NULL;
		bha_nga;
	default:
		assert(0 && "Bad flex item back type");
		success = false;
		bha_nga;
	}

	ถ้า (!success) {
		NSLOG(flex, ERROR, "box %p: layout failed", b);
	}

	คืน success;
}

/**
 * Calculate an item's base and target main sizes.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] item             Item to get sizes of
 * \param[in] available_width  Available width in pixels
 * \return true on success false on failure
 */
static anta_hstha bool layout_flex__base_and_main_sizes(
		const sa_nracanaa flex_ctx *ctx,
		sa_nracanaa flex_item_data *item,
		จำนวนเต็ม available_width)
{
	sa_nracanaa box *b = item->box;
	จำนวนเต็ม content_min_width = b->min_width;
	จำนวนเต็ม content_max_width = b->max_width;
	จำนวนเต็ม delta_outer_main = lh__delta_outer_main(ctx->flex, b);

	NSLOG(flex, DEEPDEBUG, "box %p: delta_outer_main: %i",
			b, delta_outer_main);

	ถ้า (item->basis == CSS_FLEX_BASIS_SET) {
		ถ้า (item->basis_unit == CSS_UNIT_PCT) {
			item->base_size = FPCT_OF_INT_TOINT(
					item->basis_length,
					available_width);
		} มิฉะนั้น {
			item->base_size = FIXTOINT(css_unit_len2device_px(
					b->style, ctx->unit_len_ctx,
					item->basis_length,
					item->basis_unit));
		}

	} มิฉะนั้น ถ้า (item->basis == CSS_FLEX_BASIS_AUTO) {
		item->base_size = ctx->horizontal ? b->width : b->height;
	} มิฉะนั้น {
		item->base_size = AUTO;
	}

	ถ้า (ctx->horizontal == false) {
		ถ้า (b->width == AUTO) {
			b->width = min(max(content_min_width, available_width),
					content_max_width);
			b->width -= lh__delta_outer_width(b);
		}

		ถ้า (!layout_flex_item(ctx, item, b->width)) {
			คืน false;
		}
	}

	ถ้า (item->base_size == AUTO) {
		ถ้า (ctx->horizontal == false) {
			item->base_size = b->height;
		} มิฉะนั้น {
			item->base_size = content_max_width - delta_outer_main;
		}
	}

	item->base_size += delta_outer_main;

	ถ้า (ctx->horizontal) {
		item->base_size = min(item->base_size, available_width);
		item->base_size = max(item->base_size, content_min_width);
	}

	item->target_main_size = item->base_size;
	item->main_size = item->base_size;

	ถ้า (item->max_main > 0 &&
	    item->main_size > item->max_main + delta_outer_main) {
		item->main_size = item->max_main + delta_outer_main;
	}

	ถ้า (item->main_size < item->min_main + delta_outer_main) {
		item->main_size = item->min_main + delta_outer_main;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: base_size: %i, main_size %i",
			b, item->base_size, item->main_size);

	คืน true;
}

/**
 * Fill out all item's data in a flex container.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] flex             Flex box
 * \param[in] available_width  Available width in pixels
 */
static void layout_flex_ctx__populate_item_data(
		const sa_nracanaa flex_ctx *ctx,
		const sa_nracanaa box *flex,
		จำนวนเต็ม available_width)
{
	size_t i = 0;
	bool horizontal = ctx->horizontal;

	สำหรับ (sa_nracanaa box *b = flex->children; b != NULL; b = b->next) {
		sa_nracanaa flex_item_data *item = &ctx->item.data[i++];

		b->float_container = b->parent;
		layout_find_dimensions(ctx->unit_len_ctx, available_width, -1,
				b, b->style, &b->width, &b->height,
				horizontal ? &item->max_main : &item->max_cross,
				horizontal ? &item->min_main : &item->min_cross,
				horizontal ? &item->max_cross : &item->max_main,
				horizontal ? &item->min_cross : &item->min_main,
				b->margin, b->padding, b->border);
		b->float_container = NULL;

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
static bool layout_flex_ctx__ensure_line(sa_nracanaa flex_ctx *ctx)
{
	sa_nracanaa flex_line_data *temp;
	size_t line_alloc = ctx->line.alloc * 2;

	ถ้า (ctx->line.alloc > ctx->line.count) {
		คืน true;
	}

	temp = realloc(ctx->line.data, parimaa_na(*ctx->line.data) * line_alloc);
	ถ้า (temp == NULL) {
		คืน false;
	}
	ctx->line.data = temp;

	memset(ctx->line.data + ctx->line.alloc, 0,
	       parimaa_na(*ctx->line.data) * (line_alloc - ctx->line.alloc));
	ctx->line.alloc = line_alloc;

	คืน true;
}

/**
 * Assigns flex items to the line and returns the line
 *
 * \param[in] ctx         Flex layout context
 * \param[in] item_index  Index to first item to assign to this line
 * \return Pointer to the new line, or NULL on error.
 */
static sa_nracanaa flex_line_data *layout_flex__build_line(sa_nracanaa flex_ctx *ctx,
		size_t item_index)
{
	ga_nanaa box_side start_side = layout_flex__main_start_side(ctx);
	ga_nanaa box_side end_side = layout_flex__main_end_side(ctx);
	sa_nracanaa flex_line_data *line;
	จำนวนเต็ม used_main = 0;

	ถ้า (!layout_flex_ctx__ensure_line(ctx)) {
		คืน NULL;
	}

	line = &ctx->line.data[ctx->line.count];
	line->first = item_index;

	NSLOG(flex, DEEPDEBUG, "flex container %p: available main: %i",
			ctx->flex, ctx->available_main);

	ขณะ (item_index < ctx->item.count) {
		sa_nracanaa flex_item_data *item = &ctx->item.data[item_index];
		sa_nracanaa box *b = item->box;
		จำนวนเต็ม pos_main;

		pos_main = ctx->horizontal ?
				item->main_size :
				b->height + lh__delta_outer_main(ctx->flex, b);

		ถ้า (ctx->wrap == CSS_FLEX_WRAP_NOWRAP ||
		    pos_main + used_main <= ctx->available_main ||
		    lh__box_is_absolute(item->box) ||
		    ctx->available_main == AUTO ||
		    line->count == 0 ||
		    pos_main == 0) {
			ถ้า (lh__box_is_absolute(item->box) == false) {
				line->main_size += item->main_size;
				used_main += pos_main;

				ถ้า (b->margin[start_side] == AUTO) {
					line->main_auto_margin_count++;
				}
				ถ้า (b->margin[end_side] == AUTO) {
					line->main_auto_margin_count++;
				}
			}
			item->line = ctx->line.count;
			line->count++;
			item_index++;
		} มิฉะนั้น {
			bha_nga;
		}
	}

	ถ้า (line->count > 0) {
		ctx->line.count++;
	} มิฉะนั้น {
		NSLOG(layout, ERROR, "Failed to fit any flex items");
	}

	คืน line;
}

/**
 * Freeze an item on a line
 *
 * \param[in] line  Line to containing item
 * \param[in] item  Item to freeze
 */
static anta_hstha void layout_flex__item_freeze(
		sa_nracanaa flex_line_data *line,
		sa_nracanaa flex_item_data *item)
{
	item->freeze = true;
	line->frozen++;

	ถ้า (!lh__box_is_absolute(item->box)){
		line->used_main_size += item->target_main_size;
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
static anta_hstha จำนวนเต็ม layout_flex__remaining_free_main(
		sa_nracanaa flex_ctx *ctx,
		sa_nracanaa flex_line_data *line,
		css_fixed *unfrozen_factor_sum,
		จำนวนเต็ม initial_free_main,
		จำนวนเต็ม available_main,
		bool grow)
{
	จำนวนเต็ม remaining_free_main = available_main;
	size_t item_count = line->first + line->count;

	*unfrozen_factor_sum = 0;

	สำหรับ (size_t i = line->first; i < item_count; i++) {
		sa_nracanaa flex_item_data *item = &ctx->item.data[i];

		ถ้า (item->freeze) {
			remaining_free_main -= item->target_main_size;
		} มิฉะนั้น {
			remaining_free_main -= item->base_size;

			*unfrozen_factor_sum += grow ?
					item->grow : item->shrink;
		}
	}

	ถ้า (*unfrozen_factor_sum < F_1) {
		จำนวนเต็ม free_space = FIXTOINT(FMUL(INTTOFIX(initial_free_main),
				*unfrozen_factor_sum));

		ถ้า (free_space < remaining_free_main) {
			remaining_free_main = free_space;
		}
	}

	NSLOG(flex, DEEPDEBUG, "Remaining free space: %i",
			remaining_free_main);

	คืน remaining_free_main;
}

/**
 * Clamp flex item target main size and get min/max violations
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to align items on
 * return total violation in pixels
 */
static anta_hstha จำนวนเต็ม layout_flex__get_min_max_violations(
		sa_nracanaa flex_ctx *ctx,
		sa_nracanaa flex_line_data *line)
{

	จำนวนเต็ม total_violation = 0;
	size_t item_count = line->first + line->count;

	สำหรับ (size_t i = line->first; i < item_count; i++) {
		sa_nracanaa flex_item_data *item = &ctx->item.data[i];
		จำนวนเต็ม target_main_size = item->target_main_size;

		NSLOG(flex, DEEPDEBUG, "item %p: target_main_size: %i",
					item->box, target_main_size);

		ถ้า (item->freeze) {
			continue;
		}

		ถ้า (item->max_main > 0 &&
		    target_main_size > item->max_main) {
			target_main_size = item->max_main;
			item->max_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: max_main: %i",
					item->max_main);
		}

		ถ้า (target_main_size < item->min_main) {
			target_main_size = item->min_main;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: min_main: %i",
					item->min_main);
		}

		ถ้า (target_main_size < item->box->min_width) {
			target_main_size = item->box->min_width;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: box min_width: %i",
					item->box->min_width);
		}

		ถ้า (target_main_size < 0) {
			target_main_size = 0;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: less than 0");
		}

		total_violation += target_main_size - item->target_main_size;
		item->target_main_size = target_main_size;
	}

	NSLOG(flex, DEEPDEBUG, "Total violation: %i", total_violation);

	คืน total_violation;
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
static anta_hstha void layout_flex__distribute_free_main(
		sa_nracanaa flex_ctx *ctx,
		sa_nracanaa flex_line_data *line,
		css_fixed unfrozen_factor_sum,
		จำนวนเต็ม remaining_free_main,
		bool grow)
{
	size_t item_count = line->first + line->count;

	ถ้า (grow) {
		css_fixed remainder = 0;
		สำหรับ (size_t i = line->first; i < item_count; i++) {
			sa_nracanaa flex_item_data *item = &ctx->item.data[i];
			css_fixed result;
			css_fixed ratio;

			ถ้า (item->freeze) {
				continue;
			}

			ratio = FDIV(item->grow, unfrozen_factor_sum);
			result = FMUL(INTTOFIX(remaining_free_main), ratio) +
					remainder;

			item->target_main_size = item->base_size +
					FIXTOINT(result);
			remainder = FIXFRAC(result);
		}
	} มิฉะนั้น {
		css_fixed scaled_shrink_factor_sum = 0;
		css_fixed remainder = 0;

		สำหรับ (size_t i = line->first; i < item_count; i++) {
			sa_nracanaa flex_item_data *item = &ctx->item.data[i];
			css_fixed scaled_shrink_factor;

			ถ้า (item->freeze) {
				continue;
			}

			scaled_shrink_factor = FMUL(
					item->shrink,
					INTTOFIX(item->base_size));
			scaled_shrink_factor_sum += scaled_shrink_factor;
		}

		สำหรับ (size_t i = line->first; i < item_count; i++) {
			sa_nracanaa flex_item_data *item = &ctx->item.data[i];
			css_fixed scaled_shrink_factor;
			css_fixed result;
			css_fixed ratio;

			ถ้า (item->freeze) {
				continue;
			} มิฉะนั้น ถ้า (scaled_shrink_factor_sum == 0) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(line, item);
				continue;
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
static bool layout_flex__resolve_line(
		sa_nracanaa flex_ctx *ctx,
		sa_nracanaa flex_line_data *line)
{
	size_t item_count = line->first + line->count;
	จำนวนเต็ม available_main = ctx->available_main;
	จำนวนเต็ม initial_free_main;
	bool grow;

	ถ้า (available_main == AUTO) {
		available_main = INT_MAX;
	}

	grow = (line->main_size < available_main);
	initial_free_main = available_main;

	NSLOG(flex, DEEPDEBUG, "box %p: line %zu: first: %zu, count: %zu",
			ctx->flex, line - ctx->line.data,
			line->first, line->count);
	NSLOG(flex, DEEPDEBUG, "Line main_size: %i, available_main: %i",
			line->main_size, available_main);

	สำหรับ (size_t i = line->first; i < item_count; i++) {
		sa_nracanaa flex_item_data *item = &ctx->item.data[i];

		/* 3. Size inflexible items */
		ถ้า (grow) {
			ถ้า (item->grow == 0 ||
			    item->base_size > item->main_size) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(line, item);
			}
		} มิฉะนั้น {
			ถ้า (item->shrink == 0 ||
			    item->base_size < item->main_size) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(line, item);
			}
		}

		/* 4. Calculate initial free space */
		ถ้า (item->freeze) {
			initial_free_main -= item->target_main_size;
		} มิฉะนั้น {
			initial_free_main -= item->base_size;
		}
	}

	/* 5. Loop */
	ขณะ (line->frozen < line->count) {
		css_fixed unfrozen_factor_sum;
		จำนวนเต็ม remaining_free_main;
		จำนวนเต็ม total_violation;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: Resolver pass",
				ctx->flex);

		/* b */
		remaining_free_main = layout_flex__remaining_free_main(ctx,
				line, &unfrozen_factor_sum, initial_free_main,
				available_main, grow);

		/* c */
		ถ้า (remaining_free_main != 0) {
			layout_flex__distribute_free_main(ctx,
					line, unfrozen_factor_sum,
					remaining_free_main, grow);
		}

		/* d */
		total_violation = layout_flex__get_min_max_violations(
				ctx, line);

		/* e */
		สำหรับ (size_t i = line->first; i < item_count; i++) {
			sa_nracanaa flex_item_data *item = &ctx->item.data[i];

			ถ้า (item->freeze) {
				continue;
			}

			ถ้า (total_violation == 0 ||
			    (total_violation > 0 && item->min_violation) ||
			    (total_violation < 0 && item->max_violation)) {
				layout_flex__item_freeze(line, item);
			}
		}
	}

	คืน true;
}

/**
 * Position items along a line
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to resolve
 * \return true on success, false on failure.
 */
static bool layout_flex__place_line_items_main(
		sa_nracanaa flex_ctx *ctx,
		sa_nracanaa flex_line_data *line)
{
	จำนวนเต็ม main_pos = ctx->flex->padding[layout_flex__main_start_side(ctx)];
	จำนวนเต็ม post_multiplier = ctx->main_reversed ? 0 : 1;
	จำนวนเต็ม pre_multiplier = ctx->main_reversed ? -1 : 0;
	size_t item_count = line->first + line->count;
	จำนวนเต็ม extra_remainder = 0;
	จำนวนเต็ม extra = 0;

	ถ้า (ctx->main_reversed) {
		main_pos = lh__box_size_main(ctx->horizontal, ctx->flex) -
				main_pos;
	}

	ถ้า (ctx->available_main != AUTO &&
	    ctx->available_main != UNKNOWN_WIDTH &&
	    ctx->available_main > line->used_main_size) {
		ถ้า (line->main_auto_margin_count > 0) {
			extra = ctx->available_main - line->used_main_size;

			extra_remainder = extra % line->main_auto_margin_count;
			extra /= line->main_auto_margin_count;
		}
	}

	สำหรับ (size_t i = line->first; i < item_count; i++) {
		ga_nanaa box_side main_end = ctx->horizontal ? RIGHT : BOTTOM;
		ga_nanaa box_side main_start = ctx->horizontal ? LEFT : TOP;
		sa_nracanaa flex_item_data *item = &ctx->item.data[i];
		sa_nracanaa box *b = item->box;
		จำนวนเต็ม extra_total = 0;
		จำนวนเต็ม extra_post = 0;
		จำนวนเต็ม extra_pre = 0;
		จำนวนเต็ม box_size_main;
		จำนวนเต็ม *box_pos_main;

		ถ้า (ctx->horizontal) {
			b->width = item->target_main_size -
					lh__delta_outer_width(b);

			ถ้า (!layout_flex_item(ctx, item, b->width)) {
				คืน false;
			}
		}

		box_size_main = lh__box_size_main(ctx->horizontal, b);
		box_pos_main = ctx->horizontal ? &b->x : &b->y;

		ถ้า (!lh__box_is_absolute(b)) {
			ถ้า (b->margin[main_start] == AUTO) {
				extra_pre = extra + extra_remainder;
			}
			ถ้า (b->margin[main_end] == AUTO) {
				extra_post = extra + extra_remainder;
			}
			extra_total = extra_pre + extra_post;

			main_pos += pre_multiplier *
					(extra_total + box_size_main +
					 lh__delta_outer_main(ctx->flex, b));
		}

		*box_pos_main = main_pos + lh__non_auto_margin(b, main_start) +
				extra_pre + b->border[main_start].width;

		ถ้า (!lh__box_is_absolute(b)) {
			จำนวนเต็ม cross_size;
			จำนวนเต็ม box_size_cross = lh__box_size_cross(
					ctx->horizontal, b);

			main_pos += post_multiplier *
					(extra_total + box_size_main +
					 lh__delta_outer_main(ctx->flex, b));

			cross_size = box_size_cross + lh__delta_outer_cross(
					ctx->flex, b);
			ถ้า (line->cross_size < cross_size) {
				line->cross_size = cross_size;
			}
		}
	}

	คืน true;
}

/**
 * Collect items onto lines and place items along the lines
 *
 * \param[in] ctx   Flex layout context
 * \return true on success, false on failure.
 */
static bool layout_flex__collect_items_into_lines(
		sa_nracanaa flex_ctx *ctx)
{
	size_t pos = 0;

	ขณะ (pos < ctx->item.count) {
		sa_nracanaa flex_line_data *line;

		line = layout_flex__build_line(ctx, pos);
		ถ้า (line == NULL) {
			คืน false;
		}

		pos += line->count;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: "
				"fitted: %zu (total: %zu/%zu)",
				ctx->flex, line->count,
				pos, ctx->item.count);

		ถ้า (!layout_flex__resolve_line(ctx, line)) {
			คืน false;
		}

		ถ้า (!layout_flex__place_line_items_main(ctx, line)) {
			คืน false;
		}

		ctx->cross_size += line->cross_size;
		ถ้า (ctx->main_size < line->main_size) {
			ctx->main_size = line->main_size;
		}
	}

	คืน true;
}

/**
 * Align items on a line.
 *
 * \param[in] ctx    Flex layout context
 * \param[in] line   Line to align items on
 * \param[in] extra  Extra line width in pixels
 */
static void layout_flex__place_line_items_cross(sa_nracanaa flex_ctx *ctx,
		sa_nracanaa flex_line_data *line, จำนวนเต็ม extra)
{
	ga_nanaa box_side cross_start = ctx->horizontal ? TOP : LEFT;
	size_t item_count = line->first + line->count;

	สำหรับ (size_t i = line->first; i < item_count; i++) {
		sa_nracanaa flex_item_data *item = &ctx->item.data[i];
		sa_nracanaa box *b = item->box;
		จำนวนเต็ม cross_free_space;
		จำนวนเต็ม *box_size_cross;
		จำนวนเต็ม *box_pos_cross;

		box_pos_cross = ctx->horizontal ? &b->y : &b->x;
		box_size_cross = lh__box_size_cross_ptr(ctx->horizontal, b);

		cross_free_space = line->cross_size + extra - *box_size_cross -
				lh__delta_outer_cross(ctx->flex, b);

		switch (lh__box_align_self(ctx->flex, b)) {
		default:
		pak_sa CSS_ALIGN_SELF_STRETCH:
			ถ้า (lh__box_size_cross_is_auto(ctx->horizontal, b)) {
				*box_size_cross += cross_free_space;

				/* Relayout children for stretch. */
				ถ้า (!layout_flex_item(ctx, item, b->width)) {
					คืน;
				}
			}
			fallthrough;
		pak_sa CSS_ALIGN_SELF_FLEX_START:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					line->pos +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			bha_nga;

		pak_sa CSS_ALIGN_SELF_FLEX_END:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					line->pos + cross_free_space +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			bha_nga;

		pak_sa CSS_ALIGN_SELF_BASELINE:
		pak_sa CSS_ALIGN_SELF_CENTER:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					line->pos + cross_free_space / 2 +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			bha_nga;
		}
	}
}

/**
 * Place the lines and align the items on the line.
 *
 * \param[in] ctx  Flex layout context
 */
static void layout_flex__place_lines(sa_nracanaa flex_ctx *ctx)
{
	bool reversed = ctx->wrap == CSS_FLEX_WRAP_WRAP_REVERSE;
	จำนวนเต็ม line_pos = reversed ? ctx->cross_size : 0;
	จำนวนเต็ม post_multiplier = reversed ? 0 : 1;
	จำนวนเต็ม pre_multiplier = reversed ? -1 : 0;
	จำนวนเต็ม extra_remainder = 0;
	จำนวนเต็ม extra = 0;

	ถ้า (ctx->available_cross != AUTO &&
	    ctx->available_cross > ctx->cross_size &&
	    ctx->line.count > 0) {
		extra = ctx->available_cross - ctx->cross_size;

		extra_remainder = extra % ctx->line.count;
		extra /= ctx->line.count;
	}

	สำหรับ (size_t i = 0; i < ctx->line.count; i++) {
		sa_nracanaa flex_line_data *line = &ctx->line.data[i];

		line_pos += pre_multiplier * line->cross_size;
		line->pos = line_pos;
		line_pos += post_multiplier * line->cross_size +
				extra + extra_remainder;

		layout_flex__place_line_items_cross(ctx, line,
				extra + extra_remainder);

		ถ้า (extra_remainder > 0) {
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
bool layout_flex(sa_nracanaa box *flex, จำนวนเต็ม available_width,
		html_content *content)
{
	จำนวนเต็ม max_height, min_height;
	sa_nracanaa flex_ctx *ctx;
	bool success = false;

	ctx = layout_flex_ctx__create(content, flex);
	ถ้า (ctx == NULL) {
		คืน false;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: %s, available_width %i, width: %i",
			flex, ctx->horizontal ? "horizontal" : "vertical",
			available_width, flex->width);

	layout_find_dimensions(
			ctx->unit_len_ctx, available_width, -1,
			flex, flex->style, NULL, &flex->height,
			NULL, NULL, &max_height, &min_height,
			flex->margin, flex->padding, flex->border);

	available_width = min(available_width, flex->width);

	ถ้า (ctx->horizontal) {
		ctx->available_main = available_width;
		ctx->available_cross = ctx->flex->height;
	} มิฉะนั้น {
		ctx->available_main = ctx->flex->height;
		ctx->available_cross = available_width;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: available_main: %i",
			flex, ctx->available_main);
	NSLOG(flex, DEEPDEBUG, "box %p: available_cross: %i",
			flex, ctx->available_cross);

	layout_flex_ctx__populate_item_data(ctx, flex, available_width);

	/* Place items onto lines. */
	success = layout_flex__collect_items_into_lines(ctx);
	ถ้า (!success) {
		goto cleanup;
	}

	layout_flex__place_lines(ctx);

	ถ้า (flex->height == AUTO) {
		flex->height = ctx->horizontal ?
				ctx->cross_size :
				ctx->main_size;
	}

	ถ้า (flex->height != AUTO) {
		ถ้า (max_height >= 0 && flex->height > max_height) {
			flex->height = max_height;
		}
		ถ้า (min_height >  0 && flex->height < min_height) {
			flex->height = min_height;
		}
	}

	success = true;

cleanup:
	layout_flex_ctx__destroy(ctx);

	NSLOG(flex, DEEPDEBUG, "box %p: %s: w: %i, h: %i", flex,
			success ? "success" : "failure",
			flex->width, flex->height);
	คืน success;
}
