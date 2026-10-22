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

#அந்தர்பாவ <string.h>

#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/utils.h"

#அந்தர்பாவ "html/box.h"
#அந்தர்பாவ "html/html.h"
#அந்தர்பாவ "html/private.h"
#அந்தர்பாவ "html/box_inspect.h"
#அந்தர்பாவ "html/layout_internal.h"

/**
 * Flex item data
 */
கட்டமைப்பு flex_item_data {
	எண்ணிக்கை css_flex_basis_e basis;
	css_fixed basis_length;
	css_unit basis_unit;
	கட்டமைப்பு box *box;

	css_fixed shrink;
	css_fixed grow;

	முழு min_main;
	முழு max_main;
	முழு min_cross;
	முழு max_cross;

	முழு target_main_size;
	முழு base_size;
	முழு main_size;
	size_t வரி;

	bool freeze;
	bool min_violation;
	bool max_violation;
};

/**
 * Flex line data
 */
கட்டமைப்பு flex_வரி_data {
	முழு main_size;
	முழு cross_size;

	முழு used_main_size;
	முழு main_auto_margin_மொத்தம்;

	முழு pos;

	size_t first;
	size_t மொத்தம்;
	size_t frozen;
};

/**
 * Flex layout context
 */
கட்டமைப்பு flex_ctx {
	html_content *content;
	மாறிலி கட்டமைப்பு box *flex;
	மாறிலி css_unit_ctx *unit_len_ctx;

	முழு main_size;
	முழு cross_size;

	முழு available_main;
	முழு available_cross;

	bool horizontal;
	bool main_reversed;
	எண்ணிக்கை css_flex_wrap_e wrap;

	கட்டமைப்பு flex_பொருட்கள் {
		size_t மொத்தம்;
		கட்டமைப்பு flex_item_data *data;
	} item;

	கட்டமைப்பு flex_வரிகள் {
		size_t மொத்தம்;
		size_t alloc;
		கட்டமைப்பு flex_வரி_data *data;
	} வரி;
};

/**
 * Destroy a flex layout context
 *
 * \param[in] ctx  Flex layout context
 */
நிலையான வெறுமை layout_flex_ctx__destroy(கட்டமைப்பு flex_ctx *ctx)
{
	எனில் (ctx != வெற்று) {
		விடுவி(ctx->item.data);
		விடுவி(ctx->வரி.data);
		விடுவி(ctx);
	}
}

/**
 * Create a flex layout context
 *
 * \param[in] content  HTML content containing flex box
 * \param[in] flex     Box to create layout context for
 * \return flex layout context or NULL on error
 */
நிலையான கட்டமைப்பு flex_ctx *layout_flex_ctx__create(
		html_content *content,
		மாறிலி கட்டமைப்பு box *flex)
{
	கட்டமைப்பு flex_ctx *ctx;

	ctx = calloc(1, அளவு(*ctx));
	எனில் (ctx == வெற்று) {
		திரும்பு வெற்று;
	}
	ctx->வரி.alloc = 1;

	ctx->item.மொத்தம் = box_மொத்தம்_children(flex);
	ctx->item.data = calloc(ctx->item.மொத்தம், அளவு(*ctx->item.data));
	எனில் (ctx->item.data == வெற்று) {
		layout_flex_ctx__destroy(ctx);
		திரும்பு வெற்று;
	}

	ctx->வரி.alloc = 1;
	ctx->வரி.data = calloc(ctx->வரி.alloc, அளவு(*ctx->வரி.data));
	எனில் (ctx->வரி.data == வெற்று) {
		layout_flex_ctx__destroy(ctx);
		திரும்பு வெற்று;
	}

	ctx->flex = flex;
	ctx->content = content;
	ctx->unit_len_ctx = &content->unit_len_ctx;

	ctx->wrap = css_computed_flex_wrap(flex->style);
	ctx->horizontal = lh__flex_main_is_horizontal(flex);
	ctx->main_reversed = lh__flex_direction_reversed(flex);

	திரும்பு ctx;
}

/**
 * Find box side representing the start of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the start side.
 */
நிலையான எண்ணிக்கை box_side layout_flex__main_start_side(
		மாறிலி கட்டமைப்பு flex_ctx *ctx)
{
	எனில் (ctx->horizontal) {
		திரும்பு (ctx->main_reversed) ? வலது_2 : இடது_2;
	} இல்லையெனில் {
		திரும்பு (ctx->main_reversed) ? BOTTOM : உச்சி_2;
	}
}

/**
 * Find box side representing the end of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the end side.
 */
நிலையான எண்ணிக்கை box_side layout_flex__main_end_side(
		மாறிலி கட்டமைப்பு flex_ctx *ctx)
{
	எனில் (ctx->horizontal) {
		திரும்பு (ctx->main_reversed) ? இடது_2 : வலது_2;
	} இல்லையெனில் {
		திரும்பு (ctx->main_reversed) ? உச்சி_2 : BOTTOM;
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
நிலையான bool layout_flex_item(
		மாறிலி கட்டமைப்பு flex_ctx *ctx,
		மாறிலி கட்டமைப்பு flex_item_data *item,
		முழு available_width)
{
	bool success;
	கட்டமைப்பு box *b = item->box;

	தேர்வு (b->type) {
	நிலை BOX_BLOCK:
		success = layout_block_context(b, -1, ctx->content);
		நிறுத்து;
	நிலை BOX_TABLE:
		b->float_container = b->parent;
		success = layout_table(b, available_width, ctx->content);
		b->float_container = வெற்று;
		நிறுத்து;
	நிலை BOX_FLEX:
		b->float_container = b->parent;
		success = layout_flex(b, available_width, ctx->content);
		b->float_container = வெற்று;
		நிறுத்து;
	இயல்பு:
		ப்ரதிஜ்ஞா(0 && "Bad flex item back type");
		success = false;
		நிறுத்து;
	}

	எனில் (!success) {
		NSLOG(flex, ERROR, "box %p: layout failed", b);
	}

	திரும்பு success;
}

/**
 * Calculate an item's base and target main sizes.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] item             Item to get sizes of
 * \param[in] available_width  Available width in pixels
 * \return true on success false on failure
 */
நிலையான உள்வரி bool layout_flex__base_and_main_sizes(
		மாறிலி கட்டமைப்பு flex_ctx *ctx,
		கட்டமைப்பு flex_item_data *item,
		முழு available_width)
{
	கட்டமைப்பு box *b = item->box;
	முழு content_min_width = b->min_width;
	முழு content_max_width = b->max_width;
	முழு delta_outer_main = lh__delta_outer_main(ctx->flex, b);

	NSLOG(flex, DEEPDEBUG, "box %p: delta_outer_main: %i",
			b, delta_outer_main);

	எனில் (item->basis == CSS_FLEX_BASIS_SET) {
		எனில் (item->basis_unit == CSS_UNIT_PCT) {
			item->base_size = FPCT_OF_INT_TOINT(
					item->basis_length,
					available_width);
		} இல்லையெனில் {
			item->base_size = FIXTOINT(css_unit_len2device_px(
					b->style, ctx->unit_len_ctx,
					item->basis_length,
					item->basis_unit));
		}

	} இல்லையெனில் எனில் (item->basis == CSS_FLEX_BASIS_AUTO) {
		item->base_size = ctx->horizontal ? b->width : b->height;
	} இல்லையெனில் {
		item->base_size = AUTO;
	}

	எனில் (ctx->horizontal == false) {
		எனில் (b->width == AUTO) {
			b->width = min(max(content_min_width, available_width),
					content_max_width);
			b->width -= lh__delta_outer_width(b);
		}

		எனில் (!layout_flex_item(ctx, item, b->width)) {
			திரும்பு false;
		}
	}

	எனில் (item->base_size == AUTO) {
		எனில் (ctx->horizontal == false) {
			item->base_size = b->height;
		} இல்லையெனில் {
			item->base_size = content_max_width - delta_outer_main;
		}
	}

	item->base_size += delta_outer_main;

	எனில் (ctx->horizontal) {
		item->base_size = min(item->base_size, available_width);
		item->base_size = max(item->base_size, content_min_width);
	}

	item->target_main_size = item->base_size;
	item->main_size = item->base_size;

	எனில் (item->max_main > 0 &&
	    item->main_size > item->max_main + delta_outer_main) {
		item->main_size = item->max_main + delta_outer_main;
	}

	எனில் (item->main_size < item->min_main + delta_outer_main) {
		item->main_size = item->min_main + delta_outer_main;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: base_size: %i, main_size %i",
			b, item->base_size, item->main_size);

	திரும்பு true;
}

/**
 * Fill out all item's data in a flex container.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] flex             Flex box
 * \param[in] available_width  Available width in pixels
 */
நிலையான வெறுமை layout_flex_ctx__populate_item_data(
		மாறிலி கட்டமைப்பு flex_ctx *ctx,
		மாறிலி கட்டமைப்பு box *flex,
		முழு available_width)
{
	size_t i = 0;
	bool horizontal = ctx->horizontal;

	க்கு (கட்டமைப்பு box *b = flex->children; b != வெற்று; b = b->next) {
		கட்டமைப்பு flex_item_data *item = &ctx->item.data[i++];

		b->float_container = b->parent;
		layout_find_dimensions(ctx->unit_len_ctx, available_width, -1,
				b, b->style, &b->width, &b->height,
				horizontal ? &item->max_main : &item->max_cross,
				horizontal ? &item->min_main : &item->min_cross,
				horizontal ? &item->max_cross : &item->max_main,
				horizontal ? &item->min_cross : &item->min_main,
				b->margin, b->padding, b->border);
		b->float_container = வெற்று;

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
நிலையான bool layout_flex_ctx_ensure_வரி(கட்டமைப்பு flex_ctx *ctx)
{
	கட்டமைப்பு flex_வரி_data *temp;
	size_t வரி_alloc = ctx->வரி.alloc * 2;

	எனில் (ctx->வரி.alloc > ctx->வரி.மொத்தம்) {
		திரும்பு true;
	}

	temp = realloc(ctx->வரி.data, அளவு(*ctx->வரி.data) * வரி_alloc);
	எனில் (temp == வெற்று) {
		திரும்பு false;
	}
	ctx->வரி.data = temp;

	memset(ctx->வரி.data + ctx->வரி.alloc, 0,
	       அளவு(*ctx->வரி.data) * (வரி_alloc - ctx->வரி.alloc));
	ctx->வரி.alloc = வரி_alloc;

	திரும்பு true;
}

/**
 * Assigns flex items to the line and returns the line
 *
 * \param[in] ctx         Flex layout context
 * \param[in] item_index  Index to first item to assign to this line
 * \return Pointer to the new line, or NULL on error.
 */
நிலையான கட்டமைப்பு flex_வரி_data *layout_flex_build_வரி(கட்டமைப்பு flex_ctx *ctx,
		size_t item_index)
{
	எண்ணிக்கை box_side start_side = layout_flex__main_start_side(ctx);
	எண்ணிக்கை box_side end_side = layout_flex__main_end_side(ctx);
	கட்டமைப்பு flex_வரி_data *வரி;
	முழு used_main = 0;

	எனில் (!layout_flex_ctx_ensure_வரி(ctx)) {
		திரும்பு வெற்று;
	}

	வரி = &ctx->வரி.data[ctx->வரி.மொத்தம்];
	வரி->first = item_index;

	NSLOG(flex, DEEPDEBUG, "flex container %p: available main: %i",
			ctx->flex, ctx->available_main);

	போது (item_index < ctx->item.மொத்தம்) {
		கட்டமைப்பு flex_item_data *item = &ctx->item.data[item_index];
		கட்டமைப்பு box *b = item->box;
		முழு pos_main;

		pos_main = ctx->horizontal ?
				item->main_size :
				b->height + lh__delta_outer_main(ctx->flex, b);

		எனில் (ctx->wrap == CSS_FLEX_WRAP_NOWRAP ||
		    pos_main + used_main <= ctx->available_main ||
		    lh__box_is_absolute(item->box) ||
		    ctx->available_main == AUTO ||
		    வரி->மொத்தம் == 0 ||
		    pos_main == 0) {
			எனில் (lh__box_is_absolute(item->box) == false) {
				வரி->main_size += item->main_size;
				used_main += pos_main;

				எனில் (b->margin[start_side] == AUTO) {
					வரி->main_auto_margin_மொத்தம்++;
				}
				எனில் (b->margin[end_side] == AUTO) {
					வரி->main_auto_margin_மொத்தம்++;
				}
			}
			item->வரி = ctx->வரி.மொத்தம்;
			வரி->மொத்தம்++;
			item_index++;
		} இல்லையெனில் {
			நிறுத்து;
		}
	}

	எனில் (வரி->மொத்தம் > 0) {
		ctx->வரி.மொத்தம்++;
	} இல்லையெனில் {
		NSLOG(layout, ERROR, "Failed to fit any flex items");
	}

	திரும்பு வரி;
}

/**
 * Freeze an item on a line
 *
 * \param[in] line  Line to containing item
 * \param[in] item  Item to freeze
 */
நிலையான உள்வரி வெறுமை layout_flex__item_freeze(
		கட்டமைப்பு flex_வரி_data *வரி,
		கட்டமைப்பு flex_item_data *item)
{
	item->freeze = true;
	வரி->frozen++;

	எனில் (!lh__box_is_absolute(item->box)){
		வரி->used_main_size += item->target_main_size;
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
நிலையான உள்வரி முழு layout_flex__remaining_free_main(
		கட்டமைப்பு flex_ctx *ctx,
		கட்டமைப்பு flex_வரி_data *வரி,
		css_fixed *unfrozen_factor_sum,
		முழு initial_free_main,
		முழு available_main,
		bool grow)
{
	முழு remaining_free_main = available_main;
	size_t item_மொத்தம் = வரி->first + வரி->மொத்தம்;

	*unfrozen_factor_sum = 0;

	க்கு (size_t i = வரி->first; i < item_மொத்தம்; i++) {
		கட்டமைப்பு flex_item_data *item = &ctx->item.data[i];

		எனில் (item->freeze) {
			remaining_free_main -= item->target_main_size;
		} இல்லையெனில் {
			remaining_free_main -= item->base_size;

			*unfrozen_factor_sum += grow ?
					item->grow : item->shrink;
		}
	}

	எனில் (*unfrozen_factor_sum < F_1) {
		முழு free_space = FIXTOINT(FMUL(INTTOFIX(initial_free_main),
				*unfrozen_factor_sum));

		எனில் (free_space < remaining_free_main) {
			remaining_free_main = free_space;
		}
	}

	NSLOG(flex, DEEPDEBUG, "Remaining free space: %i",
			remaining_free_main);

	திரும்பு remaining_free_main;
}

/**
 * Clamp flex item target main size and get min/max violations
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to align items on
 * return total violation in pixels
 */
நிலையான உள்வரி முழு layout_flex__get_min_max_violations(
		கட்டமைப்பு flex_ctx *ctx,
		கட்டமைப்பு flex_வரி_data *வரி)
{

	முழு total_violation = 0;
	size_t item_மொத்தம் = வரி->first + வரி->மொத்தம்;

	க்கு (size_t i = வரி->first; i < item_மொத்தம்; i++) {
		கட்டமைப்பு flex_item_data *item = &ctx->item.data[i];
		முழு target_main_size = item->target_main_size;

		NSLOG(flex, DEEPDEBUG, "item %p: target_main_size: %i",
					item->box, target_main_size);

		எனில் (item->freeze) {
			தொடர்;
		}

		எனில் (item->max_main > 0 &&
		    target_main_size > item->max_main) {
			target_main_size = item->max_main;
			item->max_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: max_main: %i",
					item->max_main);
		}

		எனில் (target_main_size < item->min_main) {
			target_main_size = item->min_main;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: min_main: %i",
					item->min_main);
		}

		எனில் (target_main_size < item->box->min_width) {
			target_main_size = item->box->min_width;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: box min_width: %i",
					item->box->min_width);
		}

		எனில் (target_main_size < 0) {
			target_main_size = 0;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: less than 0");
		}

		total_violation += target_main_size - item->target_main_size;
		item->target_main_size = target_main_size;
	}

	NSLOG(flex, DEEPDEBUG, "Total violation: %i", total_violation);

	திரும்பு total_violation;
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
நிலையான உள்வரி வெறுமை layout_flex__distribute_free_main(
		கட்டமைப்பு flex_ctx *ctx,
		கட்டமைப்பு flex_வரி_data *வரி,
		css_fixed unfrozen_factor_sum,
		முழு remaining_free_main,
		bool grow)
{
	size_t item_மொத்தம் = வரி->first + வரி->மொத்தம்;

	எனில் (grow) {
		css_fixed remainder = 0;
		க்கு (size_t i = வரி->first; i < item_மொத்தம்; i++) {
			கட்டமைப்பு flex_item_data *item = &ctx->item.data[i];
			css_fixed result;
			css_fixed ratio;

			எனில் (item->freeze) {
				தொடர்;
			}

			ratio = FDIV(item->grow, unfrozen_factor_sum);
			result = FMUL(INTTOFIX(remaining_free_main), ratio) +
					remainder;

			item->target_main_size = item->base_size +
					FIXTOINT(result);
			remainder = FIXFRAC(result);
		}
	} இல்லையெனில் {
		css_fixed scaled_shrink_factor_sum = 0;
		css_fixed remainder = 0;

		க்கு (size_t i = வரி->first; i < item_மொத்தம்; i++) {
			கட்டமைப்பு flex_item_data *item = &ctx->item.data[i];
			css_fixed scaled_shrink_factor;

			எனில் (item->freeze) {
				தொடர்;
			}

			scaled_shrink_factor = FMUL(
					item->shrink,
					INTTOFIX(item->base_size));
			scaled_shrink_factor_sum += scaled_shrink_factor;
		}

		க்கு (size_t i = வரி->first; i < item_மொத்தம்; i++) {
			கட்டமைப்பு flex_item_data *item = &ctx->item.data[i];
			css_fixed scaled_shrink_factor;
			css_fixed result;
			css_fixed ratio;

			எனில் (item->freeze) {
				தொடர்;
			} இல்லையெனில் எனில் (scaled_shrink_factor_sum == 0) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(வரி, item);
				தொடர்;
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
நிலையான bool layout_flex_resolve_வரி(
		கட்டமைப்பு flex_ctx *ctx,
		கட்டமைப்பு flex_வரி_data *வரி)
{
	size_t item_மொத்தம் = வரி->first + வரி->மொத்தம்;
	முழு available_main = ctx->available_main;
	முழு initial_free_main;
	bool grow;

	எனில் (available_main == AUTO) {
		available_main = INT_MAX;
	}

	grow = (வரி->main_size < available_main);
	initial_free_main = available_main;

	NSLOG(flex, DEEPDEBUG, "box %p: line %zu: first: %zu, count: %zu",
			ctx->flex, வரி - ctx->வரி.data,
			வரி->first, வரி->மொத்தம்);
	NSLOG(flex, DEEPDEBUG, "Line main_size: %i, available_main: %i",
			வரி->main_size, available_main);

	க்கு (size_t i = வரி->first; i < item_மொத்தம்; i++) {
		கட்டமைப்பு flex_item_data *item = &ctx->item.data[i];

		/* 3. Size inflexible items */
		எனில் (grow) {
			எனில் (item->grow == 0 ||
			    item->base_size > item->main_size) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(வரி, item);
			}
		} இல்லையெனில் {
			எனில் (item->shrink == 0 ||
			    item->base_size < item->main_size) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(வரி, item);
			}
		}

		/* 4. Calculate initial free space */
		எனில் (item->freeze) {
			initial_free_main -= item->target_main_size;
		} இல்லையெனில் {
			initial_free_main -= item->base_size;
		}
	}

	/* 5. Loop */
	போது (வரி->frozen < வரி->மொத்தம்) {
		css_fixed unfrozen_factor_sum;
		முழு remaining_free_main;
		முழு total_violation;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: Resolver pass",
				ctx->flex);

		/* b */
		remaining_free_main = layout_flex__remaining_free_main(ctx,
				வரி, &unfrozen_factor_sum, initial_free_main,
				available_main, grow);

		/* c */
		எனில் (remaining_free_main != 0) {
			layout_flex__distribute_free_main(ctx,
					வரி, unfrozen_factor_sum,
					remaining_free_main, grow);
		}

		/* d */
		total_violation = layout_flex__get_min_max_violations(
				ctx, வரி);

		/* e */
		க்கு (size_t i = வரி->first; i < item_மொத்தம்; i++) {
			கட்டமைப்பு flex_item_data *item = &ctx->item.data[i];

			எனில் (item->freeze) {
				தொடர்;
			}

			எனில் (total_violation == 0 ||
			    (total_violation > 0 && item->min_violation) ||
			    (total_violation < 0 && item->max_violation)) {
				layout_flex__item_freeze(வரி, item);
			}
		}
	}

	திரும்பு true;
}

/**
 * Position items along a line
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to resolve
 * \return true on success, false on failure.
 */
நிலையான bool layout_flex_place_வரி_பொருட்கள்_main(
		கட்டமைப்பு flex_ctx *ctx,
		கட்டமைப்பு flex_வரி_data *வரி)
{
	முழு main_pos = ctx->flex->padding[layout_flex__main_start_side(ctx)];
	முழு post_multiplier = ctx->main_reversed ? 0 : 1;
	முழு pre_multiplier = ctx->main_reversed ? -1 : 0;
	size_t item_மொத்தம் = வரி->first + வரி->மொத்தம்;
	முழு extra_remainder = 0;
	முழு extra = 0;

	எனில் (ctx->main_reversed) {
		main_pos = lh__box_size_main(ctx->horizontal, ctx->flex) -
				main_pos;
	}

	எனில் (ctx->available_main != AUTO &&
	    ctx->available_main != UNKNOWN_WIDTH &&
	    ctx->available_main > வரி->used_main_size) {
		எனில் (வரி->main_auto_margin_மொத்தம் > 0) {
			extra = ctx->available_main - வரி->used_main_size;

			extra_remainder = extra % வரி->main_auto_margin_மொத்தம்;
			extra /= வரி->main_auto_margin_மொத்தம்;
		}
	}

	க்கு (size_t i = வரி->first; i < item_மொத்தம்; i++) {
		எண்ணிக்கை box_side main_end = ctx->horizontal ? வலது_2 : BOTTOM;
		எண்ணிக்கை box_side main_start = ctx->horizontal ? இடது_2 : உச்சி_2;
		கட்டமைப்பு flex_item_data *item = &ctx->item.data[i];
		கட்டமைப்பு box *b = item->box;
		முழு extra_total = 0;
		முழு extra_post = 0;
		முழு extra_pre = 0;
		முழு box_size_main;
		முழு *box_pos_main;

		எனில் (ctx->horizontal) {
			b->width = item->target_main_size -
					lh__delta_outer_width(b);

			எனில் (!layout_flex_item(ctx, item, b->width)) {
				திரும்பு false;
			}
		}

		box_size_main = lh__box_size_main(ctx->horizontal, b);
		box_pos_main = ctx->horizontal ? &b->x : &b->y;

		எனில் (!lh__box_is_absolute(b)) {
			எனில் (b->margin[main_start] == AUTO) {
				extra_pre = extra + extra_remainder;
			}
			எனில் (b->margin[main_end] == AUTO) {
				extra_post = extra + extra_remainder;
			}
			extra_total = extra_pre + extra_post;

			main_pos += pre_multiplier *
					(extra_total + box_size_main +
					 lh__delta_outer_main(ctx->flex, b));
		}

		*box_pos_main = main_pos + lh__non_auto_margin(b, main_start) +
				extra_pre + b->border[main_start].width;

		எனில் (!lh__box_is_absolute(b)) {
			முழு cross_size;
			முழு box_size_cross = lh__box_size_cross(
					ctx->horizontal, b);

			main_pos += post_multiplier *
					(extra_total + box_size_main +
					 lh__delta_outer_main(ctx->flex, b));

			cross_size = box_size_cross + lh__delta_outer_cross(
					ctx->flex, b);
			எனில் (வரி->cross_size < cross_size) {
				வரி->cross_size = cross_size;
			}
		}
	}

	திரும்பு true;
}

/**
 * Collect items onto lines and place items along the lines
 *
 * \param[in] ctx   Flex layout context
 * \return true on success, false on failure.
 */
நிலையான bool layout_flex_collect_பொருட்கள்_into_வரிகள்(
		கட்டமைப்பு flex_ctx *ctx)
{
	size_t pos = 0;

	போது (pos < ctx->item.மொத்தம்) {
		கட்டமைப்பு flex_வரி_data *வரி;

		வரி = layout_flex_build_வரி(ctx, pos);
		எனில் (வரி == வெற்று) {
			திரும்பு false;
		}

		pos += வரி->மொத்தம்;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: "
				"fitted: %zu (total: %zu/%zu)",
				ctx->flex, வரி->மொத்தம்,
				pos, ctx->item.மொத்தம்);

		எனில் (!layout_flex_resolve_வரி(ctx, வரி)) {
			திரும்பு false;
		}

		எனில் (!layout_flex_place_வரி_பொருட்கள்_main(ctx, வரி)) {
			திரும்பு false;
		}

		ctx->cross_size += வரி->cross_size;
		எனில் (ctx->main_size < வரி->main_size) {
			ctx->main_size = வரி->main_size;
		}
	}

	திரும்பு true;
}

/**
 * Align items on a line.
 *
 * \param[in] ctx    Flex layout context
 * \param[in] line   Line to align items on
 * \param[in] extra  Extra line width in pixels
 */
நிலையான வெறுமை layout_flex_place_வரி_பொருட்கள்_cross(கட்டமைப்பு flex_ctx *ctx,
		கட்டமைப்பு flex_வரி_data *வரி, முழு extra)
{
	எண்ணிக்கை box_side cross_start = ctx->horizontal ? உச்சி_2 : இடது_2;
	size_t item_மொத்தம் = வரி->first + வரி->மொத்தம்;

	க்கு (size_t i = வரி->first; i < item_மொத்தம்; i++) {
		கட்டமைப்பு flex_item_data *item = &ctx->item.data[i];
		கட்டமைப்பு box *b = item->box;
		முழு cross_free_space;
		முழு *box_size_cross;
		முழு *box_pos_cross;

		box_pos_cross = ctx->horizontal ? &b->y : &b->x;
		box_size_cross = lh__box_size_cross_ptr(ctx->horizontal, b);

		cross_free_space = வரி->cross_size + extra - *box_size_cross -
				lh__delta_outer_cross(ctx->flex, b);

		தேர்வு (lh__box_align_self(ctx->flex, b)) {
		இயல்பு:
		நிலை CSS_ALIGN_SELF_STRETCH:
			எனில் (lh__box_size_cross_is_auto(ctx->horizontal, b)) {
				*box_size_cross += cross_free_space;

				/* Relayout children for stretch. */
				எனில் (!layout_flex_item(ctx, item, b->width)) {
					திரும்பு;
				}
			}
			fallthrough;
		நிலை CSS_ALIGN_SELF_FLEX_START:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					வரி->pos +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			நிறுத்து;

		நிலை CSS_ALIGN_SELF_FLEX_END:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					வரி->pos + cross_free_space +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			நிறுத்து;

		நிலை CSS_ALIGN_SELF_BASELINE:
		நிலை CSS_ALIGN_SELF_CENTER:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					வரி->pos + cross_free_space / 2 +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			நிறுத்து;
		}
	}
}

/**
 * Place the lines and align the items on the line.
 *
 * \param[in] ctx  Flex layout context
 */
நிலையான வெறுமை layout_flex_place_வரிகள்(கட்டமைப்பு flex_ctx *ctx)
{
	bool reversed = ctx->wrap == CSS_FLEX_WRAP_WRAP_REVERSE;
	முழு வரி_pos = reversed ? ctx->cross_size : 0;
	முழு post_multiplier = reversed ? 0 : 1;
	முழு pre_multiplier = reversed ? -1 : 0;
	முழு extra_remainder = 0;
	முழு extra = 0;

	எனில் (ctx->available_cross != AUTO &&
	    ctx->available_cross > ctx->cross_size &&
	    ctx->வரி.மொத்தம் > 0) {
		extra = ctx->available_cross - ctx->cross_size;

		extra_remainder = extra % ctx->வரி.மொத்தம்;
		extra /= ctx->வரி.மொத்தம்;
	}

	க்கு (size_t i = 0; i < ctx->வரி.மொத்தம்; i++) {
		கட்டமைப்பு flex_வரி_data *வரி = &ctx->வரி.data[i];

		வரி_pos += pre_multiplier * வரி->cross_size;
		வரி->pos = வரி_pos;
		வரி_pos += post_multiplier * வரி->cross_size +
				extra + extra_remainder;

		layout_flex_place_வரி_பொருட்கள்_cross(ctx, வரி,
				extra + extra_remainder);

		எனில் (extra_remainder > 0) {
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
bool layout_flex(கட்டமைப்பு box *flex, முழு available_width,
		html_content *content)
{
	முழு max_height, min_height;
	கட்டமைப்பு flex_ctx *ctx;
	bool success = false;

	ctx = layout_flex_ctx__create(content, flex);
	எனில் (ctx == வெற்று) {
		திரும்பு false;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: %s, available_width %i, width: %i",
			flex, ctx->horizontal ? "horizontal" : "vertical",
			available_width, flex->width);

	layout_find_dimensions(
			ctx->unit_len_ctx, available_width, -1,
			flex, flex->style, வெற்று, &flex->height,
			வெற்று, வெற்று, &max_height, &min_height,
			flex->margin, flex->padding, flex->border);

	available_width = min(available_width, flex->width);

	எனில் (ctx->horizontal) {
		ctx->available_main = available_width;
		ctx->available_cross = ctx->flex->height;
	} இல்லையெனில் {
		ctx->available_main = ctx->flex->height;
		ctx->available_cross = available_width;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: available_main: %i",
			flex, ctx->available_main);
	NSLOG(flex, DEEPDEBUG, "box %p: available_cross: %i",
			flex, ctx->available_cross);

	layout_flex_ctx__populate_item_data(ctx, flex, available_width);

	/* Place items onto lines. */
	success = layout_flex_collect_பொருட்கள்_into_வரிகள்(ctx);
	எனில் (!success) {
		செல் cleanup;
	}

	layout_flex_place_வரிகள்(ctx);

	எனில் (flex->height == AUTO) {
		flex->height = ctx->horizontal ?
				ctx->cross_size :
				ctx->main_size;
	}

	எனில் (flex->height != AUTO) {
		எனில் (max_height >= 0 && flex->height > max_height) {
			flex->height = max_height;
		}
		எனில் (min_height >  0 && flex->height < min_height) {
			flex->height = min_height;
		}
	}

	success = true;

cleanup:
	layout_flex_ctx__destroy(ctx);

	NSLOG(flex, DEEPDEBUG, "box %p: %s: w: %i, h: %i", flex,
			success ? "success" : "failure",
			flex->width, flex->height);
	திரும்பு success;
}
