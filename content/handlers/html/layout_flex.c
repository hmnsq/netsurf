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

	पूर्णांक min_main;
	पूर्णांक max_main;
	पूर्णांक min_cross;
	पूर्णांक max_cross;

	पूर्णांक target_main_size;
	पूर्णांक base_size;
	पूर्णांक main_size;
	माप_प्रकार ओळ;

	bool freeze;
	bool min_violation;
	bool max_violation;
};

/**
 * Flex line data
 */
संरचना flex_ओळ_data {
	पूर्णांक main_size;
	पूर्णांक cross_size;

	पूर्णांक used_main_size;
	पूर्णांक main_auto_margin_मोजणी;

	पूर्णांक pos;

	माप_प्रकार first;
	माप_प्रकार मोजणी;
	माप_प्रकार frozen;
};

/**
 * Flex layout context
 */
संरचना flex_ctx {
	html_content *content;
	स्थिरांक संरचना box *flex;
	स्थिरांक css_unit_ctx *unit_len_ctx;

	पूर्णांक main_size;
	पूर्णांक cross_size;

	पूर्णांक available_main;
	पूर्णांक available_cross;

	bool horizontal;
	bool main_reversed;
	गणना css_flex_wrap_e wrap;

	संरचना flex_वस्तुसंच {
		माप_प्रकार मोजणी;
		संरचना flex_item_data *data;
	} item;

	संरचना flex_ओळी {
		माप_प्रकार मोजणी;
		माप_प्रकार alloc;
		संरचना flex_ओळ_data *data;
	} ओळ;
};

/**
 * Destroy a flex layout context
 *
 * \param[in] ctx  Flex layout context
 */
स्थिर शून्य layout_flex_ctx__destroy(संरचना flex_ctx *ctx)
{
	जर (ctx != रिक्त) {
		मुक्त(ctx->item.data);
		मुक्त(ctx->ओळ.data);
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
		स्थिरांक संरचना box *flex)
{
	संरचना flex_ctx *ctx;

	ctx = सुस्मृति(1, आकार(*ctx));
	जर (ctx == रिक्त) {
		परत रिक्त;
	}
	ctx->ओळ.alloc = 1;

	ctx->item.मोजणी = box_मोजणी_children(flex);
	ctx->item.data = सुस्मृति(ctx->item.मोजणी, आकार(*ctx->item.data));
	जर (ctx->item.data == रिक्त) {
		layout_flex_ctx__destroy(ctx);
		परत रिक्त;
	}

	ctx->ओळ.alloc = 1;
	ctx->ओळ.data = सुस्मृति(ctx->ओळ.alloc, आकार(*ctx->ओळ.data));
	जर (ctx->ओळ.data == रिक्त) {
		layout_flex_ctx__destroy(ctx);
		परत रिक्त;
	}

	ctx->flex = flex;
	ctx->content = content;
	ctx->unit_len_ctx = &content->unit_len_ctx;

	ctx->wrap = css_computed_flex_wrap(flex->style);
	ctx->horizontal = lh__flex_main_is_horizontal(flex);
	ctx->main_reversed = lh__flex_direction_reversed(flex);

	परत ctx;
}

/**
 * Find box side representing the start of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the start side.
 */
स्थिर गणना box_side layout_flex__main_start_side(
		स्थिरांक संरचना flex_ctx *ctx)
{
	जर (ctx->horizontal) {
		परत (ctx->main_reversed) ? उजवा_2 : डावा_2;
	} अन्यथा {
		परत (ctx->main_reversed) ? BOTTOM : शीर्ष_2;
	}
}

/**
 * Find box side representing the end of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the end side.
 */
स्थिर गणना box_side layout_flex__main_end_side(
		स्थिरांक संरचना flex_ctx *ctx)
{
	जर (ctx->horizontal) {
		परत (ctx->main_reversed) ? डावा_2 : उजवा_2;
	} अन्यथा {
		परत (ctx->main_reversed) ? शीर्ष_2 : BOTTOM;
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
		स्थिरांक संरचना flex_ctx *ctx,
		स्थिरांक संरचना flex_item_data *item,
		पूर्णांक available_width)
{
	bool success;
	संरचना box *b = item->box;

	निवड (b->type) {
	स्थिती BOX_BLOCK:
		success = layout_block_context(b, -1, ctx->content);
		थांबा;
	स्थिती BOX_TABLE:
		b->float_container = b->parent;
		success = layout_table(b, available_width, ctx->content);
		b->float_container = रिक्त;
		थांबा;
	स्थिती BOX_FLEX:
		b->float_container = b->parent;
		success = layout_flex(b, available_width, ctx->content);
		b->float_container = रिक्त;
		थांबा;
	पूर्वनिर्धारित:
		प्रतिज्ञा(0 && "Bad flex item back type");
		success = false;
		थांबा;
	}

	जर (!success) {
		NSLOG(flex, ERROR, "box %p: layout failed", b);
	}

	परत success;
}

/**
 * Calculate an item's base and target main sizes.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] item             Item to get sizes of
 * \param[in] available_width  Available width in pixels
 * \return true on success false on failure
 */
स्थिर ओळीत bool layout_flex__base_and_main_sizes(
		स्थिरांक संरचना flex_ctx *ctx,
		संरचना flex_item_data *item,
		पूर्णांक available_width)
{
	संरचना box *b = item->box;
	पूर्णांक content_min_width = b->min_width;
	पूर्णांक content_max_width = b->max_width;
	पूर्णांक delta_outer_main = lh__delta_outer_main(ctx->flex, b);

	NSLOG(flex, DEEPDEBUG, "box %p: delta_outer_main: %i",
			b, delta_outer_main);

	जर (item->basis == CSS_FLEX_BASIS_SET) {
		जर (item->basis_unit == CSS_UNIT_PCT) {
			item->base_size = FPCT_OF_INT_TOINT(
					item->basis_length,
					available_width);
		} अन्यथा {
			item->base_size = FIXTOINT(css_unit_len2device_px(
					b->style, ctx->unit_len_ctx,
					item->basis_length,
					item->basis_unit));
		}

	} अन्यथा जर (item->basis == CSS_FLEX_BASIS_AUTO) {
		item->base_size = ctx->horizontal ? b->width : b->height;
	} अन्यथा {
		item->base_size = AUTO;
	}

	जर (ctx->horizontal == false) {
		जर (b->width == AUTO) {
			b->width = min(max(content_min_width, available_width),
					content_max_width);
			b->width -= lh__delta_outer_width(b);
		}

		जर (!layout_flex_item(ctx, item, b->width)) {
			परत false;
		}
	}

	जर (item->base_size == AUTO) {
		जर (ctx->horizontal == false) {
			item->base_size = b->height;
		} अन्यथा {
			item->base_size = content_max_width - delta_outer_main;
		}
	}

	item->base_size += delta_outer_main;

	जर (ctx->horizontal) {
		item->base_size = min(item->base_size, available_width);
		item->base_size = max(item->base_size, content_min_width);
	}

	item->target_main_size = item->base_size;
	item->main_size = item->base_size;

	जर (item->max_main > 0 &&
	    item->main_size > item->max_main + delta_outer_main) {
		item->main_size = item->max_main + delta_outer_main;
	}

	जर (item->main_size < item->min_main + delta_outer_main) {
		item->main_size = item->min_main + delta_outer_main;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: base_size: %i, main_size %i",
			b, item->base_size, item->main_size);

	परत true;
}

/**
 * Fill out all item's data in a flex container.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] flex             Flex box
 * \param[in] available_width  Available width in pixels
 */
स्थिर शून्य layout_flex_ctx__populate_item_data(
		स्थिरांक संरचना flex_ctx *ctx,
		स्थिरांक संरचना box *flex,
		पूर्णांक available_width)
{
	माप_प्रकार i = 0;
	bool horizontal = ctx->horizontal;

	साठी (संरचना box *b = flex->children; b != रिक्त; b = b->next) {
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
स्थिर bool layout_flex_ctx_ensure_ओळ(संरचना flex_ctx *ctx)
{
	संरचना flex_ओळ_data *temp;
	माप_प्रकार ओळ_alloc = ctx->ओळ.alloc * 2;

	जर (ctx->ओळ.alloc > ctx->ओळ.मोजणी) {
		परत true;
	}

	temp = पुनः_स्मृति(ctx->ओळ.data, आकार(*ctx->ओळ.data) * ओळ_alloc);
	जर (temp == रिक्त) {
		परत false;
	}
	ctx->ओळ.data = temp;

	स_रखो(ctx->ओळ.data + ctx->ओळ.alloc, 0,
	       आकार(*ctx->ओळ.data) * (ओळ_alloc - ctx->ओळ.alloc));
	ctx->ओळ.alloc = ओळ_alloc;

	परत true;
}

/**
 * Assigns flex items to the line and returns the line
 *
 * \param[in] ctx         Flex layout context
 * \param[in] item_index  Index to first item to assign to this line
 * \return Pointer to the new line, or NULL on error.
 */
स्थिर संरचना flex_ओळ_data *layout_flex_build_ओळ(संरचना flex_ctx *ctx,
		माप_प्रकार item_index)
{
	गणना box_side start_side = layout_flex__main_start_side(ctx);
	गणना box_side end_side = layout_flex__main_end_side(ctx);
	संरचना flex_ओळ_data *ओळ;
	पूर्णांक used_main = 0;

	जर (!layout_flex_ctx_ensure_ओळ(ctx)) {
		परत रिक्त;
	}

	ओळ = &ctx->ओळ.data[ctx->ओळ.मोजणी];
	ओळ->first = item_index;

	NSLOG(flex, DEEPDEBUG, "flex container %p: available main: %i",
			ctx->flex, ctx->available_main);

	पर्यंत (item_index < ctx->item.मोजणी) {
		संरचना flex_item_data *item = &ctx->item.data[item_index];
		संरचना box *b = item->box;
		पूर्णांक pos_main;

		pos_main = ctx->horizontal ?
				item->main_size :
				b->height + lh__delta_outer_main(ctx->flex, b);

		जर (ctx->wrap == CSS_FLEX_WRAP_NOWRAP ||
		    pos_main + used_main <= ctx->available_main ||
		    lh__box_is_absolute(item->box) ||
		    ctx->available_main == AUTO ||
		    ओळ->मोजणी == 0 ||
		    pos_main == 0) {
			जर (lh__box_is_absolute(item->box) == false) {
				ओळ->main_size += item->main_size;
				used_main += pos_main;

				जर (b->margin[start_side] == AUTO) {
					ओळ->main_auto_margin_मोजणी++;
				}
				जर (b->margin[end_side] == AUTO) {
					ओळ->main_auto_margin_मोजणी++;
				}
			}
			item->ओळ = ctx->ओळ.मोजणी;
			ओळ->मोजणी++;
			item_index++;
		} अन्यथा {
			थांबा;
		}
	}

	जर (ओळ->मोजणी > 0) {
		ctx->ओळ.मोजणी++;
	} अन्यथा {
		NSLOG(layout, ERROR, "Failed to fit any flex items");
	}

	परत ओळ;
}

/**
 * Freeze an item on a line
 *
 * \param[in] line  Line to containing item
 * \param[in] item  Item to freeze
 */
स्थिर ओळीत शून्य layout_flex__item_freeze(
		संरचना flex_ओळ_data *ओळ,
		संरचना flex_item_data *item)
{
	item->freeze = true;
	ओळ->frozen++;

	जर (!lh__box_is_absolute(item->box)){
		ओळ->used_main_size += item->target_main_size;
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
स्थिर ओळीत पूर्णांक layout_flex__remaining_free_main(
		संरचना flex_ctx *ctx,
		संरचना flex_ओळ_data *ओळ,
		css_fixed *unfrozen_factor_sum,
		पूर्णांक initial_free_main,
		पूर्णांक available_main,
		bool grow)
{
	पूर्णांक remaining_free_main = available_main;
	माप_प्रकार item_मोजणी = ओळ->first + ओळ->मोजणी;

	*unfrozen_factor_sum = 0;

	साठी (माप_प्रकार i = ओळ->first; i < item_मोजणी; i++) {
		संरचना flex_item_data *item = &ctx->item.data[i];

		जर (item->freeze) {
			remaining_free_main -= item->target_main_size;
		} अन्यथा {
			remaining_free_main -= item->base_size;

			*unfrozen_factor_sum += grow ?
					item->grow : item->shrink;
		}
	}

	जर (*unfrozen_factor_sum < F_1) {
		पूर्णांक free_space = FIXTOINT(FMUL(INTTOFIX(initial_free_main),
				*unfrozen_factor_sum));

		जर (free_space < remaining_free_main) {
			remaining_free_main = free_space;
		}
	}

	NSLOG(flex, DEEPDEBUG, "Remaining free space: %i",
			remaining_free_main);

	परत remaining_free_main;
}

/**
 * Clamp flex item target main size and get min/max violations
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to align items on
 * return total violation in pixels
 */
स्थिर ओळीत पूर्णांक layout_flex__get_min_max_violations(
		संरचना flex_ctx *ctx,
		संरचना flex_ओळ_data *ओळ)
{

	पूर्णांक total_violation = 0;
	माप_प्रकार item_मोजणी = ओळ->first + ओळ->मोजणी;

	साठी (माप_प्रकार i = ओळ->first; i < item_मोजणी; i++) {
		संरचना flex_item_data *item = &ctx->item.data[i];
		पूर्णांक target_main_size = item->target_main_size;

		NSLOG(flex, DEEPDEBUG, "item %p: target_main_size: %i",
					item->box, target_main_size);

		जर (item->freeze) {
			सुरूठेवा;
		}

		जर (item->max_main > 0 &&
		    target_main_size > item->max_main) {
			target_main_size = item->max_main;
			item->max_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: max_main: %i",
					item->max_main);
		}

		जर (target_main_size < item->min_main) {
			target_main_size = item->min_main;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: min_main: %i",
					item->min_main);
		}

		जर (target_main_size < item->box->min_width) {
			target_main_size = item->box->min_width;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: box min_width: %i",
					item->box->min_width);
		}

		जर (target_main_size < 0) {
			target_main_size = 0;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: less than 0");
		}

		total_violation += target_main_size - item->target_main_size;
		item->target_main_size = target_main_size;
	}

	NSLOG(flex, DEEPDEBUG, "Total violation: %i", total_violation);

	परत total_violation;
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
स्थिर ओळीत शून्य layout_flex__distribute_free_main(
		संरचना flex_ctx *ctx,
		संरचना flex_ओळ_data *ओळ,
		css_fixed unfrozen_factor_sum,
		पूर्णांक remaining_free_main,
		bool grow)
{
	माप_प्रकार item_मोजणी = ओळ->first + ओळ->मोजणी;

	जर (grow) {
		css_fixed remainder = 0;
		साठी (माप_प्रकार i = ओळ->first; i < item_मोजणी; i++) {
			संरचना flex_item_data *item = &ctx->item.data[i];
			css_fixed result;
			css_fixed ratio;

			जर (item->freeze) {
				सुरूठेवा;
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

		साठी (माप_प्रकार i = ओळ->first; i < item_मोजणी; i++) {
			संरचना flex_item_data *item = &ctx->item.data[i];
			css_fixed scaled_shrink_factor;

			जर (item->freeze) {
				सुरूठेवा;
			}

			scaled_shrink_factor = FMUL(
					item->shrink,
					INTTOFIX(item->base_size));
			scaled_shrink_factor_sum += scaled_shrink_factor;
		}

		साठी (माप_प्रकार i = ओळ->first; i < item_मोजणी; i++) {
			संरचना flex_item_data *item = &ctx->item.data[i];
			css_fixed scaled_shrink_factor;
			css_fixed result;
			css_fixed ratio;

			जर (item->freeze) {
				सुरूठेवा;
			} अन्यथा जर (scaled_shrink_factor_sum == 0) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(ओळ, item);
				सुरूठेवा;
			}

			scaled_shrink_factor = FMUL(
					item->shrink,
					INTTOFIX(item->base_size));
			ratio = FDIV(scaled_shrink_factor,
			             scaled_shrink_factor_sum);
			result = FMUL(INTTOFIX(असल(remaining_free_main)),
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
स्थिर bool layout_flex_resolve_ओळ(
		संरचना flex_ctx *ctx,
		संरचना flex_ओळ_data *ओळ)
{
	माप_प्रकार item_मोजणी = ओळ->first + ओळ->मोजणी;
	पूर्णांक available_main = ctx->available_main;
	पूर्णांक initial_free_main;
	bool grow;

	जर (available_main == AUTO) {
		available_main = पूर्णांक_उच्च;
	}

	grow = (ओळ->main_size < available_main);
	initial_free_main = available_main;

	NSLOG(flex, DEEPDEBUG, "box %p: line %zu: first: %zu, count: %zu",
			ctx->flex, ओळ - ctx->ओळ.data,
			ओळ->first, ओळ->मोजणी);
	NSLOG(flex, DEEPDEBUG, "Line main_size: %i, available_main: %i",
			ओळ->main_size, available_main);

	साठी (माप_प्रकार i = ओळ->first; i < item_मोजणी; i++) {
		संरचना flex_item_data *item = &ctx->item.data[i];

		/* 3. Size inflexible items */
		जर (grow) {
			जर (item->grow == 0 ||
			    item->base_size > item->main_size) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(ओळ, item);
			}
		} अन्यथा {
			जर (item->shrink == 0 ||
			    item->base_size < item->main_size) {
				item->target_main_size = item->main_size;
				layout_flex__item_freeze(ओळ, item);
			}
		}

		/* 4. Calculate initial free space */
		जर (item->freeze) {
			initial_free_main -= item->target_main_size;
		} अन्यथा {
			initial_free_main -= item->base_size;
		}
	}

	/* 5. Loop */
	पर्यंत (ओळ->frozen < ओळ->मोजणी) {
		css_fixed unfrozen_factor_sum;
		पूर्णांक remaining_free_main;
		पूर्णांक total_violation;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: Resolver pass",
				ctx->flex);

		/* b */
		remaining_free_main = layout_flex__remaining_free_main(ctx,
				ओळ, &unfrozen_factor_sum, initial_free_main,
				available_main, grow);

		/* c */
		जर (remaining_free_main != 0) {
			layout_flex__distribute_free_main(ctx,
					ओळ, unfrozen_factor_sum,
					remaining_free_main, grow);
		}

		/* d */
		total_violation = layout_flex__get_min_max_violations(
				ctx, ओळ);

		/* e */
		साठी (माप_प्रकार i = ओळ->first; i < item_मोजणी; i++) {
			संरचना flex_item_data *item = &ctx->item.data[i];

			जर (item->freeze) {
				सुरूठेवा;
			}

			जर (total_violation == 0 ||
			    (total_violation > 0 && item->min_violation) ||
			    (total_violation < 0 && item->max_violation)) {
				layout_flex__item_freeze(ओळ, item);
			}
		}
	}

	परत true;
}

/**
 * Position items along a line
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to resolve
 * \return true on success, false on failure.
 */
स्थिर bool layout_flex_place_ओळ_वस्तुसंच_main(
		संरचना flex_ctx *ctx,
		संरचना flex_ओळ_data *ओळ)
{
	पूर्णांक main_pos = ctx->flex->padding[layout_flex__main_start_side(ctx)];
	पूर्णांक post_multiplier = ctx->main_reversed ? 0 : 1;
	पूर्णांक pre_multiplier = ctx->main_reversed ? -1 : 0;
	माप_प्रकार item_मोजणी = ओळ->first + ओळ->मोजणी;
	पूर्णांक extra_remainder = 0;
	पूर्णांक extra = 0;

	जर (ctx->main_reversed) {
		main_pos = lh__box_size_main(ctx->horizontal, ctx->flex) -
				main_pos;
	}

	जर (ctx->available_main != AUTO &&
	    ctx->available_main != UNKNOWN_WIDTH &&
	    ctx->available_main > ओळ->used_main_size) {
		जर (ओळ->main_auto_margin_मोजणी > 0) {
			extra = ctx->available_main - ओळ->used_main_size;

			extra_remainder = extra % ओळ->main_auto_margin_मोजणी;
			extra /= ओळ->main_auto_margin_मोजणी;
		}
	}

	साठी (माप_प्रकार i = ओळ->first; i < item_मोजणी; i++) {
		गणना box_side main_end = ctx->horizontal ? उजवा_2 : BOTTOM;
		गणना box_side main_start = ctx->horizontal ? डावा_2 : शीर्ष_2;
		संरचना flex_item_data *item = &ctx->item.data[i];
		संरचना box *b = item->box;
		पूर्णांक extra_total = 0;
		पूर्णांक extra_post = 0;
		पूर्णांक extra_pre = 0;
		पूर्णांक box_size_main;
		पूर्णांक *box_pos_main;

		जर (ctx->horizontal) {
			b->width = item->target_main_size -
					lh__delta_outer_width(b);

			जर (!layout_flex_item(ctx, item, b->width)) {
				परत false;
			}
		}

		box_size_main = lh__box_size_main(ctx->horizontal, b);
		box_pos_main = ctx->horizontal ? &b->x : &b->y;

		जर (!lh__box_is_absolute(b)) {
			जर (b->margin[main_start] == AUTO) {
				extra_pre = extra + extra_remainder;
			}
			जर (b->margin[main_end] == AUTO) {
				extra_post = extra + extra_remainder;
			}
			extra_total = extra_pre + extra_post;

			main_pos += pre_multiplier *
					(extra_total + box_size_main +
					 lh__delta_outer_main(ctx->flex, b));
		}

		*box_pos_main = main_pos + lh__non_auto_margin(b, main_start) +
				extra_pre + b->border[main_start].width;

		जर (!lh__box_is_absolute(b)) {
			पूर्णांक cross_size;
			पूर्णांक box_size_cross = lh__box_size_cross(
					ctx->horizontal, b);

			main_pos += post_multiplier *
					(extra_total + box_size_main +
					 lh__delta_outer_main(ctx->flex, b));

			cross_size = box_size_cross + lh__delta_outer_cross(
					ctx->flex, b);
			जर (ओळ->cross_size < cross_size) {
				ओळ->cross_size = cross_size;
			}
		}
	}

	परत true;
}

/**
 * Collect items onto lines and place items along the lines
 *
 * \param[in] ctx   Flex layout context
 * \return true on success, false on failure.
 */
स्थिर bool layout_flex_collect_वस्तुसंच_into_ओळी(
		संरचना flex_ctx *ctx)
{
	माप_प्रकार pos = 0;

	पर्यंत (pos < ctx->item.मोजणी) {
		संरचना flex_ओळ_data *ओळ;

		ओळ = layout_flex_build_ओळ(ctx, pos);
		जर (ओळ == रिक्त) {
			परत false;
		}

		pos += ओळ->मोजणी;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: "
				"fitted: %zu (total: %zu/%zu)",
				ctx->flex, ओळ->मोजणी,
				pos, ctx->item.मोजणी);

		जर (!layout_flex_resolve_ओळ(ctx, ओळ)) {
			परत false;
		}

		जर (!layout_flex_place_ओळ_वस्तुसंच_main(ctx, ओळ)) {
			परत false;
		}

		ctx->cross_size += ओळ->cross_size;
		जर (ctx->main_size < ओळ->main_size) {
			ctx->main_size = ओळ->main_size;
		}
	}

	परत true;
}

/**
 * Align items on a line.
 *
 * \param[in] ctx    Flex layout context
 * \param[in] line   Line to align items on
 * \param[in] extra  Extra line width in pixels
 */
स्थिर शून्य layout_flex_place_ओळ_वस्तुसंच_cross(संरचना flex_ctx *ctx,
		संरचना flex_ओळ_data *ओळ, पूर्णांक extra)
{
	गणना box_side cross_start = ctx->horizontal ? शीर्ष_2 : डावा_2;
	माप_प्रकार item_मोजणी = ओळ->first + ओळ->मोजणी;

	साठी (माप_प्रकार i = ओळ->first; i < item_मोजणी; i++) {
		संरचना flex_item_data *item = &ctx->item.data[i];
		संरचना box *b = item->box;
		पूर्णांक cross_free_space;
		पूर्णांक *box_size_cross;
		पूर्णांक *box_pos_cross;

		box_pos_cross = ctx->horizontal ? &b->y : &b->x;
		box_size_cross = lh__box_size_cross_ptr(ctx->horizontal, b);

		cross_free_space = ओळ->cross_size + extra - *box_size_cross -
				lh__delta_outer_cross(ctx->flex, b);

		निवड (lh__box_align_self(ctx->flex, b)) {
		पूर्वनिर्धारित:
		स्थिती CSS_ALIGN_SELF_STRETCH:
			जर (lh__box_size_cross_is_auto(ctx->horizontal, b)) {
				*box_size_cross += cross_free_space;

				/* Relayout children for stretch. */
				जर (!layout_flex_item(ctx, item, b->width)) {
					परत;
				}
			}
			fallthrough;
		स्थिती CSS_ALIGN_SELF_FLEX_START:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					ओळ->pos +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			थांबा;

		स्थिती CSS_ALIGN_SELF_FLEX_END:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					ओळ->pos + cross_free_space +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			थांबा;

		स्थिती CSS_ALIGN_SELF_BASELINE:
		स्थिती CSS_ALIGN_SELF_CENTER:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					ओळ->pos + cross_free_space / 2 +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			थांबा;
		}
	}
}

/**
 * Place the lines and align the items on the line.
 *
 * \param[in] ctx  Flex layout context
 */
स्थिर शून्य layout_flex_place_ओळी(संरचना flex_ctx *ctx)
{
	bool reversed = ctx->wrap == CSS_FLEX_WRAP_WRAP_REVERSE;
	पूर्णांक ओळ_pos = reversed ? ctx->cross_size : 0;
	पूर्णांक post_multiplier = reversed ? 0 : 1;
	पूर्णांक pre_multiplier = reversed ? -1 : 0;
	पूर्णांक extra_remainder = 0;
	पूर्णांक extra = 0;

	जर (ctx->available_cross != AUTO &&
	    ctx->available_cross > ctx->cross_size &&
	    ctx->ओळ.मोजणी > 0) {
		extra = ctx->available_cross - ctx->cross_size;

		extra_remainder = extra % ctx->ओळ.मोजणी;
		extra /= ctx->ओळ.मोजणी;
	}

	साठी (माप_प्रकार i = 0; i < ctx->ओळ.मोजणी; i++) {
		संरचना flex_ओळ_data *ओळ = &ctx->ओळ.data[i];

		ओळ_pos += pre_multiplier * ओळ->cross_size;
		ओळ->pos = ओळ_pos;
		ओळ_pos += post_multiplier * ओळ->cross_size +
				extra + extra_remainder;

		layout_flex_place_ओळ_वस्तुसंच_cross(ctx, ओळ,
				extra + extra_remainder);

		जर (extra_remainder > 0) {
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
bool layout_flex(संरचना box *flex, पूर्णांक available_width,
		html_content *content)
{
	पूर्णांक max_height, min_height;
	संरचना flex_ctx *ctx;
	bool success = false;

	ctx = layout_flex_ctx__create(content, flex);
	जर (ctx == रिक्त) {
		परत false;
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

	जर (ctx->horizontal) {
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
	success = layout_flex_collect_वस्तुसंच_into_ओळी(ctx);
	जर (!success) {
		जा cleanup;
	}

	layout_flex_place_ओळी(ctx);

	जर (flex->height == AUTO) {
		flex->height = ctx->horizontal ?
				ctx->cross_size :
				ctx->main_size;
	}

	जर (flex->height != AUTO) {
		जर (max_height >= 0 && flex->height > max_height) {
			flex->height = max_height;
		}
		जर (min_height >  0 && flex->height < min_height) {
			flex->height = min_height;
		}
	}

	success = true;

cleanup:
	layout_flex_ctx__destroy(ctx);

	NSLOG(flex, DEEPDEBUG, "box %p: %s: w: %i, h: %i", flex,
			success ? "success" : "failure",
			flex->width, flex->height);
	परत success;
}
