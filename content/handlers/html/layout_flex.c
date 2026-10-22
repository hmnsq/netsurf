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

#समावेश <string.h>

#समावेश "utils/log.h"
#समावेश "utils/utils.h"

#समावेश "html/box.h"
#समावेश "html/html.h"
#समावेश "html/private.h"
#समावेश "html/box_inspect.h"
#समावेश "html/layout_internal.h"

/**
 * Flex item data
 */
काष्ठा flex_item_तथ्य {
	क्रमागत css_flex_basis_e basis;
	css_fixed basis_लंबाई;
	css_unit basis_unit;
	काष्ठा box *box;

	css_fixed shrink;
	css_fixed grow;

	पूर्णांक min_main;
	पूर्णांक max_main;
	पूर्णांक min_cross;
	पूर्णांक max_cross;

	पूर्णांक target_main_आकार;
	पूर्णांक base_आकार;
	पूर्णांक main_आकार;
	माप_प्रकार पंक्ति;

	bool freeze;
	bool min_violation;
	bool max_violation;
};

/**
 * Flex line data
 */
काष्ठा flex_पंक्ति_तथ्य {
	पूर्णांक main_आकार;
	पूर्णांक cross_आकार;

	पूर्णांक used_main_आकार;
	पूर्णांक main_auto_margin_गिनती;

	पूर्णांक pos;

	माप_प्रकार first;
	माप_प्रकार गिनती;
	माप_प्रकार frozen;
};

/**
 * Flex layout context
 */
काष्ठा flex_ctx {
	html_content *content;
	स्थिर काष्ठा box *flex;
	स्थिर css_unit_ctx *unit_len_ctx;

	पूर्णांक main_आकार;
	पूर्णांक cross_आकार;

	पूर्णांक available_main;
	पूर्णांक available_cross;

	bool horizontal;
	bool main_reversed;
	क्रमागत css_flex_wrap_e wrap;

	काष्ठा flex_वस्तुएँ {
		माप_प्रकार गिनती;
		काष्ठा flex_item_तथ्य *तथ्य;
	} item;

	काष्ठा flex_पंक्तियाँ {
		माप_प्रकार गिनती;
		माप_प्रकार alloc;
		काष्ठा flex_पंक्ति_तथ्य *तथ्य;
	} पंक्ति;
};

/**
 * Destroy a flex layout context
 *
 * \param[in] ctx  Flex layout context
 */
अटल व्योम layout_flex_ctx__destroy(काष्ठा flex_ctx *ctx)
{
	अगर (ctx != शून्य) {
		मुक्त(ctx->item.तथ्य);
		मुक्त(ctx->पंक्ति.तथ्य);
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
अटल काष्ठा flex_ctx *layout_flex_ctx__create(
		html_content *content,
		स्थिर काष्ठा box *flex)
{
	काष्ठा flex_ctx *ctx;

	ctx = सुस्मृति(1, माप(*ctx));
	अगर (ctx == शून्य) {
		वापस शून्य;
	}
	ctx->पंक्ति.alloc = 1;

	ctx->item.गिनती = box_गिनती_children(flex);
	ctx->item.तथ्य = सुस्मृति(ctx->item.गिनती, माप(*ctx->item.तथ्य));
	अगर (ctx->item.तथ्य == शून्य) {
		layout_flex_ctx__destroy(ctx);
		वापस शून्य;
	}

	ctx->पंक्ति.alloc = 1;
	ctx->पंक्ति.तथ्य = सुस्मृति(ctx->पंक्ति.alloc, माप(*ctx->पंक्ति.तथ्य));
	अगर (ctx->पंक्ति.तथ्य == शून्य) {
		layout_flex_ctx__destroy(ctx);
		वापस शून्य;
	}

	ctx->flex = flex;
	ctx->content = content;
	ctx->unit_len_ctx = &content->unit_len_ctx;

	ctx->wrap = css_computed_flex_wrap(flex->style);
	ctx->horizontal = lh__flex_main_is_horizontal(flex);
	ctx->main_reversed = lh__flex_direction_reversed(flex);

	वापस ctx;
}

/**
 * Find box side representing the start of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the start side.
 */
अटल क्रमागत box_side layout_flex__main_start_side(
		स्थिर काष्ठा flex_ctx *ctx)
{
	अगर (ctx->horizontal) {
		वापस (ctx->main_reversed) ? दायाँ_2 : बायाँ_2;
	} अन्यथा {
		वापस (ctx->main_reversed) ? BOTTOM : शीर्ष_2;
	}
}

/**
 * Find box side representing the end of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the end side.
 */
अटल क्रमागत box_side layout_flex__main_end_side(
		स्थिर काष्ठा flex_ctx *ctx)
{
	अगर (ctx->horizontal) {
		वापस (ctx->main_reversed) ? बायाँ_2 : दायाँ_2;
	} अन्यथा {
		वापस (ctx->main_reversed) ? शीर्ष_2 : BOTTOM;
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
अटल bool layout_flex_item(
		स्थिर काष्ठा flex_ctx *ctx,
		स्थिर काष्ठा flex_item_तथ्य *item,
		पूर्णांक available_width)
{
	bool success;
	काष्ठा box *b = item->box;

	चयन (b->type) {
	हाल BOX_BLOCK:
		success = layout_block_context(b, -1, ctx->content);
		अवरोध;
	हाल BOX_TABLE:
		b->float_container = b->parent;
		success = layout_table(b, available_width, ctx->content);
		b->float_container = शून्य;
		अवरोध;
	हाल BOX_FLEX:
		b->float_container = b->parent;
		success = layout_flex(b, available_width, ctx->content);
		b->float_container = शून्य;
		अवरोध;
	शेष:
		निश्चित(0 && "Bad flex item back type");
		success = false;
		अवरोध;
	}

	अगर (!success) {
		NSLOG(flex, ERROR, "box %p: layout failed", b);
	}

	वापस success;
}

/**
 * Calculate an item's base and target main sizes.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] item             Item to get sizes of
 * \param[in] available_width  Available width in pixels
 * \return true on success false on failure
 */
अटल अंतरभूत bool layout_flex__base_and_main_sizes(
		स्थिर काष्ठा flex_ctx *ctx,
		काष्ठा flex_item_तथ्य *item,
		पूर्णांक available_width)
{
	काष्ठा box *b = item->box;
	पूर्णांक content_min_width = b->min_width;
	पूर्णांक content_max_width = b->max_width;
	पूर्णांक delta_outer_main = lh__delta_outer_main(ctx->flex, b);

	NSLOG(flex, DEEPDEBUG, "box %p: delta_outer_main: %i",
			b, delta_outer_main);

	अगर (item->basis == CSS_FLEX_BASIS_SET) {
		अगर (item->basis_unit == CSS_UNIT_PCT) {
			item->base_आकार = FPCT_OF_INT_TOINT(
					item->basis_लंबाई,
					available_width);
		} अन्यथा {
			item->base_आकार = FIXTOINT(css_unit_len2device_px(
					b->style, ctx->unit_len_ctx,
					item->basis_लंबाई,
					item->basis_unit));
		}

	} अन्यथा अगर (item->basis == CSS_FLEX_BASIS_AUTO) {
		item->base_आकार = ctx->horizontal ? b->width : b->height;
	} अन्यथा {
		item->base_आकार = AUTO;
	}

	अगर (ctx->horizontal == false) {
		अगर (b->width == AUTO) {
			b->width = min(max(content_min_width, available_width),
					content_max_width);
			b->width -= lh__delta_outer_width(b);
		}

		अगर (!layout_flex_item(ctx, item, b->width)) {
			वापस false;
		}
	}

	अगर (item->base_आकार == AUTO) {
		अगर (ctx->horizontal == false) {
			item->base_आकार = b->height;
		} अन्यथा {
			item->base_आकार = content_max_width - delta_outer_main;
		}
	}

	item->base_आकार += delta_outer_main;

	अगर (ctx->horizontal) {
		item->base_आकार = min(item->base_आकार, available_width);
		item->base_आकार = max(item->base_आकार, content_min_width);
	}

	item->target_main_आकार = item->base_आकार;
	item->main_आकार = item->base_आकार;

	अगर (item->max_main > 0 &&
	    item->main_आकार > item->max_main + delta_outer_main) {
		item->main_आकार = item->max_main + delta_outer_main;
	}

	अगर (item->main_आकार < item->min_main + delta_outer_main) {
		item->main_आकार = item->min_main + delta_outer_main;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: base_size: %i, main_size %i",
			b, item->base_आकार, item->main_आकार);

	वापस true;
}

/**
 * Fill out all item's data in a flex container.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] flex             Flex box
 * \param[in] available_width  Available width in pixels
 */
अटल व्योम layout_flex_ctx_populate_item_तथ्य(
		स्थिर काष्ठा flex_ctx *ctx,
		स्थिर काष्ठा box *flex,
		पूर्णांक available_width)
{
	माप_प्रकार i = 0;
	bool horizontal = ctx->horizontal;

	क्रम (काष्ठा box *b = flex->children; b != शून्य; b = b->अग्र) {
		काष्ठा flex_item_तथ्य *item = &ctx->item.तथ्य[i++];

		b->float_container = b->parent;
		layout_find_dimensions(ctx->unit_len_ctx, available_width, -1,
				b, b->style, &b->width, &b->height,
				horizontal ? &item->max_main : &item->max_cross,
				horizontal ? &item->min_main : &item->min_cross,
				horizontal ? &item->max_cross : &item->max_main,
				horizontal ? &item->min_cross : &item->min_main,
				b->margin, b->padding, b->border);
		b->float_container = शून्य;

		NSLOG(flex, DEEPDEBUG, "flex-item box: %p: width: %i",
				b, b->width);

		item->box = b;
		item->basis = css_computed_flex_basis(b->style,
				&item->basis_लंबाई, &item->basis_unit);

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
अटल bool layout_flex_ctx_ensure_पंक्ति(काष्ठा flex_ctx *ctx)
{
	काष्ठा flex_पंक्ति_तथ्य *अस्थायी;
	माप_प्रकार पंक्ति_alloc = ctx->पंक्ति.alloc * 2;

	अगर (ctx->पंक्ति.alloc > ctx->पंक्ति.गिनती) {
		वापस true;
	}

	अस्थायी = पुनः_स्मृति(ctx->पंक्ति.तथ्य, माप(*ctx->पंक्ति.तथ्य) * पंक्ति_alloc);
	अगर (अस्थायी == शून्य) {
		वापस false;
	}
	ctx->पंक्ति.तथ्य = अस्थायी;

	स_रखो(ctx->पंक्ति.तथ्य + ctx->पंक्ति.alloc, 0,
	       माप(*ctx->पंक्ति.तथ्य) * (पंक्ति_alloc - ctx->पंक्ति.alloc));
	ctx->पंक्ति.alloc = पंक्ति_alloc;

	वापस true;
}

/**
 * Assigns flex items to the line and returns the line
 *
 * \param[in] ctx         Flex layout context
 * \param[in] item_index  Index to first item to assign to this line
 * \return Pointer to the new line, or NULL on error.
 */
अटल काष्ठा flex_पंक्ति_तथ्य *layout_flex_build_पंक्ति(काष्ठा flex_ctx *ctx,
		माप_प्रकार item_सूचकांक)
{
	क्रमागत box_side start_side = layout_flex__main_start_side(ctx);
	क्रमागत box_side end_side = layout_flex__main_end_side(ctx);
	काष्ठा flex_पंक्ति_तथ्य *पंक्ति;
	पूर्णांक used_main = 0;

	अगर (!layout_flex_ctx_ensure_पंक्ति(ctx)) {
		वापस शून्य;
	}

	पंक्ति = &ctx->पंक्ति.तथ्य[ctx->पंक्ति.गिनती];
	पंक्ति->first = item_सूचकांक;

	NSLOG(flex, DEEPDEBUG, "flex container %p: available main: %i",
			ctx->flex, ctx->available_main);

	जबतक (item_सूचकांक < ctx->item.गिनती) {
		काष्ठा flex_item_तथ्य *item = &ctx->item.तथ्य[item_सूचकांक];
		काष्ठा box *b = item->box;
		पूर्णांक pos_main;

		pos_main = ctx->horizontal ?
				item->main_आकार :
				b->height + lh__delta_outer_main(ctx->flex, b);

		अगर (ctx->wrap == CSS_FLEX_WRAP_NOWRAP ||
		    pos_main + used_main <= ctx->available_main ||
		    lh__box_is_absolute(item->box) ||
		    ctx->available_main == AUTO ||
		    पंक्ति->गिनती == 0 ||
		    pos_main == 0) {
			अगर (lh__box_is_absolute(item->box) == false) {
				पंक्ति->main_आकार += item->main_आकार;
				used_main += pos_main;

				अगर (b->margin[start_side] == AUTO) {
					पंक्ति->main_auto_margin_गिनती++;
				}
				अगर (b->margin[end_side] == AUTO) {
					पंक्ति->main_auto_margin_गिनती++;
				}
			}
			item->पंक्ति = ctx->पंक्ति.गिनती;
			पंक्ति->गिनती++;
			item_सूचकांक++;
		} अन्यथा {
			अवरोध;
		}
	}

	अगर (पंक्ति->गिनती > 0) {
		ctx->पंक्ति.गिनती++;
	} अन्यथा {
		NSLOG(layout, ERROR, "Failed to fit any flex items");
	}

	वापस पंक्ति;
}

/**
 * Freeze an item on a line
 *
 * \param[in] line  Line to containing item
 * \param[in] item  Item to freeze
 */
अटल अंतरभूत व्योम layout_flex__item_freeze(
		काष्ठा flex_पंक्ति_तथ्य *पंक्ति,
		काष्ठा flex_item_तथ्य *item)
{
	item->freeze = true;
	पंक्ति->frozen++;

	अगर (!lh__box_is_absolute(item->box)){
		पंक्ति->used_main_आकार += item->target_main_आकार;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: "
			"Frozen at target_main_size: %i",
			item->box, item->target_main_आकार);
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
अटल अंतरभूत पूर्णांक layout_flex__remaining_free_main(
		काष्ठा flex_ctx *ctx,
		काष्ठा flex_पंक्ति_तथ्य *पंक्ति,
		css_fixed *unfrozen_factor_योग,
		पूर्णांक initial_free_main,
		पूर्णांक available_main,
		bool grow)
{
	पूर्णांक remaining_free_main = available_main;
	माप_प्रकार item_गिनती = पंक्ति->first + पंक्ति->गिनती;

	*unfrozen_factor_योग = 0;

	क्रम (माप_प्रकार i = पंक्ति->first; i < item_गिनती; i++) {
		काष्ठा flex_item_तथ्य *item = &ctx->item.तथ्य[i];

		अगर (item->freeze) {
			remaining_free_main -= item->target_main_आकार;
		} अन्यथा {
			remaining_free_main -= item->base_आकार;

			*unfrozen_factor_योग += grow ?
					item->grow : item->shrink;
		}
	}

	अगर (*unfrozen_factor_योग < F_1) {
		पूर्णांक free_space = FIXTOINT(FMUL(INTTOFIX(initial_free_main),
				*unfrozen_factor_योग));

		अगर (free_space < remaining_free_main) {
			remaining_free_main = free_space;
		}
	}

	NSLOG(flex, DEEPDEBUG, "Remaining free space: %i",
			remaining_free_main);

	वापस remaining_free_main;
}

/**
 * Clamp flex item target main size and get min/max violations
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to align items on
 * return total violation in pixels
 */
अटल अंतरभूत पूर्णांक layout_flex__get_min_max_violations(
		काष्ठा flex_ctx *ctx,
		काष्ठा flex_पंक्ति_तथ्य *पंक्ति)
{

	पूर्णांक योग_violation = 0;
	माप_प्रकार item_गिनती = पंक्ति->first + पंक्ति->गिनती;

	क्रम (माप_प्रकार i = पंक्ति->first; i < item_गिनती; i++) {
		काष्ठा flex_item_तथ्य *item = &ctx->item.तथ्य[i];
		पूर्णांक target_main_आकार = item->target_main_आकार;

		NSLOG(flex, DEEPDEBUG, "item %p: target_main_size: %i",
					item->box, target_main_आकार);

		अगर (item->freeze) {
			जारी;
		}

		अगर (item->max_main > 0 &&
		    target_main_आकार > item->max_main) {
			target_main_आकार = item->max_main;
			item->max_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: max_main: %i",
					item->max_main);
		}

		अगर (target_main_आकार < item->min_main) {
			target_main_आकार = item->min_main;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: min_main: %i",
					item->min_main);
		}

		अगर (target_main_आकार < item->box->min_width) {
			target_main_आकार = item->box->min_width;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: box min_width: %i",
					item->box->min_width);
		}

		अगर (target_main_आकार < 0) {
			target_main_आकार = 0;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: less than 0");
		}

		योग_violation += target_main_आकार - item->target_main_आकार;
		item->target_main_आकार = target_main_आकार;
	}

	NSLOG(flex, DEEPDEBUG, "Total violation: %i", योग_violation);

	वापस योग_violation;
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
अटल अंतरभूत व्योम layout_flex__distribute_free_main(
		काष्ठा flex_ctx *ctx,
		काष्ठा flex_पंक्ति_तथ्य *पंक्ति,
		css_fixed unfrozen_factor_योग,
		पूर्णांक remaining_free_main,
		bool grow)
{
	माप_प्रकार item_गिनती = पंक्ति->first + पंक्ति->गिनती;

	अगर (grow) {
		css_fixed remainder = 0;
		क्रम (माप_प्रकार i = पंक्ति->first; i < item_गिनती; i++) {
			काष्ठा flex_item_तथ्य *item = &ctx->item.तथ्य[i];
			css_fixed परिणाम;
			css_fixed ratio;

			अगर (item->freeze) {
				जारी;
			}

			ratio = FDIV(item->grow, unfrozen_factor_योग);
			परिणाम = FMUL(INTTOFIX(remaining_free_main), ratio) +
					remainder;

			item->target_main_आकार = item->base_आकार +
					FIXTOINT(परिणाम);
			remainder = FIXFRAC(परिणाम);
		}
	} अन्यथा {
		css_fixed scaled_shrink_factor_योग = 0;
		css_fixed remainder = 0;

		क्रम (माप_प्रकार i = पंक्ति->first; i < item_गिनती; i++) {
			काष्ठा flex_item_तथ्य *item = &ctx->item.तथ्य[i];
			css_fixed scaled_shrink_factor;

			अगर (item->freeze) {
				जारी;
			}

			scaled_shrink_factor = FMUL(
					item->shrink,
					INTTOFIX(item->base_आकार));
			scaled_shrink_factor_योग += scaled_shrink_factor;
		}

		क्रम (माप_प्रकार i = पंक्ति->first; i < item_गिनती; i++) {
			काष्ठा flex_item_तथ्य *item = &ctx->item.तथ्य[i];
			css_fixed scaled_shrink_factor;
			css_fixed परिणाम;
			css_fixed ratio;

			अगर (item->freeze) {
				जारी;
			} अन्यथा अगर (scaled_shrink_factor_योग == 0) {
				item->target_main_आकार = item->main_आकार;
				layout_flex__item_freeze(पंक्ति, item);
				जारी;
			}

			scaled_shrink_factor = FMUL(
					item->shrink,
					INTTOFIX(item->base_आकार));
			ratio = FDIV(scaled_shrink_factor,
			             scaled_shrink_factor_योग);
			परिणाम = FMUL(INTTOFIX(असल(remaining_free_main)),
			              ratio) + remainder;

			item->target_main_आकार = item->base_आकार -
					FIXTOINT(परिणाम);
			remainder = FIXFRAC(परिणाम);
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
अटल bool layout_flex_resolve_पंक्ति(
		काष्ठा flex_ctx *ctx,
		काष्ठा flex_पंक्ति_तथ्य *पंक्ति)
{
	माप_प्रकार item_गिनती = पंक्ति->first + पंक्ति->गिनती;
	पूर्णांक available_main = ctx->available_main;
	पूर्णांक initial_free_main;
	bool grow;

	अगर (available_main == AUTO) {
		available_main = पूर्णांक_उच्च;
	}

	grow = (पंक्ति->main_आकार < available_main);
	initial_free_main = available_main;

	NSLOG(flex, DEEPDEBUG, "box %p: line %zu: first: %zu, count: %zu",
			ctx->flex, पंक्ति - ctx->पंक्ति.तथ्य,
			पंक्ति->first, पंक्ति->गिनती);
	NSLOG(flex, DEEPDEBUG, "Line main_size: %i, available_main: %i",
			पंक्ति->main_आकार, available_main);

	क्रम (माप_प्रकार i = पंक्ति->first; i < item_गिनती; i++) {
		काष्ठा flex_item_तथ्य *item = &ctx->item.तथ्य[i];

		/* 3. Size inflexible items */
		अगर (grow) {
			अगर (item->grow == 0 ||
			    item->base_आकार > item->main_आकार) {
				item->target_main_आकार = item->main_आकार;
				layout_flex__item_freeze(पंक्ति, item);
			}
		} अन्यथा {
			अगर (item->shrink == 0 ||
			    item->base_आकार < item->main_आकार) {
				item->target_main_आकार = item->main_आकार;
				layout_flex__item_freeze(पंक्ति, item);
			}
		}

		/* 4. Calculate initial free space */
		अगर (item->freeze) {
			initial_free_main -= item->target_main_आकार;
		} अन्यथा {
			initial_free_main -= item->base_आकार;
		}
	}

	/* 5. Loop */
	जबतक (पंक्ति->frozen < पंक्ति->गिनती) {
		css_fixed unfrozen_factor_योग;
		पूर्णांक remaining_free_main;
		पूर्णांक योग_violation;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: Resolver pass",
				ctx->flex);

		/* b */
		remaining_free_main = layout_flex__remaining_free_main(ctx,
				पंक्ति, &unfrozen_factor_योग, initial_free_main,
				available_main, grow);

		/* c */
		अगर (remaining_free_main != 0) {
			layout_flex__distribute_free_main(ctx,
					पंक्ति, unfrozen_factor_योग,
					remaining_free_main, grow);
		}

		/* d */
		योग_violation = layout_flex__get_min_max_violations(
				ctx, पंक्ति);

		/* e */
		क्रम (माप_प्रकार i = पंक्ति->first; i < item_गिनती; i++) {
			काष्ठा flex_item_तथ्य *item = &ctx->item.तथ्य[i];

			अगर (item->freeze) {
				जारी;
			}

			अगर (योग_violation == 0 ||
			    (योग_violation > 0 && item->min_violation) ||
			    (योग_violation < 0 && item->max_violation)) {
				layout_flex__item_freeze(पंक्ति, item);
			}
		}
	}

	वापस true;
}

/**
 * Position items along a line
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to resolve
 * \return true on success, false on failure.
 */
अटल bool layout_flex_place_पंक्ति_वस्तुएँ_main(
		काष्ठा flex_ctx *ctx,
		काष्ठा flex_पंक्ति_तथ्य *पंक्ति)
{
	पूर्णांक main_pos = ctx->flex->padding[layout_flex__main_start_side(ctx)];
	पूर्णांक post_multiplier = ctx->main_reversed ? 0 : 1;
	पूर्णांक pre_multiplier = ctx->main_reversed ? -1 : 0;
	माप_प्रकार item_गिनती = पंक्ति->first + पंक्ति->गिनती;
	पूर्णांक extra_remainder = 0;
	पूर्णांक extra = 0;

	अगर (ctx->main_reversed) {
		main_pos = lh_box_आकार_main(ctx->horizontal, ctx->flex) -
				main_pos;
	}

	अगर (ctx->available_main != AUTO &&
	    ctx->available_main != UNKNOWN_WIDTH &&
	    ctx->available_main > पंक्ति->used_main_आकार) {
		अगर (पंक्ति->main_auto_margin_गिनती > 0) {
			extra = ctx->available_main - पंक्ति->used_main_आकार;

			extra_remainder = extra % पंक्ति->main_auto_margin_गिनती;
			extra /= पंक्ति->main_auto_margin_गिनती;
		}
	}

	क्रम (माप_प्रकार i = पंक्ति->first; i < item_गिनती; i++) {
		क्रमागत box_side main_end = ctx->horizontal ? दायाँ_2 : BOTTOM;
		क्रमागत box_side main_start = ctx->horizontal ? बायाँ_2 : शीर्ष_2;
		काष्ठा flex_item_तथ्य *item = &ctx->item.तथ्य[i];
		काष्ठा box *b = item->box;
		पूर्णांक extra_योग = 0;
		पूर्णांक extra_post = 0;
		पूर्णांक extra_pre = 0;
		पूर्णांक box_आकार_main;
		पूर्णांक *box_pos_main;

		अगर (ctx->horizontal) {
			b->width = item->target_main_आकार -
					lh__delta_outer_width(b);

			अगर (!layout_flex_item(ctx, item, b->width)) {
				वापस false;
			}
		}

		box_आकार_main = lh_box_आकार_main(ctx->horizontal, b);
		box_pos_main = ctx->horizontal ? &b->x : &b->y;

		अगर (!lh__box_is_absolute(b)) {
			अगर (b->margin[main_start] == AUTO) {
				extra_pre = extra + extra_remainder;
			}
			अगर (b->margin[main_end] == AUTO) {
				extra_post = extra + extra_remainder;
			}
			extra_योग = extra_pre + extra_post;

			main_pos += pre_multiplier *
					(extra_योग + box_आकार_main +
					 lh__delta_outer_main(ctx->flex, b));
		}

		*box_pos_main = main_pos + lh__non_auto_margin(b, main_start) +
				extra_pre + b->border[main_start].width;

		अगर (!lh__box_is_absolute(b)) {
			पूर्णांक cross_आकार;
			पूर्णांक box_आकार_cross = lh_box_आकार_cross(
					ctx->horizontal, b);

			main_pos += post_multiplier *
					(extra_योग + box_आकार_main +
					 lh__delta_outer_main(ctx->flex, b));

			cross_आकार = box_आकार_cross + lh__delta_outer_cross(
					ctx->flex, b);
			अगर (पंक्ति->cross_आकार < cross_आकार) {
				पंक्ति->cross_आकार = cross_आकार;
			}
		}
	}

	वापस true;
}

/**
 * Collect items onto lines and place items along the lines
 *
 * \param[in] ctx   Flex layout context
 * \return true on success, false on failure.
 */
अटल bool layout_flex_collect_वस्तुएँ_into_पंक्तियाँ(
		काष्ठा flex_ctx *ctx)
{
	माप_प्रकार pos = 0;

	जबतक (pos < ctx->item.गिनती) {
		काष्ठा flex_पंक्ति_तथ्य *पंक्ति;

		पंक्ति = layout_flex_build_पंक्ति(ctx, pos);
		अगर (पंक्ति == शून्य) {
			वापस false;
		}

		pos += पंक्ति->गिनती;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: "
				"fitted: %zu (total: %zu/%zu)",
				ctx->flex, पंक्ति->गिनती,
				pos, ctx->item.गिनती);

		अगर (!layout_flex_resolve_पंक्ति(ctx, पंक्ति)) {
			वापस false;
		}

		अगर (!layout_flex_place_पंक्ति_वस्तुएँ_main(ctx, पंक्ति)) {
			वापस false;
		}

		ctx->cross_आकार += पंक्ति->cross_आकार;
		अगर (ctx->main_आकार < पंक्ति->main_आकार) {
			ctx->main_आकार = पंक्ति->main_आकार;
		}
	}

	वापस true;
}

/**
 * Align items on a line.
 *
 * \param[in] ctx    Flex layout context
 * \param[in] line   Line to align items on
 * \param[in] extra  Extra line width in pixels
 */
अटल व्योम layout_flex_place_पंक्ति_वस्तुएँ_cross(काष्ठा flex_ctx *ctx,
		काष्ठा flex_पंक्ति_तथ्य *पंक्ति, पूर्णांक extra)
{
	क्रमागत box_side cross_start = ctx->horizontal ? शीर्ष_2 : बायाँ_2;
	माप_प्रकार item_गिनती = पंक्ति->first + पंक्ति->गिनती;

	क्रम (माप_प्रकार i = पंक्ति->first; i < item_गिनती; i++) {
		काष्ठा flex_item_तथ्य *item = &ctx->item.तथ्य[i];
		काष्ठा box *b = item->box;
		पूर्णांक cross_free_space;
		पूर्णांक *box_आकार_cross;
		पूर्णांक *box_pos_cross;

		box_pos_cross = ctx->horizontal ? &b->y : &b->x;
		box_आकार_cross = lh_box_आकार_cross_ptr(ctx->horizontal, b);

		cross_free_space = पंक्ति->cross_आकार + extra - *box_आकार_cross -
				lh__delta_outer_cross(ctx->flex, b);

		चयन (lh__box_align_self(ctx->flex, b)) {
		शेष:
		हाल CSS_ALIGN_SELF_STRETCH:
			अगर (lh_box_आकार_cross_is_auto(ctx->horizontal, b)) {
				*box_आकार_cross += cross_free_space;

				/* Relayout children for stretch. */
				अगर (!layout_flex_item(ctx, item, b->width)) {
					वापस;
				}
			}
			fallthrough;
		हाल CSS_ALIGN_SELF_FLEX_START:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					पंक्ति->pos +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			अवरोध;

		हाल CSS_ALIGN_SELF_FLEX_END:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					पंक्ति->pos + cross_free_space +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			अवरोध;

		हाल CSS_ALIGN_SELF_BASELINE:
		हाल CSS_ALIGN_SELF_CENTER:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					पंक्ति->pos + cross_free_space / 2 +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].width;
			अवरोध;
		}
	}
}

/**
 * Place the lines and align the items on the line.
 *
 * \param[in] ctx  Flex layout context
 */
अटल व्योम layout_flex_place_पंक्तियाँ(काष्ठा flex_ctx *ctx)
{
	bool reversed = ctx->wrap == CSS_FLEX_WRAP_WRAP_REVERSE;
	पूर्णांक पंक्ति_pos = reversed ? ctx->cross_आकार : 0;
	पूर्णांक post_multiplier = reversed ? 0 : 1;
	पूर्णांक pre_multiplier = reversed ? -1 : 0;
	पूर्णांक extra_remainder = 0;
	पूर्णांक extra = 0;

	अगर (ctx->available_cross != AUTO &&
	    ctx->available_cross > ctx->cross_आकार &&
	    ctx->पंक्ति.गिनती > 0) {
		extra = ctx->available_cross - ctx->cross_आकार;

		extra_remainder = extra % ctx->पंक्ति.गिनती;
		extra /= ctx->पंक्ति.गिनती;
	}

	क्रम (माप_प्रकार i = 0; i < ctx->पंक्ति.गिनती; i++) {
		काष्ठा flex_पंक्ति_तथ्य *पंक्ति = &ctx->पंक्ति.तथ्य[i];

		पंक्ति_pos += pre_multiplier * पंक्ति->cross_आकार;
		पंक्ति->pos = पंक्ति_pos;
		पंक्ति_pos += post_multiplier * पंक्ति->cross_आकार +
				extra + extra_remainder;

		layout_flex_place_पंक्ति_वस्तुएँ_cross(ctx, पंक्ति,
				extra + extra_remainder);

		अगर (extra_remainder > 0) {
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
bool layout_flex(काष्ठा box *flex, पूर्णांक available_width,
		html_content *content)
{
	पूर्णांक max_height, min_height;
	काष्ठा flex_ctx *ctx;
	bool success = false;

	ctx = layout_flex_ctx__create(content, flex);
	अगर (ctx == शून्य) {
		वापस false;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: %s, available_width %i, width: %i",
			flex, ctx->horizontal ? "horizontal" : "vertical",
			available_width, flex->width);

	layout_find_dimensions(
			ctx->unit_len_ctx, available_width, -1,
			flex, flex->style, शून्य, &flex->height,
			शून्य, शून्य, &max_height, &min_height,
			flex->margin, flex->padding, flex->border);

	available_width = min(available_width, flex->width);

	अगर (ctx->horizontal) {
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

	layout_flex_ctx_populate_item_तथ्य(ctx, flex, available_width);

	/* Place items onto lines. */
	success = layout_flex_collect_वस्तुएँ_into_पंक्तियाँ(ctx);
	अगर (!success) {
		जाओ cleanup;
	}

	layout_flex_place_पंक्तियाँ(ctx);

	अगर (flex->height == AUTO) {
		flex->height = ctx->horizontal ?
				ctx->cross_आकार :
				ctx->main_आकार;
	}

	अगर (flex->height != AUTO) {
		अगर (max_height >= 0 && flex->height > max_height) {
			flex->height = max_height;
		}
		अगर (min_height >  0 && flex->height < min_height) {
			flex->height = min_height;
		}
	}

	success = true;

cleanup:
	layout_flex_ctx__destroy(ctx);

	NSLOG(flex, DEEPDEBUG, "box %p: %s: w: %i, h: %i", flex,
			success ? "success" : "failure",
			flex->width, flex->height);
	वापस success;
}
