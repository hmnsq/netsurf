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

#شامل <string.h>

#شامل "utils/log.h"
#شامل "utils/utils.h"

#شامل "html/box.h"
#شامل "html/html.h"
#شامل "html/private.h"
#شامل "html/box_inspect.h"
#شامل "html/layout_internal.h"

/**
 * Flex item data
 */
ساختار flex_item_data {
	شمارش css_flex_basis_e basis;
	css_fixed basis_length;
	css_unit basis_unit;
	ساختار box *box;

	css_fixed shrink;
	css_fixed grow;

	صحیح min_main;
	صحیح max_main;
	صحیح min_cross;
	صحیح max_cross;

	صحیح target_main_size;
	صحیح base_size;
	صحیح main_size;
	size_t سطر;

	bool freeze;
	bool min_violation;
	bool max_violation;
};

/**
 * Flex line data
 */
ساختار flex_سطر_data {
	صحیح main_size;
	صحیح cross_size;

	صحیح used_main_size;
	صحیح main_auto_margin_شمار;

	صحیح pos;

	size_t first;
	size_t شمار;
	size_t frozen;
};

/**
 * Flex layout context
 */
ساختار flex_ctx {
	html_content *content;
	ثابت ساختار box *flex;
	ثابت css_unit_ctx *unit_len_ctx;

	صحیح main_size;
	صحیح cross_size;

	صحیح available_main;
	صحیح available_cross;

	bool horizontal;
	bool main_reversed;
	شمارش css_flex_wrap_e wrap;

	ساختار flex_اقلام {
		size_t شمار;
		ساختار flex_item_data *data;
	} item;

	ساختار flex_سطرها {
		size_t شمار;
		size_t alloc;
		ساختار flex_سطر_data *data;
	} سطر;
};

/**
 * Destroy a flex layout context
 *
 * \param[in] ctx  Flex layout context
 */
ایستا تهی layout_flex_ctx__destroy(ساختار flex_ctx *ctx)
{
	اگر (ctx != پوچ) {
		آزاد(ctx->item.data);
		آزاد(ctx->سطر.data);
		آزاد(ctx);
	}
}

/**
 * Create a flex layout context
 *
 * \param[in] content  HTML content containing flex box
 * \param[in] flex     Box to create layout context for
 * \return flex layout context or NULL on error
 */
ایستا ساختار flex_ctx *layout_flex_ctx__create(
		html_content *content,
		ثابت ساختار box *flex)
{
	ساختار flex_ctx *ctx;

	ctx = calloc(1, اندازه(*ctx));
	اگر (ctx == پوچ) {
		بازگشت پوچ;
	}
	ctx->سطر.alloc = 1;

	ctx->item.شمار = box_شمار_children(flex);
	ctx->item.data = calloc(ctx->item.شمار, اندازه(*ctx->item.data));
	اگر (ctx->item.data == پوچ) {
		layout_flex_ctx__destroy(ctx);
		بازگشت پوچ;
	}

	ctx->سطر.alloc = 1;
	ctx->سطر.data = calloc(ctx->سطر.alloc, اندازه(*ctx->سطر.data));
	اگر (ctx->سطر.data == پوچ) {
		layout_flex_ctx__destroy(ctx);
		بازگشت پوچ;
	}

	ctx->flex = flex;
	ctx->content = content;
	ctx->unit_len_ctx = &content->unit_len_ctx;

	ctx->wrap = css_computed_flex_wrap(flex->style);
	ctx->horizontal = lh__flex_main_is_horizontal(flex);
	ctx->main_reversed = lh__flex_direction_reversed(flex);

	بازگشت ctx;
}

/**
 * Find box side representing the start of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the start side.
 */
ایستا شمارش box_side layout_flex__main_start_side(
		ثابت ساختار flex_ctx *ctx)
{
	اگر (ctx->horizontal) {
		بازگشت (ctx->main_reversed) ? راست_2 : چپ_2;
	} وگرنه {
		بازگشت (ctx->main_reversed) ? BOTTOM : بالاترین_2;
	}
}

/**
 * Find box side representing the end of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the end side.
 */
ایستا شمارش box_side layout_flex__main_end_side(
		ثابت ساختار flex_ctx *ctx)
{
	اگر (ctx->horizontal) {
		بازگشت (ctx->main_reversed) ? چپ_2 : راست_2;
	} وگرنه {
		بازگشت (ctx->main_reversed) ? بالاترین_2 : BOTTOM;
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
ایستا bool layout_flex_item(
		ثابت ساختار flex_ctx *ctx,
		ثابت ساختار flex_item_data *item,
		صحیح available_width)
{
	bool success;
	ساختار box *b = item->box;

	انتخاب (b->type) {
	حالت BOX_BLOCK:
		success = layout_block_context(b, -1, ctx->content);
		بشکن;
	حالت BOX_TABLE:
		b->float_container = b->parent;
		success = layout_table(b, available_width, ctx->content);
		b->float_container = پوچ;
		بشکن;
	حالت BOX_FLEX:
		b->float_container = b->parent;
		success = layout_flex(b, available_width, ctx->content);
		b->float_container = پوچ;
		بشکن;
	پیشفرض:
		أكد(0 && "Bad flex item back type");
		success = false;
		بشکن;
	}

	اگر (!success) {
		NSLOG(flex, ERROR, "box %p: layout failed", b);
	}

	بازگشت success;
}

/**
 * Calculate an item's base and target main sizes.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] item             Item to get sizes of
 * \param[in] available_width  Available width in pixels
 * \return true on success false on failure
 */
ایستا درونخطی bool layout_flex__base_and_main_sizes(
		ثابت ساختار flex_ctx *ctx,
		ساختار flex_item_data *item,
		صحیح available_width)
{
	ساختار box *b = item->box;
	صحیح content_min_width = b->min_width;
	صحیح content_max_width = b->max_width;
	صحیح delta_outer_main = lh__delta_outer_main(ctx->flex, b);

	NSLOG(flex, DEEPDEBUG, "box %p: delta_outer_main: %i",
			b, delta_outer_main);

	اگر (item->basis == CSS_FLEX_BASIS_SET) {
		اگر (item->basis_unit == CSS_UNIT_PCT) {
			item->base_size = FPCT_OF_INT_TOINT(
					item->basis_length,
					available_width);
		} وگرنه {
			item->base_size = FIXTOINT(css_unit_len2device_px(
					b->style, ctx->unit_len_ctx,
					item->basis_length,
					item->basis_unit));
		}

	} وگرنه اگر (item->basis == CSS_FLEX_BASIS_AUTO) {
		item->base_size = ctx->horizontal ? b->width : b->height;
	} وگرنه {
		item->base_size = AUTO;
	}

	اگر (ctx->horizontal == false) {
		اگر (b->width == AUTO) {
			b->width = min(max(content_min_width, available_width),
					content_max_width);
			b->width -= lh__delta_outer_width(b);
		}

		اگر (!layout_flex_item(ctx, item, b->width)) {
			بازگشت false;
		}
	}

	اگر (item->base_size == AUTO) {
		اگر (ctx->horizontal == false) {
			item->base_size = b->height;
		} وگرنه {
			item->base_size = content_max_width - delta_outer_main;
		}
	}

	item->base_size += delta_outer_main;

	اگر (ctx->horizontal) {
		item->base_size = min(item->base_size, available_width);
		item->base_size = max(item->base_size, content_min_width);
	}

	item->target_main_size = item->base_size;
	item->main_size = item->base_size;

	اگر (item->max_main > 0 &&
	    item->main_size > item->max_main + delta_outer_main) {
		item->main_size = item->max_main + delta_outer_main;
	}

	اگر (item->main_size < item->min_main + delta_outer_main) {
		item->main_size = item->min_main + delta_outer_main;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: base_size: %i, main_size %i",
			b, item->base_size, item->main_size);

	بازگشت true;
}

/**
 * Fill out all item's data in a flex container.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] flex             Flex box
 * \param[in] available_width  Available width in pixels
 */
ایستا تهی layout_flex_ctx__populate_item_data(
		ثابت ساختار flex_ctx *ctx,
		ثابت ساختار box *flex,
		صحیح available_width)
{
	size_t i = 0;
	bool horizontal = ctx->horizontal;

	برای (ساختار box *b = flex->children; b != پوچ; b = b->next) {
		ساختار flex_item_data *item = &ctx->item.data[i++];

		b->float_container = b->parent;
		layout_find_dimensions(ctx->unit_len_ctx, available_width, -1,
				b, b->style, &b->width, &b->height,
				horizontal ? &item->max_main : &item->max_cross,
				horizontal ? &item->min_main : &item->min_cross,
				horizontal ? &item->max_cross : &item->max_main,
				horizontal ? &item->min_cross : &item->min_main,
				b->margin, b->padding, b->border);
		b->float_container = پوچ;

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
ایستا bool layout_flex_ctx_ensure_سطر(ساختار flex_ctx *ctx)
{
	ساختار flex_سطر_data *temp;
	size_t سطر_alloc = ctx->سطر.alloc * 2;

	اگر (ctx->سطر.alloc > ctx->سطر.شمار) {
		بازگشت true;
	}

	temp = realloc(ctx->سطر.data, اندازه(*ctx->سطر.data) * سطر_alloc);
	اگر (temp == پوچ) {
		بازگشت false;
	}
	ctx->سطر.data = temp;

	memset(ctx->سطر.data + ctx->سطر.alloc, 0,
	       اندازه(*ctx->سطر.data) * (سطر_alloc - ctx->سطر.alloc));
	ctx->سطر.alloc = سطر_alloc;

	بازگشت true;
}

/**
 * Assigns flex items to the line and returns the line
 *
 * \param[in] ctx         Flex layout context
 * \param[in] item_index  Index to first item to assign to this line
 * \return Pointer to the new line, or NULL on error.
 */
ایستا ساختار flex_سطر_data *layout_flex_build_سطر(ساختار flex_ctx *ctx,
		size_t item_index)
{
	شمارش box_side start_side = layout_flex__main_start_side(ctx);
	شمارش box_side end_side = layout_flex__main_end_side(ctx);
	ساختار flex_سطر_data *سطر;
	صحیح used_main = 0;

	اگر (!layout_flex_ctx_ensure_سطر(ctx)) {
		بازگشت پوچ;
	}

	سطر = &ctx->سطر.data[ctx->سطر.شمار];
	سطر->first = item_index;

	NSLOG(flex, DEEPDEBUG, "flex container %p: available main: %i",
			ctx->flex, ctx->available_main);

	وقتی (item_index < ctx->item.شمار) {
		ساختار flex_item_data *item = &ctx->item.data[item_index];
		ساختار box *b = item->box;
		صحیح pos_main;

		pos_main = ctx->horizontal ?
				item->main_size :
				b->height + lh__delta_outer_main(ctx->flex, b);

		اگر (ctx->wrap == CSS_FLEX_WRAP_NOWRAP ||
		    pos_main + used_main <= ctx->available_main ||
		    lh__box_is_absolute(item->box) ||
		    ctx->available_main == AUTO ||
		    سطر->شمار == 0 ||
		    pos_main == 0) {
			اگر (lh__box_is_absolute(item->box) == false) {
				سطر->main_size += item->main_size;
				used_main += pos_main;

				اگر (b->margin[start_side] == AUTO) {
					سطر->main_auto_margin_شمار++;
				}
				اگر (b->margin[end_side] == AUTO) {
					سطر->main_auto_margin_شمار++;
				}
			}
			item->سطر = ctx->سطر.شمار;
			سطر->شمار++;
			item_index++;
		} وگرنه {
			بشکن;
		}
	}

	اگر (سطر->شمار > 0) {
		ctx->سطر.شمار++;
	} وگرنه {
		NSLOG(layout, ERROR, "Failed to fit any flex items");
	}

	بازگشت سطر;
}

/**
 * Freeze an item on a line
 *
 * \param[in] line  Line to containing item
 * \param[in] item  Item to freeze
 */
ایستا درونخطی تهی layout_flex__item_freeze(
		ساختار flex_سطر_data *سطر,
		ساختار flex_item_data *item)
{
	item->freeze = true;
	سطر->frozen++;

	اگر (!lh__box_is_absolute(item->box)){
		سطر->used_main_size += item->target_main_size;
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
ایستا درونخطی صحیح layout_flex__remaining_free_main(
		ساختار flex_ctx *ctx,
		ساختار flex_سطر_data *سطر,
		css_fixed *unfrozen_factor_sum,
		صحیح initial_free_main,
		صحیح available_main,
		bool grow)
{
	صحیح remaining_free_main = available_main;
	size_t item_شمار = سطر->first + سطر->شمار;

	*unfrozen_factor_sum = 0;

	برای (size_t i = سطر->first; i < item_شمار; i++) {
		ساختار flex_item_data *item = &ctx->item.data[i];

		اگر (item->freeze) {
			remaining_free_main -= item->target_main_size;
		} وگرنه {
			remaining_free_main -= item->base_size;

			*unfrozen_factor_sum += grow ?
					item->grow : item->shrink;
		}
	}

	اگر (*unfrozen_factor_sum < F_1) {
		صحیح free_space = FIXTOINT(FMUL(INTTOFIX(initial_free_main),
				*unfrozen_factor_sum));

		اگر (free_space < remaining_free_main) {
			remaining_free_main = free_space;
		}
	}

	NSLOG(flex, DEEPDEBUG, "Remaining free space: %i",
			remaining_free_main);

	بازگشت remaining_free_main;
}

/**
 * Clamp flex item target main size and get min/max violations
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to align items on
 * return total violation in pixels
 */
ایستا درونخطی صحیح layout_flex__get_min_max_violations(
		ساختار flex_ctx *ctx,
		ساختار flex_سطر_data *سطر)
{

	صحیح total_violation = 0;
	size_t item_شمار = سطر->first + سطر->شمار;

	برای (size_t i = سطر->first; i < item_شمار; i++) {
		ساختار flex_item_data *item = &ctx->item.data[i];
		صحیح target_main_size = item->target_main_size;

		NSLOG(flex, DEEPDEBUG, "item %p: target_main_size: %i",
					item->box, target_main_size);

		اگر (item->freeze) {
			ادامه;
		}

		اگر (item->max_main > 0 &&
		    target_main_size > item->max_main) {
			target_main_size = item->max_main;
			item->max_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: max_main: %i",
					item->max_main);
		}

		اگر (target_main_size < item->min_main) {
			target_main_size = item->min_main;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: min_main: %i",
					item->min_main);
		}

		اگر (target_main_size < item->box->min_width) {
			target_main_size = item->box->min_width;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: box min_width: %i",
					item->box->min_width);
		}

		اگر (target_main_size < 0) {
			target_main_size = 0;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: less than 0");
		}

		total_violation += target_main_size - item->target_main_size;
		item->target_main_size = target_main_size;
	}

	NSLOG(flex, DEEPDEBUG, "Total violation: %i", total_violation);

	بازگشت total_violation;
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
ایستا درونخطی تهی layout_flex__distribute_free_main(
		ساختار flex_ctx *ctx,
		ساختار flex_سطر_data *سطر,
		css_fixed unfrozen_factor_sum,
		صحیح remaining_free_main,
		bool grow)
{
	size_t item_شمار = سطر->first + سطر->شمار;

	اگر (grow) {
		css_fixed remainder = 0;
		برای (size_t i = سطر->first; i < item_شمار; i++) {
			ساختار flex_item_data *item = &ctx->item.data[i];
			css_fixed result;
			css_fixed ratio;

			اگر (item->freeze) {
				ادامه;
			}

			ratio = FDIV(item->grow, unfrozen_factor_sum);
			result = FMUL(INTTOFIX(remaining_free_main), ratio) +
					remainder;

			item->target_main_size = item->base_size +
					FIXTOINT(result);
			remainder = FIXFRAC(result);
		}
	} وگرنه {
		css_fixed scaled_shrink_factor_sum = 0;
		css_fixed remainder = 0;

		برای (size_t i = سطر->first; i < item_شمار; i++) {
			ساختار flex_item_data *item = &ctx->item.data[i];
			css_fixed scaled_shrink_factor;

			اگر (item->freeze) {
				ادامه;
			}

			scaled_shrink_factor = FMUL(
					item->shrink,
					INTTOFIX(item->base_size));
			scaled_shrink_factor_sum += scaled_shrink_factor;
		}

		برای (size_t i = سطر->first; i < item_شمار; i++) {
			ساختار flex_item_data *item = &ctx->item.data[i];
			css_fixed scaled_shrink_factor;
			css_fixed result;
			css_fixed ratio;

			اگر (item->freeze) {
				ادامه;
			} وگرنه اگر (scaled_shrink_factor_sum == 0) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(سطر, item);
				ادامه;
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
ایستا bool layout_flex_resolve_سطر(
		ساختار flex_ctx *ctx,
		ساختار flex_سطر_data *سطر)
{
	size_t item_شمار = سطر->first + سطر->شمار;
	صحیح available_main = ctx->available_main;
	صحیح initial_free_main;
	bool grow;

	اگر (available_main == AUTO) {
		available_main = INT_MAX;
	}

	grow = (سطر->main_size < available_main);
	initial_free_main = available_main;

	NSLOG(flex, DEEPDEBUG, "box %p: line %zu: first: %zu, count: %zu",
			ctx->flex, سطر - ctx->سطر.data,
			سطر->first, سطر->شمار);
	NSLOG(flex, DEEPDEBUG, "Line main_size: %i, available_main: %i",
			سطر->main_size, available_main);

	برای (size_t i = سطر->first; i < item_شمار; i++) {
		ساختار flex_item_data *item = &ctx->item.data[i];

		/* 3. Size inflexible items */
		اگر (grow) {
			اگر (item->grow == 0 ||
			    item->base_size > item->main_size) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(سطر, item);
			}
		} وگرنه {
			اگر (item->shrink == 0 ||
			    item->base_size < item->main_size) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(سطر, item);
			}
		}

		/* 4. Calculate initial free space */
		اگر (item->freeze) {
			initial_free_main -= item->target_main_size;
		} وگرنه {
			initial_free_main -= item->base_size;
		}
	}

	/* 5. Loop */
	وقتی (سطر->frozen < سطر->شمار) {
		css_fixed unfrozen_factor_sum;
		صحیح remaining_free_main;
		صحیح total_violation;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: Resolver pass",
				ctx->flex);

		/* b */
		remaining_free_main = layout_flex__remaining_free_main(ctx,
				سطر, &unfrozen_factor_sum, initial_free_main,
				available_main, grow);

		/* c */
		اگر (remaining_free_main != 0) {
			layout_flex__distribute_free_main(ctx,
					سطر, unfrozen_factor_sum,
					remaining_free_main, grow);
		}

		/* d */
		total_violation = layout_flex__get_min_max_violations(
				ctx, سطر);

		/* e */
		برای (size_t i = سطر->first; i < item_شمار; i++) {
			ساختار flex_item_data *item = &ctx->item.data[i];

			اگر (item->freeze) {
				ادامه;
			}

			اگر (total_violation == 0 ||
			    (total_violation > 0 && item->min_violation) ||
			    (total_violation < 0 && item->max_violation)) {
				layout_flex__item_freeze(سطر, item);
			}
		}
	}

	بازگشت true;
}

/**
 * Position items along a line
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to resolve
 * \return true on success, false on failure.
 */
ایستا bool layout_flex_place_سطر_اقلام_main(
		ساختار flex_ctx *ctx,
		ساختار flex_سطر_data *سطر)
{
	صحیح main_pos = ctx->flex->padding[layout_flex__main_start_side(ctx)];
	صحیح post_multiplier = ctx->main_reversed ? 0 : 1;
	صحیح pre_multiplier = ctx->main_reversed ? -1 : 0;
	size_t item_شمار = سطر->first + سطر->شمار;
	صحیح extra_remainder = 0;
	صحیح extra = 0;

	اگر (ctx->main_reversed) {
		main_pos = lh__box_size_main(ctx->horizontal, ctx->flex) -
				main_pos;
	}

	اگر (ctx->available_main != AUTO &&
	    ctx->available_main != UNKNOWN_WIDTH &&
	    ctx->available_main > سطر->used_main_size) {
		اگر (سطر->main_auto_margin_شمار > 0) {
			extra = ctx->available_main - سطر->used_main_size;

			extra_remainder = extra % سطر->main_auto_margin_شمار;
			extra /= سطر->main_auto_margin_شمار;
		}
	}

	برای (size_t i = سطر->first; i < item_شمار; i++) {
		شمارش box_side main_end = ctx->horizontal ? راست_2 : BOTTOM;
		شمارش box_side main_start = ctx->horizontal ? چپ_2 : بالاترین_2;
		ساختار flex_item_data *item = &ctx->item.data[i];
		ساختار box *b = item->box;
		صحیح extra_total = 0;
		صحیح extra_post = 0;
		صحیح extra_pre = 0;
		صحیح box_size_main;
		صحیح *box_pos_main;

		اگر (ctx->horizontal) {
			b->width = item->target_main_size -
					lh__delta_outer_width(b);

			اگر (!layout_flex_item(ctx, item, b->width)) {
				بازگشت false;
			}
		}

		box_size_main = lh__box_size_main(ctx->horizontal, b);
		box_pos_main = ctx->horizontal ? &b->x : &b->y;

		اگر (!lh__box_is_absolute(b)) {
			اگر (b->margin[main_start] == AUTO) {
				extra_pre = extra + extra_remainder;
			}
			اگر (b->margin[main_end] == AUTO) {
				extra_post = extra + extra_remainder;
			}
			extra_total = extra_pre + extra_post;

			main_pos += pre_multiplier *
					(extra_total + box_size_main +
					 lh__delta_outer_main(ctx->flex, b));
		}

		*box_pos_main = main_pos + lh__non_auto_margin(b, main_start) +
				extra_pre + b->border[main_start].width;

		اگر (!lh__box_is_absolute(b)) {
			صحیح cross_size;
			صحیح box_size_cross = lh__box_size_cross(
					ctx->horizontal, b);

			main_pos += post_multiplier *
					(extra_total + box_size_main +
					 lh__delta_outer_main(ctx->flex, b));

			cross_size = box_size_cross + lh__delta_outer_cross(
					ctx->flex, b);
			اگر (سطر->cross_size < cross_size) {
				سطر->cross_size = cross_size;
			}
		}
	}

	بازگشت true;
}

/**
 * Collect items onto lines and place items along the lines
 *
 * \param[in] ctx   Flex layout context
 * \return true on success, false on failure.
 */
ایستا bool layout_flex_collect_اقلام_into_سطرها(
		ساختار flex_ctx *ctx)
{
	size_t pos = 0;

	وقتی (pos < ctx->item.شمار) {
		ساختار flex_سطر_data *سطر;

		سطر = layout_flex_build_سطر(ctx, pos);
		اگر (سطر == پوچ) {
			بازگشت false;
		}

		pos += سطر->شمار;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: "
				"fitted: %zu (total: %zu/%zu)",
				ctx->flex, سطر->شمار,
				pos, ctx->item.شمار);

		اگر (!layout_flex_resolve_سطر(ctx, سطر)) {
			بازگشت false;
		}

		اگر (!layout_flex_place_سطر_اقلام_main(ctx, سطر)) {
			بازگشت false;
		}

		ctx->cross_size += سطر->cross_size;
		اگر (ctx->main_size < سطر->main_size) {
			ctx->main_size = سطر->main_size;
		}
	}

	بازگشت true;
}

/**
 * Align items on a line.
 *
 * \param[in] ctx    Flex layout context
 * \param[in] line   Line to align items on
 * \param[in] extra  Extra line width in pixels
 */
ایستا تهی layout_flex_place_سطر_اقلام_cross(ساختار flex_ctx *ctx,
		ساختار flex_سطر_data *سطر, صحیح extra)
{
	شمارش box_side cross_start = ctx->horizontal ? بالاترین_2 : چپ_2;
	size_t item_شمار = سطر->first + سطر->شمار;

	برای (size_t i = سطر->first; i < item_شمار; i++) {
		ساختار flex_item_data *item = &ctx->item.data[i];
		ساختار box *b = item->box;
		صحیح cross_free_space;
		صحیح *box_size_cross;
		صحیح *box_pos_cross;

		box_pos_cross = ctx->horizontal ? &b->y : &b->x;
		box_size_cross = lh__box_size_cross_ptr(ctx->horizontal, b);

		cross_free_space = سطر->cross_size + extra - *box_size_cross -
				lh__delta_outer_cross(ctx->flex, b);

		انتخاب (lh__box_align_self(ctx->flex, b)) {
		پیشفرض:
		حالت CSS_ALIGN_SELF_STRETCH:
			اگر (lh__box_size_cross_is_auto(ctx->horizontal, b)) {
				*box_size_cross += cross_free_space;

				/* Relayout children for stretch. */
				اگر (!layout_flex_item(ctx, item, b->width)) {
					بازگشت;
				}
			}
			fallthrough;
		حالت CSS_ALIGN_SELF_FLEX_START:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					سطر->pos +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			بشکن;

		حالت CSS_ALIGN_SELF_FLEX_END:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					سطر->pos + cross_free_space +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			بشکن;

		حالت CSS_ALIGN_SELF_BASELINE:
		حالت CSS_ALIGN_SELF_CENTER:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					سطر->pos + cross_free_space / 2 +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			بشکن;
		}
	}
}

/**
 * Place the lines and align the items on the line.
 *
 * \param[in] ctx  Flex layout context
 */
ایستا تهی layout_flex_place_سطرها(ساختار flex_ctx *ctx)
{
	bool reversed = ctx->wrap == CSS_FLEX_WRAP_WRAP_REVERSE;
	صحیح سطر_pos = reversed ? ctx->cross_size : 0;
	صحیح post_multiplier = reversed ? 0 : 1;
	صحیح pre_multiplier = reversed ? -1 : 0;
	صحیح extra_remainder = 0;
	صحیح extra = 0;

	اگر (ctx->available_cross != AUTO &&
	    ctx->available_cross > ctx->cross_size &&
	    ctx->سطر.شمار > 0) {
		extra = ctx->available_cross - ctx->cross_size;

		extra_remainder = extra % ctx->سطر.شمار;
		extra /= ctx->سطر.شمار;
	}

	برای (size_t i = 0; i < ctx->سطر.شمار; i++) {
		ساختار flex_سطر_data *سطر = &ctx->سطر.data[i];

		سطر_pos += pre_multiplier * سطر->cross_size;
		سطر->pos = سطر_pos;
		سطر_pos += post_multiplier * سطر->cross_size +
				extra + extra_remainder;

		layout_flex_place_سطر_اقلام_cross(ctx, سطر,
				extra + extra_remainder);

		اگر (extra_remainder > 0) {
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
bool layout_flex(ساختار box *flex, صحیح available_width,
		html_content *content)
{
	صحیح max_height, min_height;
	ساختار flex_ctx *ctx;
	bool success = false;

	ctx = layout_flex_ctx__create(content, flex);
	اگر (ctx == پوچ) {
		بازگشت false;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: %s, available_width %i, width: %i",
			flex, ctx->horizontal ? "horizontal" : "vertical",
			available_width, flex->width);

	layout_find_dimensions(
			ctx->unit_len_ctx, available_width, -1,
			flex, flex->style, پوچ, &flex->height,
			پوچ, پوچ, &max_height, &min_height,
			flex->margin, flex->padding, flex->border);

	available_width = min(available_width, flex->width);

	اگر (ctx->horizontal) {
		ctx->available_main = available_width;
		ctx->available_cross = ctx->flex->height;
	} وگرنه {
		ctx->available_main = ctx->flex->height;
		ctx->available_cross = available_width;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: available_main: %i",
			flex, ctx->available_main);
	NSLOG(flex, DEEPDEBUG, "box %p: available_cross: %i",
			flex, ctx->available_cross);

	layout_flex_ctx__populate_item_data(ctx, flex, available_width);

	/* Place items onto lines. */
	success = layout_flex_collect_اقلام_into_سطرها(ctx);
	اگر (!success) {
		برو cleanup;
	}

	layout_flex_place_سطرها(ctx);

	اگر (flex->height == AUTO) {
		flex->height = ctx->horizontal ?
				ctx->cross_size :
				ctx->main_size;
	}

	اگر (flex->height != AUTO) {
		اگر (max_height >= 0 && flex->height > max_height) {
			flex->height = max_height;
		}
		اگر (min_height >  0 && flex->height < min_height) {
			flex->height = min_height;
		}
	}

	success = true;

cleanup:
	layout_flex_ctx__destroy(ctx);

	NSLOG(flex, DEEPDEBUG, "box %p: %s: w: %i, h: %i", flex,
			success ? "success" : "failure",
			flex->width, flex->height);
	بازگشت success;
}
