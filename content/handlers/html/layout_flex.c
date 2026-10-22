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
संरचना flex_item_दत्त {
	गणना css_flex_basis_e basis;
	css_fixed basis_दीर्घता;
	css_unit basis_unit;
	संरचना box *box;

	css_fixed shrink;
	css_fixed grow;

	पूर्णाङ्क min_main;
	पूर्णाङ्क max_main;
	पूर्णाङ्क min_cross;
	पूर्णाङ्क max_cross;

	पूर्णाङ्क target_main_आकार;
	पूर्णाङ्क आधारः_आकार;
	पूर्णाङ्क main_आकार;
	size_t पङ्क्तिः;

	bool freeze;
	bool min_violation;
	bool max_violation;
};

/**
 * Flex line data
 */
संरचना flex_पङ्क्तिः_दत्त {
	पूर्णाङ्क main_आकार;
	पूर्णाङ्क cross_आकार;

	पूर्णाङ्क प्रयुक्तम्_main_आकार;
	पूर्णाङ्क main_auto_margin_गणनम्;

	पूर्णाङ्क pos;

	size_t first;
	size_t गणनम्;
	size_t frozen;
};

/**
 * Flex layout context
 */
संरचना flex_ctx {
	html_content *content;
	ध्रुव संरचना box *flex;
	ध्रुव css_unit_ctx *unit_len_ctx;

	पूर्णाङ्क main_आकार;
	पूर्णाङ्क cross_आकार;

	पूर्णाङ्क उपलब्धम्_main;
	पूर्णाङ्क उपलब्धम्_cross;

	bool horizontal;
	bool main_reversed;
	गणना css_flex_wrap_e wrap;

	संरचना flex_वस्तूनि {
		size_t गणनम्;
		संरचना flex_item_दत्त *दत्त;
	} item;

	संरचना flex_पङ्क्तयः {
		size_t गणनम्;
		size_t alloc;
		संरचना flex_पङ्क्तिः_दत्त *दत्त;
	} पङ्क्तिः;
};

/**
 * Destroy a flex layout context
 *
 * \param[in] ctx  Flex layout context
 */
स्थिर शून्य layout_flex_ctx__destroy(संरचना flex_ctx *ctx)
{
	यदि (ctx != रिक्त) {
		मुक्त(ctx->item.दत्त);
		मुक्त(ctx->पङ्क्तिः.दत्त);
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

	ctx = शून्यस्मृतिं_देहि(1, परिमाण(*ctx));
	यदि (ctx == रिक्त) {
		प्रत्यागम रिक्त;
	}
	ctx->पङ्क्तिः.alloc = 1;

	ctx->item.गणनम् = box_गणनम्_children(flex);
	ctx->item.दत्त = शून्यस्मृतिं_देहि(ctx->item.गणनम्, परिमाण(*ctx->item.दत्त));
	यदि (ctx->item.दत्त == रिक्त) {
		layout_flex_ctx__destroy(ctx);
		प्रत्यागम रिक्त;
	}

	ctx->पङ्क्तिः.alloc = 1;
	ctx->पङ्क्तिः.दत्त = शून्यस्मृतिं_देहि(ctx->पङ्क्तिः.alloc, परिमाण(*ctx->पङ्क्तिः.दत्त));
	यदि (ctx->पङ्क्तिः.दत्त == रिक्त) {
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
		प्रत्यागम (ctx->main_reversed) ? दक्षिणः_2 : वामः_2;
	} अन्यथा {
		प्रत्यागम (ctx->main_reversed) ? BOTTOM : शिखरम्_2;
	}
}

/**
 * Find box side representing the end of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the end side.
 */
स्थिर गणना box_side layout_flex_main_अन्तः_side(
		ध्रुव संरचना flex_ctx *ctx)
{
	यदि (ctx->horizontal) {
		प्रत्यागम (ctx->main_reversed) ? वामः_2 : दक्षिणः_2;
	} अन्यथा {
		प्रत्यागम (ctx->main_reversed) ? शिखरम्_2 : BOTTOM;
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
		ध्रुव संरचना flex_item_दत्त *item,
		पूर्णाङ्क उपलब्धम्_विस्तारः)
{
	bool success;
	संरचना box *b = item->box;

	विकल्प (b->type) {
	पक्ष BOX_खण्डम्:
		success = layout_खण्डम्_context(b, -1, ctx->content);
		भङ्ग;
	पक्ष BOX_सारणी:
		b->float_container = b->parent;
		success = layout_सारणी(b, उपलब्धम्_विस्तारः, ctx->content);
		b->float_container = रिक्त;
		भङ्ग;
	पक्ष BOX_FLEX:
		b->float_container = b->parent;
		success = layout_flex(b, उपलब्धम्_विस्तारः, ctx->content);
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
स्थिर अन्तःस्थ bool layout_flex_आधारः_and_main_sizes(
		ध्रुव संरचना flex_ctx *ctx,
		संरचना flex_item_दत्त *item,
		पूर्णाङ्क उपलब्धम्_विस्तारः)
{
	संरचना box *b = item->box;
	पूर्णाङ्क content_min_विस्तारः = b->min_विस्तारः;
	पूर्णाङ्क content_max_विस्तारः = b->max_विस्तारः;
	पूर्णाङ्क delta_outer_main = lh__delta_outer_main(ctx->flex, b);

	NSLOG(flex, DEEPDEBUG, "box %p: delta_outer_main: %i",
			b, delta_outer_main);

	यदि (item->basis == CSS_FLEX_BASIS_समुच्चयः) {
		यदि (item->basis_unit == CSS_UNIT_PCT) {
			item->आधारः_आकार = FPCT_सम्बन्धि_INT_TOINT(
					item->basis_दीर्घता,
					उपलब्धम्_विस्तारः);
		} अन्यथा {
			item->आधारः_आकार = FIXTOINT(css_unit_len2device_px(
					b->style, ctx->unit_len_ctx,
					item->basis_दीर्घता,
					item->basis_unit));
		}

	} अन्यथा यदि (item->basis == CSS_FLEX_BASIS_AUTO) {
		item->आधारः_आकार = ctx->horizontal ? b->विस्तारः : b->height;
	} अन्यथा {
		item->आधारः_आकार = AUTO;
	}

	यदि (ctx->horizontal == false) {
		यदि (b->विस्तारः == AUTO) {
			b->विस्तारः = min(max(content_min_विस्तारः, उपलब्धम्_विस्तारः),
					content_max_विस्तारः);
			b->विस्तारः -= lh_delta_outer_विस्तारः(b);
		}

		यदि (!layout_flex_item(ctx, item, b->विस्तारः)) {
			प्रत्यागम false;
		}
	}

	यदि (item->आधारः_आकार == AUTO) {
		यदि (ctx->horizontal == false) {
			item->आधारः_आकार = b->height;
		} अन्यथा {
			item->आधारः_आकार = content_max_विस्तारः - delta_outer_main;
		}
	}

	item->आधारः_आकार += delta_outer_main;

	यदि (ctx->horizontal) {
		item->आधारः_आकार = min(item->आधारः_आकार, उपलब्धम्_विस्तारः);
		item->आधारः_आकार = max(item->आधारः_आकार, content_min_विस्तारः);
	}

	item->target_main_आकार = item->आधारः_आकार;
	item->main_आकार = item->आधारः_आकार;

	यदि (item->max_main > 0 &&
	    item->main_आकार > item->max_main + delta_outer_main) {
		item->main_आकार = item->max_main + delta_outer_main;
	}

	यदि (item->main_आकार < item->min_main + delta_outer_main) {
		item->main_आकार = item->min_main + delta_outer_main;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: base_size: %i, main_size %i",
			b, item->आधारः_आकार, item->main_आकार);

	प्रत्यागम true;
}

/**
 * Fill out all item's data in a flex container.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] flex             Flex box
 * \param[in] available_width  Available width in pixels
 */
स्थिर शून्य layout_flex_ctx_populate_item_दत्त(
		ध्रुव संरचना flex_ctx *ctx,
		ध्रुव संरचना box *flex,
		पूर्णाङ्क उपलब्धम्_विस्तारः)
{
	size_t i = 0;
	bool horizontal = ctx->horizontal;

	प्रति (संरचना box *b = flex->children; b != रिक्त; b = b->अग्र) {
		संरचना flex_item_दत्त *item = &ctx->item.दत्त[i++];

		b->float_container = b->parent;
		layout_find_dimensions(ctx->unit_len_ctx, उपलब्धम्_विस्तारः, -1,
				b, b->style, &b->विस्तारः, &b->height,
				horizontal ? &item->max_main : &item->max_cross,
				horizontal ? &item->min_main : &item->min_cross,
				horizontal ? &item->max_cross : &item->max_main,
				horizontal ? &item->min_cross : &item->min_main,
				b->margin, b->padding, b->border);
		b->float_container = रिक्त;

		NSLOG(flex, DEEPDEBUG, "flex-item box: %p: width: %i",
				b, b->विस्तारः);

		item->box = b;
		item->basis = css_computed_flex_basis(b->style,
				&item->basis_दीर्घता, &item->basis_unit);

		css_computed_flex_shrink(b->style, &item->shrink);
		css_computed_flex_grow(b->style, &item->grow);

		layout_flex_आधारः_and_main_sizes(ctx, item, उपलब्धम्_विस्तारः);
	}
}

/**
 * Ensure context's lines array has a free space
 *
 * \param[in] ctx  Flex layout context
 * \return true on success false on out of memory
 */
स्थिर bool layout_flex_ctx_ensure_पङ्क्तिः(संरचना flex_ctx *ctx)
{
	संरचना flex_पङ्क्तिः_दत्त *अस्थायी;
	size_t पङ्क्तिः_alloc = ctx->पङ्क्तिः.alloc * 2;

	यदि (ctx->पङ्क्तिः.alloc > ctx->पङ्क्तिः.गणनम्) {
		प्रत्यागम true;
	}

	अस्थायी = realloc(ctx->पङ्क्तिः.दत्त, परिमाण(*ctx->पङ्क्तिः.दत्त) * पङ्क्तिः_alloc);
	यदि (अस्थायी == रिक्त) {
		प्रत्यागम false;
	}
	ctx->पङ्क्तिः.दत्त = अस्थायी;

	स्मृतिं_पूरय(ctx->पङ्क्तिः.दत्त + ctx->पङ्क्तिः.alloc, 0,
	       परिमाण(*ctx->पङ्क्तिः.दत्त) * (पङ्क्तिः_alloc - ctx->पङ्क्तिः.alloc));
	ctx->पङ्क्तिः.alloc = पङ्क्तिः_alloc;

	प्रत्यागम true;
}

/**
 * Assigns flex items to the line and returns the line
 *
 * \param[in] ctx         Flex layout context
 * \param[in] item_index  Index to first item to assign to this line
 * \return Pointer to the new line, or NULL on error.
 */
स्थिर संरचना flex_पङ्क्तिः_दत्त *layout_flex_build_पङ्क्तिः(संरचना flex_ctx *ctx,
		size_t item_सूचकांक)
{
	गणना box_side start_side = layout_flex__main_start_side(ctx);
	गणना box_side अन्तः_side = layout_flex_main_अन्तः_side(ctx);
	संरचना flex_पङ्क्तिः_दत्त *पङ्क्तिः;
	पूर्णाङ्क प्रयुक्तम्_main = 0;

	यदि (!layout_flex_ctx_ensure_पङ्क्तिः(ctx)) {
		प्रत्यागम रिक्त;
	}

	पङ्क्तिः = &ctx->पङ्क्तिः.दत्त[ctx->पङ्क्तिः.गणनम्];
	पङ्क्तिः->first = item_सूचकांक;

	NSLOG(flex, DEEPDEBUG, "flex container %p: available main: %i",
			ctx->flex, ctx->उपलब्धम्_main);

	यावत् (item_सूचकांक < ctx->item.गणनम्) {
		संरचना flex_item_दत्त *item = &ctx->item.दत्त[item_सूचकांक];
		संरचना box *b = item->box;
		पूर्णाङ्क pos_main;

		pos_main = ctx->horizontal ?
				item->main_आकार :
				b->height + lh__delta_outer_main(ctx->flex, b);

		यदि (ctx->wrap == CSS_FLEX_WRAP_NOWRAP ||
		    pos_main + प्रयुक्तम्_main <= ctx->उपलब्धम्_main ||
		    lh__box_is_absolute(item->box) ||
		    ctx->उपलब्धम्_main == AUTO ||
		    पङ्क्तिः->गणनम् == 0 ||
		    pos_main == 0) {
			यदि (lh__box_is_absolute(item->box) == false) {
				पङ्क्तिः->main_आकार += item->main_आकार;
				प्रयुक्तम्_main += pos_main;

				यदि (b->margin[start_side] == AUTO) {
					पङ्क्तिः->main_auto_margin_गणनम्++;
				}
				यदि (b->margin[अन्तः_side] == AUTO) {
					पङ्क्तिः->main_auto_margin_गणनम्++;
				}
			}
			item->पङ्क्तिः = ctx->पङ्क्तिः.गणनम्;
			पङ्क्तिः->गणनम्++;
			item_सूचकांक++;
		} अन्यथा {
			भङ्ग;
		}
	}

	यदि (पङ्क्तिः->गणनम् > 0) {
		ctx->पङ्क्तिः.गणनम्++;
	} अन्यथा {
		NSLOG(layout, ERROR, "Failed to fit any flex items");
	}

	प्रत्यागम पङ्क्तिः;
}

/**
 * Freeze an item on a line
 *
 * \param[in] line  Line to containing item
 * \param[in] item  Item to freeze
 */
स्थिर अन्तःस्थ शून्य layout_flex__item_freeze(
		संरचना flex_पङ्क्तिः_दत्त *पङ्क्तिः,
		संरचना flex_item_दत्त *item)
{
	item->freeze = true;
	पङ्क्तिः->frozen++;

	यदि (!lh__box_is_absolute(item->box)){
		पङ्क्तिः->प्रयुक्तम्_main_आकार += item->target_main_आकार;
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
स्थिर अन्तःस्थ पूर्णाङ्क layout_flex__remaining_free_main(
		संरचना flex_ctx *ctx,
		संरचना flex_पङ्क्तिः_दत्त *पङ्क्तिः,
		css_fixed *unfrozen_गुणकः_योग,
		पूर्णाङ्क initial_free_main,
		पूर्णाङ्क उपलब्धम्_main,
		bool grow)
{
	पूर्णाङ्क remaining_free_main = उपलब्धम्_main;
	size_t item_गणनम् = पङ्क्तिः->first + पङ्क्तिः->गणनम्;

	*unfrozen_गुणकः_योग = 0;

	प्रति (size_t i = पङ्क्तिः->first; i < item_गणनम्; i++) {
		संरचना flex_item_दत्त *item = &ctx->item.दत्त[i];

		यदि (item->freeze) {
			remaining_free_main -= item->target_main_आकार;
		} अन्यथा {
			remaining_free_main -= item->आधारः_आकार;

			*unfrozen_गुणकः_योग += grow ?
					item->grow : item->shrink;
		}
	}

	यदि (*unfrozen_गुणकः_योग < F_1) {
		पूर्णाङ्क free_space = FIXTOINT(FMUL(INTTOFIX(initial_free_main),
				*unfrozen_गुणकः_योग));

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
		संरचना flex_पङ्क्तिः_दत्त *पङ्क्तिः)
{

	पूर्णाङ्क योग_violation = 0;
	size_t item_गणनम् = पङ्क्तिः->first + पङ्क्तिः->गणनम्;

	प्रति (size_t i = पङ्क्तिः->first; i < item_गणनम्; i++) {
		संरचना flex_item_दत्त *item = &ctx->item.दत्त[i];
		पूर्णाङ्क target_main_आकार = item->target_main_आकार;

		NSLOG(flex, DEEPDEBUG, "item %p: target_main_size: %i",
					item->box, target_main_आकार);

		यदि (item->freeze) {
			अनुवर्त;
		}

		यदि (item->max_main > 0 &&
		    target_main_आकार > item->max_main) {
			target_main_आकार = item->max_main;
			item->max_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: max_main: %i",
					item->max_main);
		}

		यदि (target_main_आकार < item->min_main) {
			target_main_आकार = item->min_main;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: min_main: %i",
					item->min_main);
		}

		यदि (target_main_आकार < item->box->min_विस्तारः) {
			target_main_आकार = item->box->min_विस्तारः;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: box min_width: %i",
					item->box->min_विस्तारः);
		}

		यदि (target_main_आकार < 0) {
			target_main_आकार = 0;
			item->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: less than 0");
		}

		योग_violation += target_main_आकार - item->target_main_आकार;
		item->target_main_आकार = target_main_आकार;
	}

	NSLOG(flex, DEEPDEBUG, "Total violation: %i", योग_violation);

	प्रत्यागम योग_violation;
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
		संरचना flex_पङ्क्तिः_दत्त *पङ्क्तिः,
		css_fixed unfrozen_गुणकः_योग,
		पूर्णाङ्क remaining_free_main,
		bool grow)
{
	size_t item_गणनम् = पङ्क्तिः->first + पङ्क्तिः->गणनम्;

	यदि (grow) {
		css_fixed remainder = 0;
		प्रति (size_t i = पङ्क्तिः->first; i < item_गणनम्; i++) {
			संरचना flex_item_दत्त *item = &ctx->item.दत्त[i];
			css_fixed परिणाम;
			css_fixed अनुपातः;

			यदि (item->freeze) {
				अनुवर्त;
			}

			अनुपातः = FDIV(item->grow, unfrozen_गुणकः_योग);
			परिणाम = FMUL(INTTOFIX(remaining_free_main), अनुपातः) +
					remainder;

			item->target_main_आकार = item->आधारः_आकार +
					FIXTOINT(परिणाम);
			remainder = FIXFRAC(परिणाम);
		}
	} अन्यथा {
		css_fixed scaled_shrink_गुणकः_योग = 0;
		css_fixed remainder = 0;

		प्रति (size_t i = पङ्क्तिः->first; i < item_गणनम्; i++) {
			संरचना flex_item_दत्त *item = &ctx->item.दत्त[i];
			css_fixed scaled_shrink_गुणकः;

			यदि (item->freeze) {
				अनुवर्त;
			}

			scaled_shrink_गुणकः = FMUL(
					item->shrink,
					INTTOFIX(item->आधारः_आकार));
			scaled_shrink_गुणकः_योग += scaled_shrink_गुणकः;
		}

		प्रति (size_t i = पङ्क्तिः->first; i < item_गणनम्; i++) {
			संरचना flex_item_दत्त *item = &ctx->item.दत्त[i];
			css_fixed scaled_shrink_गुणकः;
			css_fixed परिणाम;
			css_fixed अनुपातः;

			यदि (item->freeze) {
				अनुवर्त;
			} अन्यथा यदि (scaled_shrink_गुणकः_योग == 0) {
				item->target_main_आकार = item->main_आकार;
				layout_flex__item_freeze(पङ्क्तिः, item);
				अनुवर्त;
			}

			scaled_shrink_गुणकः = FMUL(
					item->shrink,
					INTTOFIX(item->आधारः_आकार));
			अनुपातः = FDIV(scaled_shrink_गुणकः,
			             scaled_shrink_गुणकः_योग);
			परिणाम = FMUL(INTTOFIX(निरपेक्षम्(remaining_free_main)),
			              अनुपातः) + remainder;

			item->target_main_आकार = item->आधारः_आकार -
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
स्थिर bool layout_flex_resolve_पङ्क्तिः(
		संरचना flex_ctx *ctx,
		संरचना flex_पङ्क्तिः_दत्त *पङ्क्तिः)
{
	size_t item_गणनम् = पङ्क्तिः->first + पङ्क्तिः->गणनम्;
	पूर्णाङ्क उपलब्धम्_main = ctx->उपलब्धम्_main;
	पूर्णाङ्क initial_free_main;
	bool grow;

	यदि (उपलब्धम्_main == AUTO) {
		उपलब्धम्_main = अधिकतम_पूर्णाङ्कः;
	}

	grow = (पङ्क्तिः->main_आकार < उपलब्धम्_main);
	initial_free_main = उपलब्धम्_main;

	NSLOG(flex, DEEPDEBUG, "box %p: line %zu: first: %zu, count: %zu",
			ctx->flex, पङ्क्तिः - ctx->पङ्क्तिः.दत्त,
			पङ्क्तिः->first, पङ्क्तिः->गणनम्);
	NSLOG(flex, DEEPDEBUG, "Line main_size: %i, available_main: %i",
			पङ्क्तिः->main_आकार, उपलब्धम्_main);

	प्रति (size_t i = पङ्क्तिः->first; i < item_गणनम्; i++) {
		संरचना flex_item_दत्त *item = &ctx->item.दत्त[i];

		/* 3. Size inflexible items */
		यदि (grow) {
			यदि (item->grow == 0 ||
			    item->आधारः_आकार > item->main_आकार) {
				item->target_main_आकार = item->main_आकार;
				layout_flex__item_freeze(पङ्क्तिः, item);
			}
		} अन्यथा {
			यदि (item->shrink == 0 ||
			    item->आधारः_आकार < item->main_आकार) {
				item->target_main_आकार = item->main_आकार;
				layout_flex__item_freeze(पङ्क्तिः, item);
			}
		}

		/* 4. Calculate initial free space */
		यदि (item->freeze) {
			initial_free_main -= item->target_main_आकार;
		} अन्यथा {
			initial_free_main -= item->आधारः_आकार;
		}
	}

	/* 5. Loop */
	यावत् (पङ्क्तिः->frozen < पङ्क्तिः->गणनम्) {
		css_fixed unfrozen_गुणकः_योग;
		पूर्णाङ्क remaining_free_main;
		पूर्णाङ्क योग_violation;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: Resolver pass",
				ctx->flex);

		/* b */
		remaining_free_main = layout_flex__remaining_free_main(ctx,
				पङ्क्तिः, &unfrozen_गुणकः_योग, initial_free_main,
				उपलब्धम्_main, grow);

		/* c */
		यदि (remaining_free_main != 0) {
			layout_flex__distribute_free_main(ctx,
					पङ्क्तिः, unfrozen_गुणकः_योग,
					remaining_free_main, grow);
		}

		/* d */
		योग_violation = layout_flex__get_min_max_violations(
				ctx, पङ्क्तिः);

		/* e */
		प्रति (size_t i = पङ्क्तिः->first; i < item_गणनम्; i++) {
			संरचना flex_item_दत्त *item = &ctx->item.दत्त[i];

			यदि (item->freeze) {
				अनुवर्त;
			}

			यदि (योग_violation == 0 ||
			    (योग_violation > 0 && item->min_violation) ||
			    (योग_violation < 0 && item->max_violation)) {
				layout_flex__item_freeze(पङ्क्तिः, item);
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
स्थिर bool layout_flex_place_पङ्क्तिः_वस्तूनि_main(
		संरचना flex_ctx *ctx,
		संरचना flex_पङ्क्तिः_दत्त *पङ्क्तिः)
{
	पूर्णाङ्क main_pos = ctx->flex->padding[layout_flex__main_start_side(ctx)];
	पूर्णाङ्क post_multiplier = ctx->main_reversed ? 0 : 1;
	पूर्णाङ्क pre_multiplier = ctx->main_reversed ? -1 : 0;
	size_t item_गणनम् = पङ्क्तिः->first + पङ्क्तिः->गणनम्;
	पूर्णाङ्क extra_remainder = 0;
	पूर्णाङ्क extra = 0;

	यदि (ctx->main_reversed) {
		main_pos = lh_box_आकार_main(ctx->horizontal, ctx->flex) -
				main_pos;
	}

	यदि (ctx->उपलब्धम्_main != AUTO &&
	    ctx->उपलब्धम्_main != UNKNOWN_विस्तारः &&
	    ctx->उपलब्धम्_main > पङ्क्तिः->प्रयुक्तम्_main_आकार) {
		यदि (पङ्क्तिः->main_auto_margin_गणनम् > 0) {
			extra = ctx->उपलब्धम्_main - पङ्क्तिः->प्रयुक्तम्_main_आकार;

			extra_remainder = extra % पङ्क्तिः->main_auto_margin_गणनम्;
			extra /= पङ्क्तिः->main_auto_margin_गणनम्;
		}
	}

	प्रति (size_t i = पङ्क्तिः->first; i < item_गणनम्; i++) {
		गणना box_side main_अन्तः = ctx->horizontal ? दक्षिणः_2 : BOTTOM;
		गणना box_side main_start = ctx->horizontal ? वामः_2 : शिखरम्_2;
		संरचना flex_item_दत्त *item = &ctx->item.दत्त[i];
		संरचना box *b = item->box;
		पूर्णाङ्क extra_योग = 0;
		पूर्णाङ्क extra_post = 0;
		पूर्णाङ्क extra_pre = 0;
		पूर्णाङ्क box_आकार_main;
		पूर्णाङ्क *box_pos_main;

		यदि (ctx->horizontal) {
			b->विस्तारः = item->target_main_आकार -
					lh_delta_outer_विस्तारः(b);

			यदि (!layout_flex_item(ctx, item, b->विस्तारः)) {
				प्रत्यागम false;
			}
		}

		box_आकार_main = lh_box_आकार_main(ctx->horizontal, b);
		box_pos_main = ctx->horizontal ? &b->x : &b->y;

		यदि (!lh__box_is_absolute(b)) {
			यदि (b->margin[main_start] == AUTO) {
				extra_pre = extra + extra_remainder;
			}
			यदि (b->margin[main_अन्तः] == AUTO) {
				extra_post = extra + extra_remainder;
			}
			extra_योग = extra_pre + extra_post;

			main_pos += pre_multiplier *
					(extra_योग + box_आकार_main +
					 lh__delta_outer_main(ctx->flex, b));
		}

		*box_pos_main = main_pos + lh__non_auto_margin(b, main_start) +
				extra_pre + b->border[main_start].विस्तारः;

		यदि (!lh__box_is_absolute(b)) {
			पूर्णाङ्क cross_आकार;
			पूर्णाङ्क box_आकार_cross = lh_box_आकार_cross(
					ctx->horizontal, b);

			main_pos += post_multiplier *
					(extra_योग + box_आकार_main +
					 lh__delta_outer_main(ctx->flex, b));

			cross_आकार = box_आकार_cross + lh__delta_outer_cross(
					ctx->flex, b);
			यदि (पङ्क्तिः->cross_आकार < cross_आकार) {
				पङ्क्तिः->cross_आकार = cross_आकार;
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
स्थिर bool layout_flex_collect_वस्तूनि_into_पङ्क्तयः(
		संरचना flex_ctx *ctx)
{
	size_t pos = 0;

	यावत् (pos < ctx->item.गणनम्) {
		संरचना flex_पङ्क्तिः_दत्त *पङ्क्तिः;

		पङ्क्तिः = layout_flex_build_पङ्क्तिः(ctx, pos);
		यदि (पङ्क्तिः == रिक्त) {
			प्रत्यागम false;
		}

		pos += पङ्क्तिः->गणनम्;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: "
				"fitted: %zu (total: %zu/%zu)",
				ctx->flex, पङ्क्तिः->गणनम्,
				pos, ctx->item.गणनम्);

		यदि (!layout_flex_resolve_पङ्क्तिः(ctx, पङ्क्तिः)) {
			प्रत्यागम false;
		}

		यदि (!layout_flex_place_पङ्क्तिः_वस्तूनि_main(ctx, पङ्क्तिः)) {
			प्रत्यागम false;
		}

		ctx->cross_आकार += पङ्क्तिः->cross_आकार;
		यदि (ctx->main_आकार < पङ्क्तिः->main_आकार) {
			ctx->main_आकार = पङ्क्तिः->main_आकार;
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
स्थिर शून्य layout_flex_place_पङ्क्तिः_वस्तूनि_cross(संरचना flex_ctx *ctx,
		संरचना flex_पङ्क्तिः_दत्त *पङ्क्तिः, पूर्णाङ्क extra)
{
	गणना box_side cross_start = ctx->horizontal ? शिखरम्_2 : वामः_2;
	size_t item_गणनम् = पङ्क्तिः->first + पङ्क्तिः->गणनम्;

	प्रति (size_t i = पङ्क्तिः->first; i < item_गणनम्; i++) {
		संरचना flex_item_दत्त *item = &ctx->item.दत्त[i];
		संरचना box *b = item->box;
		पूर्णाङ्क cross_free_space;
		पूर्णाङ्क *box_आकार_cross;
		पूर्णाङ्क *box_pos_cross;

		box_pos_cross = ctx->horizontal ? &b->y : &b->x;
		box_आकार_cross = lh_box_आकार_cross_ptr(ctx->horizontal, b);

		cross_free_space = पङ्क्तिः->cross_आकार + extra - *box_आकार_cross -
				lh__delta_outer_cross(ctx->flex, b);

		विकल्प (lh__box_align_self(ctx->flex, b)) {
		स्वभाव:
		पक्ष CSS_ALIGN_SELF_STRETCH:
			यदि (lh_box_आकार_cross_is_auto(ctx->horizontal, b)) {
				*box_आकार_cross += cross_free_space;

				/* Relayout children for stretch. */
				यदि (!layout_flex_item(ctx, item, b->विस्तारः)) {
					प्रत्यागम;
				}
			}
			fallthrough;
		पक्ष CSS_ALIGN_SELF_FLEX_START:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					पङ्क्तिः->pos +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].विस्तारः;
			भङ्ग;

		पक्ष CSS_ALIGN_SELF_FLEX_अन्तः:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					पङ्क्तिः->pos + cross_free_space +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].विस्तारः;
			भङ्ग;

		पक्ष CSS_ALIGN_SELF_BASELINE:
		पक्ष CSS_ALIGN_SELF_CENTER:
			*box_pos_cross = ctx->flex->padding[cross_start] +
					पङ्क्तिः->pos + cross_free_space / 2 +
					lh__non_auto_margin(b, cross_start) +
					b->border[cross_start].विस्तारः;
			भङ्ग;
		}
	}
}

/**
 * Place the lines and align the items on the line.
 *
 * \param[in] ctx  Flex layout context
 */
स्थिर शून्य layout_flex_place_पङ्क्तयः(संरचना flex_ctx *ctx)
{
	bool reversed = ctx->wrap == CSS_FLEX_WRAP_WRAP_REVERSE;
	पूर्णाङ्क पङ्क्तिः_pos = reversed ? ctx->cross_आकार : 0;
	पूर्णाङ्क post_multiplier = reversed ? 0 : 1;
	पूर्णाङ्क pre_multiplier = reversed ? -1 : 0;
	पूर्णाङ्क extra_remainder = 0;
	पूर्णाङ्क extra = 0;

	यदि (ctx->उपलब्धम्_cross != AUTO &&
	    ctx->उपलब्धम्_cross > ctx->cross_आकार &&
	    ctx->पङ्क्तिः.गणनम् > 0) {
		extra = ctx->उपलब्धम्_cross - ctx->cross_आकार;

		extra_remainder = extra % ctx->पङ्क्तिः.गणनम्;
		extra /= ctx->पङ्क्तिः.गणनम्;
	}

	प्रति (size_t i = 0; i < ctx->पङ्क्तिः.गणनम्; i++) {
		संरचना flex_पङ्क्तिः_दत्त *पङ्क्तिः = &ctx->पङ्क्तिः.दत्त[i];

		पङ्क्तिः_pos += pre_multiplier * पङ्क्तिः->cross_आकार;
		पङ्क्तिः->pos = पङ्क्तिः_pos;
		पङ्क्तिः_pos += post_multiplier * पङ्क्तिः->cross_आकार +
				extra + extra_remainder;

		layout_flex_place_पङ्क्तिः_वस्तूनि_cross(ctx, पङ्क्तिः,
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
bool layout_flex(संरचना box *flex, पूर्णाङ्क उपलब्धम्_विस्तारः,
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
			उपलब्धम्_विस्तारः, flex->विस्तारः);

	layout_find_dimensions(
			ctx->unit_len_ctx, उपलब्धम्_विस्तारः, -1,
			flex, flex->style, रिक्त, &flex->height,
			रिक्त, रिक्त, &max_height, &min_height,
			flex->margin, flex->padding, flex->border);

	उपलब्धम्_विस्तारः = min(उपलब्धम्_विस्तारः, flex->विस्तारः);

	यदि (ctx->horizontal) {
		ctx->उपलब्धम्_main = उपलब्धम्_विस्तारः;
		ctx->उपलब्धम्_cross = ctx->flex->height;
	} अन्यथा {
		ctx->उपलब्धम्_main = ctx->flex->height;
		ctx->उपलब्धम्_cross = उपलब्धम्_विस्तारः;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: available_main: %i",
			flex, ctx->उपलब्धम्_main);
	NSLOG(flex, DEEPDEBUG, "box %p: available_cross: %i",
			flex, ctx->उपलब्धम्_cross);

	layout_flex_ctx_populate_item_दत्त(ctx, flex, उपलब्धम्_विस्तारः);

	/* Place items onto lines. */
	success = layout_flex_collect_वस्तूनि_into_पङ्क्तयः(ctx);
	यदि (!success) {
		गच्छ cleanup;
	}

	layout_flex_place_पङ्क्तयः(ctx);

	यदि (flex->height == AUTO) {
		flex->height = ctx->horizontal ?
				ctx->cross_आकार :
				ctx->main_आकार;
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
			flex->विस्तारः, flex->height);
	प्रत्यागम success;
}
