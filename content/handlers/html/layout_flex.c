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

#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/utils.h"

#അന്തര്ഭാവ "html/box.h"
#അന്തര്ഭാവ "html/html.h"
#അന്തര്ഭാവ "html/private.h"
#അന്തര്ഭാവ "html/box_inspect.h"
#അന്തര്ഭാവ "html/layout_internal.h"

/**
 * Flex item data
 */
ഘടന flex_item_data {
	എണ്ണൽ css_flex_basis_e basis;
	css_fixed basis_length;
	css_unit basis_unit;
	ഘടന box *box;

	css_fixed shrink;
	css_fixed grow;

	പൂർണ്ണസംഖ്യ min_main;
	പൂർണ്ണസംഖ്യ max_main;
	പൂർണ്ണസംഖ്യ min_cross;
	പൂർണ്ണസംഖ്യ max_cross;

	പൂർണ്ണസംഖ്യ target_main_size;
	പൂർണ്ണസംഖ്യ base_size;
	പൂർണ്ണസംഖ്യ main_size;
	size_t വരി;

	bool freeze;
	bool min_violation;
	bool max_violation;
};

/**
 * Flex line data
 */
ഘടന flex_വരി_data {
	പൂർണ്ണസംഖ്യ main_size;
	പൂർണ്ണസംഖ്യ cross_size;

	പൂർണ്ണസംഖ്യ used_main_size;
	പൂർണ്ണസംഖ്യ main_auto_margin_എണ്ണം;

	പൂർണ്ണസംഖ്യ pos;

	size_t first;
	size_t എണ്ണം;
	size_t frozen;
};

/**
 * Flex layout context
 */
ഘടന flex_ctx {
	html_content *content;
	സ്ഥിരം ഘടന box *flex;
	സ്ഥിരം css_unit_ctx *unit_len_ctx;

	പൂർണ്ണസംഖ്യ main_size;
	പൂർണ്ണസംഖ്യ cross_size;

	പൂർണ്ണസംഖ്യ available_main;
	പൂർണ്ണസംഖ്യ available_cross;

	bool horizontal;
	bool main_reversed;
	എണ്ണൽ css_flex_wrap_e wrap;

	ഘടന flex_ഇനങ്ങൾ {
		size_t എണ്ണം;
		ഘടന flex_item_data *data;
	} item;

	ഘടന flex_വരികൾ {
		size_t എണ്ണം;
		size_t alloc;
		ഘടന flex_വരി_data *data;
	} വരി;
};

/**
 * Destroy a flex layout context
 *
 * \param[in] ctx  Flex layout context
 */
സ്ഥിതം ശൂന്യം layout_flex_ctx__destroy(ഘടന flex_ctx *ctx)
{
	എങ്കിൽ (ctx != ശൂന്യ) {
		സ്വതന്ത്രം(ctx->item.data);
		സ്വതന്ത്രം(ctx->വരി.data);
		സ്വതന്ത്രം(ctx);
	}
}

/**
 * Create a flex layout context
 *
 * \param[in] content  HTML content containing flex box
 * \param[in] flex     Box to create layout context for
 * \return flex layout context or NULL on error
 */
സ്ഥിതം ഘടന flex_ctx *layout_flex_ctx__create(
		html_content *content,
		സ്ഥിരം ഘടന box *flex)
{
	ഘടന flex_ctx *ctx;

	ctx = calloc(1, വലുപ്പം(*ctx));
	എങ്കിൽ (ctx == ശൂന്യ) {
		തിരികെ ശൂന്യ;
	}
	ctx->വരി.alloc = 1;

	ctx->item.എണ്ണം = box_എണ്ണം_children(flex);
	ctx->item.data = calloc(ctx->item.എണ്ണം, വലുപ്പം(*ctx->item.data));
	എങ്കിൽ (ctx->item.data == ശൂന്യ) {
		layout_flex_ctx__destroy(ctx);
		തിരികെ ശൂന്യ;
	}

	ctx->വരി.alloc = 1;
	ctx->വരി.data = calloc(ctx->വരി.alloc, വലുപ്പം(*ctx->വരി.data));
	എങ്കിൽ (ctx->വരി.data == ശൂന്യ) {
		layout_flex_ctx__destroy(ctx);
		തിരികെ ശൂന്യ;
	}

	ctx->flex = flex;
	ctx->content = content;
	ctx->unit_len_ctx = &content->unit_len_ctx;

	ctx->wrap = css_computed_flex_wrap(flex->style);
	ctx->horizontal = lh__flex_main_is_horizontal(flex);
	ctx->main_reversed = lh__flex_direction_reversed(flex);

	തിരികെ ctx;
}

/**
 * Find box side representing the start of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the start side.
 */
സ്ഥിതം എണ്ണൽ box_side layout_flex__main_start_side(
		സ്ഥിരം ഘടന flex_ctx *ctx)
{
	എങ്കിൽ (ctx->horizontal) {
		തിരികെ (ctx->main_reversed) ? വലത്_2 : ഇടത്_2;
	} അല്ലെങ്കിൽ {
		തിരികെ (ctx->main_reversed) ? BOTTOM : മുകൾ_2;
	}
}

/**
 * Find box side representing the end of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the end side.
 */
സ്ഥിതം എണ്ണൽ box_side layout_flex__main_end_side(
		സ്ഥിരം ഘടന flex_ctx *ctx)
{
	എങ്കിൽ (ctx->horizontal) {
		തിരികെ (ctx->main_reversed) ? ഇടത്_2 : വലത്_2;
	} അല്ലെങ്കിൽ {
		തിരികെ (ctx->main_reversed) ? മുകൾ_2 : BOTTOM;
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
സ്ഥിതം bool layout_flex_item(
		സ്ഥിരം ഘടന flex_ctx *ctx,
		സ്ഥിരം ഘടന flex_item_data *item,
		പൂർണ്ണസംഖ്യ available_width)
{
	bool success;
	ഘടന box *b = item->box;

	തിരഞ്ഞെടുപ്പ് (b->type) {
	അവസ്ഥ BOX_BLOCK:
		success = layout_block_context(b, -1, ctx->content);
		നിർത്തുക;
	അവസ്ഥ BOX_TABLE:
		b->float_container = b->parent;
		success = layout_table(b, available_width, ctx->content);
		b->float_container = ശൂന്യ;
		നിർത്തുക;
	അവസ്ഥ BOX_FLEX:
		b->float_container = b->parent;
		success = layout_flex(b, available_width, ctx->content);
		b->float_container = ശൂന്യ;
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		പ്രതിജ്ഞാ(0 && "Bad flex item back type");
		success = false;
		നിർത്തുക;
	}

	എങ്കിൽ (!success) {
		NSLOG(flex, ERROR, "box %p: layout failed", b);
	}

	തിരികെ success;
}

/**
 * Calculate an item's base and target main sizes.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] item             Item to get sizes of
 * \param[in] available_width  Available width in pixels
 * \return true on success false on failure
 */
സ്ഥിതം വരിയിൽ bool layout_flex__base_and_main_sizes(
		സ്ഥിരം ഘടന flex_ctx *ctx,
		ഘടന flex_item_data *item,
		പൂർണ്ണസംഖ്യ available_width)
{
	ഘടന box *b = item->box;
	പൂർണ്ണസംഖ്യ content_min_width = b->min_width;
	പൂർണ്ണസംഖ്യ content_max_width = b->max_width;
	പൂർണ്ണസംഖ്യ delta_outer_main = lh__delta_outer_main(ctx->flex, b);

	NSLOG(flex, DEEPDEBUG, "box %p: delta_outer_main: %i",
			b, delta_outer_main);

	എങ്കിൽ (item->basis == CSS_FLEX_BASIS_SET) {
		എങ്കിൽ (item->basis_unit == CSS_UNIT_PCT) {
			item->base_size = FPCT_OF_INT_TOINT(
					item->basis_length,
					available_width);
		} അല്ലെങ്കിൽ {
			item->base_size = FIXTOINT(css_unit_len2device_px(
					b->style, ctx->unit_len_ctx,
					item->basis_length,
					item->basis_unit));
		}

	} അല്ലെങ്കിൽ എങ്കിൽ (item->basis == CSS_FLEX_BASIS_AUTO) {
		item->base_size = ctx->horizontal ? b->width : b->height;
	} അല്ലെങ്കിൽ {
		item->base_size = AUTO;
	}

	എങ്കിൽ (ctx->horizontal == false) {
		എങ്കിൽ (b->width == AUTO) {
			b->width = min(max(content_min_width, available_width),
					content_max_width);
			b->width -= lh__delta_outer_width(b);
		}

		എങ്കിൽ (!layout_flex_item(ctx, item, b->width)) {
			തിരികെ false;
		}
	}

	എങ്കിൽ (item->base_size == AUTO) {
		എങ്കിൽ (ctx->horizontal == false) {
			item->base_size = b->height;
		} അല്ലെങ്കിൽ {
			item->base_size = content_max_width - delta_outer_main;
		}
	}

	item->base_size += delta_outer_main;

	എങ്കിൽ (ctx->horizontal) {
		item->base_size = min(item->base_size, available_width);
		item->base_size = max(item->base_size, content_min_width);
	}

	item->target_main_size = item->base_size;
	item->main_size = item->base_size;

	എങ്കിൽ (item->max_main > 0 &&
	    item->main_size > item->max_main + delta_outer_main) {
		item->main_size = item->max_main + delta_outer_main;
	}

	എങ്കിൽ (item->main_size < item->min_main + delta_outer_main) {
		item->main_size = item->min_main + delta_outer_main;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: base_size: %i, main_size %i",
			b, item->base_size, item->main_size);

	തിരികെ true;
}

/**
 * Fill out all item's data in a flex container.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] flex             Flex box
 * \param[in] available_width  Available width in pixels
 */
സ്ഥിതം ശൂന്യം layout_flex_ctx__populate_item_data(
		സ്ഥിരം ഘടന flex_ctx *ctx,
		സ്ഥിരം ഘടന box *flex,
		പൂർണ്ണസംഖ്യ available_width)
{
	size_t i = 0;
	bool horizontal = ctx->horizontal;

	വേണ്ടി (ഘടന box *b = flex->children; b != ശൂന്യ; b = b->next) {
		ഘടന flex_item_data *item = &ctx->item.data[i++];

		b->float_container = b->parent;
		layout_find_dimensions(ctx->unit_len_ctx, available_width, -1,
				b, b->style, &b->width, &b->height,
				horizontal ? &item->max_main : &item->max_cross,
				horizontal ? &item->min_main : &item->min_cross,
				horizontal ? &item->max_cross : &item->max_main,
				horizontal ? &item->min_cross : &item->min_main,
				b->margin, b->padding, b->border);
		b->float_container = ശൂന്യ;

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
സ്ഥിതം bool layout_flex_ctx_ensure_വരി(ഘടന flex_ctx *ctx)
{
	ഘടന flex_വരി_data *temp;
	size_t വരി_alloc = ctx->വരി.alloc * 2;

	എങ്കിൽ (ctx->വരി.alloc > ctx->വരി.എണ്ണം) {
		തിരികെ true;
	}

	temp = realloc(ctx->വരി.data, വലുപ്പം(*ctx->വരി.data) * വരി_alloc);
	എങ്കിൽ (temp == ശൂന്യ) {
		തിരികെ false;
	}
	ctx->വരി.data = temp;

	memset(ctx->വരി.data + ctx->വരി.alloc, 0,
	       വലുപ്പം(*ctx->വരി.data) * (വരി_alloc - ctx->വരി.alloc));
	ctx->വരി.alloc = വരി_alloc;

	തിരികെ true;
}

/**
 * Assigns flex items to the line and returns the line
 *
 * \param[in] ctx         Flex layout context
 * \param[in] item_index  Index to first item to assign to this line
 * \return Pointer to the new line, or NULL on error.
 */
സ്ഥിതം ഘടന flex_വരി_data *layout_flex_build_വരി(ഘടന flex_ctx *ctx,
		size_t item_index)
{
	എണ്ണൽ box_side start_side = layout_flex__main_start_side(ctx);
	എണ്ണൽ box_side end_side = layout_flex__main_end_side(ctx);
	ഘടന flex_വരി_data *വരി;
	പൂർണ്ണസംഖ്യ used_main = 0;

	എങ്കിൽ (!layout_flex_ctx_ensure_വരി(ctx)) {
		തിരികെ ശൂന്യ;
	}

	വരി = &ctx->വരി.data[ctx->വരി.എണ്ണം];
	വരി->first = item_index;

	NSLOG(flex, DEEPDEBUG, "flex container %p: available main: %i",
			ctx->flex, ctx->available_main);

	ആകുമ്പോൾ (item_index < ctx->item.എണ്ണം) {
		ഘടന flex_item_data *item = &ctx->item.data[item_index];
		ഘടന box *b = item->box;
		പൂർണ്ണസംഖ്യ pos_main;

		pos_main = ctx->horizontal ?
				item->main_size :
				b->height + lh__delta_outer_main(ctx->flex, b);

		എങ്കിൽ (ctx->wrap == CSS_FLEX_WRAP_NOWRAP ||
		    pos_main + used_main <= ctx->available_main ||
		    lh__box_is_absolute(item->box) ||
		    ctx->available_main == AUTO ||
		    വരി->എണ്ണം == 0 ||
		    pos_main == 0) {
			എങ്കിൽ (lh__box_is_absolute(item->box) == false) {
				വരി->main_size += item->main_size;
				used_main += pos_main;

				എങ്കിൽ (b->margin[start_side] == AUTO) {
					വരി->main_auto_margin_എണ്ണം++;
				}
				എങ്കിൽ (b->margin[end_side] == AUTO) {
					വരി->main_auto_margin_എണ്ണം++;
				}
			}
			item->വരി = ctx->വരി.എണ്ണം;
			വരി->എണ്ണം++;
			item_index++;
		} അല്ലെങ്കിൽ {
			നിർത്തുക;
		}
	}

	എങ്കിൽ (വരി->എണ്ണം > 0) {
		ctx->വരി.എണ്ണം++;
	} അല്ലെങ്കിൽ {
		NSLOG(layout, ERROR, "Failed to fit any flex items");
	}

	തിരികെ വരി;
}

/**
 * Freeze an item on a line
 *
 * \param[in] line  Line to containing item
 * \param[in] item  Item to freeze
 */
സ്ഥിതം വരിയിൽ ശൂന്യം layout_flex__item_freeze(
		ഘടന flex_വരി_data *വരി,
		ഘടന flex_item_data *item)
{
	item->freeze = true;
	വരി->frozen++;

	എങ്കിൽ (!lh__box_is_absolute(item->box)){
		വരി->used_main_size += item->target_main_size;
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
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ layout_flex__remaining_free_main(
		ഘടന flex_ctx *ctx,
		ഘടന flex_വരി_data *വരി,
		css_fixed *unfrozen_factor_sum,
		പൂർണ്ണസംഖ്യ initial_free_main,
		പൂർണ്ണസംഖ്യ available_main,
		bool grow)
{
	പൂർണ്ണസംഖ്യ remaining_free_main = available_main;
	size_t item_എണ്ണം = വരി->first + വരി->എണ്ണം;

	*unfrozen_factor_sum = 0;

	വേണ്ടി (size_t i = വരി->first; i < item_എണ്ണം; i++) {
		ഘടന flex_item_data *item = &ctx->item.data[i];

		എങ്കിൽ (item->freeze) {
			remaining_free_main -= item->target_main_size;
		} അല്ലെങ്കിൽ {
			remaining_free_main -= item->base_size;

			*unfrozen_factor_sum += grow ?
					item->grow : item->shrink;
		}
	}

	എങ്കിൽ (*unfrozen_factor_sum < F_1) {
		പൂർണ്ണസംഖ്യ free_space = FIXTOINT(FMUL(INTTOFIX(initial_free_main),
				*unfrozen_factor_sum));

		എങ്കിൽ (free_space < remaining_free_main) {
			remaining_free_main = free_space;
		}
	}

	NSLOG(flex, DEEPDEBUG, "Remaining free space: %i",
			remaining_free_main);

	തിരികെ remaining_free_main;
}

/**
 * Clamp flex item target main size and get min/max violations
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to align items on
 * return total violation in pixels
 */
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ layout_flex__get_min_max_violations(
		ഘടന flex_ctx *ctx,
		ഘടന flex_വരി_data *വരി)
{

	പൂർണ്ണസംഖ്യ total_violation = 0;
	size_t item_എണ്ണം = വരി->first + വരി->എണ്ണം;

	വേണ്ടി (size_t i = വരി->first; i < item_എണ്ണം; i++) {
		ഘടന flex_item_data *item = &ctx->item.data[i];
		പൂർണ്ണസംഖ്യ target_main_size = item->target_main_size;

		NSLOG(flex, DEEPDEBUG, "item %p: target_main_size: %i",
					item->box, target_main_size);

		എങ്കിൽ (item->freeze) {
			തുടരുക;
		}

		എങ്കിൽ (item->max_main > 0 &&
		    target_main_size > item->max_main) {
			target_main_size = item->max_main;
			item->max_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: max_main: %i",
					item->max_main);
		}

		എങ്കിൽ (target_main_size < item->min_main) {
			target_main_size = item->min_main;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: min_main: %i",
					item->min_main);
		}

		എങ്കിൽ (target_main_size < item->box->min_width) {
			target_main_size = item->box->min_width;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: box min_width: %i",
					item->box->min_width);
		}

		എങ്കിൽ (target_main_size < 0) {
			target_main_size = 0;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: less than 0");
		}

		total_violation += target_main_size - item->target_main_size;
		item->target_main_size = target_main_size;
	}

	NSLOG(flex, DEEPDEBUG, "Total violation: %i", total_violation);

	തിരികെ total_violation;
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
സ്ഥിതം വരിയിൽ ശൂന്യം layout_flex__distribute_free_main(
		ഘടന flex_ctx *ctx,
		ഘടന flex_വരി_data *വരി,
		css_fixed unfrozen_factor_sum,
		പൂർണ്ണസംഖ്യ remaining_free_main,
		bool grow)
{
	size_t item_എണ്ണം = വരി->first + വരി->എണ്ണം;

	എങ്കിൽ (grow) {
		css_fixed remainder = 0;
		വേണ്ടി (size_t i = വരി->first; i < item_എണ്ണം; i++) {
			ഘടന flex_item_data *item = &ctx->item.data[i];
			css_fixed result;
			css_fixed ratio;

			എങ്കിൽ (item->freeze) {
				തുടരുക;
			}

			ratio = FDIV(item->grow, unfrozen_factor_sum);
			result = FMUL(INTTOFIX(remaining_free_main), ratio) +
					remainder;

			item->target_main_size = item->base_size +
					FIXTOINT(result);
			remainder = FIXFRAC(result);
		}
	} അല്ലെങ്കിൽ {
		css_fixed scaled_shrink_factor_sum = 0;
		css_fixed remainder = 0;

		വേണ്ടി (size_t i = വരി->first; i < item_എണ്ണം; i++) {
			ഘടന flex_item_data *item = &ctx->item.data[i];
			css_fixed scaled_shrink_factor;

			എങ്കിൽ (item->freeze) {
				തുടരുക;
			}

			scaled_shrink_factor = FMUL(
					item->shrink,
					INTTOFIX(item->base_size));
			scaled_shrink_factor_sum += scaled_shrink_factor;
		}

		വേണ്ടി (size_t i = വരി->first; i < item_എണ്ണം; i++) {
			ഘടന flex_item_data *item = &ctx->item.data[i];
			css_fixed scaled_shrink_factor;
			css_fixed result;
			css_fixed ratio;

			എങ്കിൽ (item->freeze) {
				തുടരുക;
			} അല്ലെങ്കിൽ എങ്കിൽ (scaled_shrink_factor_sum == 0) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(വരി, item);
				തുടരുക;
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
സ്ഥിതം bool layout_flex_resolve_വരി(
		ഘടന flex_ctx *ctx,
		ഘടന flex_വരി_data *വരി)
{
	size_t item_എണ്ണം = വരി->first + വരി->എണ്ണം;
	പൂർണ്ണസംഖ്യ available_main = ctx->available_main;
	പൂർണ്ണസംഖ്യ initial_free_main;
	bool grow;

	എങ്കിൽ (available_main == AUTO) {
		available_main = INT_MAX;
	}

	grow = (വരി->main_size < available_main);
	initial_free_main = available_main;

	NSLOG(flex, DEEPDEBUG, "box %p: line %zu: first: %zu, count: %zu",
			ctx->flex, വരി - ctx->വരി.data,
			വരി->first, വരി->എണ്ണം);
	NSLOG(flex, DEEPDEBUG, "Line main_size: %i, available_main: %i",
			വരി->main_size, available_main);

	വേണ്ടി (size_t i = വരി->first; i < item_എണ്ണം; i++) {
		ഘടന flex_item_data *item = &ctx->item.data[i];

		/* 3. Size inflexible items */
		എങ്കിൽ (grow) {
			എങ്കിൽ (item->grow == 0 ||
			    item->base_size > item->main_size) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(വരി, item);
			}
		} അല്ലെങ്കിൽ {
			എങ്കിൽ (item->shrink == 0 ||
			    item->base_size < item->main_size) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(വരി, item);
			}
		}

		/* 4. Calculate initial free space */
		എങ്കിൽ (item->freeze) {
			initial_free_main -= item->target_main_size;
		} അല്ലെങ്കിൽ {
			initial_free_main -= item->base_size;
		}
	}

	/* 5. Loop */
	ആകുമ്പോൾ (വരി->frozen < വരി->എണ്ണം) {
		css_fixed unfrozen_factor_sum;
		പൂർണ്ണസംഖ്യ remaining_free_main;
		പൂർണ്ണസംഖ്യ total_violation;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: Resolver pass",
				ctx->flex);

		/* b */
		remaining_free_main = layout_flex__remaining_free_main(ctx,
				വരി, &unfrozen_factor_sum, initial_free_main,
				available_main, grow);

		/* c */
		എങ്കിൽ (remaining_free_main != 0) {
			layout_flex__distribute_free_main(ctx,
					വരി, unfrozen_factor_sum,
					remaining_free_main, grow);
		}

		/* d */
		total_violation = layout_flex__get_min_max_violations(
				ctx, വരി);

		/* e */
		വേണ്ടി (size_t i = വരി->first; i < item_എണ്ണം; i++) {
			ഘടന flex_item_data *item = &ctx->item.data[i];

			എങ്കിൽ (item->freeze) {
				തുടരുക;
			}

			എങ്കിൽ (total_violation == 0 ||
			    (total_violation > 0 && item->min_violation) ||
			    (total_violation < 0 && item->max_violation)) {
				layout_flex__item_freeze(വരി, item);
			}
		}
	}

	തിരികെ true;
}

/**
 * Position items along a line
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to resolve
 * \return true on success, false on failure.
 */
സ്ഥിതം bool layout_flex_place_വരി_ഇനങ്ങൾ_main(
		ഘടന flex_ctx *ctx,
		ഘടന flex_വരി_data *വരി)
{
	പൂർണ്ണസംഖ്യ main_pos = ctx->flex->padding[layout_flex__main_start_side(ctx)];
	പൂർണ്ണസംഖ്യ post_multiplier = ctx->main_reversed ? 0 : 1;
	പൂർണ്ണസംഖ്യ pre_multiplier = ctx->main_reversed ? -1 : 0;
	size_t item_എണ്ണം = വരി->first + വരി->എണ്ണം;
	പൂർണ്ണസംഖ്യ extra_remainder = 0;
	പൂർണ്ണസംഖ്യ extra = 0;

	എങ്കിൽ (ctx->main_reversed) {
		main_pos = lh__box_size_main(ctx->horizontal, ctx->flex) -
				main_pos;
	}

	എങ്കിൽ (ctx->available_main != AUTO &&
	    ctx->available_main != UNKNOWN_WIDTH &&
	    ctx->available_main > വരി->used_main_size) {
		എങ്കിൽ (വരി->main_auto_margin_എണ്ണം > 0) {
			extra = ctx->available_main - വരി->used_main_size;

			extra_remainder = extra % വരി->main_auto_margin_എണ്ണം;
			extra /= വരി->main_auto_margin_എണ്ണം;
		}
	}

	വേണ്ടി (size_t i = വരി->first; i < item_എണ്ണം; i++) {
		എണ്ണൽ box_side main_end = ctx->horizontal ? വലത്_2 : BOTTOM;
		എണ്ണൽ box_side main_start = ctx->horizontal ? ഇടത്_2 : മുകൾ_2;
		ഘടന flex_item_data *item = &ctx->item.data[i];
		ഘടന box *b = item->box;
		പൂർണ്ണസംഖ്യ extra_total = 0;
		പൂർണ്ണസംഖ്യ extra_post = 0;
		പൂർണ്ണസംഖ്യ extra_pre = 0;
		പൂർണ്ണസംഖ്യ box_size_main;
		പൂർണ്ണസംഖ്യ *box_pos_main;

		എങ്കിൽ (ctx->horizontal) {
			b->width = item->target_main_size -
					lh__delta_outer_width(b);

			എങ്കിൽ (!layout_flex_item(ctx, item, b->width)) {
				തിരികെ false;
			}
		}

		box_size_main = lh__box_size_main(ctx->horizontal, b);
		box_pos_main = ctx->horizontal ? &b->x : &b->y;

		എങ്കിൽ (!lh__box_is_absolute(b)) {
			എങ്കിൽ (b->margin[main_start] == AUTO) {
				extra_pre = extra + extra_remainder;
			}
			എങ്കിൽ (b->margin[main_end] == AUTO) {
				extra_post = extra + extra_remainder;
			}
			extra_total = extra_pre + extra_post;

			main_pos += pre_multiplier *
					(extra_total + box_size_main +
					 lh__delta_outer_main(ctx->flex, b));
		}

		*box_pos_main = main_pos + lh__non_auto_margin(b, main_start) +
				extra_pre + b->border[main_start].width;

		എങ്കിൽ (!lh__box_is_absolute(b)) {
			പൂർണ്ണസംഖ്യ cross_size;
			പൂർണ്ണസംഖ്യ box_size_cross = lh__box_size_cross(
					ctx->horizontal, b);

			main_pos += post_multiplier *
					(extra_total + box_size_main +
					 lh__delta_outer_main(ctx->flex, b));

			cross_size = box_size_cross + lh__delta_outer_cross(
					ctx->flex, b);
			എങ്കിൽ (വരി->cross_size < cross_size) {
				വരി->cross_size = cross_size;
			}
		}
	}

	തിരികെ true;
}

/**
 * Collect items onto lines and place items along the lines
 *
 * \param[in] ctx   Flex layout context
 * \return true on success, false on failure.
 */
സ്ഥിതം bool layout_flex_collect_ഇനങ്ങൾ_into_വരികൾ(
		ഘടന flex_ctx *ctx)
{
	size_t pos = 0;

	ആകുമ്പോൾ (pos < ctx->item.എണ്ണം) {
		ഘടന flex_വരി_data *വരി;

		വരി = layout_flex_build_വരി(ctx, pos);
		എങ്കിൽ (വരി == ശൂന്യ) {
			തിരികെ false;
		}

		pos += വരി->എണ്ണം;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: "
				"fitted: %zu (total: %zu/%zu)",
				ctx->flex, വരി->എണ്ണം,
				pos, ctx->item.എണ്ണം);

		എങ്കിൽ (!layout_flex_resolve_വരി(ctx, വരി)) {
			തിരികെ false;
		}

		എങ്കിൽ (!layout_flex_place_വരി_ഇനങ്ങൾ_main(ctx, വരി)) {
			തിരികെ false;
		}

		ctx->cross_size += വരി->cross_size;
		എങ്കിൽ (ctx->main_size < വരി->main_size) {
			ctx->main_size = വരി->main_size;
		}
	}

	തിരികെ true;
}

/**
 * Align items on a line.
 *
 * \param[in] ctx    Flex layout context
 * \param[in] line   Line to align items on
 * \param[in] extra  Extra line width in pixels
 */
സ്ഥിതം ശൂന്യം layout_flex_place_വരി_ഇനങ്ങൾ_cross(ഘടന flex_ctx *ctx,
		ഘടന flex_വരി_data *വരി, പൂർണ്ണസംഖ്യ extra)
{
	എണ്ണൽ box_side cross_start = ctx->horizontal ? മുകൾ_2 : ഇടത്_2;
	size_t item_എണ്ണം = വരി->first + വരി->എണ്ണം;

	വേണ്ടി (size_t i = വരി->first; i < item_എണ്ണം; i++) {
		ഘടന flex_item_data *item = &ctx->item.data[i];
		ഘടന box *b = item->box;
		പൂർണ്ണസംഖ്യ cross_free_space;
		പൂർണ്ണസംഖ്യ *box_size_cross;
		പൂർണ്ണസംഖ്യ *box_pos_cross;

		box_pos_cross = ctx->horizontal ? &b->y : &b->x;
		box_size_cross = lh__box_size_cross_ptr(ctx->horizontal, b);

		cross_free_space = വരി->cross_size + extra - *box_size_cross -
				lh__delta_outer_cross(ctx->flex, b);

		തിരഞ്ഞെടുപ്പ് (lh__box_align_self(ctx->flex, b)) {
		സ്ഥിരസ്ഥിതി:
		അവസ്ഥ CSS_ALIGN_SELF_STRETCH:
			എങ്കിൽ (lh__box_size_cross_is_auto(ctx->horizontal, b)) {
				*box_size_cross += cross_free_space;

				/* Relayout children for stretch. */
				എങ്കിൽ (!layout_flex_item(ctx, item, b->width)) {
					തിരികെ;
				}
			}
			fallthrough;
		അവസ്ഥ CSS_ALIGN_SELF_FLEX_START:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					വരി->pos +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			നിർത്തുക;

		അവസ്ഥ CSS_ALIGN_SELF_FLEX_END:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					വരി->pos + cross_free_space +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			നിർത്തുക;

		അവസ്ഥ CSS_ALIGN_SELF_BASELINE:
		അവസ്ഥ CSS_ALIGN_SELF_CENTER:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					വരി->pos + cross_free_space / 2 +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			നിർത്തുക;
		}
	}
}

/**
 * Place the lines and align the items on the line.
 *
 * \param[in] ctx  Flex layout context
 */
സ്ഥിതം ശൂന്യം layout_flex_place_വരികൾ(ഘടന flex_ctx *ctx)
{
	bool reversed = ctx->wrap == CSS_FLEX_WRAP_WRAP_REVERSE;
	പൂർണ്ണസംഖ്യ വരി_pos = reversed ? ctx->cross_size : 0;
	പൂർണ്ണസംഖ്യ post_multiplier = reversed ? 0 : 1;
	പൂർണ്ണസംഖ്യ pre_multiplier = reversed ? -1 : 0;
	പൂർണ്ണസംഖ്യ extra_remainder = 0;
	പൂർണ്ണസംഖ്യ extra = 0;

	എങ്കിൽ (ctx->available_cross != AUTO &&
	    ctx->available_cross > ctx->cross_size &&
	    ctx->വരി.എണ്ണം > 0) {
		extra = ctx->available_cross - ctx->cross_size;

		extra_remainder = extra % ctx->വരി.എണ്ണം;
		extra /= ctx->വരി.എണ്ണം;
	}

	വേണ്ടി (size_t i = 0; i < ctx->വരി.എണ്ണം; i++) {
		ഘടന flex_വരി_data *വരി = &ctx->വരി.data[i];

		വരി_pos += pre_multiplier * വരി->cross_size;
		വരി->pos = വരി_pos;
		വരി_pos += post_multiplier * വരി->cross_size +
				extra + extra_remainder;

		layout_flex_place_വരി_ഇനങ്ങൾ_cross(ctx, വരി,
				extra + extra_remainder);

		എങ്കിൽ (extra_remainder > 0) {
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
bool layout_flex(ഘടന box *flex, പൂർണ്ണസംഖ്യ available_width,
		html_content *content)
{
	പൂർണ്ണസംഖ്യ max_height, min_height;
	ഘടന flex_ctx *ctx;
	bool success = false;

	ctx = layout_flex_ctx__create(content, flex);
	എങ്കിൽ (ctx == ശൂന്യ) {
		തിരികെ false;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: %s, available_width %i, width: %i",
			flex, ctx->horizontal ? "horizontal" : "vertical",
			available_width, flex->width);

	layout_find_dimensions(
			ctx->unit_len_ctx, available_width, -1,
			flex, flex->style, ശൂന്യ, &flex->height,
			ശൂന്യ, ശൂന്യ, &max_height, &min_height,
			flex->margin, flex->padding, flex->border);

	available_width = min(available_width, flex->width);

	എങ്കിൽ (ctx->horizontal) {
		ctx->available_main = available_width;
		ctx->available_cross = ctx->flex->height;
	} അല്ലെങ്കിൽ {
		ctx->available_main = ctx->flex->height;
		ctx->available_cross = available_width;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: available_main: %i",
			flex, ctx->available_main);
	NSLOG(flex, DEEPDEBUG, "box %p: available_cross: %i",
			flex, ctx->available_cross);

	layout_flex_ctx__populate_item_data(ctx, flex, available_width);

	/* Place items onto lines. */
	success = layout_flex_collect_ഇനങ്ങൾ_into_വരികൾ(ctx);
	എങ്കിൽ (!success) {
		പോകുക cleanup;
	}

	layout_flex_place_വരികൾ(ctx);

	എങ്കിൽ (flex->height == AUTO) {
		flex->height = ctx->horizontal ?
				ctx->cross_size :
				ctx->main_size;
	}

	എങ്കിൽ (flex->height != AUTO) {
		എങ്കിൽ (max_height >= 0 && flex->height > max_height) {
			flex->height = max_height;
		}
		എങ്കിൽ (min_height >  0 && flex->height < min_height) {
			flex->height = min_height;
		}
	}

	success = true;

cleanup:
	layout_flex_ctx__destroy(ctx);

	NSLOG(flex, DEEPDEBUG, "box %p: %s: w: %i, h: %i", flex,
			success ? "success" : "failure",
			flex->width, flex->height);
	തിരികെ success;
}
