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

#ضمن <string.h>

#ضمن "utils/log.h"
#ضمن "utils/utils.h"

#ضمن "html/box.h"
#ضمن "html/html.h"
#ضمن "html/private.h"
#ضمن "html/box_inspect.h"
#ضمن "html/layout_internal.h"

/**
 * Flex item data
 */
بنية flex_item_data {
	تعداد css_flex_basis_e basis;
	css_fixed basis_length;
	css_unit basis_unit;
	بنية box *box;

	css_fixed shrink;
	css_fixed grow;

	صحيح min_main;
	صحيح max_main;
	صحيح min_cross;
	صحيح max_cross;

	صحيح target_main_size;
	صحيح base_size;
	صحيح main_size;
	size_t لي;

	bool freeze;
	bool min_violation;
	bool max_violation;
};

/**
 * Flex line data
 */
بنية flex_لي_data {
	صحيح main_size;
	صحيح cross_size;

	صحيح used_main_size;
	صحيح main_auto_margin_كدي;

	صحيح pos;

	size_t first;
	size_t كدي;
	size_t frozen;
};

/**
 * Flex layout context
 */
بنية flex_ctx {
	html_content *content;
	ثابت بنية box *flex;
	ثابت css_unit_ctx *unit_len_ctx;

	صحيح main_size;
	صحيح cross_size;

	صحيح available_main;
	صحيح available_cross;

	bool horizontal;
	bool main_reversed;
	تعداد css_flex_wrap_e wrap;

	بنية flex_ابوبو {
		size_t كدي;
		بنية flex_item_data *data;
	} item;

	بنية flex_ليوك {
		size_t كدي;
		size_t alloc;
		بنية flex_لي_data *data;
	} لي;
};

/**
 * Destroy a flex layout context
 *
 * \param[in] ctx  Flex layout context
 */
ساكن فراغ layout_flex_ctx__destroy(بنية flex_ctx *ctx)
{
	إذا (ctx != عدم) {
		حرر(ctx->item.data);
		حرر(ctx->لي.data);
		حرر(ctx);
	}
}

/**
 * Create a flex layout context
 *
 * \param[in] content  HTML content containing flex box
 * \param[in] flex     Box to create layout context for
 * \return flex layout context or NULL on error
 */
ساكن بنية flex_ctx *layout_flex_ctx__create(
		html_content *content,
		ثابت بنية box *flex)
{
	بنية flex_ctx *ctx;

	ctx = calloc(1, حجم(*ctx));
	إذا (ctx == عدم) {
		أرجع عدم;
	}
	ctx->لي.alloc = 1;

	ctx->item.كدي = box_كدي_children(flex);
	ctx->item.data = calloc(ctx->item.كدي, حجم(*ctx->item.data));
	إذا (ctx->item.data == عدم) {
		layout_flex_ctx__destroy(ctx);
		أرجع عدم;
	}

	ctx->لي.alloc = 1;
	ctx->لي.data = calloc(ctx->لي.alloc, حجم(*ctx->لي.data));
	إذا (ctx->لي.data == عدم) {
		layout_flex_ctx__destroy(ctx);
		أرجع عدم;
	}

	ctx->flex = flex;
	ctx->content = content;
	ctx->unit_len_ctx = &content->unit_len_ctx;

	ctx->wrap = css_computed_flex_wrap(flex->style);
	ctx->horizontal = lh__flex_main_is_horizontal(flex);
	ctx->main_reversed = lh__flex_direction_reversed(flex);

	أرجع ctx;
}

/**
 * Find box side representing the start of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the start side.
 */
ساكن تعداد box_side layout_flex__main_start_side(
		ثابت بنية flex_ctx *ctx)
{
	إذا (ctx->horizontal) {
		أرجع (ctx->main_reversed) ? دام_2 : هغو_2;
	} وإلا {
		أرجع (ctx->main_reversed) ? BOTTOM : كولي_2;
	}
}

/**
 * Find box side representing the end of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the end side.
 */
ساكن تعداد box_side layout_flex__main_end_side(
		ثابت بنية flex_ctx *ctx)
{
	إذا (ctx->horizontal) {
		أرجع (ctx->main_reversed) ? هغو_2 : دام_2;
	} وإلا {
		أرجع (ctx->main_reversed) ? كولي_2 : BOTTOM;
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
ساكن bool layout_flex_item(
		ثابت بنية flex_ctx *ctx,
		ثابت بنية flex_item_data *item,
		صحيح available_width)
{
	bool success;
	بنية box *b = item->box;

	بدل (b->type) {
	حالة BOX_BLOCK:
		success = layout_block_context(b, -1, ctx->content);
		اكسر;
	حالة BOX_TABLE:
		b->float_container = b->parent;
		success = layout_table(b, available_width, ctx->content);
		b->float_container = عدم;
		اكسر;
	حالة BOX_FLEX:
		b->float_container = b->parent;
		success = layout_flex(b, available_width, ctx->content);
		b->float_container = عدم;
		اكسر;
	افتراضي:
		أكد(0 && "Bad flex item back type");
		success = false;
		اكسر;
	}

	إذا (!success) {
		NSLOG(flex, ERROR, "box %p: layout failed", b);
	}

	أرجع success;
}

/**
 * Calculate an item's base and target main sizes.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] item             Item to get sizes of
 * \param[in] available_width  Available width in pixels
 * \return true on success false on failure
 */
ساكن مضمن bool layout_flex__base_and_main_sizes(
		ثابت بنية flex_ctx *ctx,
		بنية flex_item_data *item,
		صحيح available_width)
{
	بنية box *b = item->box;
	صحيح content_min_width = b->min_width;
	صحيح content_max_width = b->max_width;
	صحيح delta_outer_main = lh__delta_outer_main(ctx->flex, b);

	NSLOG(flex, DEEPDEBUG, "box %p: delta_outer_main: %i",
			b, delta_outer_main);

	إذا (item->basis == CSS_FLEX_BASIS_SET) {
		إذا (item->basis_unit == CSS_UNIT_PCT) {
			item->base_size = FPCT_OF_INT_TOINT(
					item->basis_length,
					available_width);
		} وإلا {
			item->base_size = FIXTOINT(css_unit_len2device_px(
					b->style, ctx->unit_len_ctx,
					item->basis_length,
					item->basis_unit));
		}

	} وإلا إذا (item->basis == CSS_FLEX_BASIS_AUTO) {
		item->base_size = ctx->horizontal ? b->width : b->height;
	} وإلا {
		item->base_size = AUTO;
	}

	إذا (ctx->horizontal == false) {
		إذا (b->width == AUTO) {
			b->width = min(max(content_min_width, available_width),
					content_max_width);
			b->width -= lh__delta_outer_width(b);
		}

		إذا (!layout_flex_item(ctx, item, b->width)) {
			أرجع false;
		}
	}

	إذا (item->base_size == AUTO) {
		إذا (ctx->horizontal == false) {
			item->base_size = b->height;
		} وإلا {
			item->base_size = content_max_width - delta_outer_main;
		}
	}

	item->base_size += delta_outer_main;

	إذا (ctx->horizontal) {
		item->base_size = min(item->base_size, available_width);
		item->base_size = max(item->base_size, content_min_width);
	}

	item->target_main_size = item->base_size;
	item->main_size = item->base_size;

	إذا (item->max_main > 0 &&
	    item->main_size > item->max_main + delta_outer_main) {
		item->main_size = item->max_main + delta_outer_main;
	}

	إذا (item->main_size < item->min_main + delta_outer_main) {
		item->main_size = item->min_main + delta_outer_main;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: base_size: %i, main_size %i",
			b, item->base_size, item->main_size);

	أرجع true;
}

/**
 * Fill out all item's data in a flex container.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] flex             Flex box
 * \param[in] available_width  Available width in pixels
 */
ساكن فراغ layout_flex_ctx__populate_item_data(
		ثابت بنية flex_ctx *ctx,
		ثابت بنية box *flex,
		صحيح available_width)
{
	size_t i = 0;
	bool horizontal = ctx->horizontal;

	لكل (بنية box *b = flex->children; b != عدم; b = b->next) {
		بنية flex_item_data *item = &ctx->item.data[i++];

		b->float_container = b->parent;
		layout_find_dimensions(ctx->unit_len_ctx, available_width, -1,
				b, b->style, &b->width, &b->height,
				horizontal ? &item->max_main : &item->max_cross,
				horizontal ? &item->min_main : &item->min_cross,
				horizontal ? &item->max_cross : &item->max_main,
				horizontal ? &item->min_cross : &item->min_main,
				b->margin, b->padding, b->border);
		b->float_container = عدم;

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
ساكن bool layout_flex_ctx_ensure_لي(بنية flex_ctx *ctx)
{
	بنية flex_لي_data *temp;
	size_t لي_alloc = ctx->لي.alloc * 2;

	إذا (ctx->لي.alloc > ctx->لي.كدي) {
		أرجع true;
	}

	temp = realloc(ctx->لي.data, حجم(*ctx->لي.data) * لي_alloc);
	إذا (temp == عدم) {
		أرجع false;
	}
	ctx->لي.data = temp;

	memset(ctx->لي.data + ctx->لي.alloc, 0,
	       حجم(*ctx->لي.data) * (لي_alloc - ctx->لي.alloc));
	ctx->لي.alloc = لي_alloc;

	أرجع true;
}

/**
 * Assigns flex items to the line and returns the line
 *
 * \param[in] ctx         Flex layout context
 * \param[in] item_index  Index to first item to assign to this line
 * \return Pointer to the new line, or NULL on error.
 */
ساكن بنية flex_لي_data *layout_flex_build_لي(بنية flex_ctx *ctx,
		size_t item_index)
{
	تعداد box_side start_side = layout_flex__main_start_side(ctx);
	تعداد box_side end_side = layout_flex__main_end_side(ctx);
	بنية flex_لي_data *لي;
	صحيح used_main = 0;

	إذا (!layout_flex_ctx_ensure_لي(ctx)) {
		أرجع عدم;
	}

	لي = &ctx->لي.data[ctx->لي.كدي];
	لي->first = item_index;

	NSLOG(flex, DEEPDEBUG, "flex container %p: available main: %i",
			ctx->flex, ctx->available_main);

	طالما (item_index < ctx->item.كدي) {
		بنية flex_item_data *item = &ctx->item.data[item_index];
		بنية box *b = item->box;
		صحيح pos_main;

		pos_main = ctx->horizontal ?
				item->main_size :
				b->height + lh__delta_outer_main(ctx->flex, b);

		إذا (ctx->wrap == CSS_FLEX_WRAP_NOWRAP ||
		    pos_main + used_main <= ctx->available_main ||
		    lh__box_is_absolute(item->box) ||
		    ctx->available_main == AUTO ||
		    لي->كدي == 0 ||
		    pos_main == 0) {
			إذا (lh__box_is_absolute(item->box) == false) {
				لي->main_size += item->main_size;
				used_main += pos_main;

				إذا (b->margin[start_side] == AUTO) {
					لي->main_auto_margin_كدي++;
				}
				إذا (b->margin[end_side] == AUTO) {
					لي->main_auto_margin_كدي++;
				}
			}
			item->لي = ctx->لي.كدي;
			لي->كدي++;
			item_index++;
		} وإلا {
			اكسر;
		}
	}

	إذا (لي->كدي > 0) {
		ctx->لي.كدي++;
	} وإلا {
		NSLOG(layout, ERROR, "Failed to fit any flex items");
	}

	أرجع لي;
}

/**
 * Freeze an item on a line
 *
 * \param[in] line  Line to containing item
 * \param[in] item  Item to freeze
 */
ساكن مضمن فراغ layout_flex__item_freeze(
		بنية flex_لي_data *لي,
		بنية flex_item_data *item)
{
	item->freeze = true;
	لي->frozen++;

	إذا (!lh__box_is_absolute(item->box)){
		لي->used_main_size += item->target_main_size;
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
ساكن مضمن صحيح layout_flex__remaining_free_main(
		بنية flex_ctx *ctx,
		بنية flex_لي_data *لي,
		css_fixed *unfrozen_factor_sum,
		صحيح initial_free_main,
		صحيح available_main,
		bool grow)
{
	صحيح remaining_free_main = available_main;
	size_t item_كدي = لي->first + لي->كدي;

	*unfrozen_factor_sum = 0;

	لكل (size_t i = لي->first; i < item_كدي; i++) {
		بنية flex_item_data *item = &ctx->item.data[i];

		إذا (item->freeze) {
			remaining_free_main -= item->target_main_size;
		} وإلا {
			remaining_free_main -= item->base_size;

			*unfrozen_factor_sum += grow ?
					item->grow : item->shrink;
		}
	}

	إذا (*unfrozen_factor_sum < F_1) {
		صحيح free_space = FIXTOINT(FMUL(INTTOFIX(initial_free_main),
				*unfrozen_factor_sum));

		إذا (free_space < remaining_free_main) {
			remaining_free_main = free_space;
		}
	}

	NSLOG(flex, DEEPDEBUG, "Remaining free space: %i",
			remaining_free_main);

	أرجع remaining_free_main;
}

/**
 * Clamp flex item target main size and get min/max violations
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to align items on
 * return total violation in pixels
 */
ساكن مضمن صحيح layout_flex__get_min_max_violations(
		بنية flex_ctx *ctx,
		بنية flex_لي_data *لي)
{

	صحيح total_violation = 0;
	size_t item_كدي = لي->first + لي->كدي;

	لكل (size_t i = لي->first; i < item_كدي; i++) {
		بنية flex_item_data *item = &ctx->item.data[i];
		صحيح target_main_size = item->target_main_size;

		NSLOG(flex, DEEPDEBUG, "item %p: target_main_size: %i",
					item->box, target_main_size);

		إذا (item->freeze) {
			استمر;
		}

		إذا (item->max_main > 0 &&
		    target_main_size > item->max_main) {
			target_main_size = item->max_main;
			item->max_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: max_main: %i",
					item->max_main);
		}

		إذا (target_main_size < item->min_main) {
			target_main_size = item->min_main;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: min_main: %i",
					item->min_main);
		}

		إذا (target_main_size < item->box->min_width) {
			target_main_size = item->box->min_width;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: box min_width: %i",
					item->box->min_width);
		}

		إذا (target_main_size < 0) {
			target_main_size = 0;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: less than 0");
		}

		total_violation += target_main_size - item->target_main_size;
		item->target_main_size = target_main_size;
	}

	NSLOG(flex, DEEPDEBUG, "Total violation: %i", total_violation);

	أرجع total_violation;
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
ساكن مضمن فراغ layout_flex__distribute_free_main(
		بنية flex_ctx *ctx,
		بنية flex_لي_data *لي,
		css_fixed unfrozen_factor_sum,
		صحيح remaining_free_main,
		bool grow)
{
	size_t item_كدي = لي->first + لي->كدي;

	إذا (grow) {
		css_fixed remainder = 0;
		لكل (size_t i = لي->first; i < item_كدي; i++) {
			بنية flex_item_data *item = &ctx->item.data[i];
			css_fixed result;
			css_fixed ratio;

			إذا (item->freeze) {
				استمر;
			}

			ratio = FDIV(item->grow, unfrozen_factor_sum);
			result = FMUL(INTTOFIX(remaining_free_main), ratio) +
					remainder;

			item->target_main_size = item->base_size +
					FIXTOINT(result);
			remainder = FIXFRAC(result);
		}
	} وإلا {
		css_fixed scaled_shrink_factor_sum = 0;
		css_fixed remainder = 0;

		لكل (size_t i = لي->first; i < item_كدي; i++) {
			بنية flex_item_data *item = &ctx->item.data[i];
			css_fixed scaled_shrink_factor;

			إذا (item->freeze) {
				استمر;
			}

			scaled_shrink_factor = FMUL(
					item->shrink,
					INTTOFIX(item->base_size));
			scaled_shrink_factor_sum += scaled_shrink_factor;
		}

		لكل (size_t i = لي->first; i < item_كدي; i++) {
			بنية flex_item_data *item = &ctx->item.data[i];
			css_fixed scaled_shrink_factor;
			css_fixed result;
			css_fixed ratio;

			إذا (item->freeze) {
				استمر;
			} وإلا إذا (scaled_shrink_factor_sum == 0) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(لي, item);
				استمر;
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
ساكن bool layout_flex_resolve_لي(
		بنية flex_ctx *ctx,
		بنية flex_لي_data *لي)
{
	size_t item_كدي = لي->first + لي->كدي;
	صحيح available_main = ctx->available_main;
	صحيح initial_free_main;
	bool grow;

	إذا (available_main == AUTO) {
		available_main = INT_MAX;
	}

	grow = (لي->main_size < available_main);
	initial_free_main = available_main;

	NSLOG(flex, DEEPDEBUG, "box %p: line %zu: first: %zu, count: %zu",
			ctx->flex, لي - ctx->لي.data,
			لي->first, لي->كدي);
	NSLOG(flex, DEEPDEBUG, "Line main_size: %i, available_main: %i",
			لي->main_size, available_main);

	لكل (size_t i = لي->first; i < item_كدي; i++) {
		بنية flex_item_data *item = &ctx->item.data[i];

		/* 3. Size inflexible items */
		إذا (grow) {
			إذا (item->grow == 0 ||
			    item->base_size > item->main_size) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(لي, item);
			}
		} وإلا {
			إذا (item->shrink == 0 ||
			    item->base_size < item->main_size) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(لي, item);
			}
		}

		/* 4. Calculate initial free space */
		إذا (item->freeze) {
			initial_free_main -= item->target_main_size;
		} وإلا {
			initial_free_main -= item->base_size;
		}
	}

	/* 5. Loop */
	طالما (لي->frozen < لي->كدي) {
		css_fixed unfrozen_factor_sum;
		صحيح remaining_free_main;
		صحيح total_violation;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: Resolver pass",
				ctx->flex);

		/* b */
		remaining_free_main = layout_flex__remaining_free_main(ctx,
				لي, &unfrozen_factor_sum, initial_free_main,
				available_main, grow);

		/* c */
		إذا (remaining_free_main != 0) {
			layout_flex__distribute_free_main(ctx,
					لي, unfrozen_factor_sum,
					remaining_free_main, grow);
		}

		/* d */
		total_violation = layout_flex__get_min_max_violations(
				ctx, لي);

		/* e */
		لكل (size_t i = لي->first; i < item_كدي; i++) {
			بنية flex_item_data *item = &ctx->item.data[i];

			إذا (item->freeze) {
				استمر;
			}

			إذا (total_violation == 0 ||
			    (total_violation > 0 && item->min_violation) ||
			    (total_violation < 0 && item->max_violation)) {
				layout_flex__item_freeze(لي, item);
			}
		}
	}

	أرجع true;
}

/**
 * Position items along a line
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to resolve
 * \return true on success, false on failure.
 */
ساكن bool layout_flex_place_لي_ابوبو_main(
		بنية flex_ctx *ctx,
		بنية flex_لي_data *لي)
{
	صحيح main_pos = ctx->flex->padding[layout_flex__main_start_side(ctx)];
	صحيح post_multiplier = ctx->main_reversed ? 0 : 1;
	صحيح pre_multiplier = ctx->main_reversed ? -1 : 0;
	size_t item_كدي = لي->first + لي->كدي;
	صحيح extra_remainder = 0;
	صحيح extra = 0;

	إذا (ctx->main_reversed) {
		main_pos = lh__box_size_main(ctx->horizontal, ctx->flex) -
				main_pos;
	}

	إذا (ctx->available_main != AUTO &&
	    ctx->available_main != UNKNOWN_WIDTH &&
	    ctx->available_main > لي->used_main_size) {
		إذا (لي->main_auto_margin_كدي > 0) {
			extra = ctx->available_main - لي->used_main_size;

			extra_remainder = extra % لي->main_auto_margin_كدي;
			extra /= لي->main_auto_margin_كدي;
		}
	}

	لكل (size_t i = لي->first; i < item_كدي; i++) {
		تعداد box_side main_end = ctx->horizontal ? دام_2 : BOTTOM;
		تعداد box_side main_start = ctx->horizontal ? هغو_2 : كولي_2;
		بنية flex_item_data *item = &ctx->item.data[i];
		بنية box *b = item->box;
		صحيح extra_total = 0;
		صحيح extra_post = 0;
		صحيح extra_pre = 0;
		صحيح box_size_main;
		صحيح *box_pos_main;

		إذا (ctx->horizontal) {
			b->width = item->target_main_size -
					lh__delta_outer_width(b);

			إذا (!layout_flex_item(ctx, item, b->width)) {
				أرجع false;
			}
		}

		box_size_main = lh__box_size_main(ctx->horizontal, b);
		box_pos_main = ctx->horizontal ? &b->x : &b->y;

		إذا (!lh__box_is_absolute(b)) {
			إذا (b->margin[main_start] == AUTO) {
				extra_pre = extra + extra_remainder;
			}
			إذا (b->margin[main_end] == AUTO) {
				extra_post = extra + extra_remainder;
			}
			extra_total = extra_pre + extra_post;

			main_pos += pre_multiplier *
					(extra_total + box_size_main +
					 lh__delta_outer_main(ctx->flex, b));
		}

		*box_pos_main = main_pos + lh__non_auto_margin(b, main_start) +
				extra_pre + b->border[main_start].width;

		إذا (!lh__box_is_absolute(b)) {
			صحيح cross_size;
			صحيح box_size_cross = lh__box_size_cross(
					ctx->horizontal, b);

			main_pos += post_multiplier *
					(extra_total + box_size_main +
					 lh__delta_outer_main(ctx->flex, b));

			cross_size = box_size_cross + lh__delta_outer_cross(
					ctx->flex, b);
			إذا (لي->cross_size < cross_size) {
				لي->cross_size = cross_size;
			}
		}
	}

	أرجع true;
}

/**
 * Collect items onto lines and place items along the lines
 *
 * \param[in] ctx   Flex layout context
 * \return true on success, false on failure.
 */
ساكن bool layout_flex_collect_ابوبو_into_ليوك(
		بنية flex_ctx *ctx)
{
	size_t pos = 0;

	طالما (pos < ctx->item.كدي) {
		بنية flex_لي_data *لي;

		لي = layout_flex_build_لي(ctx, pos);
		إذا (لي == عدم) {
			أرجع false;
		}

		pos += لي->كدي;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: "
				"fitted: %zu (total: %zu/%zu)",
				ctx->flex, لي->كدي,
				pos, ctx->item.كدي);

		إذا (!layout_flex_resolve_لي(ctx, لي)) {
			أرجع false;
		}

		إذا (!layout_flex_place_لي_ابوبو_main(ctx, لي)) {
			أرجع false;
		}

		ctx->cross_size += لي->cross_size;
		إذا (ctx->main_size < لي->main_size) {
			ctx->main_size = لي->main_size;
		}
	}

	أرجع true;
}

/**
 * Align items on a line.
 *
 * \param[in] ctx    Flex layout context
 * \param[in] line   Line to align items on
 * \param[in] extra  Extra line width in pixels
 */
ساكن فراغ layout_flex_place_لي_ابوبو_cross(بنية flex_ctx *ctx,
		بنية flex_لي_data *لي, صحيح extra)
{
	تعداد box_side cross_start = ctx->horizontal ? كولي_2 : هغو_2;
	size_t item_كدي = لي->first + لي->كدي;

	لكل (size_t i = لي->first; i < item_كدي; i++) {
		بنية flex_item_data *item = &ctx->item.data[i];
		بنية box *b = item->box;
		صحيح cross_free_space;
		صحيح *box_size_cross;
		صحيح *box_pos_cross;

		box_pos_cross = ctx->horizontal ? &b->y : &b->x;
		box_size_cross = lh__box_size_cross_ptr(ctx->horizontal, b);

		cross_free_space = لي->cross_size + extra - *box_size_cross -
				lh__delta_outer_cross(ctx->flex, b);

		بدل (lh__box_align_self(ctx->flex, b)) {
		افتراضي:
		حالة CSS_ALIGN_SELF_STRETCH:
			إذا (lh__box_size_cross_is_auto(ctx->horizontal, b)) {
				*box_size_cross += cross_free_space;

				/* Relayout children for stretch. */
				إذا (!layout_flex_item(ctx, item, b->width)) {
					أرجع;
				}
			}
			fallthrough;
		حالة CSS_ALIGN_SELF_FLEX_START:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					لي->pos +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			اكسر;

		حالة CSS_ALIGN_SELF_FLEX_END:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					لي->pos + cross_free_space +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			اكسر;

		حالة CSS_ALIGN_SELF_BASELINE:
		حالة CSS_ALIGN_SELF_CENTER:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					لي->pos + cross_free_space / 2 +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			اكسر;
		}
	}
}

/**
 * Place the lines and align the items on the line.
 *
 * \param[in] ctx  Flex layout context
 */
ساكن فراغ layout_flex_place_ليوك(بنية flex_ctx *ctx)
{
	bool reversed = ctx->wrap == CSS_FLEX_WRAP_WRAP_REVERSE;
	صحيح لي_pos = reversed ? ctx->cross_size : 0;
	صحيح post_multiplier = reversed ? 0 : 1;
	صحيح pre_multiplier = reversed ? -1 : 0;
	صحيح extra_remainder = 0;
	صحيح extra = 0;

	إذا (ctx->available_cross != AUTO &&
	    ctx->available_cross > ctx->cross_size &&
	    ctx->لي.كدي > 0) {
		extra = ctx->available_cross - ctx->cross_size;

		extra_remainder = extra % ctx->لي.كدي;
		extra /= ctx->لي.كدي;
	}

	لكل (size_t i = 0; i < ctx->لي.كدي; i++) {
		بنية flex_لي_data *لي = &ctx->لي.data[i];

		لي_pos += pre_multiplier * لي->cross_size;
		لي->pos = لي_pos;
		لي_pos += post_multiplier * لي->cross_size +
				extra + extra_remainder;

		layout_flex_place_لي_ابوبو_cross(ctx, لي,
				extra + extra_remainder);

		إذا (extra_remainder > 0) {
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
bool layout_flex(بنية box *flex, صحيح available_width,
		html_content *content)
{
	صحيح max_height, min_height;
	بنية flex_ctx *ctx;
	bool success = false;

	ctx = layout_flex_ctx__create(content, flex);
	إذا (ctx == عدم) {
		أرجع false;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: %s, available_width %i, width: %i",
			flex, ctx->horizontal ? "horizontal" : "vertical",
			available_width, flex->width);

	layout_find_dimensions(
			ctx->unit_len_ctx, available_width, -1,
			flex, flex->style, عدم, &flex->height,
			عدم, عدم, &max_height, &min_height,
			flex->margin, flex->padding, flex->border);

	available_width = min(available_width, flex->width);

	إذا (ctx->horizontal) {
		ctx->available_main = available_width;
		ctx->available_cross = ctx->flex->height;
	} وإلا {
		ctx->available_main = ctx->flex->height;
		ctx->available_cross = available_width;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: available_main: %i",
			flex, ctx->available_main);
	NSLOG(flex, DEEPDEBUG, "box %p: available_cross: %i",
			flex, ctx->available_cross);

	layout_flex_ctx__populate_item_data(ctx, flex, available_width);

	/* Place items onto lines. */
	success = layout_flex_collect_ابوبو_into_ليوك(ctx);
	إذا (!success) {
		اذهب cleanup;
	}

	layout_flex_place_ليوك(ctx);

	إذا (flex->height == AUTO) {
		flex->height = ctx->horizontal ?
				ctx->cross_size :
				ctx->main_size;
	}

	إذا (flex->height != AUTO) {
		إذا (max_height >= 0 && flex->height > max_height) {
			flex->height = max_height;
		}
		إذا (min_height >  0 && flex->height < min_height) {
			flex->height = min_height;
		}
	}

	success = true;

cleanup:
	layout_flex_ctx__destroy(ctx);

	NSLOG(flex, DEEPDEBUG, "box %p: %s: w: %i, h: %i", flex,
			success ? "success" : "failure",
			flex->width, flex->height);
	أرجع success;
}
