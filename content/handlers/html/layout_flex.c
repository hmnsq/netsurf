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

#अन्तर्भाव <string.h>

#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/utils.h"

#अन्तर्भाव "html/box.h"
#अन्तर्भाव "html/html.h"
#अन्तर्भाव "html/private.h"
#अन्तर्भाव "html/box_inspect.h"
#अन्तर्भाव "html/layout_internal.h"

/**
 * Flex item data
 */
संरचना flex_item_data {
	गणना css_flex_basis_e basis;
	css_fixed basis_length;
	css_unit basis_unit;
	संरचना box *box;

	css_fixed shrink;
	css_fixed grow;

	पूर्णाङ्क min_main;
	पूर्णाङ्क max_main;
	पूर्णाङ्क min_cross;
	पूर्णाङ्क max_cross;

	पूर्णाङ्क target_main_size;
	पूर्णाङ्क base_size;
	पूर्णाङ्क main_size;
	size_t पंक्ति;

	bool freeze;
	bool min_violation;
	bool max_violation;
};

/**
 * Flex line data
 */
संरचना flex_पंक्ति_data {
	पूर्णाङ्क main_size;
	पूर्णाङ्क cross_size;

	पूर्णाङ्क used_main_size;
	पूर्णाङ्क main_auto_margin_गिनती;

	पूर्णाङ्क pos;

	size_t first;
	size_t गिनती;
	size_t frozen;
};

/**
 * Flex layout context
 */
संरचना flex_ctx {
	html_content *content;
	ध्रुव संरचना box *flex;
	ध्रुव css_unit_ctx *unit_len_ctx;

	पूर्णाङ्क main_size;
	पूर्णाङ्क cross_size;

	पूर्णाङ्क available_main;
	पूर्णाङ्क available_cross;

	bool horizontal;
	bool main_reversed;
	गणना css_flex_wrap_e wrap;

	संरचना flex_वस्तुएँ {
		size_t गिनती;
		संरचना flex_item_data *data;
	} item;

	संरचना flex_पंक्तियाँ {
		size_t गिनती;
		size_t alloc;
		संरचना flex_पंक्ति_data *data;
	} पंक्ति;
};

/**
 * Destroy a flex layout context
 *
 * \param[in] ctx  Flex layout context
 */
स्थिर शून्य layout_flex_ctx__destroy(संरचना flex_ctx *ctx)
{
	यदि (ctx != रिक्त) {
		मुक्त(ctx->item.data);
		मुक्त(ctx->पंक्ति.data);
		मुक्त(ctx);
	}
}

/**
 * Create a flex layout context
 *
 * \param[in] content  HTML content containing flex box
 * \param[in] flex     Box to create layout context for
 * \return flex layout context or NULL on error
 */
स्थिर संरचना flex_ctx *layout_flex_ctx__create(
		html_content *content,
		ध्रुव संरचना box *flex)
{
	संरचना flex_ctx *ctx;

	ctx = calloc(1, परिमाण(*ctx));
	यदि (ctx == रिक्त) {
		प्रत्यागम रिक्त;
	}
	ctx->पंक्ति.alloc = 1;

	ctx->item.गिनती = box_गिनती_children(flex);
	ctx->item.data = calloc(ctx->item.गिनती, परिमाण(*ctx->item.data));
	यदि (ctx->item.data == रिक्त) {
		layout_flex_ctx__destroy(ctx);
		प्रत्यागम रिक्त;
	}

	ctx->पंक्ति.alloc = 1;
	ctx->पंक्ति.data = calloc(ctx->पंक्ति.alloc, परिमाण(*ctx->पंक्ति.data));
	यदि (ctx->पंक्ति.data == रिक्त) {
		layout_flex_ctx__destroy(ctx);
		प्रत्यागम रिक्त;
	}

	ctx->flex = flex;
	ctx->content = content;
	ctx->unit_len_ctx = &content->unit_len_ctx;

	ctx->wrap = css_computed_flex_wrap(flex->style);
	ctx->horizontal = lh__flex_main_is_horizontal(flex);
	ctx->main_reversed = lh__flex_direction_reversed(flex);

	प्रत्यागम ctx;
}

/**
 * Find box side representing the start of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the start side.
 */
स्थिर गणना box_side layout_flex__main_start_side(
		ध्रुव संरचना flex_ctx *ctx)
{
	यदि (ctx->horizontal) {
		प्रत्यागम (ctx->main_reversed) ? दायाँ_2 : बायाँ_2;
	} अन्यथा {
		प्रत्यागम (ctx->main_reversed) ? BOTTOM : शीर्ष_2;
	}
}

/**
 * Find box side representing the end of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the end side.
 */
स्थिर गणना box_side layout_flex__main_end_side(
		ध्रुव संरचना flex_ctx *ctx)
{
	यदि (ctx->horizontal) {
		प्रत्यागम (ctx->main_reversed) ? बायाँ_2 : दायाँ_2;
	} अन्यथा {
		प्रत्यागम (ctx->main_reversed) ? शीर्ष_2 : BOTTOM;
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
स्थिर bool layout_flex_item(
		ध्रुव संरचना flex_ctx *ctx,
		ध्रुव संरचना flex_item_data *item,
		पूर्णाङ्क available_width)
{
	bool success;
	संरचना box *b = item->box;

	विकल्प (b->type) {
	पक्ष BOX_BLOCK:
		success = layout_block_context(b, -1, ctx->content);
		भङ्ग;
	पक्ष BOX_TABLE:
		b->float_container = b->parent;
		success = layout_table(b, available_width, ctx->content);
		b->float_container = रिक्त;
		भङ्ग;
	पक्ष BOX_FLEX:
		b->float_container = b->parent;
		success = layout_flex(b, available_width, ctx->content);
		b->float_container = रिक्त;
		भङ्ग;
	स्वभाव:
		प्रतिज्ञा(0 && "Bad flex item back type");
		success = false;
		भङ्ग;
	}

	यदि (!success) {
		NSLOG(flex, ERROR, "box %p: layout failed", b);
	}

	प्रत्यागम success;
}

/**
 * Calculate an item's base and target main sizes.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] item             Item to get sizes of
 * \param[in] available_width  Available width in pixels
 * \return true on success false on failure
 */
स्थिर अन्तःस्थ bool layout_flex__base_and_main_sizes(
		ध्रुव संरचना flex_ctx *ctx,
		संरचना flex_item_data *item,
		पूर्णाङ्क available_width)
{
	संरचना box *b = item->box;
	पूर्णाङ्क content_min_width = b->min_width;
	पूर्णाङ्क content_max_width = b->max_width;
	पूर्णाङ्क delta_outer_main = lh__delta_outer_main(ctx->flex, b);

	NSLOG(flex, DEEPDEBUG, "box %p: delta_outer_main: %i",
			b, delta_outer_main);

	यदि (item->basis == CSS_FLEX_BASIS_SET) {
		यदि (item->basis_unit == CSS_UNIT_PCT) {
			item->base_size = FPCT_OF_INT_TOINT(
					item->basis_length,
					available_width);
		} अन्यथा {
			item->base_size = FIXTOINT(css_unit_len2device_px(
					b->style, ctx->unit_len_ctx,
					item->basis_length,
					item->basis_unit));
		}

	} अन्यथा यदि (item->basis == CSS_FLEX_BASIS_AUTO) {
		item->base_size = ctx->horizontal ? b->width : b->height;
	} अन्यथा {
		item->base_size = AUTO;
	}

	यदि (ctx->horizontal == false) {
		यदि (b->width == AUTO) {
			b->width = min(max(content_min_width, available_width),
					content_max_width);
			b->width -= lh__delta_outer_width(b);
		}

		यदि (!layout_flex_item(ctx, item, b->width)) {
			प्रत्यागम false;
		}
	}

	यदि (item->base_size == AUTO) {
		यदि (ctx->horizontal == false) {
			item->base_size = b->height;
		} अन्यथा {
			item->base_size = content_max_width - delta_outer_main;
		}
	}

	item->base_size += delta_outer_main;

	यदि (ctx->horizontal) {
		item->base_size = min(item->base_size, available_width);
		item->base_size = max(item->base_size, content_min_width);
	}

	item->target_main_size = item->base_size;
	item->main_size = item->base_size;

	यदि (item->max_main > 0 &&
	    item->main_size > item->max_main + delta_outer_main) {
		item->main_size = item->max_main + delta_outer_main;
	}

	यदि (item->main_size < item->min_main + delta_outer_main) {
		item->main_size = item->min_main + delta_outer_main;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: base_size: %i, main_size %i",
			b, item->base_size, item->main_size);

	प्रत्यागम true;
}

/**
 * Fill out all item's data in a flex container.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] flex             Flex box
 * \param[in] available_width  Available width in pixels
 */
स्थिर शून्य layout_flex_ctx__populate_item_data(
		ध्रुव संरचना flex_ctx *ctx,
		ध्रुव संरचना box *flex,
		पूर्णाङ्क available_width)
{
	size_t i = 0;
	bool horizontal = ctx->horizontal;

	प्रति (संरचना box *b = flex->children; b != रिक्त; b = b->next) {
		संरचना flex_item_data *item = &ctx->item.data[i++];

		b->float_container = b->parent;
		layout_find_dimensions(ctx->unit_len_ctx, available_width, -1,
				b, b->style, &b->width, &b->height,
				horizontal ? &item->max_main : &item->max_cross,
				horizontal ? &item->min_main : &item->min_cross,
				horizontal ? &item->max_cross : &item->max_main,
				horizontal ? &item->min_cross : &item->min_main,
				b->margin, b->padding, b->border);
		b->float_container = रिक्त;

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
स्थिर bool layout_flex_ctx_ensure_पंक्ति(संरचना flex_ctx *ctx)
{
	संरचना flex_पंक्ति_data *temp;
	size_t पंक्ति_alloc = ctx->पंक्ति.alloc * 2;

	यदि (ctx->पंक्ति.alloc > ctx->पंक्ति.गिनती) {
		प्रत्यागम true;
	}

	temp = realloc(ctx->पंक्ति.data, परिमाण(*ctx->पंक्ति.data) * पंक्ति_alloc);
	यदि (temp == रिक्त) {
		प्रत्यागम false;
	}
	ctx->पंक्ति.data = temp;

	memset(ctx->पंक्ति.data + ctx->पंक्ति.alloc, 0,
	       परिमाण(*ctx->पंक्ति.data) * (पंक्ति_alloc - ctx->पंक्ति.alloc));
	ctx->पंक्ति.alloc = पंक्ति_alloc;

	प्रत्यागम true;
}

/**
 * Assigns flex items to the line and returns the line
 *
 * \param[in] ctx         Flex layout context
 * \param[in] item_index  Index to first item to assign to this line
 * \return Pointer to the new line, or NULL on error.
 */
स्थिर संरचना flex_पंक्ति_data *layout_flex_build_पंक्ति(संरचना flex_ctx *ctx,
		size_t item_index)
{
	गणना box_side start_side = layout_flex__main_start_side(ctx);
	गणना box_side end_side = layout_flex__main_end_side(ctx);
	संरचना flex_पंक्ति_data *पंक्ति;
	पूर्णाङ्क used_main = 0;

	यदि (!layout_flex_ctx_ensure_पंक्ति(ctx)) {
		प्रत्यागम रिक्त;
	}

	पंक्ति = &ctx->पंक्ति.data[ctx->पंक्ति.गिनती];
	पंक्ति->first = item_index;

	NSLOG(flex, DEEPDEBUG, "flex container %p: available main: %i",
			ctx->flex, ctx->available_main);

	यावत् (item_index < ctx->item.गिनती) {
		संरचना flex_item_data *item = &ctx->item.data[item_index];
		संरचना box *b = item->box;
		पूर्णाङ्क pos_main;

		pos_main = ctx->horizontal ?
				item->main_size :
				b->height + lh__delta_outer_main(ctx->flex, b);

		यदि (ctx->wrap == CSS_FLEX_WRAP_NOWRAP ||
		    pos_main + used_main <= ctx->available_main ||
		    lh__box_is_absolute(item->box) ||
		    ctx->available_main == AUTO ||
		    पंक्ति->गिनती == 0 ||
		    pos_main == 0) {
			यदि (lh__box_is_absolute(item->box) == false) {
				पंक्ति->main_size += item->main_size;
				used_main += pos_main;

				यदि (b->margin[start_side] == AUTO) {
					पंक्ति->main_auto_margin_गिनती++;
				}
				यदि (b->margin[end_side] == AUTO) {
					पंक्ति->main_auto_margin_गिनती++;
				}
			}
			item->पंक्ति = ctx->पंक्ति.गिनती;
			पंक्ति->गिनती++;
			item_index++;
		} अन्यथा {
			भङ्ग;
		}
	}

	यदि (पंक्ति->गिनती > 0) {
		ctx->पंक्ति.गिनती++;
	} अन्यथा {
		NSLOG(layout, ERROR, "Failed to fit any flex items");
	}

	प्रत्यागम पंक्ति;
}

/**
 * Freeze an item on a line
 *
 * \param[in] line  Line to containing item
 * \param[in] item  Item to freeze
 */
स्थिर अन्तःस्थ शून्य layout_flex__item_freeze(
		संरचना flex_पंक्ति_data *पंक्ति,
		संरचना flex_item_data *item)
{
	item->freeze = true;
	पंक्ति->frozen++;

	यदि (!lh__box_is_absolute(item->box)){
		पंक्ति->used_main_size += item->target_main_size;
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
स्थिर अन्तःस्थ पूर्णाङ्क layout_flex__remaining_free_main(
		संरचना flex_ctx *ctx,
		संरचना flex_पंक्ति_data *पंक्ति,
		css_fixed *unfrozen_factor_sum,
		पूर्णाङ्क initial_free_main,
		पूर्णाङ्क available_main,
		bool grow)
{
	पूर्णाङ्क remaining_free_main = available_main;
	size_t item_गिनती = पंक्ति->first + पंक्ति->गिनती;

	*unfrozen_factor_sum = 0;

	प्रति (size_t i = पंक्ति->first; i < item_गिनती; i++) {
		संरचना flex_item_data *item = &ctx->item.data[i];

		यदि (item->freeze) {
			remaining_free_main -= item->target_main_size;
		} अन्यथा {
			remaining_free_main -= item->base_size;

			*unfrozen_factor_sum += grow ?
					item->grow : item->shrink;
		}
	}

	यदि (*unfrozen_factor_sum < F_1) {
		पूर्णाङ्क free_space = FIXTOINT(FMUL(INTTOFIX(initial_free_main),
				*unfrozen_factor_sum));

		यदि (free_space < remaining_free_main) {
			remaining_free_main = free_space;
		}
	}

	NSLOG(flex, DEEPDEBUG, "Remaining free space: %i",
			remaining_free_main);

	प्रत्यागम remaining_free_main;
}

/**
 * Clamp flex item target main size and get min/max violations
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to align items on
 * return total violation in pixels
 */
स्थिर अन्तःस्थ पूर्णाङ्क layout_flex__get_min_max_violations(
		संरचना flex_ctx *ctx,
		संरचना flex_पंक्ति_data *पंक्ति)
{

	पूर्णाङ्क total_violation = 0;
	size_t item_गिनती = पंक्ति->first + पंक्ति->गिनती;

	प्रति (size_t i = पंक्ति->first; i < item_गिनती; i++) {
		संरचना flex_item_data *item = &ctx->item.data[i];
		पूर्णाङ्क target_main_size = item->target_main_size;

		NSLOG(flex, DEEPDEBUG, "item %p: target_main_size: %i",
					item->box, target_main_size);

		यदि (item->freeze) {
			अनुवर्त;
		}

		यदि (item->max_main > 0 &&
		    target_main_size > item->max_main) {
			target_main_size = item->max_main;
			item->max_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: max_main: %i",
					item->max_main);
		}

		यदि (target_main_size < item->min_main) {
			target_main_size = item->min_main;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: min_main: %i",
					item->min_main);
		}

		यदि (target_main_size < item->box->min_width) {
			target_main_size = item->box->min_width;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: box min_width: %i",
					item->box->min_width);
		}

		यदि (target_main_size < 0) {
			target_main_size = 0;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: less than 0");
		}

		total_violation += target_main_size - item->target_main_size;
		item->target_main_size = target_main_size;
	}

	NSLOG(flex, DEEPDEBUG, "Total violation: %i", total_violation);

	प्रत्यागम total_violation;
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
स्थिर अन्तःस्थ शून्य layout_flex__distribute_free_main(
		संरचना flex_ctx *ctx,
		संरचना flex_पंक्ति_data *पंक्ति,
		css_fixed unfrozen_factor_sum,
		पूर्णाङ्क remaining_free_main,
		bool grow)
{
	size_t item_गिनती = पंक्ति->first + पंक्ति->गिनती;

	यदि (grow) {
		css_fixed remainder = 0;
		प्रति (size_t i = पंक्ति->first; i < item_गिनती; i++) {
			संरचना flex_item_data *item = &ctx->item.data[i];
			css_fixed result;
			css_fixed ratio;

			यदि (item->freeze) {
				अनुवर्त;
			}

			ratio = FDIV(item->grow, unfrozen_factor_sum);
			result = FMUL(INTTOFIX(remaining_free_main), ratio) +
					remainder;

			item->target_main_size = item->base_size +
					FIXTOINT(result);
			remainder = FIXFRAC(result);
		}
	} अन्यथा {
		css_fixed scaled_shrink_factor_sum = 0;
		css_fixed remainder = 0;

		प्रति (size_t i = पंक्ति->first; i < item_गिनती; i++) {
			संरचना flex_item_data *item = &ctx->item.data[i];
			css_fixed scaled_shrink_factor;

			यदि (item->freeze) {
				अनुवर्त;
			}

			scaled_shrink_factor = FMUL(
					item->shrink,
					INTTOFIX(item->base_size));
			scaled_shrink_factor_sum += scaled_shrink_factor;
		}

		प्रति (size_t i = पंक्ति->first; i < item_गिनती; i++) {
			संरचना flex_item_data *item = &ctx->item.data[i];
			css_fixed scaled_shrink_factor;
			css_fixed result;
			css_fixed ratio;

			यदि (item->freeze) {
				अनुवर्त;
			} अन्यथा यदि (scaled_shrink_factor_sum == 0) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(पंक्ति, item);
				अनुवर्त;
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
स्थिर bool layout_flex_resolve_पंक्ति(
		संरचना flex_ctx *ctx,
		संरचना flex_पंक्ति_data *पंक्ति)
{
	size_t item_गिनती = पंक्ति->first + पंक्ति->गिनती;
	पूर्णाङ्क available_main = ctx->available_main;
	पूर्णाङ्क initial_free_main;
	bool grow;

	यदि (available_main == AUTO) {
		available_main = INT_MAX;
	}

	grow = (पंक्ति->main_size < available_main);
	initial_free_main = available_main;

	NSLOG(flex, DEEPDEBUG, "box %p: line %zu: first: %zu, count: %zu",
			ctx->flex, पंक्ति - ctx->पंक्ति.data,
			पंक्ति->first, पंक्ति->गिनती);
	NSLOG(flex, DEEPDEBUG, "Line main_size: %i, available_main: %i",
			पंक्ति->main_size, available_main);

	प्रति (size_t i = पंक्ति->first; i < item_गिनती; i++) {
		संरचना flex_item_data *item = &ctx->item.data[i];

		/* 3. Size inflexible items */
		यदि (grow) {
			यदि (item->grow == 0 ||
			    item->base_size > item->main_size) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(पंक्ति, item);
			}
		} अन्यथा {
			यदि (item->shrink == 0 ||
			    item->base_size < item->main_size) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(पंक्ति, item);
			}
		}

		/* 4. Calculate initial free space */
		यदि (item->freeze) {
			initial_free_main -= item->target_main_size;
		} अन्यथा {
			initial_free_main -= item->base_size;
		}
	}

	/* 5. Loop */
	यावत् (पंक्ति->frozen < पंक्ति->गिनती) {
		css_fixed unfrozen_factor_sum;
		पूर्णाङ्क remaining_free_main;
		पूर्णाङ्क total_violation;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: Resolver pass",
				ctx->flex);

		/* b */
		remaining_free_main = layout_flex__remaining_free_main(ctx,
				पंक्ति, &unfrozen_factor_sum, initial_free_main,
				available_main, grow);

		/* c */
		यदि (remaining_free_main != 0) {
			layout_flex__distribute_free_main(ctx,
					पंक्ति, unfrozen_factor_sum,
					remaining_free_main, grow);
		}

		/* d */
		total_violation = layout_flex__get_min_max_violations(
				ctx, पंक्ति);

		/* e */
		प्रति (size_t i = पंक्ति->first; i < item_गिनती; i++) {
			संरचना flex_item_data *item = &ctx->item.data[i];

			यदि (item->freeze) {
				अनुवर्त;
			}

			यदि (total_violation == 0 ||
			    (total_violation > 0 && item->min_violation) ||
			    (total_violation < 0 && item->max_violation)) {
				layout_flex__item_freeze(पंक्ति, item);
			}
		}
	}

	प्रत्यागम true;
}

/**
 * Position items along a line
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to resolve
 * \return true on success, false on failure.
 */
स्थिर bool layout_flex_place_पंक्ति_वस्तुएँ_main(
		संरचना flex_ctx *ctx,
		संरचना flex_पंक्ति_data *पंक्ति)
{
	पूर्णाङ्क main_pos = ctx->flex->padding[layout_flex__main_start_side(ctx)];
	पूर्णाङ्क post_multiplier = ctx->main_reversed ? 0 : 1;
	पूर्णाङ्क pre_multiplier = ctx->main_reversed ? -1 : 0;
	size_t item_गिनती = पंक्ति->first + पंक्ति->गिनती;
	पूर्णाङ्क extra_remainder = 0;
	पूर्णाङ्क extra = 0;

	यदि (ctx->main_reversed) {
		main_pos = lh__box_size_main(ctx->horizontal, ctx->flex) -
				main_pos;
	}

	यदि (ctx->available_main != AUTO &&
	    ctx->available_main != UNKNOWN_WIDTH &&
	    ctx->available_main > पंक्ति->used_main_size) {
		यदि (पंक्ति->main_auto_margin_गिनती > 0) {
			extra = ctx->available_main - पंक्ति->used_main_size;

			extra_remainder = extra % पंक्ति->main_auto_margin_गिनती;
			extra /= पंक्ति->main_auto_margin_गिनती;
		}
	}

	प्रति (size_t i = पंक्ति->first; i < item_गिनती; i++) {
		गणना box_side main_end = ctx->horizontal ? दायाँ_2 : BOTTOM;
		गणना box_side main_start = ctx->horizontal ? बायाँ_2 : शीर्ष_2;
		संरचना flex_item_data *item = &ctx->item.data[i];
		संरचना box *b = item->box;
		पूर्णाङ्क extra_total = 0;
		पूर्णाङ्क extra_post = 0;
		पूर्णाङ्क extra_pre = 0;
		पूर्णाङ्क box_size_main;
		पूर्णाङ्क *box_pos_main;

		यदि (ctx->horizontal) {
			b->width = item->target_main_size -
					lh__delta_outer_width(b);

			यदि (!layout_flex_item(ctx, item, b->width)) {
				प्रत्यागम false;
			}
		}

		box_size_main = lh__box_size_main(ctx->horizontal, b);
		box_pos_main = ctx->horizontal ? &b->x : &b->y;

		यदि (!lh__box_is_absolute(b)) {
			यदि (b->margin[main_start] == AUTO) {
				extra_pre = extra + extra_remainder;
			}
			यदि (b->margin[main_end] == AUTO) {
				extra_post = extra + extra_remainder;
			}
			extra_total = extra_pre + extra_post;

			main_pos += pre_multiplier *
					(extra_total + box_size_main +
					 lh__delta_outer_main(ctx->flex, b));
		}

		*box_pos_main = main_pos + lh__non_auto_margin(b, main_start) +
				extra_pre + b->border[main_start].width;

		यदि (!lh__box_is_absolute(b)) {
			पूर्णाङ्क cross_size;
			पूर्णाङ्क box_size_cross = lh__box_size_cross(
					ctx->horizontal, b);

			main_pos += post_multiplier *
					(extra_total + box_size_main +
					 lh__delta_outer_main(ctx->flex, b));

			cross_size = box_size_cross + lh__delta_outer_cross(
					ctx->flex, b);
			यदि (पंक्ति->cross_size < cross_size) {
				पंक्ति->cross_size = cross_size;
			}
		}
	}

	प्रत्यागम true;
}

/**
 * Collect items onto lines and place items along the lines
 *
 * \param[in] ctx   Flex layout context
 * \return true on success, false on failure.
 */
स्थिर bool layout_flex_collect_वस्तुएँ_into_पंक्तियाँ(
		संरचना flex_ctx *ctx)
{
	size_t pos = 0;

	यावत् (pos < ctx->item.गिनती) {
		संरचना flex_पंक्ति_data *पंक्ति;

		पंक्ति = layout_flex_build_पंक्ति(ctx, pos);
		यदि (पंक्ति == रिक्त) {
			प्रत्यागम false;
		}

		pos += पंक्ति->गिनती;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: "
				"fitted: %zu (total: %zu/%zu)",
				ctx->flex, पंक्ति->गिनती,
				pos, ctx->item.गिनती);

		यदि (!layout_flex_resolve_पंक्ति(ctx, पंक्ति)) {
			प्रत्यागम false;
		}

		यदि (!layout_flex_place_पंक्ति_वस्तुएँ_main(ctx, पंक्ति)) {
			प्रत्यागम false;
		}

		ctx->cross_size += पंक्ति->cross_size;
		यदि (ctx->main_size < पंक्ति->main_size) {
			ctx->main_size = पंक्ति->main_size;
		}
	}

	प्रत्यागम true;
}

/**
 * Align items on a line.
 *
 * \param[in] ctx    Flex layout context
 * \param[in] line   Line to align items on
 * \param[in] extra  Extra line width in pixels
 */
स्थिर शून्य layout_flex_place_पंक्ति_वस्तुएँ_cross(संरचना flex_ctx *ctx,
		संरचना flex_पंक्ति_data *पंक्ति, पूर्णाङ्क extra)
{
	गणना box_side cross_start = ctx->horizontal ? शीर्ष_2 : बायाँ_2;
	size_t item_गिनती = पंक्ति->first + पंक्ति->गिनती;

	प्रति (size_t i = पंक्ति->first; i < item_गिनती; i++) {
		संरचना flex_item_data *item = &ctx->item.data[i];
		संरचना box *b = item->box;
		पूर्णाङ्क cross_free_space;
		पूर्णाङ्क *box_size_cross;
		पूर्णाङ्क *box_pos_cross;

		box_pos_cross = ctx->horizontal ? &b->y : &b->x;
		box_size_cross = lh__box_size_cross_ptr(ctx->horizontal, b);

		cross_free_space = पंक्ति->cross_size + extra - *box_size_cross -
				lh__delta_outer_cross(ctx->flex, b);

		विकल्प (lh__box_align_self(ctx->flex, b)) {
		स्वभाव:
		पक्ष CSS_ALIGN_SELF_STRETCH:
			यदि (lh__box_size_cross_is_auto(ctx->horizontal, b)) {
				*box_size_cross += cross_free_space;

				/* Relayout children for stretch. */
				यदि (!layout_flex_item(ctx, item, b->width)) {
					प्रत्यागम;
				}
			}
			fallthrough;
		पक्ष CSS_ALIGN_SELF_FLEX_START:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					पंक्ति->pos +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			भङ्ग;

		पक्ष CSS_ALIGN_SELF_FLEX_END:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					पंक्ति->pos + cross_free_space +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			भङ्ग;

		पक्ष CSS_ALIGN_SELF_BASELINE:
		पक्ष CSS_ALIGN_SELF_CENTER:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					पंक्ति->pos + cross_free_space / 2 +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			भङ्ग;
		}
	}
}

/**
 * Place the lines and align the items on the line.
 *
 * \param[in] ctx  Flex layout context
 */
स्थिर शून्य layout_flex_place_पंक्तियाँ(संरचना flex_ctx *ctx)
{
	bool reversed = ctx->wrap == CSS_FLEX_WRAP_WRAP_REVERSE;
	पूर्णाङ्क पंक्ति_pos = reversed ? ctx->cross_size : 0;
	पूर्णाङ्क post_multiplier = reversed ? 0 : 1;
	पूर्णाङ्क pre_multiplier = reversed ? -1 : 0;
	पूर्णाङ्क extra_remainder = 0;
	पूर्णाङ्क extra = 0;

	यदि (ctx->available_cross != AUTO &&
	    ctx->available_cross > ctx->cross_size &&
	    ctx->पंक्ति.गिनती > 0) {
		extra = ctx->available_cross - ctx->cross_size;

		extra_remainder = extra % ctx->पंक्ति.गिनती;
		extra /= ctx->पंक्ति.गिनती;
	}

	प्रति (size_t i = 0; i < ctx->पंक्ति.गिनती; i++) {
		संरचना flex_पंक्ति_data *पंक्ति = &ctx->पंक्ति.data[i];

		पंक्ति_pos += pre_multiplier * पंक्ति->cross_size;
		पंक्ति->pos = पंक्ति_pos;
		पंक्ति_pos += post_multiplier * पंक्ति->cross_size +
				extra + extra_remainder;

		layout_flex_place_पंक्ति_वस्तुएँ_cross(ctx, पंक्ति,
				extra + extra_remainder);

		यदि (extra_remainder > 0) {
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
bool layout_flex(संरचना box *flex, पूर्णाङ्क available_width,
		html_content *content)
{
	पूर्णाङ्क max_height, min_height;
	संरचना flex_ctx *ctx;
	bool success = false;

	ctx = layout_flex_ctx__create(content, flex);
	यदि (ctx == रिक्त) {
		प्रत्यागम false;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: %s, available_width %i, width: %i",
			flex, ctx->horizontal ? "horizontal" : "vertical",
			available_width, flex->width);

	layout_find_dimensions(
			ctx->unit_len_ctx, available_width, -1,
			flex, flex->style, रिक्त, &flex->height,
			रिक्त, रिक्त, &max_height, &min_height,
			flex->margin, flex->padding, flex->border);

	available_width = min(available_width, flex->width);

	यदि (ctx->horizontal) {
		ctx->available_main = available_width;
		ctx->available_cross = ctx->flex->height;
	} अन्यथा {
		ctx->available_main = ctx->flex->height;
		ctx->available_cross = available_width;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: available_main: %i",
			flex, ctx->available_main);
	NSLOG(flex, DEEPDEBUG, "box %p: available_cross: %i",
			flex, ctx->available_cross);

	layout_flex_ctx__populate_item_data(ctx, flex, available_width);

	/* Place items onto lines. */
	success = layout_flex_collect_वस्तुएँ_into_पंक्तियाँ(ctx);
	यदि (!success) {
		गच्छ cleanup;
	}

	layout_flex_place_पंक्तियाँ(ctx);

	यदि (flex->height == AUTO) {
		flex->height = ctx->horizontal ?
				ctx->cross_size :
				ctx->main_size;
	}

	यदि (flex->height != AUTO) {
		यदि (max_height >= 0 && flex->height > max_height) {
			flex->height = max_height;
		}
		यदि (min_height >  0 && flex->height < min_height) {
			flex->height = min_height;
		}
	}

	success = true;

cleanup:
	layout_flex_ctx__destroy(ctx);

	NSLOG(flex, DEEPDEBUG, "box %p: %s: w: %i, h: %i", flex,
			success ? "success" : "failure",
			flex->width, flex->height);
	प्रत्यागम success;
}
