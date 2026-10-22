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
কাঠামো flex_item_তথ্য {
	গণনা css_flex_basis_e basis;
	css_fixed basis_দৈর্ঘ্য;
	css_unit basis_unit;
	কাঠামো box *box;

	css_fixed shrink;
	css_fixed grow;

	পূর্ণসংখ্যা min_main;
	পূর্ণসংখ্যা max_main;
	পূর্ণসংখ্যা min_cross;
	পূর্ণসংখ্যা max_cross;

	পূর্ণসংখ্যা target_main_আকার;
	পূর্ণসংখ্যা base_আকার;
	পূর্ণসংখ্যা main_আকার;
	size_t সারি;

	bool freeze;
	bool min_violation;
	bool max_violation;
};

/**
 * Flex line data
 */
কাঠামো flex_সারি_তথ্য {
	পূর্ণসংখ্যা main_আকার;
	পূর্ণসংখ্যা cross_আকার;

	পূর্ণসংখ্যা used_main_আকার;
	পূর্ণসংখ্যা main_auto_margin_গুনতি;

	পূর্ণসংখ্যা pos;

	size_t first;
	size_t গুনতি;
	size_t frozen;
};

/**
 * Flex layout context
 */
কাঠামো flex_ctx {
	html_content *content;
	ধ্রুবক কাঠামো box *flex;
	ধ্রুবক css_unit_ctx *unit_len_ctx;

	পূর্ণসংখ্যা main_আকার;
	পূর্ণসংখ্যা cross_আকার;

	পূর্ণসংখ্যা available_main;
	পূর্ণসংখ্যা available_cross;

	bool horizontal;
	bool main_reversed;
	গণনা css_flex_wrap_e wrap;

	কাঠামো flex_বস্তুসমূহ {
		size_t গুনতি;
		কাঠামো flex_item_তথ্য *তথ্য;
	} item;

	কাঠামো flex_সারিগুলি {
		size_t গুনতি;
		size_t alloc;
		কাঠামো flex_সারি_তথ্য *তথ্য;
	} সারি;
};

/**
 * Destroy a flex layout context
 *
 * \param[in] ctx  Flex layout context
 */
স্থির শূন্য layout_flex_ctx__destroy(কাঠামো flex_ctx *ctx)
{
	যদি (ctx != ফাঁকা) {
		মুক্ত(ctx->item.তথ্য);
		মুক্ত(ctx->সারি.তথ্য);
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
স্থির কাঠামো flex_ctx *layout_flex_ctx__create(
		html_content *content,
		ধ্রুবক কাঠামো box *flex)
{
	কাঠামো flex_ctx *ctx;

	ctx = calloc(1, আকার(*ctx));
	যদি (ctx == ফাঁকা) {
		ফেরত ফাঁকা;
	}
	ctx->সারি.alloc = 1;

	ctx->item.গুনতি = box_গুনতি_children(flex);
	ctx->item.তথ্য = calloc(ctx->item.গুনতি, আকার(*ctx->item.তথ্য));
	যদি (ctx->item.তথ্য == ফাঁকা) {
		layout_flex_ctx__destroy(ctx);
		ফেরত ফাঁকা;
	}

	ctx->সারি.alloc = 1;
	ctx->সারি.তথ্য = calloc(ctx->সারি.alloc, আকার(*ctx->সারি.তথ্য));
	যদি (ctx->সারি.তথ্য == ফাঁকা) {
		layout_flex_ctx__destroy(ctx);
		ফেরত ফাঁকা;
	}

	ctx->flex = flex;
	ctx->content = content;
	ctx->unit_len_ctx = &content->unit_len_ctx;

	ctx->wrap = css_computed_flex_wrap(flex->style);
	ctx->horizontal = lh__flex_main_is_horizontal(flex);
	ctx->main_reversed = lh__flex_direction_reversed(flex);

	ফেরত ctx;
}

/**
 * Find box side representing the start of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the start side.
 */
স্থির গণনা box_side layout_flex__main_start_side(
		ধ্রুবক কাঠামো flex_ctx *ctx)
{
	যদি (ctx->horizontal) {
		ফেরত (ctx->main_reversed) ? ডান_2 : বাম_2;
	} অন্যথা {
		ফেরত (ctx->main_reversed) ? BOTTOM : শীর্ষ_2;
	}
}

/**
 * Find box side representing the end of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the end side.
 */
স্থির গণনা box_side layout_flex__main_end_side(
		ধ্রুবক কাঠামো flex_ctx *ctx)
{
	যদি (ctx->horizontal) {
		ফেরত (ctx->main_reversed) ? বাম_2 : ডান_2;
	} অন্যথা {
		ফেরত (ctx->main_reversed) ? শীর্ষ_2 : BOTTOM;
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
		ধ্রুবক কাঠামো flex_ctx *ctx,
		ধ্রুবক কাঠামো flex_item_তথ্য *item,
		পূর্ণসংখ্যা available_width)
{
	bool success;
	কাঠামো box *b = item->box;

	নির্বাচন (b->type) {
	ক্ষেত্র BOX_BLOCK:
		success = layout_block_context(b, -1, ctx->content);
		বিরতি;
	ক্ষেত্র BOX_TABLE:
		b->float_container = b->parent;
		success = layout_table(b, available_width, ctx->content);
		b->float_container = ফাঁকা;
		বিরতি;
	ক্ষেত্র BOX_FLEX:
		b->float_container = b->parent;
		success = layout_flex(b, available_width, ctx->content);
		b->float_container = ফাঁকা;
		বিরতি;
	পূর্বনির্ধারিত:
		প্রতিজ্ঞা(0 && "Bad flex item back type");
		success = false;
		বিরতি;
	}

	যদি (!success) {
		NSLOG(flex, ERROR, "box %p: layout failed", b);
	}

	ফেরত success;
}

/**
 * Calculate an item's base and target main sizes.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] item             Item to get sizes of
 * \param[in] available_width  Available width in pixels
 * \return true on success false on failure
 */
স্থির অন্তঃরেখ bool layout_flex__base_and_main_sizes(
		ধ্রুবক কাঠামো flex_ctx *ctx,
		কাঠামো flex_item_তথ্য *item,
		পূর্ণসংখ্যা available_width)
{
	কাঠামো box *b = item->box;
	পূর্ণসংখ্যা content_min_width = b->min_width;
	পূর্ণসংখ্যা content_max_width = b->max_width;
	পূর্ণসংখ্যা delta_outer_main = lh__delta_outer_main(ctx->flex, b);

	NSLOG(flex, DEEPDEBUG, "box %p: delta_outer_main: %i",
			b, delta_outer_main);

	যদি (item->basis == CSS_FLEX_BASIS_SET) {
		যদি (item->basis_unit == CSS_UNIT_PCT) {
			item->base_আকার = FPCT_OF_INT_TOINT(
					item->basis_দৈর্ঘ্য,
					available_width);
		} অন্যথা {
			item->base_আকার = FIXTOINT(css_unit_len2device_px(
					b->style, ctx->unit_len_ctx,
					item->basis_দৈর্ঘ্য,
					item->basis_unit));
		}

	} অন্যথা যদি (item->basis == CSS_FLEX_BASIS_AUTO) {
		item->base_আকার = ctx->horizontal ? b->width : b->height;
	} অন্যথা {
		item->base_আকার = AUTO;
	}

	যদি (ctx->horizontal == false) {
		যদি (b->width == AUTO) {
			b->width = min(max(content_min_width, available_width),
					content_max_width);
			b->width -= lh__delta_outer_width(b);
		}

		যদি (!layout_flex_item(ctx, item, b->width)) {
			ফেরত false;
		}
	}

	যদি (item->base_আকার == AUTO) {
		যদি (ctx->horizontal == false) {
			item->base_আকার = b->height;
		} অন্যথা {
			item->base_আকার = content_max_width - delta_outer_main;
		}
	}

	item->base_আকার += delta_outer_main;

	যদি (ctx->horizontal) {
		item->base_আকার = min(item->base_আকার, available_width);
		item->base_আকার = max(item->base_আকার, content_min_width);
	}

	item->target_main_আকার = item->base_আকার;
	item->main_আকার = item->base_আকার;

	যদি (item->max_main > 0 &&
	    item->main_আকার > item->max_main + delta_outer_main) {
		item->main_আকার = item->max_main + delta_outer_main;
	}

	যদি (item->main_আকার < item->min_main + delta_outer_main) {
		item->main_আকার = item->min_main + delta_outer_main;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: base_size: %i, main_size %i",
			b, item->base_আকার, item->main_আকার);

	ফেরত true;
}

/**
 * Fill out all item's data in a flex container.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] flex             Flex box
 * \param[in] available_width  Available width in pixels
 */
স্থির শূন্য layout_flex_ctx_populate_item_তথ্য(
		ধ্রুবক কাঠামো flex_ctx *ctx,
		ধ্রুবক কাঠামো box *flex,
		পূর্ণসংখ্যা available_width)
{
	size_t i = 0;
	bool horizontal = ctx->horizontal;

	জন্য (কাঠামো box *b = flex->children; b != ফাঁকা; b = b->পরবর্তী) {
		কাঠামো flex_item_তথ্য *item = &ctx->item.তথ্য[i++];

		b->float_container = b->parent;
		layout_find_dimensions(ctx->unit_len_ctx, available_width, -1,
				b, b->style, &b->width, &b->height,
				horizontal ? &item->max_main : &item->max_cross,
				horizontal ? &item->min_main : &item->min_cross,
				horizontal ? &item->max_cross : &item->max_main,
				horizontal ? &item->min_cross : &item->min_main,
				b->margin, b->padding, b->border);
		b->float_container = ফাঁকা;

		NSLOG(flex, DEEPDEBUG, "flex-item box: %p: width: %i",
				b, b->width);

		item->box = b;
		item->basis = css_computed_flex_basis(b->style,
				&item->basis_দৈর্ঘ্য, &item->basis_unit);

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
স্থির bool layout_flex_ctx_ensure_সারি(কাঠামো flex_ctx *ctx)
{
	কাঠামো flex_সারি_তথ্য *অস্থায়ী;
	size_t সারি_alloc = ctx->সারি.alloc * 2;

	যদি (ctx->সারি.alloc > ctx->সারি.গুনতি) {
		ফেরত true;
	}

	অস্থায়ী = realloc(ctx->সারি.তথ্য, আকার(*ctx->সারি.তথ্য) * সারি_alloc);
	যদি (অস্থায়ী == ফাঁকা) {
		ফেরত false;
	}
	ctx->সারি.তথ্য = অস্থায়ী;

	memset(ctx->সারি.তথ্য + ctx->সারি.alloc, 0,
	       আকার(*ctx->সারি.তথ্য) * (সারি_alloc - ctx->সারি.alloc));
	ctx->সারি.alloc = সারি_alloc;

	ফেরত true;
}

/**
 * Assigns flex items to the line and returns the line
 *
 * \param[in] ctx         Flex layout context
 * \param[in] item_index  Index to first item to assign to this line
 * \return Pointer to the new line, or NULL on error.
 */
স্থির কাঠামো flex_সারি_তথ্য *layout_flex_build_সারি(কাঠামো flex_ctx *ctx,
		size_t item_সূচক)
{
	গণনা box_side start_side = layout_flex__main_start_side(ctx);
	গণনা box_side end_side = layout_flex__main_end_side(ctx);
	কাঠামো flex_সারি_তথ্য *সারি;
	পূর্ণসংখ্যা used_main = 0;

	যদি (!layout_flex_ctx_ensure_সারি(ctx)) {
		ফেরত ফাঁকা;
	}

	সারি = &ctx->সারি.তথ্য[ctx->সারি.গুনতি];
	সারি->first = item_সূচক;

	NSLOG(flex, DEEPDEBUG, "flex container %p: available main: %i",
			ctx->flex, ctx->available_main);

	যখন (item_সূচক < ctx->item.গুনতি) {
		কাঠামো flex_item_তথ্য *item = &ctx->item.তথ্য[item_সূচক];
		কাঠামো box *b = item->box;
		পূর্ণসংখ্যা pos_main;

		pos_main = ctx->horizontal ?
				item->main_আকার :
				b->height + lh__delta_outer_main(ctx->flex, b);

		যদি (ctx->wrap == CSS_FLEX_WRAP_NOWRAP ||
		    pos_main + used_main <= ctx->available_main ||
		    lh__box_is_absolute(item->box) ||
		    ctx->available_main == AUTO ||
		    সারি->গুনতি == 0 ||
		    pos_main == 0) {
			যদি (lh__box_is_absolute(item->box) == false) {
				সারি->main_আকার += item->main_আকার;
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
			item_সূচক++;
		} অন্যথা {
			বিরতি;
		}
	}

	যদি (সারি->গুনতি > 0) {
		ctx->সারি.গুনতি++;
	} অন্যথা {
		NSLOG(layout, ERROR, "Failed to fit any flex items");
	}

	ফেরত সারি;
}

/**
 * Freeze an item on a line
 *
 * \param[in] line  Line to containing item
 * \param[in] item  Item to freeze
 */
স্থির অন্তঃরেখ শূন্য layout_flex__item_freeze(
		কাঠামো flex_সারি_তথ্য *সারি,
		কাঠামো flex_item_তথ্য *item)
{
	item->freeze = true;
	সারি->frozen++;

	যদি (!lh__box_is_absolute(item->box)){
		সারি->used_main_আকার += item->target_main_আকার;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: "
			"Frozen at target_main_size: %i",
			item->box, item->target_main_আকার);
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
স্থির অন্তঃরেখ পূর্ণসংখ্যা layout_flex__remaining_free_main(
		কাঠামো flex_ctx *ctx,
		কাঠামো flex_সারি_তথ্য *সারি,
		css_fixed *unfrozen_factor_যোগ,
		পূর্ণসংখ্যা initial_free_main,
		পূর্ণসংখ্যা available_main,
		bool grow)
{
	পূর্ণসংখ্যা remaining_free_main = available_main;
	size_t item_গুনতি = সারি->first + সারি->গুনতি;

	*unfrozen_factor_যোগ = 0;

	জন্য (size_t i = সারি->first; i < item_গুনতি; i++) {
		কাঠামো flex_item_তথ্য *item = &ctx->item.তথ্য[i];

		যদি (item->freeze) {
			remaining_free_main -= item->target_main_আকার;
		} অন্যথা {
			remaining_free_main -= item->base_আকার;

			*unfrozen_factor_যোগ += grow ?
					item->grow : item->shrink;
		}
	}

	যদি (*unfrozen_factor_যোগ < F_1) {
		পূর্ণসংখ্যা free_space = FIXTOINT(FMUL(INTTOFIX(initial_free_main),
				*unfrozen_factor_যোগ));

		যদি (free_space < remaining_free_main) {
			remaining_free_main = free_space;
		}
	}

	NSLOG(flex, DEEPDEBUG, "Remaining free space: %i",
			remaining_free_main);

	ফেরত remaining_free_main;
}

/**
 * Clamp flex item target main size and get min/max violations
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to align items on
 * return total violation in pixels
 */
স্থির অন্তঃরেখ পূর্ণসংখ্যা layout_flex__get_min_max_violations(
		কাঠামো flex_ctx *ctx,
		কাঠামো flex_সারি_তথ্য *সারি)
{

	পূর্ণসংখ্যা যোগ_violation = 0;
	size_t item_গুনতি = সারি->first + সারি->গুনতি;

	জন্য (size_t i = সারি->first; i < item_গুনতি; i++) {
		কাঠামো flex_item_তথ্য *item = &ctx->item.তথ্য[i];
		পূর্ণসংখ্যা target_main_আকার = item->target_main_আকার;

		NSLOG(flex, DEEPDEBUG, "item %p: target_main_size: %i",
					item->box, target_main_আকার);

		যদি (item->freeze) {
			চালু;
		}

		যদি (item->max_main > 0 &&
		    target_main_আকার > item->max_main) {
			target_main_আকার = item->max_main;
			item->max_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: max_main: %i",
					item->max_main);
		}

		যদি (target_main_আকার < item->min_main) {
			target_main_আকার = item->min_main;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: min_main: %i",
					item->min_main);
		}

		যদি (target_main_আকার < item->box->min_width) {
			target_main_আকার = item->box->min_width;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: box min_width: %i",
					item->box->min_width);
		}

		যদি (target_main_আকার < 0) {
			target_main_আকার = 0;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: less than 0");
		}

		যোগ_violation += target_main_আকার - item->target_main_আকার;
		item->target_main_আকার = target_main_আকার;
	}

	NSLOG(flex, DEEPDEBUG, "Total violation: %i", যোগ_violation);

	ফেরত যোগ_violation;
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
স্থির অন্তঃরেখ শূন্য layout_flex__distribute_free_main(
		কাঠামো flex_ctx *ctx,
		কাঠামো flex_সারি_তথ্য *সারি,
		css_fixed unfrozen_factor_যোগ,
		পূর্ণসংখ্যা remaining_free_main,
		bool grow)
{
	size_t item_গুনতি = সারি->first + সারি->গুনতি;

	যদি (grow) {
		css_fixed remainder = 0;
		জন্য (size_t i = সারি->first; i < item_গুনতি; i++) {
			কাঠামো flex_item_তথ্য *item = &ctx->item.তথ্য[i];
			css_fixed ফলাফল;
			css_fixed ratio;

			যদি (item->freeze) {
				চালু;
			}

			ratio = FDIV(item->grow, unfrozen_factor_যোগ);
			ফলাফল = FMUL(INTTOFIX(remaining_free_main), ratio) +
					remainder;

			item->target_main_আকার = item->base_আকার +
					FIXTOINT(ফলাফল);
			remainder = FIXFRAC(ফলাফল);
		}
	} অন্যথা {
		css_fixed scaled_shrink_factor_যোগ = 0;
		css_fixed remainder = 0;

		জন্য (size_t i = সারি->first; i < item_গুনতি; i++) {
			কাঠামো flex_item_তথ্য *item = &ctx->item.তথ্য[i];
			css_fixed scaled_shrink_factor;

			যদি (item->freeze) {
				চালু;
			}

			scaled_shrink_factor = FMUL(
					item->shrink,
					INTTOFIX(item->base_আকার));
			scaled_shrink_factor_যোগ += scaled_shrink_factor;
		}

		জন্য (size_t i = সারি->first; i < item_গুনতি; i++) {
			কাঠামো flex_item_তথ্য *item = &ctx->item.তথ্য[i];
			css_fixed scaled_shrink_factor;
			css_fixed ফলাফল;
			css_fixed ratio;

			যদি (item->freeze) {
				চালু;
			} অন্যথা যদি (scaled_shrink_factor_যোগ == 0) {
				item->target_main_আকার = item->main_আকার;
				layout_flex__item_freeze(সারি, item);
				চালু;
			}

			scaled_shrink_factor = FMUL(
					item->shrink,
					INTTOFIX(item->base_আকার));
			ratio = FDIV(scaled_shrink_factor,
			             scaled_shrink_factor_যোগ);
			ফলাফল = FMUL(INTTOFIX(abs(remaining_free_main)),
			              ratio) + remainder;

			item->target_main_আকার = item->base_আকার -
					FIXTOINT(ফলাফল);
			remainder = FIXFRAC(ফলাফল);
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
		কাঠামো flex_ctx *ctx,
		কাঠামো flex_সারি_তথ্য *সারি)
{
	size_t item_গুনতি = সারি->first + সারি->গুনতি;
	পূর্ণসংখ্যা available_main = ctx->available_main;
	পূর্ণসংখ্যা initial_free_main;
	bool grow;

	যদি (available_main == AUTO) {
		available_main = INT_MAX;
	}

	grow = (সারি->main_আকার < available_main);
	initial_free_main = available_main;

	NSLOG(flex, DEEPDEBUG, "box %p: line %zu: first: %zu, count: %zu",
			ctx->flex, সারি - ctx->সারি.তথ্য,
			সারি->first, সারি->গুনতি);
	NSLOG(flex, DEEPDEBUG, "Line main_size: %i, available_main: %i",
			সারি->main_আকার, available_main);

	জন্য (size_t i = সারি->first; i < item_গুনতি; i++) {
		কাঠামো flex_item_তথ্য *item = &ctx->item.তথ্য[i];

		/* 3. Size inflexible items */
		যদি (grow) {
			যদি (item->grow == 0 ||
			    item->base_আকার > item->main_আকার) {
				item->target_main_আকার = item->main_আকার;
				layout_flex__item_freeze(সারি, item);
			}
		} অন্যথা {
			যদি (item->shrink == 0 ||
			    item->base_আকার < item->main_আকার) {
				item->target_main_আকার = item->main_আকার;
				layout_flex__item_freeze(সারি, item);
			}
		}

		/* 4. Calculate initial free space */
		যদি (item->freeze) {
			initial_free_main -= item->target_main_আকার;
		} অন্যথা {
			initial_free_main -= item->base_আকার;
		}
	}

	/* 5. Loop */
	যখন (সারি->frozen < সারি->গুনতি) {
		css_fixed unfrozen_factor_যোগ;
		পূর্ণসংখ্যা remaining_free_main;
		পূর্ণসংখ্যা যোগ_violation;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: Resolver pass",
				ctx->flex);

		/* b */
		remaining_free_main = layout_flex__remaining_free_main(ctx,
				সারি, &unfrozen_factor_যোগ, initial_free_main,
				available_main, grow);

		/* c */
		যদি (remaining_free_main != 0) {
			layout_flex__distribute_free_main(ctx,
					সারি, unfrozen_factor_যোগ,
					remaining_free_main, grow);
		}

		/* d */
		যোগ_violation = layout_flex__get_min_max_violations(
				ctx, সারি);

		/* e */
		জন্য (size_t i = সারি->first; i < item_গুনতি; i++) {
			কাঠামো flex_item_তথ্য *item = &ctx->item.তথ্য[i];

			যদি (item->freeze) {
				চালু;
			}

			যদি (যোগ_violation == 0 ||
			    (যোগ_violation > 0 && item->min_violation) ||
			    (যোগ_violation < 0 && item->max_violation)) {
				layout_flex__item_freeze(সারি, item);
			}
		}
	}

	ফেরত true;
}

/**
 * Position items along a line
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to resolve
 * \return true on success, false on failure.
 */
স্থির bool layout_flex_place_সারি_বস্তুসমূহ_main(
		কাঠামো flex_ctx *ctx,
		কাঠামো flex_সারি_তথ্য *সারি)
{
	পূর্ণসংখ্যা main_pos = ctx->flex->padding[layout_flex__main_start_side(ctx)];
	পূর্ণসংখ্যা post_multiplier = ctx->main_reversed ? 0 : 1;
	পূর্ণসংখ্যা pre_multiplier = ctx->main_reversed ? -1 : 0;
	size_t item_গুনতি = সারি->first + সারি->গুনতি;
	পূর্ণসংখ্যা extra_remainder = 0;
	পূর্ণসংখ্যা extra = 0;

	যদি (ctx->main_reversed) {
		main_pos = lh_box_আকার_main(ctx->horizontal, ctx->flex) -
				main_pos;
	}

	যদি (ctx->available_main != AUTO &&
	    ctx->available_main != UNKNOWN_WIDTH &&
	    ctx->available_main > সারি->used_main_আকার) {
		যদি (সারি->main_auto_margin_গুনতি > 0) {
			extra = ctx->available_main - সারি->used_main_আকার;

			extra_remainder = extra % সারি->main_auto_margin_গুনতি;
			extra /= সারি->main_auto_margin_গুনতি;
		}
	}

	জন্য (size_t i = সারি->first; i < item_গুনতি; i++) {
		গণনা box_side main_end = ctx->horizontal ? ডান_2 : BOTTOM;
		গণনা box_side main_start = ctx->horizontal ? বাম_2 : শীর্ষ_2;
		কাঠামো flex_item_তথ্য *item = &ctx->item.তথ্য[i];
		কাঠামো box *b = item->box;
		পূর্ণসংখ্যা extra_যোগ = 0;
		পূর্ণসংখ্যা extra_post = 0;
		পূর্ণসংখ্যা extra_pre = 0;
		পূর্ণসংখ্যা box_আকার_main;
		পূর্ণসংখ্যা *box_pos_main;

		যদি (ctx->horizontal) {
			b->width = item->target_main_আকার -
					lh__delta_outer_width(b);

			যদি (!layout_flex_item(ctx, item, b->width)) {
				ফেরত false;
			}
		}

		box_আকার_main = lh_box_আকার_main(ctx->horizontal, b);
		box_pos_main = ctx->horizontal ? &b->x : &b->y;

		যদি (!lh__box_is_absolute(b)) {
			যদি (b->margin[main_start] == AUTO) {
				extra_pre = extra + extra_remainder;
			}
			যদি (b->margin[main_end] == AUTO) {
				extra_post = extra + extra_remainder;
			}
			extra_যোগ = extra_pre + extra_post;

			main_pos += pre_multiplier *
					(extra_যোগ + box_আকার_main +
					 lh__delta_outer_main(ctx->flex, b));
		}

		*box_pos_main = main_pos + lh__non_auto_margin(b, main_start) +
				extra_pre + b->border[main_start].width;

		যদি (!lh__box_is_absolute(b)) {
			পূর্ণসংখ্যা cross_আকার;
			পূর্ণসংখ্যা box_আকার_cross = lh_box_আকার_cross(
					ctx->horizontal, b);

			main_pos += post_multiplier *
					(extra_যোগ + box_আকার_main +
					 lh__delta_outer_main(ctx->flex, b));

			cross_আকার = box_আকার_cross + lh__delta_outer_cross(
					ctx->flex, b);
			যদি (সারি->cross_আকার < cross_আকার) {
				সারি->cross_আকার = cross_আকার;
			}
		}
	}

	ফেরত true;
}

/**
 * Collect items onto lines and place items along the lines
 *
 * \param[in] ctx   Flex layout context
 * \return true on success, false on failure.
 */
স্থির bool layout_flex_collect_বস্তুসমূহ_into_সারিগুলি(
		কাঠামো flex_ctx *ctx)
{
	size_t pos = 0;

	যখন (pos < ctx->item.গুনতি) {
		কাঠামো flex_সারি_তথ্য *সারি;

		সারি = layout_flex_build_সারি(ctx, pos);
		যদি (সারি == ফাঁকা) {
			ফেরত false;
		}

		pos += সারি->গুনতি;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: "
				"fitted: %zu (total: %zu/%zu)",
				ctx->flex, সারি->গুনতি,
				pos, ctx->item.গুনতি);

		যদি (!layout_flex_resolve_সারি(ctx, সারি)) {
			ফেরত false;
		}

		যদি (!layout_flex_place_সারি_বস্তুসমূহ_main(ctx, সারি)) {
			ফেরত false;
		}

		ctx->cross_আকার += সারি->cross_আকার;
		যদি (ctx->main_আকার < সারি->main_আকার) {
			ctx->main_আকার = সারি->main_আকার;
		}
	}

	ফেরত true;
}

/**
 * Align items on a line.
 *
 * \param[in] ctx    Flex layout context
 * \param[in] line   Line to align items on
 * \param[in] extra  Extra line width in pixels
 */
স্থির শূন্য layout_flex_place_সারি_বস্তুসমূহ_cross(কাঠামো flex_ctx *ctx,
		কাঠামো flex_সারি_তথ্য *সারি, পূর্ণসংখ্যা extra)
{
	গণনা box_side cross_start = ctx->horizontal ? শীর্ষ_2 : বাম_2;
	size_t item_গুনতি = সারি->first + সারি->গুনতি;

	জন্য (size_t i = সারি->first; i < item_গুনতি; i++) {
		কাঠামো flex_item_তথ্য *item = &ctx->item.তথ্য[i];
		কাঠামো box *b = item->box;
		পূর্ণসংখ্যা cross_free_space;
		পূর্ণসংখ্যা *box_আকার_cross;
		পূর্ণসংখ্যা *box_pos_cross;

		box_pos_cross = ctx->horizontal ? &b->y : &b->x;
		box_আকার_cross = lh_box_আকার_cross_ptr(ctx->horizontal, b);

		cross_free_space = সারি->cross_আকার + extra - *box_আকার_cross -
				lh__delta_outer_cross(ctx->flex, b);

		নির্বাচন (lh__box_align_self(ctx->flex, b)) {
		পূর্বনির্ধারিত:
		ক্ষেত্র CSS_ALIGN_SELF_STRETCH:
			যদি (lh_box_আকার_cross_is_auto(ctx->horizontal, b)) {
				*box_আকার_cross += cross_free_space;

				/* Relayout children for stretch. */
				যদি (!layout_flex_item(ctx, item, b->width)) {
					ফেরত;
				}
			}
			fallthrough;
		ক্ষেত্র CSS_ALIGN_SELF_FLEX_START:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					সারি->pos +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			বিরতি;

		ক্ষেত্র CSS_ALIGN_SELF_FLEX_END:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					সারি->pos + cross_free_space +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			বিরতি;

		ক্ষেত্র CSS_ALIGN_SELF_BASELINE:
		ক্ষেত্র CSS_ALIGN_SELF_CENTER:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					সারি->pos + cross_free_space / 2 +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			বিরতি;
		}
	}
}

/**
 * Place the lines and align the items on the line.
 *
 * \param[in] ctx  Flex layout context
 */
স্থির শূন্য layout_flex_place_সারিগুলি(কাঠামো flex_ctx *ctx)
{
	bool reversed = ctx->wrap == CSS_FLEX_WRAP_WRAP_REVERSE;
	পূর্ণসংখ্যা সারি_pos = reversed ? ctx->cross_আকার : 0;
	পূর্ণসংখ্যা post_multiplier = reversed ? 0 : 1;
	পূর্ণসংখ্যা pre_multiplier = reversed ? -1 : 0;
	পূর্ণসংখ্যা extra_remainder = 0;
	পূর্ণসংখ্যা extra = 0;

	যদি (ctx->available_cross != AUTO &&
	    ctx->available_cross > ctx->cross_আকার &&
	    ctx->সারি.গুনতি > 0) {
		extra = ctx->available_cross - ctx->cross_আকার;

		extra_remainder = extra % ctx->সারি.গুনতি;
		extra /= ctx->সারি.গুনতি;
	}

	জন্য (size_t i = 0; i < ctx->সারি.গুনতি; i++) {
		কাঠামো flex_সারি_তথ্য *সারি = &ctx->সারি.তথ্য[i];

		সারি_pos += pre_multiplier * সারি->cross_আকার;
		সারি->pos = সারি_pos;
		সারি_pos += post_multiplier * সারি->cross_আকার +
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
bool layout_flex(কাঠামো box *flex, পূর্ণসংখ্যা available_width,
		html_content *content)
{
	পূর্ণসংখ্যা max_height, min_height;
	কাঠামো flex_ctx *ctx;
	bool success = false;

	ctx = layout_flex_ctx__create(content, flex);
	যদি (ctx == ফাঁকা) {
		ফেরত false;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: %s, available_width %i, width: %i",
			flex, ctx->horizontal ? "horizontal" : "vertical",
			available_width, flex->width);

	layout_find_dimensions(
			ctx->unit_len_ctx, available_width, -1,
			flex, flex->style, ফাঁকা, &flex->height,
			ফাঁকা, ফাঁকা, &max_height, &min_height,
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

	layout_flex_ctx_populate_item_তথ্য(ctx, flex, available_width);

	/* Place items onto lines. */
	success = layout_flex_collect_বস্তুসমূহ_into_সারিগুলি(ctx);
	যদি (!success) {
		যাও cleanup;
	}

	layout_flex_place_সারিগুলি(ctx);

	যদি (flex->height == AUTO) {
		flex->height = ctx->horizontal ?
				ctx->cross_আকার :
				ctx->main_আকার;
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
	ফেরত success;
}
