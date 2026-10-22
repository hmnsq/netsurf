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

#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"

#ଅନ୍ତର୍ଭାଵ "html/box.h"
#ଅନ୍ତର୍ଭାଵ "html/html.h"
#ଅନ୍ତର୍ଭାଵ "html/private.h"
#ଅନ୍ତର୍ଭାଵ "html/box_inspect.h"
#ଅନ୍ତର୍ଭାଵ "html/layout_internal.h"

/**
 * Flex item data
 */
ସଂରଚନା flex_item_data {
	ଗଣନା css_flex_basis_e basis;
	css_fixed basis_length;
	css_unit basis_unit;
	ସଂରଚନା box *box;

	css_fixed shrink;
	css_fixed grow;

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା min_main;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_main;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା min_cross;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_cross;

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା target_main_size;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା base_size;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା main_size;
	size_t ଧାଡ଼ି;

	bool freeze;
	bool min_violation;
	bool max_violation;
};

/**
 * Flex line data
 */
ସଂରଚନା flex_ଧାଡ଼ି_data {
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା main_size;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା cross_size;

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା used_main_size;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା main_auto_margin_ଗଣନା;

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା pos;

	size_t first;
	size_t ଗଣନା_2;
	size_t frozen;
};

/**
 * Flex layout context
 */
ସଂରଚନା flex_ctx {
	html_content *content;
	ଅଚଳ ସଂରଚନା box *flex;
	ଅଚଳ css_unit_ctx *unit_len_ctx;

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା main_size;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା cross_size;

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା available_main;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା available_cross;

	bool horizontal;
	bool main_reversed;
	ଗଣନା css_flex_wrap_e wrap;

	ସଂରଚନା flex_ବସ୍ତୁଗୁଡ଼ିକ {
		size_t ଗଣନା_2;
		ସଂରଚନା flex_item_data *data;
	} item;

	ସଂରଚନା flex_ଧାଡ଼ିଗୁଡ଼ିକ {
		size_t ଗଣନା_2;
		size_t alloc;
		ସଂରଚନା flex_ଧାଡ଼ି_data *data;
	} ଧାଡ଼ି;
};

/**
 * Destroy a flex layout context
 *
 * \param[in] ctx  Flex layout context
 */
ସ୍ଥିର ଶୂନ୍ୟ layout_flex_ctx__destroy(ସଂରଚନା flex_ctx *ctx)
{
	ଯଦି (ctx != ଖାଲି) {
		ମୁକ୍ତ(ctx->item.data);
		ମୁକ୍ତ(ctx->ଧାଡ଼ି.data);
		ମୁକ୍ତ(ctx);
	}
}

/**
 * Create a flex layout context
 *
 * \param[in] content  HTML content containing flex box
 * \param[in] flex     Box to create layout context for
 * \return flex layout context or NULL on error
 */
ସ୍ଥିର ସଂରଚନା flex_ctx *layout_flex_ctx__create(
		html_content *content,
		ଅଚଳ ସଂରଚନା box *flex)
{
	ସଂରଚନା flex_ctx *ctx;

	ctx = calloc(1, ଆକାର(*ctx));
	ଯଦି (ctx == ଖାଲି) {
		ଫେରନ୍ତୁ ଖାଲି;
	}
	ctx->ଧାଡ଼ି.alloc = 1;

	ctx->item.ଗଣନା_2 = box_ଗଣନା_children(flex);
	ctx->item.data = calloc(ctx->item.ଗଣନା_2, ଆକାର(*ctx->item.data));
	ଯଦି (ctx->item.data == ଖାଲି) {
		layout_flex_ctx__destroy(ctx);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ctx->ଧାଡ଼ି.alloc = 1;
	ctx->ଧାଡ଼ି.data = calloc(ctx->ଧାଡ଼ି.alloc, ଆକାର(*ctx->ଧାଡ଼ି.data));
	ଯଦି (ctx->ଧାଡ଼ି.data == ଖାଲି) {
		layout_flex_ctx__destroy(ctx);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ctx->flex = flex;
	ctx->content = content;
	ctx->unit_len_ctx = &content->unit_len_ctx;

	ctx->wrap = css_computed_flex_wrap(flex->style);
	ctx->horizontal = lh__flex_main_is_horizontal(flex);
	ctx->main_reversed = lh__flex_direction_reversed(flex);

	ଫେରନ୍ତୁ ctx;
}

/**
 * Find box side representing the start of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the start side.
 */
ସ୍ଥିର ଗଣନା box_side layout_flex__main_start_side(
		ଅଚଳ ସଂରଚନା flex_ctx *ctx)
{
	ଯଦି (ctx->horizontal) {
		ଫେରନ୍ତୁ (ctx->main_reversed) ? ଡାହାଣ_2 : ବାମ_2;
	} ଅନ୍ୟଥା {
		ଫେରନ୍ତୁ (ctx->main_reversed) ? BOTTOM : ଶୀର୍ଷ_2;
	}
}

/**
 * Find box side representing the end of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the end side.
 */
ସ୍ଥିର ଗଣନା box_side layout_flex__main_end_side(
		ଅଚଳ ସଂରଚନା flex_ctx *ctx)
{
	ଯଦି (ctx->horizontal) {
		ଫେରନ୍ତୁ (ctx->main_reversed) ? ବାମ_2 : ଡାହାଣ_2;
	} ଅନ୍ୟଥା {
		ଫେରନ୍ତୁ (ctx->main_reversed) ? ଶୀର୍ଷ_2 : BOTTOM;
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
ସ୍ଥିର bool layout_flex_item(
		ଅଚଳ ସଂରଚନା flex_ctx *ctx,
		ଅଚଳ ସଂରଚନା flex_item_data *item,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା available_width)
{
	bool success;
	ସଂରଚନା box *b = item->box;

	ବାଛ (b->type) {
	ସ୍ଥିତି BOX_BLOCK:
		success = layout_block_context(b, -1, ctx->content);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି BOX_TABLE:
		b->float_container = b->parent;
		success = layout_table(b, available_width, ctx->content);
		b->float_container = ଖାଲି;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି BOX_FLEX:
		b->float_container = b->parent;
		success = layout_flex(b, available_width, ctx->content);
		b->float_container = ଖାଲି;
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ପ୍ରତିଜ୍ଞା(0 && "Bad flex item back type");
		success = false;
		ଭାଙ୍ଗ;
	}

	ଯଦି (!success) {
		NSLOG(flex, ERROR, "box %p: layout failed", b);
	}

	ଫେରନ୍ତୁ success;
}

/**
 * Calculate an item's base and target main sizes.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] item             Item to get sizes of
 * \param[in] available_width  Available width in pixels
 * \return true on success false on failure
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool layout_flex__base_and_main_sizes(
		ଅଚଳ ସଂରଚନା flex_ctx *ctx,
		ସଂରଚନା flex_item_data *item,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା available_width)
{
	ସଂରଚନା box *b = item->box;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା content_min_width = b->min_width;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା content_max_width = b->max_width;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା delta_outer_main = lh__delta_outer_main(ctx->flex, b);

	NSLOG(flex, DEEPDEBUG, "box %p: delta_outer_main: %i",
			b, delta_outer_main);

	ଯଦି (item->basis == CSS_FLEX_BASIS_SET) {
		ଯଦି (item->basis_unit == CSS_UNIT_PCT) {
			item->base_size = FPCT_OF_INT_TOINT(
					item->basis_length,
					available_width);
		} ଅନ୍ୟଥା {
			item->base_size = FIXTOINT(css_unit_len2device_px(
					b->style, ctx->unit_len_ctx,
					item->basis_length,
					item->basis_unit));
		}

	} ଅନ୍ୟଥା ଯଦି (item->basis == CSS_FLEX_BASIS_AUTO) {
		item->base_size = ctx->horizontal ? b->width : b->height;
	} ଅନ୍ୟଥା {
		item->base_size = AUTO;
	}

	ଯଦି (ctx->horizontal == false) {
		ଯଦି (b->width == AUTO) {
			b->width = min(max(content_min_width, available_width),
					content_max_width);
			b->width -= lh__delta_outer_width(b);
		}

		ଯଦି (!layout_flex_item(ctx, item, b->width)) {
			ଫେରନ୍ତୁ false;
		}
	}

	ଯଦି (item->base_size == AUTO) {
		ଯଦି (ctx->horizontal == false) {
			item->base_size = b->height;
		} ଅନ୍ୟଥା {
			item->base_size = content_max_width - delta_outer_main;
		}
	}

	item->base_size += delta_outer_main;

	ଯଦି (ctx->horizontal) {
		item->base_size = min(item->base_size, available_width);
		item->base_size = max(item->base_size, content_min_width);
	}

	item->target_main_size = item->base_size;
	item->main_size = item->base_size;

	ଯଦି (item->max_main > 0 &&
	    item->main_size > item->max_main + delta_outer_main) {
		item->main_size = item->max_main + delta_outer_main;
	}

	ଯଦି (item->main_size < item->min_main + delta_outer_main) {
		item->main_size = item->min_main + delta_outer_main;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: base_size: %i, main_size %i",
			b, item->base_size, item->main_size);

	ଫେରନ୍ତୁ true;
}

/**
 * Fill out all item's data in a flex container.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] flex             Flex box
 * \param[in] available_width  Available width in pixels
 */
ସ୍ଥିର ଶୂନ୍ୟ layout_flex_ctx__populate_item_data(
		ଅଚଳ ସଂରଚନା flex_ctx *ctx,
		ଅଚଳ ସଂରଚନା box *flex,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା available_width)
{
	size_t i = 0;
	bool horizontal = ctx->horizontal;

	ପାଇଁ (ସଂରଚନା box *b = flex->children; b != ଖାଲି; b = b->next) {
		ସଂରଚନା flex_item_data *item = &ctx->item.data[i++];

		b->float_container = b->parent;
		layout_find_dimensions(ctx->unit_len_ctx, available_width, -1,
				b, b->style, &b->width, &b->height,
				horizontal ? &item->max_main : &item->max_cross,
				horizontal ? &item->min_main : &item->min_cross,
				horizontal ? &item->max_cross : &item->max_main,
				horizontal ? &item->min_cross : &item->min_main,
				b->margin, b->padding, b->border);
		b->float_container = ଖାଲି;

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
ସ୍ଥିର bool layout_flex_ctx_ensure_ଧାଡ଼ି(ସଂରଚନା flex_ctx *ctx)
{
	ସଂରଚନା flex_ଧାଡ଼ି_data *temp;
	size_t ଧାଡ଼ି_alloc = ctx->ଧାଡ଼ି.alloc * 2;

	ଯଦି (ctx->ଧାଡ଼ି.alloc > ctx->ଧାଡ଼ି.ଗଣନା_2) {
		ଫେରନ୍ତୁ true;
	}

	temp = realloc(ctx->ଧାଡ଼ି.data, ଆକାର(*ctx->ଧାଡ଼ି.data) * ଧାଡ଼ି_alloc);
	ଯଦି (temp == ଖାଲି) {
		ଫେରନ୍ତୁ false;
	}
	ctx->ଧାଡ଼ି.data = temp;

	memset(ctx->ଧାଡ଼ି.data + ctx->ଧାଡ଼ି.alloc, 0,
	       ଆକାର(*ctx->ଧାଡ଼ି.data) * (ଧାଡ଼ି_alloc - ctx->ଧାଡ଼ି.alloc));
	ctx->ଧାଡ଼ି.alloc = ଧାଡ଼ି_alloc;

	ଫେରନ୍ତୁ true;
}

/**
 * Assigns flex items to the line and returns the line
 *
 * \param[in] ctx         Flex layout context
 * \param[in] item_index  Index to first item to assign to this line
 * \return Pointer to the new line, or NULL on error.
 */
ସ୍ଥିର ସଂରଚନା flex_ଧାଡ଼ି_data *layout_flex_build_ଧାଡ଼ି(ସଂରଚନା flex_ctx *ctx,
		size_t item_index)
{
	ଗଣନା box_side start_side = layout_flex__main_start_side(ctx);
	ଗଣନା box_side end_side = layout_flex__main_end_side(ctx);
	ସଂରଚନା flex_ଧାଡ଼ି_data *ଧାଡ଼ି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା used_main = 0;

	ଯଦି (!layout_flex_ctx_ensure_ଧାଡ଼ି(ctx)) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଧାଡ଼ି = &ctx->ଧାଡ଼ି.data[ctx->ଧାଡ଼ି.ଗଣନା_2];
	ଧାଡ଼ି->first = item_index;

	NSLOG(flex, DEEPDEBUG, "flex container %p: available main: %i",
			ctx->flex, ctx->available_main);

	ଯେତେବେଳେ (item_index < ctx->item.ଗଣନା_2) {
		ସଂରଚନା flex_item_data *item = &ctx->item.data[item_index];
		ସଂରଚନା box *b = item->box;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା pos_main;

		pos_main = ctx->horizontal ?
				item->main_size :
				b->height + lh__delta_outer_main(ctx->flex, b);

		ଯଦି (ctx->wrap == CSS_FLEX_WRAP_NOWRAP ||
		    pos_main + used_main <= ctx->available_main ||
		    lh__box_is_absolute(item->box) ||
		    ctx->available_main == AUTO ||
		    ଧାଡ଼ି->ଗଣନା_2 == 0 ||
		    pos_main == 0) {
			ଯଦି (lh__box_is_absolute(item->box) == false) {
				ଧାଡ଼ି->main_size += item->main_size;
				used_main += pos_main;

				ଯଦି (b->margin[start_side] == AUTO) {
					ଧାଡ଼ି->main_auto_margin_ଗଣନା++;
				}
				ଯଦି (b->margin[end_side] == AUTO) {
					ଧାଡ଼ି->main_auto_margin_ଗଣନା++;
				}
			}
			item->ଧାଡ଼ି = ctx->ଧାଡ଼ି.ଗଣନା_2;
			ଧାଡ଼ି->ଗଣନା_2++;
			item_index++;
		} ଅନ୍ୟଥା {
			ଭାଙ୍ଗ;
		}
	}

	ଯଦି (ଧାଡ଼ି->ଗଣନା_2 > 0) {
		ctx->ଧାଡ଼ି.ଗଣନା_2++;
	} ଅନ୍ୟଥା {
		NSLOG(layout, ERROR, "Failed to fit any flex items");
	}

	ଫେରନ୍ତୁ ଧାଡ଼ି;
}

/**
 * Freeze an item on a line
 *
 * \param[in] line  Line to containing item
 * \param[in] item  Item to freeze
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ layout_flex__item_freeze(
		ସଂରଚନା flex_ଧାଡ଼ି_data *ଧାଡ଼ି,
		ସଂରଚନା flex_item_data *item)
{
	item->freeze = true;
	ଧାଡ଼ି->frozen++;

	ଯଦି (!lh__box_is_absolute(item->box)){
		ଧାଡ଼ି->used_main_size += item->target_main_size;
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
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା layout_flex__remaining_free_main(
		ସଂରଚନା flex_ctx *ctx,
		ସଂରଚନା flex_ଧାଡ଼ି_data *ଧାଡ଼ି,
		css_fixed *unfrozen_factor_sum,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା initial_free_main,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା available_main,
		bool grow)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା remaining_free_main = available_main;
	size_t item_ଗଣନା = ଧାଡ଼ି->first + ଧାଡ଼ି->ଗଣନା_2;

	*unfrozen_factor_sum = 0;

	ପାଇଁ (size_t i = ଧାଡ଼ି->first; i < item_ଗଣନା; i++) {
		ସଂରଚନା flex_item_data *item = &ctx->item.data[i];

		ଯଦି (item->freeze) {
			remaining_free_main -= item->target_main_size;
		} ଅନ୍ୟଥା {
			remaining_free_main -= item->base_size;

			*unfrozen_factor_sum += grow ?
					item->grow : item->shrink;
		}
	}

	ଯଦି (*unfrozen_factor_sum < F_1) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା free_space = FIXTOINT(FMUL(INTTOFIX(initial_free_main),
				*unfrozen_factor_sum));

		ଯଦି (free_space < remaining_free_main) {
			remaining_free_main = free_space;
		}
	}

	NSLOG(flex, DEEPDEBUG, "Remaining free space: %i",
			remaining_free_main);

	ଫେରନ୍ତୁ remaining_free_main;
}

/**
 * Clamp flex item target main size and get min/max violations
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to align items on
 * return total violation in pixels
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା layout_flex__get_min_max_violations(
		ସଂରଚନା flex_ctx *ctx,
		ସଂରଚନା flex_ଧାଡ଼ି_data *ଧାଡ଼ି)
{

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା total_violation = 0;
	size_t item_ଗଣନା = ଧାଡ଼ି->first + ଧାଡ଼ି->ଗଣନା_2;

	ପାଇଁ (size_t i = ଧାଡ଼ି->first; i < item_ଗଣନା; i++) {
		ସଂରଚନା flex_item_data *item = &ctx->item.data[i];
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା target_main_size = item->target_main_size;

		NSLOG(flex, DEEPDEBUG, "item %p: target_main_size: %i",
					item->box, target_main_size);

		ଯଦି (item->freeze) {
			ଜାରି;
		}

		ଯଦି (item->max_main > 0 &&
		    target_main_size > item->max_main) {
			target_main_size = item->max_main;
			item->max_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: max_main: %i",
					item->max_main);
		}

		ଯଦି (target_main_size < item->min_main) {
			target_main_size = item->min_main;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: min_main: %i",
					item->min_main);
		}

		ଯଦି (target_main_size < item->box->min_width) {
			target_main_size = item->box->min_width;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: box min_width: %i",
					item->box->min_width);
		}

		ଯଦି (target_main_size < 0) {
			target_main_size = 0;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: less than 0");
		}

		total_violation += target_main_size - item->target_main_size;
		item->target_main_size = target_main_size;
	}

	NSLOG(flex, DEEPDEBUG, "Total violation: %i", total_violation);

	ଫେରନ୍ତୁ total_violation;
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
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ layout_flex__distribute_free_main(
		ସଂରଚନା flex_ctx *ctx,
		ସଂରଚନା flex_ଧାଡ଼ି_data *ଧାଡ଼ି,
		css_fixed unfrozen_factor_sum,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା remaining_free_main,
		bool grow)
{
	size_t item_ଗଣନା = ଧାଡ଼ି->first + ଧାଡ଼ି->ଗଣନା_2;

	ଯଦି (grow) {
		css_fixed remainder = 0;
		ପାଇଁ (size_t i = ଧାଡ଼ି->first; i < item_ଗଣନା; i++) {
			ସଂରଚନା flex_item_data *item = &ctx->item.data[i];
			css_fixed result;
			css_fixed ratio;

			ଯଦି (item->freeze) {
				ଜାରି;
			}

			ratio = FDIV(item->grow, unfrozen_factor_sum);
			result = FMUL(INTTOFIX(remaining_free_main), ratio) +
					remainder;

			item->target_main_size = item->base_size +
					FIXTOINT(result);
			remainder = FIXFRAC(result);
		}
	} ଅନ୍ୟଥା {
		css_fixed scaled_shrink_factor_sum = 0;
		css_fixed remainder = 0;

		ପାଇଁ (size_t i = ଧାଡ଼ି->first; i < item_ଗଣନା; i++) {
			ସଂରଚନା flex_item_data *item = &ctx->item.data[i];
			css_fixed scaled_shrink_factor;

			ଯଦି (item->freeze) {
				ଜାରି;
			}

			scaled_shrink_factor = FMUL(
					item->shrink,
					INTTOFIX(item->base_size));
			scaled_shrink_factor_sum += scaled_shrink_factor;
		}

		ପାଇଁ (size_t i = ଧାଡ଼ି->first; i < item_ଗଣନା; i++) {
			ସଂରଚନା flex_item_data *item = &ctx->item.data[i];
			css_fixed scaled_shrink_factor;
			css_fixed result;
			css_fixed ratio;

			ଯଦି (item->freeze) {
				ଜାରି;
			} ଅନ୍ୟଥା ଯଦି (scaled_shrink_factor_sum == 0) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(ଧାଡ଼ି, item);
				ଜାରି;
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
ସ୍ଥିର bool layout_flex_resolve_ଧାଡ଼ି(
		ସଂରଚନା flex_ctx *ctx,
		ସଂରଚନା flex_ଧାଡ଼ି_data *ଧାଡ଼ି)
{
	size_t item_ଗଣନା = ଧାଡ଼ି->first + ଧାଡ଼ି->ଗଣନା_2;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା available_main = ctx->available_main;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା initial_free_main;
	bool grow;

	ଯଦି (available_main == AUTO) {
		available_main = INT_MAX;
	}

	grow = (ଧାଡ଼ି->main_size < available_main);
	initial_free_main = available_main;

	NSLOG(flex, DEEPDEBUG, "box %p: line %zu: first: %zu, count: %zu",
			ctx->flex, ଧାଡ଼ି - ctx->ଧାଡ଼ି.data,
			ଧାଡ଼ି->first, ଧାଡ଼ି->ଗଣନା_2);
	NSLOG(flex, DEEPDEBUG, "Line main_size: %i, available_main: %i",
			ଧାଡ଼ି->main_size, available_main);

	ପାଇଁ (size_t i = ଧାଡ଼ି->first; i < item_ଗଣନା; i++) {
		ସଂରଚନା flex_item_data *item = &ctx->item.data[i];

		/* 3. Size inflexible items */
		ଯଦି (grow) {
			ଯଦି (item->grow == 0 ||
			    item->base_size > item->main_size) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(ଧାଡ଼ି, item);
			}
		} ଅନ୍ୟଥା {
			ଯଦି (item->shrink == 0 ||
			    item->base_size < item->main_size) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(ଧାଡ଼ି, item);
			}
		}

		/* 4. Calculate initial free space */
		ଯଦି (item->freeze) {
			initial_free_main -= item->target_main_size;
		} ଅନ୍ୟଥା {
			initial_free_main -= item->base_size;
		}
	}

	/* 5. Loop */
	ଯେତେବେଳେ (ଧାଡ଼ି->frozen < ଧାଡ଼ି->ଗଣନା_2) {
		css_fixed unfrozen_factor_sum;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା remaining_free_main;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା total_violation;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: Resolver pass",
				ctx->flex);

		/* b */
		remaining_free_main = layout_flex__remaining_free_main(ctx,
				ଧାଡ଼ି, &unfrozen_factor_sum, initial_free_main,
				available_main, grow);

		/* c */
		ଯଦି (remaining_free_main != 0) {
			layout_flex__distribute_free_main(ctx,
					ଧାଡ଼ି, unfrozen_factor_sum,
					remaining_free_main, grow);
		}

		/* d */
		total_violation = layout_flex__get_min_max_violations(
				ctx, ଧାଡ଼ି);

		/* e */
		ପାଇଁ (size_t i = ଧାଡ଼ି->first; i < item_ଗଣନା; i++) {
			ସଂରଚନା flex_item_data *item = &ctx->item.data[i];

			ଯଦି (item->freeze) {
				ଜାରି;
			}

			ଯଦି (total_violation == 0 ||
			    (total_violation > 0 && item->min_violation) ||
			    (total_violation < 0 && item->max_violation)) {
				layout_flex__item_freeze(ଧାଡ଼ି, item);
			}
		}
	}

	ଫେରନ୍ତୁ true;
}

/**
 * Position items along a line
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to resolve
 * \return true on success, false on failure.
 */
ସ୍ଥିର bool layout_flex_place_ଧାଡ଼ି_ବସ୍ତୁଗୁଡ଼ିକ_main(
		ସଂରଚନା flex_ctx *ctx,
		ସଂରଚନା flex_ଧାଡ଼ି_data *ଧାଡ଼ି)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା main_pos = ctx->flex->padding[layout_flex__main_start_side(ctx)];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା post_multiplier = ctx->main_reversed ? 0 : 1;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା pre_multiplier = ctx->main_reversed ? -1 : 0;
	size_t item_ଗଣନା = ଧାଡ଼ି->first + ଧାଡ଼ି->ଗଣନା_2;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା extra_remainder = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା extra = 0;

	ଯଦି (ctx->main_reversed) {
		main_pos = lh__box_size_main(ctx->horizontal, ctx->flex) -
				main_pos;
	}

	ଯଦି (ctx->available_main != AUTO &&
	    ctx->available_main != UNKNOWN_WIDTH &&
	    ctx->available_main > ଧାଡ଼ି->used_main_size) {
		ଯଦି (ଧାଡ଼ି->main_auto_margin_ଗଣନା > 0) {
			extra = ctx->available_main - ଧାଡ଼ି->used_main_size;

			extra_remainder = extra % ଧାଡ଼ି->main_auto_margin_ଗଣନା;
			extra /= ଧାଡ଼ି->main_auto_margin_ଗଣନା;
		}
	}

	ପାଇଁ (size_t i = ଧାଡ଼ି->first; i < item_ଗଣନା; i++) {
		ଗଣନା box_side main_end = ctx->horizontal ? ଡାହାଣ_2 : BOTTOM;
		ଗଣନା box_side main_start = ctx->horizontal ? ବାମ_2 : ଶୀର୍ଷ_2;
		ସଂରଚନା flex_item_data *item = &ctx->item.data[i];
		ସଂରଚନା box *b = item->box;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା extra_total = 0;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା extra_post = 0;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା extra_pre = 0;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା box_size_main;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା *box_pos_main;

		ଯଦି (ctx->horizontal) {
			b->width = item->target_main_size -
					lh__delta_outer_width(b);

			ଯଦି (!layout_flex_item(ctx, item, b->width)) {
				ଫେରନ୍ତୁ false;
			}
		}

		box_size_main = lh__box_size_main(ctx->horizontal, b);
		box_pos_main = ctx->horizontal ? &b->x : &b->y;

		ଯଦି (!lh__box_is_absolute(b)) {
			ଯଦି (b->margin[main_start] == AUTO) {
				extra_pre = extra + extra_remainder;
			}
			ଯଦି (b->margin[main_end] == AUTO) {
				extra_post = extra + extra_remainder;
			}
			extra_total = extra_pre + extra_post;

			main_pos += pre_multiplier *
					(extra_total + box_size_main +
					 lh__delta_outer_main(ctx->flex, b));
		}

		*box_pos_main = main_pos + lh__non_auto_margin(b, main_start) +
				extra_pre + b->border[main_start].width;

		ଯଦି (!lh__box_is_absolute(b)) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା cross_size;
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା box_size_cross = lh__box_size_cross(
					ctx->horizontal, b);

			main_pos += post_multiplier *
					(extra_total + box_size_main +
					 lh__delta_outer_main(ctx->flex, b));

			cross_size = box_size_cross + lh__delta_outer_cross(
					ctx->flex, b);
			ଯଦି (ଧାଡ଼ି->cross_size < cross_size) {
				ଧାଡ଼ି->cross_size = cross_size;
			}
		}
	}

	ଫେରନ୍ତୁ true;
}

/**
 * Collect items onto lines and place items along the lines
 *
 * \param[in] ctx   Flex layout context
 * \return true on success, false on failure.
 */
ସ୍ଥିର bool layout_flex_collect_ବସ୍ତୁଗୁଡ଼ିକ_into_ଧାଡ଼ିଗୁଡ଼ିକ(
		ସଂରଚନା flex_ctx *ctx)
{
	size_t pos = 0;

	ଯେତେବେଳେ (pos < ctx->item.ଗଣନା_2) {
		ସଂରଚନା flex_ଧାଡ଼ି_data *ଧାଡ଼ି;

		ଧାଡ଼ି = layout_flex_build_ଧାଡ଼ି(ctx, pos);
		ଯଦି (ଧାଡ଼ି == ଖାଲି) {
			ଫେରନ୍ତୁ false;
		}

		pos += ଧାଡ଼ି->ଗଣନା_2;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: "
				"fitted: %zu (total: %zu/%zu)",
				ctx->flex, ଧାଡ଼ି->ଗଣନା_2,
				pos, ctx->item.ଗଣନା_2);

		ଯଦି (!layout_flex_resolve_ଧାଡ଼ି(ctx, ଧାଡ଼ି)) {
			ଫେରନ୍ତୁ false;
		}

		ଯଦି (!layout_flex_place_ଧାଡ଼ି_ବସ୍ତୁଗୁଡ଼ିକ_main(ctx, ଧାଡ଼ି)) {
			ଫେରନ୍ତୁ false;
		}

		ctx->cross_size += ଧାଡ଼ି->cross_size;
		ଯଦି (ctx->main_size < ଧାଡ଼ି->main_size) {
			ctx->main_size = ଧାଡ଼ି->main_size;
		}
	}

	ଫେରନ୍ତୁ true;
}

/**
 * Align items on a line.
 *
 * \param[in] ctx    Flex layout context
 * \param[in] line   Line to align items on
 * \param[in] extra  Extra line width in pixels
 */
ସ୍ଥିର ଶୂନ୍ୟ layout_flex_place_ଧାଡ଼ି_ବସ୍ତୁଗୁଡ଼ିକ_cross(ସଂରଚନା flex_ctx *ctx,
		ସଂରଚନା flex_ଧାଡ଼ି_data *ଧାଡ଼ି, ପୂର୍ଣ୍ଣସଂଖ୍ୟା extra)
{
	ଗଣନା box_side cross_start = ctx->horizontal ? ଶୀର୍ଷ_2 : ବାମ_2;
	size_t item_ଗଣନା = ଧାଡ଼ି->first + ଧାଡ଼ି->ଗଣନା_2;

	ପାଇଁ (size_t i = ଧାଡ଼ି->first; i < item_ଗଣନା; i++) {
		ସଂରଚନା flex_item_data *item = &ctx->item.data[i];
		ସଂରଚନା box *b = item->box;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା cross_free_space;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା *box_size_cross;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା *box_pos_cross;

		box_pos_cross = ctx->horizontal ? &b->y : &b->x;
		box_size_cross = lh__box_size_cross_ptr(ctx->horizontal, b);

		cross_free_space = ଧାଡ଼ି->cross_size + extra - *box_size_cross -
				lh__delta_outer_cross(ctx->flex, b);

		ବାଛ (lh__box_align_self(ctx->flex, b)) {
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ସ୍ଥିତି CSS_ALIGN_SELF_STRETCH:
			ଯଦି (lh__box_size_cross_is_auto(ctx->horizontal, b)) {
				*box_size_cross += cross_free_space;

				/* Relayout children for stretch. */
				ଯଦି (!layout_flex_item(ctx, item, b->width)) {
					ଫେରନ୍ତୁ;
				}
			}
			fallthrough;
		ସ୍ଥିତି CSS_ALIGN_SELF_FLEX_START:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					ଧାଡ଼ି->pos +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି CSS_ALIGN_SELF_FLEX_END:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					ଧାଡ଼ି->pos + cross_free_space +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି CSS_ALIGN_SELF_BASELINE:
		ସ୍ଥିତି CSS_ALIGN_SELF_CENTER:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					ଧାଡ଼ି->pos + cross_free_space / 2 +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			ଭାଙ୍ଗ;
		}
	}
}

/**
 * Place the lines and align the items on the line.
 *
 * \param[in] ctx  Flex layout context
 */
ସ୍ଥିର ଶୂନ୍ୟ layout_flex_place_ଧାଡ଼ିଗୁଡ଼ିକ(ସଂରଚନା flex_ctx *ctx)
{
	bool reversed = ctx->wrap == CSS_FLEX_WRAP_WRAP_REVERSE;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି_pos = reversed ? ctx->cross_size : 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା post_multiplier = reversed ? 0 : 1;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା pre_multiplier = reversed ? -1 : 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା extra_remainder = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା extra = 0;

	ଯଦି (ctx->available_cross != AUTO &&
	    ctx->available_cross > ctx->cross_size &&
	    ctx->ଧାଡ଼ି.ଗଣନା_2 > 0) {
		extra = ctx->available_cross - ctx->cross_size;

		extra_remainder = extra % ctx->ଧାଡ଼ି.ଗଣନା_2;
		extra /= ctx->ଧାଡ଼ି.ଗଣନା_2;
	}

	ପାଇଁ (size_t i = 0; i < ctx->ଧାଡ଼ି.ଗଣନା_2; i++) {
		ସଂରଚନା flex_ଧାଡ଼ି_data *ଧାଡ଼ି = &ctx->ଧାଡ଼ି.data[i];

		ଧାଡ଼ି_pos += pre_multiplier * ଧାଡ଼ି->cross_size;
		ଧାଡ଼ି->pos = ଧାଡ଼ି_pos;
		ଧାଡ଼ି_pos += post_multiplier * ଧାଡ଼ି->cross_size +
				extra + extra_remainder;

		layout_flex_place_ଧାଡ଼ି_ବସ୍ତୁଗୁଡ଼ିକ_cross(ctx, ଧାଡ଼ି,
				extra + extra_remainder);

		ଯଦି (extra_remainder > 0) {
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
bool layout_flex(ସଂରଚନା box *flex, ପୂର୍ଣ୍ଣସଂଖ୍ୟା available_width,
		html_content *content)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_height, min_height;
	ସଂରଚନା flex_ctx *ctx;
	bool success = false;

	ctx = layout_flex_ctx__create(content, flex);
	ଯଦି (ctx == ଖାଲି) {
		ଫେରନ୍ତୁ false;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: %s, available_width %i, width: %i",
			flex, ctx->horizontal ? "horizontal" : "vertical",
			available_width, flex->width);

	layout_find_dimensions(
			ctx->unit_len_ctx, available_width, -1,
			flex, flex->style, ଖାଲି, &flex->height,
			ଖାଲି, ଖାଲି, &max_height, &min_height,
			flex->margin, flex->padding, flex->border);

	available_width = min(available_width, flex->width);

	ଯଦି (ctx->horizontal) {
		ctx->available_main = available_width;
		ctx->available_cross = ctx->flex->height;
	} ଅନ୍ୟଥା {
		ctx->available_main = ctx->flex->height;
		ctx->available_cross = available_width;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: available_main: %i",
			flex, ctx->available_main);
	NSLOG(flex, DEEPDEBUG, "box %p: available_cross: %i",
			flex, ctx->available_cross);

	layout_flex_ctx__populate_item_data(ctx, flex, available_width);

	/* Place items onto lines. */
	success = layout_flex_collect_ବସ୍ତୁଗୁଡ଼ିକ_into_ଧାଡ଼ିଗୁଡ଼ିକ(ctx);
	ଯଦି (!success) {
		ଯାଅ cleanup;
	}

	layout_flex_place_ଧାଡ଼ିଗୁଡ଼ିକ(ctx);

	ଯଦି (flex->height == AUTO) {
		flex->height = ctx->horizontal ?
				ctx->cross_size :
				ctx->main_size;
	}

	ଯଦି (flex->height != AUTO) {
		ଯଦି (max_height >= 0 && flex->height > max_height) {
			flex->height = max_height;
		}
		ଯଦି (min_height >  0 && flex->height < min_height) {
			flex->height = min_height;
		}
	}

	success = true;

cleanup:
	layout_flex_ctx__destroy(ctx);

	NSLOG(flex, DEEPDEBUG, "box %p: %s: w: %i, h: %i", flex,
			success ? "success" : "failure",
			flex->width, flex->height);
	ଫେରନ୍ତୁ success;
}
