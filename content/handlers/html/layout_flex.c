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

#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/utils.h"

#અન્તર્ભાવ "html/box.h"
#અન્તર્ભાવ "html/html.h"
#અન્તર્ભાવ "html/private.h"
#અન્તર્ભાવ "html/box_inspect.h"
#અન્તર્ભાવ "html/layout_internal.h"

/**
 * Flex item data
 */
સંરચના flex_item_data {
	ગણના css_flex_basis_e basis;
	css_fixed basis_length;
	css_unit basis_unit;
	સંરચના box *box;

	css_fixed shrink;
	css_fixed grow;

	પૂર્ણાંક min_main;
	પૂર્ણાંક max_main;
	પૂર્ણાંક min_cross;
	પૂર્ણાંક max_cross;

	પૂર્ણાંક target_main_size;
	પૂર્ણાંક base_size;
	પૂર્ણાંક main_size;
	size_t લીટી;

	bool freeze;
	bool min_violation;
	bool max_violation;
};

/**
 * Flex line data
 */
સંરચના flex_લીટી_data {
	પૂર્ણાંક main_size;
	પૂર્ણાંક cross_size;

	પૂર્ણાંક used_main_size;
	પૂર્ણાંક main_auto_margin_ગણતરી;

	પૂર્ણાંક pos;

	size_t first;
	size_t ગણતરી;
	size_t frozen;
};

/**
 * Flex layout context
 */
સંરચના flex_ctx {
	html_content *content;
	અચલ સંરચના box *flex;
	અચલ css_unit_ctx *unit_len_ctx;

	પૂર્ણાંક main_size;
	પૂર્ણાંક cross_size;

	પૂર્ણાંક available_main;
	પૂર્ણાંક available_cross;

	bool horizontal;
	bool main_reversed;
	ગણના css_flex_wrap_e wrap;

	સંરચના flex_વસ્તુઓ {
		size_t ગણતરી;
		સંરચના flex_item_data *data;
	} item;

	સંરચના flex_લીટીઓ {
		size_t ગણતરી;
		size_t alloc;
		સંરચના flex_લીટી_data *data;
	} લીટી;
};

/**
 * Destroy a flex layout context
 *
 * \param[in] ctx  Flex layout context
 */
સ્થિર શૂન્ય layout_flex_ctx__destroy(સંરચના flex_ctx *ctx)
{
	જો (ctx != ખાલી) {
		મુક્ત(ctx->item.data);
		મુક્ત(ctx->લીટી.data);
		મુક્ત(ctx);
	}
}

/**
 * Create a flex layout context
 *
 * \param[in] content  HTML content containing flex box
 * \param[in] flex     Box to create layout context for
 * \return flex layout context or NULL on error
 */
સ્થિર સંરચના flex_ctx *layout_flex_ctx__create(
		html_content *content,
		અચલ સંરચના box *flex)
{
	સંરચના flex_ctx *ctx;

	ctx = calloc(1, કદ(*ctx));
	જો (ctx == ખાલી) {
		પાછા ખાલી;
	}
	ctx->લીટી.alloc = 1;

	ctx->item.ગણતરી = box_ગણતરી_children(flex);
	ctx->item.data = calloc(ctx->item.ગણતરી, કદ(*ctx->item.data));
	જો (ctx->item.data == ખાલી) {
		layout_flex_ctx__destroy(ctx);
		પાછા ખાલી;
	}

	ctx->લીટી.alloc = 1;
	ctx->લીટી.data = calloc(ctx->લીટી.alloc, કદ(*ctx->લીટી.data));
	જો (ctx->લીટી.data == ખાલી) {
		layout_flex_ctx__destroy(ctx);
		પાછા ખાલી;
	}

	ctx->flex = flex;
	ctx->content = content;
	ctx->unit_len_ctx = &content->unit_len_ctx;

	ctx->wrap = css_computed_flex_wrap(flex->style);
	ctx->horizontal = lh__flex_main_is_horizontal(flex);
	ctx->main_reversed = lh__flex_direction_reversed(flex);

	પાછા ctx;
}

/**
 * Find box side representing the start of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the start side.
 */
સ્થિર ગણના box_side layout_flex__main_start_side(
		અચલ સંરચના flex_ctx *ctx)
{
	જો (ctx->horizontal) {
		પાછા (ctx->main_reversed) ? જમણું_2 : ડાબું_2;
	} નહીંતર {
		પાછા (ctx->main_reversed) ? BOTTOM : ટોચ_2;
	}
}

/**
 * Find box side representing the end of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the end side.
 */
સ્થિર ગણના box_side layout_flex__main_end_side(
		અચલ સંરચના flex_ctx *ctx)
{
	જો (ctx->horizontal) {
		પાછા (ctx->main_reversed) ? ડાબું_2 : જમણું_2;
	} નહીંતર {
		પાછા (ctx->main_reversed) ? ટોચ_2 : BOTTOM;
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
સ્થિર bool layout_flex_item(
		અચલ સંરચના flex_ctx *ctx,
		અચલ સંરચના flex_item_data *item,
		પૂર્ણાંક available_width)
{
	bool success;
	સંરચના box *b = item->box;

	પસંદગી (b->type) {
	સ્થિતિ BOX_BLOCK:
		success = layout_block_context(b, -1, ctx->content);
		તોડો;
	સ્થિતિ BOX_TABLE:
		b->float_container = b->parent;
		success = layout_table(b, available_width, ctx->content);
		b->float_container = ખાલી;
		તોડો;
	સ્થિતિ BOX_FLEX:
		b->float_container = b->parent;
		success = layout_flex(b, available_width, ctx->content);
		b->float_container = ખાલી;
		તોડો;
	મૂળભૂત:
		પ્રતિજ્ઞા(0 && "Bad flex item back type");
		success = false;
		તોડો;
	}

	જો (!success) {
		NSLOG(flex, ERROR, "box %p: layout failed", b);
	}

	પાછા success;
}

/**
 * Calculate an item's base and target main sizes.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] item             Item to get sizes of
 * \param[in] available_width  Available width in pixels
 * \return true on success false on failure
 */
સ્થિર અંતઃરેખ bool layout_flex__base_and_main_sizes(
		અચલ સંરચના flex_ctx *ctx,
		સંરચના flex_item_data *item,
		પૂર્ણાંક available_width)
{
	સંરચના box *b = item->box;
	પૂર્ણાંક content_min_width = b->min_width;
	પૂર્ણાંક content_max_width = b->max_width;
	પૂર્ણાંક delta_outer_main = lh__delta_outer_main(ctx->flex, b);

	NSLOG(flex, DEEPDEBUG, "box %p: delta_outer_main: %i",
			b, delta_outer_main);

	જો (item->basis == CSS_FLEX_BASIS_SET) {
		જો (item->basis_unit == CSS_UNIT_PCT) {
			item->base_size = FPCT_OF_INT_TOINT(
					item->basis_length,
					available_width);
		} નહીંતર {
			item->base_size = FIXTOINT(css_unit_len2device_px(
					b->style, ctx->unit_len_ctx,
					item->basis_length,
					item->basis_unit));
		}

	} નહીંતર જો (item->basis == CSS_FLEX_BASIS_AUTO) {
		item->base_size = ctx->horizontal ? b->width : b->height;
	} નહીંતર {
		item->base_size = AUTO;
	}

	જો (ctx->horizontal == false) {
		જો (b->width == AUTO) {
			b->width = min(max(content_min_width, available_width),
					content_max_width);
			b->width -= lh__delta_outer_width(b);
		}

		જો (!layout_flex_item(ctx, item, b->width)) {
			પાછા false;
		}
	}

	જો (item->base_size == AUTO) {
		જો (ctx->horizontal == false) {
			item->base_size = b->height;
		} નહીંતર {
			item->base_size = content_max_width - delta_outer_main;
		}
	}

	item->base_size += delta_outer_main;

	જો (ctx->horizontal) {
		item->base_size = min(item->base_size, available_width);
		item->base_size = max(item->base_size, content_min_width);
	}

	item->target_main_size = item->base_size;
	item->main_size = item->base_size;

	જો (item->max_main > 0 &&
	    item->main_size > item->max_main + delta_outer_main) {
		item->main_size = item->max_main + delta_outer_main;
	}

	જો (item->main_size < item->min_main + delta_outer_main) {
		item->main_size = item->min_main + delta_outer_main;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: base_size: %i, main_size %i",
			b, item->base_size, item->main_size);

	પાછા true;
}

/**
 * Fill out all item's data in a flex container.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] flex             Flex box
 * \param[in] available_width  Available width in pixels
 */
સ્થિર શૂન્ય layout_flex_ctx__populate_item_data(
		અચલ સંરચના flex_ctx *ctx,
		અચલ સંરચના box *flex,
		પૂર્ણાંક available_width)
{
	size_t i = 0;
	bool horizontal = ctx->horizontal;

	માટે (સંરચના box *b = flex->children; b != ખાલી; b = b->next) {
		સંરચના flex_item_data *item = &ctx->item.data[i++];

		b->float_container = b->parent;
		layout_find_dimensions(ctx->unit_len_ctx, available_width, -1,
				b, b->style, &b->width, &b->height,
				horizontal ? &item->max_main : &item->max_cross,
				horizontal ? &item->min_main : &item->min_cross,
				horizontal ? &item->max_cross : &item->max_main,
				horizontal ? &item->min_cross : &item->min_main,
				b->margin, b->padding, b->border);
		b->float_container = ખાલી;

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
સ્થિર bool layout_flex_ctx_ensure_લીટી(સંરચના flex_ctx *ctx)
{
	સંરચના flex_લીટી_data *temp;
	size_t લીટી_alloc = ctx->લીટી.alloc * 2;

	જો (ctx->લીટી.alloc > ctx->લીટી.ગણતરી) {
		પાછા true;
	}

	temp = realloc(ctx->લીટી.data, કદ(*ctx->લીટી.data) * લીટી_alloc);
	જો (temp == ખાલી) {
		પાછા false;
	}
	ctx->લીટી.data = temp;

	memset(ctx->લીટી.data + ctx->લીટી.alloc, 0,
	       કદ(*ctx->લીટી.data) * (લીટી_alloc - ctx->લીટી.alloc));
	ctx->લીટી.alloc = લીટી_alloc;

	પાછા true;
}

/**
 * Assigns flex items to the line and returns the line
 *
 * \param[in] ctx         Flex layout context
 * \param[in] item_index  Index to first item to assign to this line
 * \return Pointer to the new line, or NULL on error.
 */
સ્થિર સંરચના flex_લીટી_data *layout_flex_build_લીટી(સંરચના flex_ctx *ctx,
		size_t item_index)
{
	ગણના box_side start_side = layout_flex__main_start_side(ctx);
	ગણના box_side end_side = layout_flex__main_end_side(ctx);
	સંરચના flex_લીટી_data *લીટી;
	પૂર્ણાંક used_main = 0;

	જો (!layout_flex_ctx_ensure_લીટી(ctx)) {
		પાછા ખાલી;
	}

	લીટી = &ctx->લીટી.data[ctx->લીટી.ગણતરી];
	લીટી->first = item_index;

	NSLOG(flex, DEEPDEBUG, "flex container %p: available main: %i",
			ctx->flex, ctx->available_main);

	જ્યારે (item_index < ctx->item.ગણતરી) {
		સંરચના flex_item_data *item = &ctx->item.data[item_index];
		સંરચના box *b = item->box;
		પૂર્ણાંક pos_main;

		pos_main = ctx->horizontal ?
				item->main_size :
				b->height + lh__delta_outer_main(ctx->flex, b);

		જો (ctx->wrap == CSS_FLEX_WRAP_NOWRAP ||
		    pos_main + used_main <= ctx->available_main ||
		    lh__box_is_absolute(item->box) ||
		    ctx->available_main == AUTO ||
		    લીટી->ગણતરી == 0 ||
		    pos_main == 0) {
			જો (lh__box_is_absolute(item->box) == false) {
				લીટી->main_size += item->main_size;
				used_main += pos_main;

				જો (b->margin[start_side] == AUTO) {
					લીટી->main_auto_margin_ગણતરી++;
				}
				જો (b->margin[end_side] == AUTO) {
					લીટી->main_auto_margin_ગણતરી++;
				}
			}
			item->લીટી = ctx->લીટી.ગણતરી;
			લીટી->ગણતરી++;
			item_index++;
		} નહીંતર {
			તોડો;
		}
	}

	જો (લીટી->ગણતરી > 0) {
		ctx->લીટી.ગણતરી++;
	} નહીંતર {
		NSLOG(layout, ERROR, "Failed to fit any flex items");
	}

	પાછા લીટી;
}

/**
 * Freeze an item on a line
 *
 * \param[in] line  Line to containing item
 * \param[in] item  Item to freeze
 */
સ્થિર અંતઃરેખ શૂન્ય layout_flex__item_freeze(
		સંરચના flex_લીટી_data *લીટી,
		સંરચના flex_item_data *item)
{
	item->freeze = true;
	લીટી->frozen++;

	જો (!lh__box_is_absolute(item->box)){
		લીટી->used_main_size += item->target_main_size;
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
સ્થિર અંતઃરેખ પૂર્ણાંક layout_flex__remaining_free_main(
		સંરચના flex_ctx *ctx,
		સંરચના flex_લીટી_data *લીટી,
		css_fixed *unfrozen_factor_sum,
		પૂર્ણાંક initial_free_main,
		પૂર્ણાંક available_main,
		bool grow)
{
	પૂર્ણાંક remaining_free_main = available_main;
	size_t item_ગણતરી = લીટી->first + લીટી->ગણતરી;

	*unfrozen_factor_sum = 0;

	માટે (size_t i = લીટી->first; i < item_ગણતરી; i++) {
		સંરચના flex_item_data *item = &ctx->item.data[i];

		જો (item->freeze) {
			remaining_free_main -= item->target_main_size;
		} નહીંતર {
			remaining_free_main -= item->base_size;

			*unfrozen_factor_sum += grow ?
					item->grow : item->shrink;
		}
	}

	જો (*unfrozen_factor_sum < F_1) {
		પૂર્ણાંક free_space = FIXTOINT(FMUL(INTTOFIX(initial_free_main),
				*unfrozen_factor_sum));

		જો (free_space < remaining_free_main) {
			remaining_free_main = free_space;
		}
	}

	NSLOG(flex, DEEPDEBUG, "Remaining free space: %i",
			remaining_free_main);

	પાછા remaining_free_main;
}

/**
 * Clamp flex item target main size and get min/max violations
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to align items on
 * return total violation in pixels
 */
સ્થિર અંતઃરેખ પૂર્ણાંક layout_flex__get_min_max_violations(
		સંરચના flex_ctx *ctx,
		સંરચના flex_લીટી_data *લીટી)
{

	પૂર્ણાંક total_violation = 0;
	size_t item_ગણતરી = લીટી->first + લીટી->ગણતરી;

	માટે (size_t i = લીટી->first; i < item_ગણતરી; i++) {
		સંરચના flex_item_data *item = &ctx->item.data[i];
		પૂર્ણાંક target_main_size = item->target_main_size;

		NSLOG(flex, DEEPDEBUG, "item %p: target_main_size: %i",
					item->box, target_main_size);

		જો (item->freeze) {
			ચાલુ;
		}

		જો (item->max_main > 0 &&
		    target_main_size > item->max_main) {
			target_main_size = item->max_main;
			item->max_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: max_main: %i",
					item->max_main);
		}

		જો (target_main_size < item->min_main) {
			target_main_size = item->min_main;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: min_main: %i",
					item->min_main);
		}

		જો (target_main_size < item->box->min_width) {
			target_main_size = item->box->min_width;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: box min_width: %i",
					item->box->min_width);
		}

		જો (target_main_size < 0) {
			target_main_size = 0;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: less than 0");
		}

		total_violation += target_main_size - item->target_main_size;
		item->target_main_size = target_main_size;
	}

	NSLOG(flex, DEEPDEBUG, "Total violation: %i", total_violation);

	પાછા total_violation;
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
સ્થિર અંતઃરેખ શૂન્ય layout_flex__distribute_free_main(
		સંરચના flex_ctx *ctx,
		સંરચના flex_લીટી_data *લીટી,
		css_fixed unfrozen_factor_sum,
		પૂર્ણાંક remaining_free_main,
		bool grow)
{
	size_t item_ગણતરી = લીટી->first + લીટી->ગણતરી;

	જો (grow) {
		css_fixed remainder = 0;
		માટે (size_t i = લીટી->first; i < item_ગણતરી; i++) {
			સંરચના flex_item_data *item = &ctx->item.data[i];
			css_fixed result;
			css_fixed ratio;

			જો (item->freeze) {
				ચાલુ;
			}

			ratio = FDIV(item->grow, unfrozen_factor_sum);
			result = FMUL(INTTOFIX(remaining_free_main), ratio) +
					remainder;

			item->target_main_size = item->base_size +
					FIXTOINT(result);
			remainder = FIXFRAC(result);
		}
	} નહીંતર {
		css_fixed scaled_shrink_factor_sum = 0;
		css_fixed remainder = 0;

		માટે (size_t i = લીટી->first; i < item_ગણતરી; i++) {
			સંરચના flex_item_data *item = &ctx->item.data[i];
			css_fixed scaled_shrink_factor;

			જો (item->freeze) {
				ચાલુ;
			}

			scaled_shrink_factor = FMUL(
					item->shrink,
					INTTOFIX(item->base_size));
			scaled_shrink_factor_sum += scaled_shrink_factor;
		}

		માટે (size_t i = લીટી->first; i < item_ગણતરી; i++) {
			સંરચના flex_item_data *item = &ctx->item.data[i];
			css_fixed scaled_shrink_factor;
			css_fixed result;
			css_fixed ratio;

			જો (item->freeze) {
				ચાલુ;
			} નહીંતર જો (scaled_shrink_factor_sum == 0) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(લીટી, item);
				ચાલુ;
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
સ્થિર bool layout_flex_resolve_લીટી(
		સંરચના flex_ctx *ctx,
		સંરચના flex_લીટી_data *લીટી)
{
	size_t item_ગણતરી = લીટી->first + લીટી->ગણતરી;
	પૂર્ણાંક available_main = ctx->available_main;
	પૂર્ણાંક initial_free_main;
	bool grow;

	જો (available_main == AUTO) {
		available_main = INT_MAX;
	}

	grow = (લીટી->main_size < available_main);
	initial_free_main = available_main;

	NSLOG(flex, DEEPDEBUG, "box %p: line %zu: first: %zu, count: %zu",
			ctx->flex, લીટી - ctx->લીટી.data,
			લીટી->first, લીટી->ગણતરી);
	NSLOG(flex, DEEPDEBUG, "Line main_size: %i, available_main: %i",
			લીટી->main_size, available_main);

	માટે (size_t i = લીટી->first; i < item_ગણતરી; i++) {
		સંરચના flex_item_data *item = &ctx->item.data[i];

		/* 3. Size inflexible items */
		જો (grow) {
			જો (item->grow == 0 ||
			    item->base_size > item->main_size) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(લીટી, item);
			}
		} નહીંતર {
			જો (item->shrink == 0 ||
			    item->base_size < item->main_size) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(લીટી, item);
			}
		}

		/* 4. Calculate initial free space */
		જો (item->freeze) {
			initial_free_main -= item->target_main_size;
		} નહીંતર {
			initial_free_main -= item->base_size;
		}
	}

	/* 5. Loop */
	જ્યારે (લીટી->frozen < લીટી->ગણતરી) {
		css_fixed unfrozen_factor_sum;
		પૂર્ણાંક remaining_free_main;
		પૂર્ણાંક total_violation;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: Resolver pass",
				ctx->flex);

		/* b */
		remaining_free_main = layout_flex__remaining_free_main(ctx,
				લીટી, &unfrozen_factor_sum, initial_free_main,
				available_main, grow);

		/* c */
		જો (remaining_free_main != 0) {
			layout_flex__distribute_free_main(ctx,
					લીટી, unfrozen_factor_sum,
					remaining_free_main, grow);
		}

		/* d */
		total_violation = layout_flex__get_min_max_violations(
				ctx, લીટી);

		/* e */
		માટે (size_t i = લીટી->first; i < item_ગણતરી; i++) {
			સંરચના flex_item_data *item = &ctx->item.data[i];

			જો (item->freeze) {
				ચાલુ;
			}

			જો (total_violation == 0 ||
			    (total_violation > 0 && item->min_violation) ||
			    (total_violation < 0 && item->max_violation)) {
				layout_flex__item_freeze(લીટી, item);
			}
		}
	}

	પાછા true;
}

/**
 * Position items along a line
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to resolve
 * \return true on success, false on failure.
 */
સ્થિર bool layout_flex_place_લીટી_વસ્તુઓ_main(
		સંરચના flex_ctx *ctx,
		સંરચના flex_લીટી_data *લીટી)
{
	પૂર્ણાંક main_pos = ctx->flex->padding[layout_flex__main_start_side(ctx)];
	પૂર્ણાંક post_multiplier = ctx->main_reversed ? 0 : 1;
	પૂર્ણાંક pre_multiplier = ctx->main_reversed ? -1 : 0;
	size_t item_ગણતરી = લીટી->first + લીટી->ગણતરી;
	પૂર્ણાંક extra_remainder = 0;
	પૂર્ણાંક extra = 0;

	જો (ctx->main_reversed) {
		main_pos = lh__box_size_main(ctx->horizontal, ctx->flex) -
				main_pos;
	}

	જો (ctx->available_main != AUTO &&
	    ctx->available_main != UNKNOWN_WIDTH &&
	    ctx->available_main > લીટી->used_main_size) {
		જો (લીટી->main_auto_margin_ગણતરી > 0) {
			extra = ctx->available_main - લીટી->used_main_size;

			extra_remainder = extra % લીટી->main_auto_margin_ગણતરી;
			extra /= લીટી->main_auto_margin_ગણતરી;
		}
	}

	માટે (size_t i = લીટી->first; i < item_ગણતરી; i++) {
		ગણના box_side main_end = ctx->horizontal ? જમણું_2 : BOTTOM;
		ગણના box_side main_start = ctx->horizontal ? ડાબું_2 : ટોચ_2;
		સંરચના flex_item_data *item = &ctx->item.data[i];
		સંરચના box *b = item->box;
		પૂર્ણાંક extra_total = 0;
		પૂર્ણાંક extra_post = 0;
		પૂર્ણાંક extra_pre = 0;
		પૂર્ણાંક box_size_main;
		પૂર્ણાંક *box_pos_main;

		જો (ctx->horizontal) {
			b->width = item->target_main_size -
					lh__delta_outer_width(b);

			જો (!layout_flex_item(ctx, item, b->width)) {
				પાછા false;
			}
		}

		box_size_main = lh__box_size_main(ctx->horizontal, b);
		box_pos_main = ctx->horizontal ? &b->x : &b->y;

		જો (!lh__box_is_absolute(b)) {
			જો (b->margin[main_start] == AUTO) {
				extra_pre = extra + extra_remainder;
			}
			જો (b->margin[main_end] == AUTO) {
				extra_post = extra + extra_remainder;
			}
			extra_total = extra_pre + extra_post;

			main_pos += pre_multiplier *
					(extra_total + box_size_main +
					 lh__delta_outer_main(ctx->flex, b));
		}

		*box_pos_main = main_pos + lh__non_auto_margin(b, main_start) +
				extra_pre + b->border[main_start].width;

		જો (!lh__box_is_absolute(b)) {
			પૂર્ણાંક cross_size;
			પૂર્ણાંક box_size_cross = lh__box_size_cross(
					ctx->horizontal, b);

			main_pos += post_multiplier *
					(extra_total + box_size_main +
					 lh__delta_outer_main(ctx->flex, b));

			cross_size = box_size_cross + lh__delta_outer_cross(
					ctx->flex, b);
			જો (લીટી->cross_size < cross_size) {
				લીટી->cross_size = cross_size;
			}
		}
	}

	પાછા true;
}

/**
 * Collect items onto lines and place items along the lines
 *
 * \param[in] ctx   Flex layout context
 * \return true on success, false on failure.
 */
સ્થિર bool layout_flex_collect_વસ્તુઓ_into_લીટીઓ(
		સંરચના flex_ctx *ctx)
{
	size_t pos = 0;

	જ્યારે (pos < ctx->item.ગણતરી) {
		સંરચના flex_લીટી_data *લીટી;

		લીટી = layout_flex_build_લીટી(ctx, pos);
		જો (લીટી == ખાલી) {
			પાછા false;
		}

		pos += લીટી->ગણતરી;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: "
				"fitted: %zu (total: %zu/%zu)",
				ctx->flex, લીટી->ગણતરી,
				pos, ctx->item.ગણતરી);

		જો (!layout_flex_resolve_લીટી(ctx, લીટી)) {
			પાછા false;
		}

		જો (!layout_flex_place_લીટી_વસ્તુઓ_main(ctx, લીટી)) {
			પાછા false;
		}

		ctx->cross_size += લીટી->cross_size;
		જો (ctx->main_size < લીટી->main_size) {
			ctx->main_size = લીટી->main_size;
		}
	}

	પાછા true;
}

/**
 * Align items on a line.
 *
 * \param[in] ctx    Flex layout context
 * \param[in] line   Line to align items on
 * \param[in] extra  Extra line width in pixels
 */
સ્થિર શૂન્ય layout_flex_place_લીટી_વસ્તુઓ_cross(સંરચના flex_ctx *ctx,
		સંરચના flex_લીટી_data *લીટી, પૂર્ણાંક extra)
{
	ગણના box_side cross_start = ctx->horizontal ? ટોચ_2 : ડાબું_2;
	size_t item_ગણતરી = લીટી->first + લીટી->ગણતરી;

	માટે (size_t i = લીટી->first; i < item_ગણતરી; i++) {
		સંરચના flex_item_data *item = &ctx->item.data[i];
		સંરચના box *b = item->box;
		પૂર્ણાંક cross_free_space;
		પૂર્ણાંક *box_size_cross;
		પૂર્ણાંક *box_pos_cross;

		box_pos_cross = ctx->horizontal ? &b->y : &b->x;
		box_size_cross = lh__box_size_cross_ptr(ctx->horizontal, b);

		cross_free_space = લીટી->cross_size + extra - *box_size_cross -
				lh__delta_outer_cross(ctx->flex, b);

		પસંદગી (lh__box_align_self(ctx->flex, b)) {
		મૂળભૂત:
		સ્થિતિ CSS_ALIGN_SELF_STRETCH:
			જો (lh__box_size_cross_is_auto(ctx->horizontal, b)) {
				*box_size_cross += cross_free_space;

				/* Relayout children for stretch. */
				જો (!layout_flex_item(ctx, item, b->width)) {
					પાછા;
				}
			}
			fallthrough;
		સ્થિતિ CSS_ALIGN_SELF_FLEX_START:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					લીટી->pos +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			તોડો;

		સ્થિતિ CSS_ALIGN_SELF_FLEX_END:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					લીટી->pos + cross_free_space +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			તોડો;

		સ્થિતિ CSS_ALIGN_SELF_BASELINE:
		સ્થિતિ CSS_ALIGN_SELF_CENTER:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					લીટી->pos + cross_free_space / 2 +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			તોડો;
		}
	}
}

/**
 * Place the lines and align the items on the line.
 *
 * \param[in] ctx  Flex layout context
 */
સ્થિર શૂન્ય layout_flex_place_લીટીઓ(સંરચના flex_ctx *ctx)
{
	bool reversed = ctx->wrap == CSS_FLEX_WRAP_WRAP_REVERSE;
	પૂર્ણાંક લીટી_pos = reversed ? ctx->cross_size : 0;
	પૂર્ણાંક post_multiplier = reversed ? 0 : 1;
	પૂર્ણાંક pre_multiplier = reversed ? -1 : 0;
	પૂર્ણાંક extra_remainder = 0;
	પૂર્ણાંક extra = 0;

	જો (ctx->available_cross != AUTO &&
	    ctx->available_cross > ctx->cross_size &&
	    ctx->લીટી.ગણતરી > 0) {
		extra = ctx->available_cross - ctx->cross_size;

		extra_remainder = extra % ctx->લીટી.ગણતરી;
		extra /= ctx->લીટી.ગણતરી;
	}

	માટે (size_t i = 0; i < ctx->લીટી.ગણતરી; i++) {
		સંરચના flex_લીટી_data *લીટી = &ctx->લીટી.data[i];

		લીટી_pos += pre_multiplier * લીટી->cross_size;
		લીટી->pos = લીટી_pos;
		લીટી_pos += post_multiplier * લીટી->cross_size +
				extra + extra_remainder;

		layout_flex_place_લીટી_વસ્તુઓ_cross(ctx, લીટી,
				extra + extra_remainder);

		જો (extra_remainder > 0) {
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
bool layout_flex(સંરચના box *flex, પૂર્ણાંક available_width,
		html_content *content)
{
	પૂર્ણાંક max_height, min_height;
	સંરચના flex_ctx *ctx;
	bool success = false;

	ctx = layout_flex_ctx__create(content, flex);
	જો (ctx == ખાલી) {
		પાછા false;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: %s, available_width %i, width: %i",
			flex, ctx->horizontal ? "horizontal" : "vertical",
			available_width, flex->width);

	layout_find_dimensions(
			ctx->unit_len_ctx, available_width, -1,
			flex, flex->style, ખાલી, &flex->height,
			ખાલી, ખાલી, &max_height, &min_height,
			flex->margin, flex->padding, flex->border);

	available_width = min(available_width, flex->width);

	જો (ctx->horizontal) {
		ctx->available_main = available_width;
		ctx->available_cross = ctx->flex->height;
	} નહીંતર {
		ctx->available_main = ctx->flex->height;
		ctx->available_cross = available_width;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: available_main: %i",
			flex, ctx->available_main);
	NSLOG(flex, DEEPDEBUG, "box %p: available_cross: %i",
			flex, ctx->available_cross);

	layout_flex_ctx__populate_item_data(ctx, flex, available_width);

	/* Place items onto lines. */
	success = layout_flex_collect_વસ્તુઓ_into_લીટીઓ(ctx);
	જો (!success) {
		જાઓ cleanup;
	}

	layout_flex_place_લીટીઓ(ctx);

	જો (flex->height == AUTO) {
		flex->height = ctx->horizontal ?
				ctx->cross_size :
				ctx->main_size;
	}

	જો (flex->height != AUTO) {
		જો (max_height >= 0 && flex->height > max_height) {
			flex->height = max_height;
		}
		જો (min_height >  0 && flex->height < min_height) {
			flex->height = min_height;
		}
	}

	success = true;

cleanup:
	layout_flex_ctx__destroy(ctx);

	NSLOG(flex, DEEPDEBUG, "box %p: %s: w: %i, h: %i", flex,
			success ? "success" : "failure",
			flex->width, flex->height);
	પાછા success;
}
