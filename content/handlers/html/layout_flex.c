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

#অন্তর্ভাব <string.h>

#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/utils.h"

#অন্তর্ভাব "html/box.h"
#অন্তর্ভাব "html/html.h"
#অন্তর্ভাব "html/private.h"
#অন্তর্ভাব "html/box_inspect.h"
#অন্তর্ভাব "html/layout_internal.h"

/**
 * Flex item data
 */
সংরচনা flex_item_data {
	গণনা css_flex_basis_e basis;
	css_fixed basis_length;
	css_unit basis_unit;
	সংরচনা box *box;

	css_fixed shrink;
	css_fixed grow;

	পূর্ণাঙ্ক min_main;
	পূর্ণাঙ্ক max_main;
	পূর্ণাঙ্ক min_cross;
	পূর্ণাঙ্ক max_cross;

	পূর্ণাঙ্ক target_main_size;
	পূর্ণাঙ্ক base_size;
	পূর্ণাঙ্ক main_size;
	size_t সারি;

	bool freeze;
	bool min_violation;
	bool max_violation;
};

/**
 * Flex line data
 */
সংরচনা flex_সারি_data {
	পূর্ণাঙ্ক main_size;
	পূর্ণাঙ্ক cross_size;

	পূর্ণাঙ্ক used_main_size;
	পূর্ণাঙ্ক main_auto_margin_গুনতি;

	পূর্ণাঙ্ক pos;

	size_t first;
	size_t গুনতি;
	size_t frozen;
};

/**
 * Flex layout context
 */
সংরচনা flex_ctx {
	html_content *content;
	ধ্রুব সংরচনা box *flex;
	ধ্রুব css_unit_ctx *unit_len_ctx;

	পূর্ণাঙ্ক main_size;
	পূর্ণাঙ্ক cross_size;

	পূর্ণাঙ্ক available_main;
	পূর্ণাঙ্ক available_cross;

	bool horizontal;
	bool main_reversed;
	গণনা css_flex_wrap_e wrap;

	সংরচনা flex_বস্তুসমূহ {
		size_t গুনতি;
		সংরচনা flex_item_data *data;
	} item;

	সংরচনা flex_সারিগুলি {
		size_t গুনতি;
		size_t alloc;
		সংরচনা flex_সারি_data *data;
	} সারি;
};

/**
 * Destroy a flex layout context
 *
 * \param[in] ctx  Flex layout context
 */
স্থির শূন্য layout_flex_ctx__destroy(সংরচনা flex_ctx *ctx)
{
	যদি (ctx != রিক্ত) {
		মুক্ত(ctx->item.data);
		মুক্ত(ctx->সারি.data);
		মুক্ত(ctx);
	}
}

/**
 * Create a flex layout context
 *
 * \param[in] content  HTML content containing flex box
 * \param[in] flex     Box to create layout context for
 * \return flex layout context or NULL on error
 */
স্থির সংরচনা flex_ctx *layout_flex_ctx__create(
		html_content *content,
		ধ্রুব সংরচনা box *flex)
{
	সংরচনা flex_ctx *ctx;

	ctx = calloc(1, পরিমাণ(*ctx));
	যদি (ctx == রিক্ত) {
		প্রত্যাগম রিক্ত;
	}
	ctx->সারি.alloc = 1;

	ctx->item.গুনতি = box_গুনতি_children(flex);
	ctx->item.data = calloc(ctx->item.গুনতি, পরিমাণ(*ctx->item.data));
	যদি (ctx->item.data == রিক্ত) {
		layout_flex_ctx__destroy(ctx);
		প্রত্যাগম রিক্ত;
	}

	ctx->সারি.alloc = 1;
	ctx->সারি.data = calloc(ctx->সারি.alloc, পরিমাণ(*ctx->সারি.data));
	যদি (ctx->সারি.data == রিক্ত) {
		layout_flex_ctx__destroy(ctx);
		প্রত্যাগম রিক্ত;
	}

	ctx->flex = flex;
	ctx->content = content;
	ctx->unit_len_ctx = &content->unit_len_ctx;

	ctx->wrap = css_computed_flex_wrap(flex->style);
	ctx->horizontal = lh__flex_main_is_horizontal(flex);
	ctx->main_reversed = lh__flex_direction_reversed(flex);

	প্রত্যাগম ctx;
}

/**
 * Find box side representing the start of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the start side.
 */
স্থির গণনা box_side layout_flex__main_start_side(
		ধ্রুব সংরচনা flex_ctx *ctx)
{
	যদি (ctx->horizontal) {
		প্রত্যাগম (ctx->main_reversed) ? ডান_2 : বাম_2;
	} অন্যথা {
		প্রত্যাগম (ctx->main_reversed) ? BOTTOM : শীর্ষ_2;
	}
}

/**
 * Find box side representing the end of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the end side.
 */
স্থির গণনা box_side layout_flex__main_end_side(
		ধ্রুব সংরচনা flex_ctx *ctx)
{
	যদি (ctx->horizontal) {
		প্রত্যাগম (ctx->main_reversed) ? বাম_2 : ডান_2;
	} অন্যথা {
		প্রত্যাগম (ctx->main_reversed) ? শীর্ষ_2 : BOTTOM;
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
স্থির bool layout_flex_item(
		ধ্রুব সংরচনা flex_ctx *ctx,
		ধ্রুব সংরচনা flex_item_data *item,
		পূর্ণাঙ্ক available_width)
{
	bool success;
	সংরচনা box *b = item->box;

	বিকল্প (b->type) {
	পক্ষ BOX_BLOCK:
		success = layout_block_context(b, -1, ctx->content);
		ভঙ্গ;
	পক্ষ BOX_TABLE:
		b->float_container = b->parent;
		success = layout_table(b, available_width, ctx->content);
		b->float_container = রিক্ত;
		ভঙ্গ;
	পক্ষ BOX_FLEX:
		b->float_container = b->parent;
		success = layout_flex(b, available_width, ctx->content);
		b->float_container = রিক্ত;
		ভঙ্গ;
	স্বভাব:
		প্রতিজ্ঞা(0 && "Bad flex item back type");
		success = false;
		ভঙ্গ;
	}

	যদি (!success) {
		NSLOG(flex, ERROR, "box %p: layout failed", b);
	}

	প্রত্যাগম success;
}

/**
 * Calculate an item's base and target main sizes.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] item             Item to get sizes of
 * \param[in] available_width  Available width in pixels
 * \return true on success false on failure
 */
স্থির অন্তঃস্থ bool layout_flex__base_and_main_sizes(
		ধ্রুব সংরচনা flex_ctx *ctx,
		সংরচনা flex_item_data *item,
		পূর্ণাঙ্ক available_width)
{
	সংরচনা box *b = item->box;
	পূর্ণাঙ্ক content_min_width = b->min_width;
	পূর্ণাঙ্ক content_max_width = b->max_width;
	পূর্ণাঙ্ক delta_outer_main = lh__delta_outer_main(ctx->flex, b);

	NSLOG(flex, DEEPDEBUG, "box %p: delta_outer_main: %i",
			b, delta_outer_main);

	যদি (item->basis == CSS_FLEX_BASIS_SET) {
		যদি (item->basis_unit == CSS_UNIT_PCT) {
			item->base_size = FPCT_OF_INT_TOINT(
					item->basis_length,
					available_width);
		} অন্যথা {
			item->base_size = FIXTOINT(css_unit_len2device_px(
					b->style, ctx->unit_len_ctx,
					item->basis_length,
					item->basis_unit));
		}

	} অন্যথা যদি (item->basis == CSS_FLEX_BASIS_AUTO) {
		item->base_size = ctx->horizontal ? b->width : b->height;
	} অন্যথা {
		item->base_size = AUTO;
	}

	যদি (ctx->horizontal == false) {
		যদি (b->width == AUTO) {
			b->width = min(max(content_min_width, available_width),
					content_max_width);
			b->width -= lh__delta_outer_width(b);
		}

		যদি (!layout_flex_item(ctx, item, b->width)) {
			প্রত্যাগম false;
		}
	}

	যদি (item->base_size == AUTO) {
		যদি (ctx->horizontal == false) {
			item->base_size = b->height;
		} অন্যথা {
			item->base_size = content_max_width - delta_outer_main;
		}
	}

	item->base_size += delta_outer_main;

	যদি (ctx->horizontal) {
		item->base_size = min(item->base_size, available_width);
		item->base_size = max(item->base_size, content_min_width);
	}

	item->target_main_size = item->base_size;
	item->main_size = item->base_size;

	যদি (item->max_main > 0 &&
	    item->main_size > item->max_main + delta_outer_main) {
		item->main_size = item->max_main + delta_outer_main;
	}

	যদি (item->main_size < item->min_main + delta_outer_main) {
		item->main_size = item->min_main + delta_outer_main;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: base_size: %i, main_size %i",
			b, item->base_size, item->main_size);

	প্রত্যাগম true;
}

/**
 * Fill out all item's data in a flex container.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] flex             Flex box
 * \param[in] available_width  Available width in pixels
 */
স্থির শূন্য layout_flex_ctx__populate_item_data(
		ধ্রুব সংরচনা flex_ctx *ctx,
		ধ্রুব সংরচনা box *flex,
		পূর্ণাঙ্ক available_width)
{
	size_t i = 0;
	bool horizontal = ctx->horizontal;

	প্রতি (সংরচনা box *b = flex->children; b != রিক্ত; b = b->next) {
		সংরচনা flex_item_data *item = &ctx->item.data[i++];

		b->float_container = b->parent;
		layout_find_dimensions(ctx->unit_len_ctx, available_width, -1,
				b, b->style, &b->width, &b->height,
				horizontal ? &item->max_main : &item->max_cross,
				horizontal ? &item->min_main : &item->min_cross,
				horizontal ? &item->max_cross : &item->max_main,
				horizontal ? &item->min_cross : &item->min_main,
				b->margin, b->padding, b->border);
		b->float_container = রিক্ত;

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
স্থির bool layout_flex_ctx_ensure_সারি(সংরচনা flex_ctx *ctx)
{
	সংরচনা flex_সারি_data *temp;
	size_t সারি_alloc = ctx->সারি.alloc * 2;

	যদি (ctx->সারি.alloc > ctx->সারি.গুনতি) {
		প্রত্যাগম true;
	}

	temp = realloc(ctx->সারি.data, পরিমাণ(*ctx->সারি.data) * সারি_alloc);
	যদি (temp == রিক্ত) {
		প্রত্যাগম false;
	}
	ctx->সারি.data = temp;

	memset(ctx->সারি.data + ctx->সারি.alloc, 0,
	       পরিমাণ(*ctx->সারি.data) * (সারি_alloc - ctx->সারি.alloc));
	ctx->সারি.alloc = সারি_alloc;

	প্রত্যাগম true;
}

/**
 * Assigns flex items to the line and returns the line
 *
 * \param[in] ctx         Flex layout context
 * \param[in] item_index  Index to first item to assign to this line
 * \return Pointer to the new line, or NULL on error.
 */
স্থির সংরচনা flex_সারি_data *layout_flex_build_সারি(সংরচনা flex_ctx *ctx,
		size_t item_index)
{
	গণনা box_side start_side = layout_flex__main_start_side(ctx);
	গণনা box_side end_side = layout_flex__main_end_side(ctx);
	সংরচনা flex_সারি_data *সারি;
	পূর্ণাঙ্ক used_main = 0;

	যদি (!layout_flex_ctx_ensure_সারি(ctx)) {
		প্রত্যাগম রিক্ত;
	}

	সারি = &ctx->সারি.data[ctx->সারি.গুনতি];
	সারি->first = item_index;

	NSLOG(flex, DEEPDEBUG, "flex container %p: available main: %i",
			ctx->flex, ctx->available_main);

	যাবত্ (item_index < ctx->item.গুনতি) {
		সংরচনা flex_item_data *item = &ctx->item.data[item_index];
		সংরচনা box *b = item->box;
		পূর্ণাঙ্ক pos_main;

		pos_main = ctx->horizontal ?
				item->main_size :
				b->height + lh__delta_outer_main(ctx->flex, b);

		যদি (ctx->wrap == CSS_FLEX_WRAP_NOWRAP ||
		    pos_main + used_main <= ctx->available_main ||
		    lh__box_is_absolute(item->box) ||
		    ctx->available_main == AUTO ||
		    সারি->গুনতি == 0 ||
		    pos_main == 0) {
			যদি (lh__box_is_absolute(item->box) == false) {
				সারি->main_size += item->main_size;
				used_main += pos_main;

				যদি (b->margin[start_side] == AUTO) {
					সারি->main_auto_margin_গুনতি++;
				}
				যদি (b->margin[end_side] == AUTO) {
					সারি->main_auto_margin_গুনতি++;
				}
			}
			item->সারি = ctx->সারি.গুনতি;
			সারি->গুনতি++;
			item_index++;
		} অন্যথা {
			ভঙ্গ;
		}
	}

	যদি (সারি->গুনতি > 0) {
		ctx->সারি.গুনতি++;
	} অন্যথা {
		NSLOG(layout, ERROR, "Failed to fit any flex items");
	}

	প্রত্যাগম সারি;
}

/**
 * Freeze an item on a line
 *
 * \param[in] line  Line to containing item
 * \param[in] item  Item to freeze
 */
স্থির অন্তঃস্থ শূন্য layout_flex__item_freeze(
		সংরচনা flex_সারি_data *সারি,
		সংরচনা flex_item_data *item)
{
	item->freeze = true;
	সারি->frozen++;

	যদি (!lh__box_is_absolute(item->box)){
		সারি->used_main_size += item->target_main_size;
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
স্থির অন্তঃস্থ পূর্ণাঙ্ক layout_flex__remaining_free_main(
		সংরচনা flex_ctx *ctx,
		সংরচনা flex_সারি_data *সারি,
		css_fixed *unfrozen_factor_sum,
		পূর্ণাঙ্ক initial_free_main,
		পূর্ণাঙ্ক available_main,
		bool grow)
{
	পূর্ণাঙ্ক remaining_free_main = available_main;
	size_t item_গুনতি = সারি->first + সারি->গুনতি;

	*unfrozen_factor_sum = 0;

	প্রতি (size_t i = সারি->first; i < item_গুনতি; i++) {
		সংরচনা flex_item_data *item = &ctx->item.data[i];

		যদি (item->freeze) {
			remaining_free_main -= item->target_main_size;
		} অন্যথা {
			remaining_free_main -= item->base_size;

			*unfrozen_factor_sum += grow ?
					item->grow : item->shrink;
		}
	}

	যদি (*unfrozen_factor_sum < F_1) {
		পূর্ণাঙ্ক free_space = FIXTOINT(FMUL(INTTOFIX(initial_free_main),
				*unfrozen_factor_sum));

		যদি (free_space < remaining_free_main) {
			remaining_free_main = free_space;
		}
	}

	NSLOG(flex, DEEPDEBUG, "Remaining free space: %i",
			remaining_free_main);

	প্রত্যাগম remaining_free_main;
}

/**
 * Clamp flex item target main size and get min/max violations
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to align items on
 * return total violation in pixels
 */
স্থির অন্তঃস্থ পূর্ণাঙ্ক layout_flex__get_min_max_violations(
		সংরচনা flex_ctx *ctx,
		সংরচনা flex_সারি_data *সারি)
{

	পূর্ণাঙ্ক total_violation = 0;
	size_t item_গুনতি = সারি->first + সারি->গুনতি;

	প্রতি (size_t i = সারি->first; i < item_গুনতি; i++) {
		সংরচনা flex_item_data *item = &ctx->item.data[i];
		পূর্ণাঙ্ক target_main_size = item->target_main_size;

		NSLOG(flex, DEEPDEBUG, "item %p: target_main_size: %i",
					item->box, target_main_size);

		যদি (item->freeze) {
			অনুবর্ত;
		}

		যদি (item->max_main > 0 &&
		    target_main_size > item->max_main) {
			target_main_size = item->max_main;
			item->max_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: max_main: %i",
					item->max_main);
		}

		যদি (target_main_size < item->min_main) {
			target_main_size = item->min_main;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: min_main: %i",
					item->min_main);
		}

		যদি (target_main_size < item->box->min_width) {
			target_main_size = item->box->min_width;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: box min_width: %i",
					item->box->min_width);
		}

		যদি (target_main_size < 0) {
			target_main_size = 0;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: less than 0");
		}

		total_violation += target_main_size - item->target_main_size;
		item->target_main_size = target_main_size;
	}

	NSLOG(flex, DEEPDEBUG, "Total violation: %i", total_violation);

	প্রত্যাগম total_violation;
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
স্থির অন্তঃস্থ শূন্য layout_flex__distribute_free_main(
		সংরচনা flex_ctx *ctx,
		সংরচনা flex_সারি_data *সারি,
		css_fixed unfrozen_factor_sum,
		পূর্ণাঙ্ক remaining_free_main,
		bool grow)
{
	size_t item_গুনতি = সারি->first + সারি->গুনতি;

	যদি (grow) {
		css_fixed remainder = 0;
		প্রতি (size_t i = সারি->first; i < item_গুনতি; i++) {
			সংরচনা flex_item_data *item = &ctx->item.data[i];
			css_fixed result;
			css_fixed ratio;

			যদি (item->freeze) {
				অনুবর্ত;
			}

			ratio = FDIV(item->grow, unfrozen_factor_sum);
			result = FMUL(INTTOFIX(remaining_free_main), ratio) +
					remainder;

			item->target_main_size = item->base_size +
					FIXTOINT(result);
			remainder = FIXFRAC(result);
		}
	} অন্যথা {
		css_fixed scaled_shrink_factor_sum = 0;
		css_fixed remainder = 0;

		প্রতি (size_t i = সারি->first; i < item_গুনতি; i++) {
			সংরচনা flex_item_data *item = &ctx->item.data[i];
			css_fixed scaled_shrink_factor;

			যদি (item->freeze) {
				অনুবর্ত;
			}

			scaled_shrink_factor = FMUL(
					item->shrink,
					INTTOFIX(item->base_size));
			scaled_shrink_factor_sum += scaled_shrink_factor;
		}

		প্রতি (size_t i = সারি->first; i < item_গুনতি; i++) {
			সংরচনা flex_item_data *item = &ctx->item.data[i];
			css_fixed scaled_shrink_factor;
			css_fixed result;
			css_fixed ratio;

			যদি (item->freeze) {
				অনুবর্ত;
			} অন্যথা যদি (scaled_shrink_factor_sum == 0) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(সারি, item);
				অনুবর্ত;
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
স্থির bool layout_flex_resolve_সারি(
		সংরচনা flex_ctx *ctx,
		সংরচনা flex_সারি_data *সারি)
{
	size_t item_গুনতি = সারি->first + সারি->গুনতি;
	পূর্ণাঙ্ক available_main = ctx->available_main;
	পূর্ণাঙ্ক initial_free_main;
	bool grow;

	যদি (available_main == AUTO) {
		available_main = INT_MAX;
	}

	grow = (সারি->main_size < available_main);
	initial_free_main = available_main;

	NSLOG(flex, DEEPDEBUG, "box %p: line %zu: first: %zu, count: %zu",
			ctx->flex, সারি - ctx->সারি.data,
			সারি->first, সারি->গুনতি);
	NSLOG(flex, DEEPDEBUG, "Line main_size: %i, available_main: %i",
			সারি->main_size, available_main);

	প্রতি (size_t i = সারি->first; i < item_গুনতি; i++) {
		সংরচনা flex_item_data *item = &ctx->item.data[i];

		/* 3. Size inflexible items */
		যদি (grow) {
			যদি (item->grow == 0 ||
			    item->base_size > item->main_size) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(সারি, item);
			}
		} অন্যথা {
			যদি (item->shrink == 0 ||
			    item->base_size < item->main_size) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(সারি, item);
			}
		}

		/* 4. Calculate initial free space */
		যদি (item->freeze) {
			initial_free_main -= item->target_main_size;
		} অন্যথা {
			initial_free_main -= item->base_size;
		}
	}

	/* 5. Loop */
	যাবত্ (সারি->frozen < সারি->গুনতি) {
		css_fixed unfrozen_factor_sum;
		পূর্ণাঙ্ক remaining_free_main;
		পূর্ণাঙ্ক total_violation;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: Resolver pass",
				ctx->flex);

		/* b */
		remaining_free_main = layout_flex__remaining_free_main(ctx,
				সারি, &unfrozen_factor_sum, initial_free_main,
				available_main, grow);

		/* c */
		যদি (remaining_free_main != 0) {
			layout_flex__distribute_free_main(ctx,
					সারি, unfrozen_factor_sum,
					remaining_free_main, grow);
		}

		/* d */
		total_violation = layout_flex__get_min_max_violations(
				ctx, সারি);

		/* e */
		প্রতি (size_t i = সারি->first; i < item_গুনতি; i++) {
			সংরচনা flex_item_data *item = &ctx->item.data[i];

			যদি (item->freeze) {
				অনুবর্ত;
			}

			যদি (total_violation == 0 ||
			    (total_violation > 0 && item->min_violation) ||
			    (total_violation < 0 && item->max_violation)) {
				layout_flex__item_freeze(সারি, item);
			}
		}
	}

	প্রত্যাগম true;
}

/**
 * Position items along a line
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to resolve
 * \return true on success, false on failure.
 */
স্থির bool layout_flex_place_সারি_বস্তুসমূহ_main(
		সংরচনা flex_ctx *ctx,
		সংরচনা flex_সারি_data *সারি)
{
	পূর্ণাঙ্ক main_pos = ctx->flex->padding[layout_flex__main_start_side(ctx)];
	পূর্ণাঙ্ক post_multiplier = ctx->main_reversed ? 0 : 1;
	পূর্ণাঙ্ক pre_multiplier = ctx->main_reversed ? -1 : 0;
	size_t item_গুনতি = সারি->first + সারি->গুনতি;
	পূর্ণাঙ্ক extra_remainder = 0;
	পূর্ণাঙ্ক extra = 0;

	যদি (ctx->main_reversed) {
		main_pos = lh__box_size_main(ctx->horizontal, ctx->flex) -
				main_pos;
	}

	যদি (ctx->available_main != AUTO &&
	    ctx->available_main != UNKNOWN_WIDTH &&
	    ctx->available_main > সারি->used_main_size) {
		যদি (সারি->main_auto_margin_গুনতি > 0) {
			extra = ctx->available_main - সারি->used_main_size;

			extra_remainder = extra % সারি->main_auto_margin_গুনতি;
			extra /= সারি->main_auto_margin_গুনতি;
		}
	}

	প্রতি (size_t i = সারি->first; i < item_গুনতি; i++) {
		গণনা box_side main_end = ctx->horizontal ? ডান_2 : BOTTOM;
		গণনা box_side main_start = ctx->horizontal ? বাম_2 : শীর্ষ_2;
		সংরচনা flex_item_data *item = &ctx->item.data[i];
		সংরচনা box *b = item->box;
		পূর্ণাঙ্ক extra_total = 0;
		পূর্ণাঙ্ক extra_post = 0;
		পূর্ণাঙ্ক extra_pre = 0;
		পূর্ণাঙ্ক box_size_main;
		পূর্ণাঙ্ক *box_pos_main;

		যদি (ctx->horizontal) {
			b->width = item->target_main_size -
					lh__delta_outer_width(b);

			যদি (!layout_flex_item(ctx, item, b->width)) {
				প্রত্যাগম false;
			}
		}

		box_size_main = lh__box_size_main(ctx->horizontal, b);
		box_pos_main = ctx->horizontal ? &b->x : &b->y;

		যদি (!lh__box_is_absolute(b)) {
			যদি (b->margin[main_start] == AUTO) {
				extra_pre = extra + extra_remainder;
			}
			যদি (b->margin[main_end] == AUTO) {
				extra_post = extra + extra_remainder;
			}
			extra_total = extra_pre + extra_post;

			main_pos += pre_multiplier *
					(extra_total + box_size_main +
					 lh__delta_outer_main(ctx->flex, b));
		}

		*box_pos_main = main_pos + lh__non_auto_margin(b, main_start) +
				extra_pre + b->border[main_start].width;

		যদি (!lh__box_is_absolute(b)) {
			পূর্ণাঙ্ক cross_size;
			পূর্ণাঙ্ক box_size_cross = lh__box_size_cross(
					ctx->horizontal, b);

			main_pos += post_multiplier *
					(extra_total + box_size_main +
					 lh__delta_outer_main(ctx->flex, b));

			cross_size = box_size_cross + lh__delta_outer_cross(
					ctx->flex, b);
			যদি (সারি->cross_size < cross_size) {
				সারি->cross_size = cross_size;
			}
		}
	}

	প্রত্যাগম true;
}

/**
 * Collect items onto lines and place items along the lines
 *
 * \param[in] ctx   Flex layout context
 * \return true on success, false on failure.
 */
স্থির bool layout_flex_collect_বস্তুসমূহ_into_সারিগুলি(
		সংরচনা flex_ctx *ctx)
{
	size_t pos = 0;

	যাবত্ (pos < ctx->item.গুনতি) {
		সংরচনা flex_সারি_data *সারি;

		সারি = layout_flex_build_সারি(ctx, pos);
		যদি (সারি == রিক্ত) {
			প্রত্যাগম false;
		}

		pos += সারি->গুনতি;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: "
				"fitted: %zu (total: %zu/%zu)",
				ctx->flex, সারি->গুনতি,
				pos, ctx->item.গুনতি);

		যদি (!layout_flex_resolve_সারি(ctx, সারি)) {
			প্রত্যাগম false;
		}

		যদি (!layout_flex_place_সারি_বস্তুসমূহ_main(ctx, সারি)) {
			প্রত্যাগম false;
		}

		ctx->cross_size += সারি->cross_size;
		যদি (ctx->main_size < সারি->main_size) {
			ctx->main_size = সারি->main_size;
		}
	}

	প্রত্যাগম true;
}

/**
 * Align items on a line.
 *
 * \param[in] ctx    Flex layout context
 * \param[in] line   Line to align items on
 * \param[in] extra  Extra line width in pixels
 */
স্থির শূন্য layout_flex_place_সারি_বস্তুসমূহ_cross(সংরচনা flex_ctx *ctx,
		সংরচনা flex_সারি_data *সারি, পূর্ণাঙ্ক extra)
{
	গণনা box_side cross_start = ctx->horizontal ? শীর্ষ_2 : বাম_2;
	size_t item_গুনতি = সারি->first + সারি->গুনতি;

	প্রতি (size_t i = সারি->first; i < item_গুনতি; i++) {
		সংরচনা flex_item_data *item = &ctx->item.data[i];
		সংরচনা box *b = item->box;
		পূর্ণাঙ্ক cross_free_space;
		পূর্ণাঙ্ক *box_size_cross;
		পূর্ণাঙ্ক *box_pos_cross;

		box_pos_cross = ctx->horizontal ? &b->y : &b->x;
		box_size_cross = lh__box_size_cross_ptr(ctx->horizontal, b);

		cross_free_space = সারি->cross_size + extra - *box_size_cross -
				lh__delta_outer_cross(ctx->flex, b);

		বিকল্প (lh__box_align_self(ctx->flex, b)) {
		স্বভাব:
		পক্ষ CSS_ALIGN_SELF_STRETCH:
			যদি (lh__box_size_cross_is_auto(ctx->horizontal, b)) {
				*box_size_cross += cross_free_space;

				/* Relayout children for stretch. */
				যদি (!layout_flex_item(ctx, item, b->width)) {
					প্রত্যাগম;
				}
			}
			fallthrough;
		পক্ষ CSS_ALIGN_SELF_FLEX_START:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					সারি->pos +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			ভঙ্গ;

		পক্ষ CSS_ALIGN_SELF_FLEX_END:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					সারি->pos + cross_free_space +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			ভঙ্গ;

		পক্ষ CSS_ALIGN_SELF_BASELINE:
		পক্ষ CSS_ALIGN_SELF_CENTER:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					সারি->pos + cross_free_space / 2 +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			ভঙ্গ;
		}
	}
}

/**
 * Place the lines and align the items on the line.
 *
 * \param[in] ctx  Flex layout context
 */
স্থির শূন্য layout_flex_place_সারিগুলি(সংরচনা flex_ctx *ctx)
{
	bool reversed = ctx->wrap == CSS_FLEX_WRAP_WRAP_REVERSE;
	পূর্ণাঙ্ক সারি_pos = reversed ? ctx->cross_size : 0;
	পূর্ণাঙ্ক post_multiplier = reversed ? 0 : 1;
	পূর্ণাঙ্ক pre_multiplier = reversed ? -1 : 0;
	পূর্ণাঙ্ক extra_remainder = 0;
	পূর্ণাঙ্ক extra = 0;

	যদি (ctx->available_cross != AUTO &&
	    ctx->available_cross > ctx->cross_size &&
	    ctx->সারি.গুনতি > 0) {
		extra = ctx->available_cross - ctx->cross_size;

		extra_remainder = extra % ctx->সারি.গুনতি;
		extra /= ctx->সারি.গুনতি;
	}

	প্রতি (size_t i = 0; i < ctx->সারি.গুনতি; i++) {
		সংরচনা flex_সারি_data *সারি = &ctx->সারি.data[i];

		সারি_pos += pre_multiplier * সারি->cross_size;
		সারি->pos = সারি_pos;
		সারি_pos += post_multiplier * সারি->cross_size +
				extra + extra_remainder;

		layout_flex_place_সারি_বস্তুসমূহ_cross(ctx, সারি,
				extra + extra_remainder);

		যদি (extra_remainder > 0) {
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
bool layout_flex(সংরচনা box *flex, পূর্ণাঙ্ক available_width,
		html_content *content)
{
	পূর্ণাঙ্ক max_height, min_height;
	সংরচনা flex_ctx *ctx;
	bool success = false;

	ctx = layout_flex_ctx__create(content, flex);
	যদি (ctx == রিক্ত) {
		প্রত্যাগম false;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: %s, available_width %i, width: %i",
			flex, ctx->horizontal ? "horizontal" : "vertical",
			available_width, flex->width);

	layout_find_dimensions(
			ctx->unit_len_ctx, available_width, -1,
			flex, flex->style, রিক্ত, &flex->height,
			রিক্ত, রিক্ত, &max_height, &min_height,
			flex->margin, flex->padding, flex->border);

	available_width = min(available_width, flex->width);

	যদি (ctx->horizontal) {
		ctx->available_main = available_width;
		ctx->available_cross = ctx->flex->height;
	} অন্যথা {
		ctx->available_main = ctx->flex->height;
		ctx->available_cross = available_width;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: available_main: %i",
			flex, ctx->available_main);
	NSLOG(flex, DEEPDEBUG, "box %p: available_cross: %i",
			flex, ctx->available_cross);

	layout_flex_ctx__populate_item_data(ctx, flex, available_width);

	/* Place items onto lines. */
	success = layout_flex_collect_বস্তুসমূহ_into_সারিগুলি(ctx);
	যদি (!success) {
		গচ্ছ cleanup;
	}

	layout_flex_place_সারিগুলি(ctx);

	যদি (flex->height == AUTO) {
		flex->height = ctx->horizontal ?
				ctx->cross_size :
				ctx->main_size;
	}

	যদি (flex->height != AUTO) {
		যদি (max_height >= 0 && flex->height > max_height) {
			flex->height = max_height;
		}
		যদি (min_height >  0 && flex->height < min_height) {
			flex->height = min_height;
		}
	}

	success = true;

cleanup:
	layout_flex_ctx__destroy(ctx);

	NSLOG(flex, DEEPDEBUG, "box %p: %s: w: %i, h: %i", flex,
			success ? "success" : "failure",
			flex->width, flex->height);
	প্রত্যাগম success;
}
