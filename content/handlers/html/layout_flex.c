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

#ضمن <string.h>

#ضمن "utils/log.h"
#ضمن "utils/utils.h"

#ضمن "html/box.h"
#ضمن "html/html.h"
#ضمن "html/private.h"
#ضمن "html/box_inspect.h"
#ضمن "html/layout_internal.h"

/**
 * Flex item data
 */
بنية flex_شي_معلومات {
	تعداد css_flex_basis_e basis;
	css_fixed basis_ڊيگهائي;
	css_unit basis_unit;
	بنية box *box;

	css_fixed shrink;
	css_fixed grow;

	صحيح min_main;
	صحيح max_main;
	صحيح min_cross;
	صحيح max_cross;

	صحيح هدف_main_سائيز;
	صحيح بنياد_سائيز;
	صحيح main_سائيز;
	size_t سٽ;

	bool freeze;
	bool min_violation;
	bool max_violation;
};

/**
 * Flex line data
 */
بنية flex_سٽ_معلومات {
	صحيح main_سائيز;
	صحيح cross_سائيز;

	صحيح استعمال_ٿيل_main_سائيز;
	صحيح main_auto_margin_ڳڻپ;

	صحيح pos;

	size_t پهريون;
	size_t ڳڻپ;
	size_t frozen;
};

/**
 * Flex layout context
 */
بنية flex_ctx {
	html_content *content;
	ثابت بنية box *flex;
	ثابت css_unit_ctx *unit_len_ctx;

	صحيح main_سائيز;
	صحيح cross_سائيز;

	صحيح موجود_main;
	صحيح موجود_cross;

	bool horizontal;
	bool main_reversed;
	تعداد css_flex_wrap_e wrap;

	بنية flex_شيون {
		size_t ڳڻپ;
		بنية flex_شي_معلومات *معلومات;
	} شي;

	بنية flex_سٽون {
		size_t ڳڻپ;
		size_t alloc;
		بنية flex_سٽ_معلومات *معلومات;
	} سٽ;
};

/**
 * Destroy a flex layout context
 *
 * \param[in] ctx  Flex layout context
 */
ساكن فراغ layout_flex_ctx__destroy(بنية flex_ctx *ctx)
{
	إذا (ctx != عدم) {
		حرر(ctx->شي.معلومات);
		حرر(ctx->سٽ.معلومات);
		حرر(ctx);
	}
}

/**
 * Create a flex layout context
 *
 * \param[in] content  HTML content containing flex box
 * \param[in] flex     Box to create layout context for
 * \return flex layout context or NULL on error
 */
ساكن بنية flex_ctx *layout_flex_ctx__create(
		html_content *content,
		ثابت بنية box *flex)
{
	بنية flex_ctx *ctx;

	ctx = صفر_يادگيري_ڏيو(1, حجم(*ctx));
	إذا (ctx == عدم) {
		أرجع عدم;
	}
	ctx->سٽ.alloc = 1;

	ctx->شي.ڳڻپ = box_ڳڻپ_children(flex);
	ctx->شي.معلومات = صفر_يادگيري_ڏيو(ctx->شي.ڳڻپ, حجم(*ctx->شي.معلومات));
	إذا (ctx->شي.معلومات == عدم) {
		layout_flex_ctx__destroy(ctx);
		أرجع عدم;
	}

	ctx->سٽ.alloc = 1;
	ctx->سٽ.معلومات = صفر_يادگيري_ڏيو(ctx->سٽ.alloc, حجم(*ctx->سٽ.معلومات));
	إذا (ctx->سٽ.معلومات == عدم) {
		layout_flex_ctx__destroy(ctx);
		أرجع عدم;
	}

	ctx->flex = flex;
	ctx->content = content;
	ctx->unit_len_ctx = &content->unit_len_ctx;

	ctx->wrap = css_computed_flex_wrap(flex->style);
	ctx->horizontal = lh__flex_main_is_horizontal(flex);
	ctx->main_reversed = lh__flex_direction_reversed(flex);

	أرجع ctx;
}

/**
 * Find box side representing the start of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the start side.
 */
ساكن تعداد box_side layout_flex_main_شروعات_side(
		ثابت بنية flex_ctx *ctx)
{
	إذا (ctx->horizontal) {
		أرجع (ctx->main_reversed) ? ساڄو_2 : کاٻو_2;
	} وإلا {
		أرجع (ctx->main_reversed) ? BOTTOM : چوٽي_2;
	}
}

/**
 * Find box side representing the end of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the end side.
 */
ساكن تعداد box_side layout_flex_main_پڇاڙي_side(
		ثابت بنية flex_ctx *ctx)
{
	إذا (ctx->horizontal) {
		أرجع (ctx->main_reversed) ? کاٻو_2 : ساڄو_2;
	} وإلا {
		أرجع (ctx->main_reversed) ? چوٽي_2 : BOTTOM;
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
ساكن bool layout_flex_شي(
		ثابت بنية flex_ctx *ctx,
		ثابت بنية flex_شي_معلومات *شي,
		صحيح موجود_ويڪر)
{
	bool success;
	بنية box *b = شي->box;

	بدل (b->type) {
	حالة BOX_بلاڪ:
		success = layout_بلاڪ_context(b, -1, ctx->content);
		اكسر;
	حالة BOX_جدول:
		b->float_container = b->والد;
		success = layout_جدول(b, موجود_ويڪر, ctx->content);
		b->float_container = عدم;
		اكسر;
	حالة BOX_FLEX:
		b->float_container = b->والد;
		success = layout_flex(b, موجود_ويڪر, ctx->content);
		b->float_container = عدم;
		اكسر;
	افتراضي:
		أكد(0 && "Bad flex item back type");
		success = false;
		اكسر;
	}

	إذا (!success) {
		NSLOG(flex, ERROR, "box %p: layout failed", b);
	}

	أرجع success;
}

/**
 * Calculate an item's base and target main sizes.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] item             Item to get sizes of
 * \param[in] available_width  Available width in pixels
 * \return true on success false on failure
 */
ساكن مضمن bool layout_flex_بنياد_and_main_sizes(
		ثابت بنية flex_ctx *ctx,
		بنية flex_شي_معلومات *شي,
		صحيح موجود_ويڪر)
{
	بنية box *b = شي->box;
	صحيح content_min_ويڪر = b->min_ويڪر;
	صحيح content_max_ويڪر = b->max_ويڪر;
	صحيح delta_outer_main = lh__delta_outer_main(ctx->flex, b);

	NSLOG(flex, DEEPDEBUG, "box %p: delta_outer_main: %i",
			b, delta_outer_main);

	إذا (شي->basis == CSS_FLEX_BASIS_سيٽ) {
		إذا (شي->basis_unit == CSS_UNIT_PCT) {
			شي->بنياد_سائيز = FPCT_جو_INT_TOINT(
					شي->basis_ڊيگهائي,
					موجود_ويڪر);
		} وإلا {
			شي->بنياد_سائيز = FIXTOINT(css_unit_len2device_px(
					b->style, ctx->unit_len_ctx,
					شي->basis_ڊيگهائي,
					شي->basis_unit));
		}

	} وإلا إذا (شي->basis == CSS_FLEX_BASIS_AUTO) {
		شي->بنياد_سائيز = ctx->horizontal ? b->ويڪر : b->اوچائي;
	} وإلا {
		شي->بنياد_سائيز = AUTO;
	}

	إذا (ctx->horizontal == false) {
		إذا (b->ويڪر == AUTO) {
			b->ويڪر = min(max(content_min_ويڪر, موجود_ويڪر),
					content_max_ويڪر);
			b->ويڪر -= lh_delta_outer_ويڪر(b);
		}

		إذا (!layout_flex_شي(ctx, شي, b->ويڪر)) {
			أرجع false;
		}
	}

	إذا (شي->بنياد_سائيز == AUTO) {
		إذا (ctx->horizontal == false) {
			شي->بنياد_سائيز = b->اوچائي;
		} وإلا {
			شي->بنياد_سائيز = content_max_ويڪر - delta_outer_main;
		}
	}

	شي->بنياد_سائيز += delta_outer_main;

	إذا (ctx->horizontal) {
		شي->بنياد_سائيز = min(شي->بنياد_سائيز, موجود_ويڪر);
		شي->بنياد_سائيز = max(شي->بنياد_سائيز, content_min_ويڪر);
	}

	شي->هدف_main_سائيز = شي->بنياد_سائيز;
	شي->main_سائيز = شي->بنياد_سائيز;

	إذا (شي->max_main > 0 &&
	    شي->main_سائيز > شي->max_main + delta_outer_main) {
		شي->main_سائيز = شي->max_main + delta_outer_main;
	}

	إذا (شي->main_سائيز < شي->min_main + delta_outer_main) {
		شي->main_سائيز = شي->min_main + delta_outer_main;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: base_size: %i, main_size %i",
			b, شي->بنياد_سائيز, شي->main_سائيز);

	أرجع true;
}

/**
 * Fill out all item's data in a flex container.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] flex             Flex box
 * \param[in] available_width  Available width in pixels
 */
ساكن فراغ layout_flex_ctx_populate_شي_معلومات(
		ثابت بنية flex_ctx *ctx,
		ثابت بنية box *flex,
		صحيح موجود_ويڪر)
{
	size_t i = 0;
	bool horizontal = ctx->horizontal;

	لكل (بنية box *b = flex->children; b != عدم; b = b->ايندڙ) {
		بنية flex_شي_معلومات *شي = &ctx->شي.معلومات[i++];

		b->float_container = b->والد;
		layout_ڳوليو_dimensions(ctx->unit_len_ctx, موجود_ويڪر, -1,
				b, b->style, &b->ويڪر, &b->اوچائي,
				horizontal ? &شي->max_main : &شي->max_cross,
				horizontal ? &شي->min_main : &شي->min_cross,
				horizontal ? &شي->max_cross : &شي->max_main,
				horizontal ? &شي->min_cross : &شي->min_main,
				b->margin, b->padding, b->border);
		b->float_container = عدم;

		NSLOG(flex, DEEPDEBUG, "flex-item box: %p: width: %i",
				b, b->ويڪر);

		شي->box = b;
		شي->basis = css_computed_flex_basis(b->style,
				&شي->basis_ڊيگهائي, &شي->basis_unit);

		css_computed_flex_shrink(b->style, &شي->shrink);
		css_computed_flex_grow(b->style, &شي->grow);

		layout_flex_بنياد_and_main_sizes(ctx, شي, موجود_ويڪر);
	}
}

/**
 * Ensure context's lines array has a free space
 *
 * \param[in] ctx  Flex layout context
 * \return true on success false on out of memory
 */
ساكن bool layout_flex_ctx_ensure_سٽ(بنية flex_ctx *ctx)
{
	بنية flex_سٽ_معلومات *عارضي;
	size_t سٽ_alloc = ctx->سٽ.alloc * 2;

	إذا (ctx->سٽ.alloc > ctx->سٽ.ڳڻپ) {
		أرجع true;
	}

	عارضي = realloc(ctx->سٽ.معلومات, حجم(*ctx->سٽ.معلومات) * سٽ_alloc);
	إذا (عارضي == عدم) {
		أرجع false;
	}
	ctx->سٽ.معلومات = عارضي;

	memset(ctx->سٽ.معلومات + ctx->سٽ.alloc, 0,
	       حجم(*ctx->سٽ.معلومات) * (سٽ_alloc - ctx->سٽ.alloc));
	ctx->سٽ.alloc = سٽ_alloc;

	أرجع true;
}

/**
 * Assigns flex items to the line and returns the line
 *
 * \param[in] ctx         Flex layout context
 * \param[in] item_index  Index to first item to assign to this line
 * \return Pointer to the new line, or NULL on error.
 */
ساكن بنية flex_سٽ_معلومات *layout_flex_build_سٽ(بنية flex_ctx *ctx,
		size_t شي_اشاريو)
{
	تعداد box_side شروعات_side = layout_flex_main_شروعات_side(ctx);
	تعداد box_side پڇاڙي_side = layout_flex_main_پڇاڙي_side(ctx);
	بنية flex_سٽ_معلومات *سٽ;
	صحيح استعمال_ٿيل_main = 0;

	إذا (!layout_flex_ctx_ensure_سٽ(ctx)) {
		أرجع عدم;
	}

	سٽ = &ctx->سٽ.معلومات[ctx->سٽ.ڳڻپ];
	سٽ->پهريون = شي_اشاريو;

	NSLOG(flex, DEEPDEBUG, "flex container %p: available main: %i",
			ctx->flex, ctx->موجود_main);

	طالما (شي_اشاريو < ctx->شي.ڳڻپ) {
		بنية flex_شي_معلومات *شي = &ctx->شي.معلومات[شي_اشاريو];
		بنية box *b = شي->box;
		صحيح pos_main;

		pos_main = ctx->horizontal ?
				شي->main_سائيز :
				b->اوچائي + lh__delta_outer_main(ctx->flex, b);

		إذا (ctx->wrap == CSS_FLEX_WRAP_NOWRAP ||
		    pos_main + استعمال_ٿيل_main <= ctx->موجود_main ||
		    lh__box_is_absolute(شي->box) ||
		    ctx->موجود_main == AUTO ||
		    سٽ->ڳڻپ == 0 ||
		    pos_main == 0) {
			إذا (lh__box_is_absolute(شي->box) == false) {
				سٽ->main_سائيز += شي->main_سائيز;
				استعمال_ٿيل_main += pos_main;

				إذا (b->margin[شروعات_side] == AUTO) {
					سٽ->main_auto_margin_ڳڻپ++;
				}
				إذا (b->margin[پڇاڙي_side] == AUTO) {
					سٽ->main_auto_margin_ڳڻپ++;
				}
			}
			شي->سٽ = ctx->سٽ.ڳڻپ;
			سٽ->ڳڻپ++;
			شي_اشاريو++;
		} وإلا {
			اكسر;
		}
	}

	إذا (سٽ->ڳڻپ > 0) {
		ctx->سٽ.ڳڻپ++;
	} وإلا {
		NSLOG(layout, ERROR, "Failed to fit any flex items");
	}

	أرجع سٽ;
}

/**
 * Freeze an item on a line
 *
 * \param[in] line  Line to containing item
 * \param[in] item  Item to freeze
 */
ساكن مضمن فراغ layout_flex_شي_freeze(
		بنية flex_سٽ_معلومات *سٽ,
		بنية flex_شي_معلومات *شي)
{
	شي->freeze = true;
	سٽ->frozen++;

	إذا (!lh__box_is_absolute(شي->box)){
		سٽ->استعمال_ٿيل_main_سائيز += شي->هدف_main_سائيز;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: "
			"Frozen at target_main_size: %i",
			شي->box, شي->هدف_main_سائيز);
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
ساكن مضمن صحيح layout_flex_بچيل_free_main(
		بنية flex_ctx *ctx,
		بنية flex_سٽ_معلومات *سٽ,
		css_fixed *unfrozen_ضرب_ڪندڙ_جوڙ,
		صحيح initial_free_main,
		صحيح موجود_main,
		bool grow)
{
	صحيح بچيل_free_main = موجود_main;
	size_t شي_ڳڻپ = سٽ->پهريون + سٽ->ڳڻپ;

	*unfrozen_ضرب_ڪندڙ_جوڙ = 0;

	لكل (size_t i = سٽ->پهريون; i < شي_ڳڻپ; i++) {
		بنية flex_شي_معلومات *شي = &ctx->شي.معلومات[i];

		إذا (شي->freeze) {
			بچيل_free_main -= شي->هدف_main_سائيز;
		} وإلا {
			بچيل_free_main -= شي->بنياد_سائيز;

			*unfrozen_ضرب_ڪندڙ_جوڙ += grow ?
					شي->grow : شي->shrink;
		}
	}

	إذا (*unfrozen_ضرب_ڪندڙ_جوڙ < F_1) {
		صحيح free_space = FIXTOINT(FMUL(INTTOFIX(initial_free_main),
				*unfrozen_ضرب_ڪندڙ_جوڙ));

		إذا (free_space < بچيل_free_main) {
			بچيل_free_main = free_space;
		}
	}

	NSLOG(flex, DEEPDEBUG, "Remaining free space: %i",
			بچيل_free_main);

	أرجع بچيل_free_main;
}

/**
 * Clamp flex item target main size and get min/max violations
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to align items on
 * return total violation in pixels
 */
ساكن مضمن صحيح layout_flex__get_min_max_violations(
		بنية flex_ctx *ctx,
		بنية flex_سٽ_معلومات *سٽ)
{

	صحيح ڪل_violation = 0;
	size_t شي_ڳڻپ = سٽ->پهريون + سٽ->ڳڻپ;

	لكل (size_t i = سٽ->پهريون; i < شي_ڳڻپ; i++) {
		بنية flex_شي_معلومات *شي = &ctx->شي.معلومات[i];
		صحيح هدف_main_سائيز = شي->هدف_main_سائيز;

		NSLOG(flex, DEEPDEBUG, "item %p: target_main_size: %i",
					شي->box, هدف_main_سائيز);

		إذا (شي->freeze) {
			استمر;
		}

		إذا (شي->max_main > 0 &&
		    هدف_main_سائيز > شي->max_main) {
			هدف_main_سائيز = شي->max_main;
			شي->max_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: max_main: %i",
					شي->max_main);
		}

		إذا (هدف_main_سائيز < شي->min_main) {
			هدف_main_سائيز = شي->min_main;
			شي->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: min_main: %i",
					شي->min_main);
		}

		إذا (هدف_main_سائيز < شي->box->min_ويڪر) {
			هدف_main_سائيز = شي->box->min_ويڪر;
			شي->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: box min_width: %i",
					شي->box->min_ويڪر);
		}

		إذا (هدف_main_سائيز < 0) {
			هدف_main_سائيز = 0;
			شي->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: less than 0");
		}

		ڪل_violation += هدف_main_سائيز - شي->هدف_main_سائيز;
		شي->هدف_main_سائيز = هدف_main_سائيز;
	}

	NSLOG(flex, DEEPDEBUG, "Total violation: %i", ڪل_violation);

	أرجع ڪل_violation;
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
ساكن مضمن فراغ layout_flex__distribute_free_main(
		بنية flex_ctx *ctx,
		بنية flex_سٽ_معلومات *سٽ,
		css_fixed unfrozen_ضرب_ڪندڙ_جوڙ,
		صحيح بچيل_free_main,
		bool grow)
{
	size_t شي_ڳڻپ = سٽ->پهريون + سٽ->ڳڻپ;

	إذا (grow) {
		css_fixed remainder = 0;
		لكل (size_t i = سٽ->پهريون; i < شي_ڳڻپ; i++) {
			بنية flex_شي_معلومات *شي = &ctx->شي.معلومات[i];
			css_fixed نتيجو;
			css_fixed تناسب;

			إذا (شي->freeze) {
				استمر;
			}

			تناسب = FDIV(شي->grow, unfrozen_ضرب_ڪندڙ_جوڙ);
			نتيجو = FMUL(INTTOFIX(بچيل_free_main), تناسب) +
					remainder;

			شي->هدف_main_سائيز = شي->بنياد_سائيز +
					FIXTOINT(نتيجو);
			remainder = FIXFRAC(نتيجو);
		}
	} وإلا {
		css_fixed scaled_shrink_ضرب_ڪندڙ_جوڙ = 0;
		css_fixed remainder = 0;

		لكل (size_t i = سٽ->پهريون; i < شي_ڳڻپ; i++) {
			بنية flex_شي_معلومات *شي = &ctx->شي.معلومات[i];
			css_fixed scaled_shrink_ضرب_ڪندڙ;

			إذا (شي->freeze) {
				استمر;
			}

			scaled_shrink_ضرب_ڪندڙ = FMUL(
					شي->shrink,
					INTTOFIX(شي->بنياد_سائيز));
			scaled_shrink_ضرب_ڪندڙ_جوڙ += scaled_shrink_ضرب_ڪندڙ;
		}

		لكل (size_t i = سٽ->پهريون; i < شي_ڳڻپ; i++) {
			بنية flex_شي_معلومات *شي = &ctx->شي.معلومات[i];
			css_fixed scaled_shrink_ضرب_ڪندڙ;
			css_fixed نتيجو;
			css_fixed تناسب;

			إذا (شي->freeze) {
				استمر;
			} وإلا إذا (scaled_shrink_ضرب_ڪندڙ_جوڙ == 0) {
				شي->هدف_main_سائيز = شي->main_سائيز;
				layout_flex_شي_freeze(سٽ, شي);
				استمر;
			}

			scaled_shrink_ضرب_ڪندڙ = FMUL(
					شي->shrink,
					INTTOFIX(شي->بنياد_سائيز));
			تناسب = FDIV(scaled_shrink_ضرب_ڪندڙ,
			             scaled_shrink_ضرب_ڪندڙ_جوڙ);
			نتيجو = FMUL(INTTOFIX(مطلق_قدر(بچيل_free_main)),
			              تناسب) + remainder;

			شي->هدف_main_سائيز = شي->بنياد_سائيز -
					FIXTOINT(نتيجو);
			remainder = FIXFRAC(نتيجو);
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
ساكن bool layout_flex_resolve_سٽ(
		بنية flex_ctx *ctx,
		بنية flex_سٽ_معلومات *سٽ)
{
	size_t شي_ڳڻپ = سٽ->پهريون + سٽ->ڳڻپ;
	صحيح موجود_main = ctx->موجود_main;
	صحيح initial_free_main;
	bool grow;

	إذا (موجود_main == AUTO) {
		موجود_main = وڌ_کان_وڌ_عدد;
	}

	grow = (سٽ->main_سائيز < موجود_main);
	initial_free_main = موجود_main;

	NSLOG(flex, DEEPDEBUG, "box %p: line %zu: first: %zu, count: %zu",
			ctx->flex, سٽ - ctx->سٽ.معلومات,
			سٽ->پهريون, سٽ->ڳڻپ);
	NSLOG(flex, DEEPDEBUG, "Line main_size: %i, available_main: %i",
			سٽ->main_سائيز, موجود_main);

	لكل (size_t i = سٽ->پهريون; i < شي_ڳڻپ; i++) {
		بنية flex_شي_معلومات *شي = &ctx->شي.معلومات[i];

		/* 3. Size inflexible items */
		إذا (grow) {
			إذا (شي->grow == 0 ||
			    شي->بنياد_سائيز > شي->main_سائيز) {
				شي->هدف_main_سائيز = شي->main_سائيز;
				layout_flex_شي_freeze(سٽ, شي);
			}
		} وإلا {
			إذا (شي->shrink == 0 ||
			    شي->بنياد_سائيز < شي->main_سائيز) {
				شي->هدف_main_سائيز = شي->main_سائيز;
				layout_flex_شي_freeze(سٽ, شي);
			}
		}

		/* 4. Calculate initial free space */
		إذا (شي->freeze) {
			initial_free_main -= شي->هدف_main_سائيز;
		} وإلا {
			initial_free_main -= شي->بنياد_سائيز;
		}
	}

	/* 5. Loop */
	طالما (سٽ->frozen < سٽ->ڳڻپ) {
		css_fixed unfrozen_ضرب_ڪندڙ_جوڙ;
		صحيح بچيل_free_main;
		صحيح ڪل_violation;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: Resolver pass",
				ctx->flex);

		/* b */
		بچيل_free_main = layout_flex_بچيل_free_main(ctx,
				سٽ, &unfrozen_ضرب_ڪندڙ_جوڙ, initial_free_main,
				موجود_main, grow);

		/* c */
		إذا (بچيل_free_main != 0) {
			layout_flex__distribute_free_main(ctx,
					سٽ, unfrozen_ضرب_ڪندڙ_جوڙ,
					بچيل_free_main, grow);
		}

		/* d */
		ڪل_violation = layout_flex__get_min_max_violations(
				ctx, سٽ);

		/* e */
		لكل (size_t i = سٽ->پهريون; i < شي_ڳڻپ; i++) {
			بنية flex_شي_معلومات *شي = &ctx->شي.معلومات[i];

			إذا (شي->freeze) {
				استمر;
			}

			إذا (ڪل_violation == 0 ||
			    (ڪل_violation > 0 && شي->min_violation) ||
			    (ڪل_violation < 0 && شي->max_violation)) {
				layout_flex_شي_freeze(سٽ, شي);
			}
		}
	}

	أرجع true;
}

/**
 * Position items along a line
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to resolve
 * \return true on success, false on failure.
 */
ساكن bool layout_flex_رکو_سٽ_شيون_main(
		بنية flex_ctx *ctx,
		بنية flex_سٽ_معلومات *سٽ)
{
	صحيح main_pos = ctx->flex->padding[layout_flex_main_شروعات_side(ctx)];
	صحيح post_multiplier = ctx->main_reversed ? 0 : 1;
	صحيح pre_multiplier = ctx->main_reversed ? -1 : 0;
	size_t شي_ڳڻپ = سٽ->پهريون + سٽ->ڳڻپ;
	صحيح extra_remainder = 0;
	صحيح extra = 0;

	إذا (ctx->main_reversed) {
		main_pos = lh_box_سائيز_main(ctx->horizontal, ctx->flex) -
				main_pos;
	}

	إذا (ctx->موجود_main != AUTO &&
	    ctx->موجود_main != UNKNOWN_ويڪر &&
	    ctx->موجود_main > سٽ->استعمال_ٿيل_main_سائيز) {
		إذا (سٽ->main_auto_margin_ڳڻپ > 0) {
			extra = ctx->موجود_main - سٽ->استعمال_ٿيل_main_سائيز;

			extra_remainder = extra % سٽ->main_auto_margin_ڳڻپ;
			extra /= سٽ->main_auto_margin_ڳڻپ;
		}
	}

	لكل (size_t i = سٽ->پهريون; i < شي_ڳڻپ; i++) {
		تعداد box_side main_پڇاڙي = ctx->horizontal ? ساڄو_2 : BOTTOM;
		تعداد box_side main_شروعات = ctx->horizontal ? کاٻو_2 : چوٽي_2;
		بنية flex_شي_معلومات *شي = &ctx->شي.معلومات[i];
		بنية box *b = شي->box;
		صحيح extra_ڪل = 0;
		صحيح extra_post = 0;
		صحيح extra_pre = 0;
		صحيح box_سائيز_main;
		صحيح *box_pos_main;

		إذا (ctx->horizontal) {
			b->ويڪر = شي->هدف_main_سائيز -
					lh_delta_outer_ويڪر(b);

			إذا (!layout_flex_شي(ctx, شي, b->ويڪر)) {
				أرجع false;
			}
		}

		box_سائيز_main = lh_box_سائيز_main(ctx->horizontal, b);
		box_pos_main = ctx->horizontal ? &b->x : &b->y;

		إذا (!lh__box_is_absolute(b)) {
			إذا (b->margin[main_شروعات] == AUTO) {
				extra_pre = extra + extra_remainder;
			}
			إذا (b->margin[main_پڇاڙي] == AUTO) {
				extra_post = extra + extra_remainder;
			}
			extra_ڪل = extra_pre + extra_post;

			main_pos += pre_multiplier *
					(extra_ڪل + box_سائيز_main +
					 lh__delta_outer_main(ctx->flex, b));
		}

		*box_pos_main = main_pos + lh__non_auto_margin(b, main_شروعات) +
				extra_pre + b->border[main_شروعات].ويڪر;

		إذا (!lh__box_is_absolute(b)) {
			صحيح cross_سائيز;
			صحيح box_سائيز_cross = lh_box_سائيز_cross(
					ctx->horizontal, b);

			main_pos += post_multiplier *
					(extra_ڪل + box_سائيز_main +
					 lh__delta_outer_main(ctx->flex, b));

			cross_سائيز = box_سائيز_cross + lh__delta_outer_cross(
					ctx->flex, b);
			إذا (سٽ->cross_سائيز < cross_سائيز) {
				سٽ->cross_سائيز = cross_سائيز;
			}
		}
	}

	أرجع true;
}

/**
 * Collect items onto lines and place items along the lines
 *
 * \param[in] ctx   Flex layout context
 * \return true on success, false on failure.
 */
ساكن bool layout_flex_collect_شيون_into_سٽون(
		بنية flex_ctx *ctx)
{
	size_t pos = 0;

	طالما (pos < ctx->شي.ڳڻپ) {
		بنية flex_سٽ_معلومات *سٽ;

		سٽ = layout_flex_build_سٽ(ctx, pos);
		إذا (سٽ == عدم) {
			أرجع false;
		}

		pos += سٽ->ڳڻپ;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: "
				"fitted: %zu (total: %zu/%zu)",
				ctx->flex, سٽ->ڳڻپ,
				pos, ctx->شي.ڳڻپ);

		إذا (!layout_flex_resolve_سٽ(ctx, سٽ)) {
			أرجع false;
		}

		إذا (!layout_flex_رکو_سٽ_شيون_main(ctx, سٽ)) {
			أرجع false;
		}

		ctx->cross_سائيز += سٽ->cross_سائيز;
		إذا (ctx->main_سائيز < سٽ->main_سائيز) {
			ctx->main_سائيز = سٽ->main_سائيز;
		}
	}

	أرجع true;
}

/**
 * Align items on a line.
 *
 * \param[in] ctx    Flex layout context
 * \param[in] line   Line to align items on
 * \param[in] extra  Extra line width in pixels
 */
ساكن فراغ layout_flex_رکو_سٽ_شيون_cross(بنية flex_ctx *ctx,
		بنية flex_سٽ_معلومات *سٽ, صحيح extra)
{
	تعداد box_side cross_شروعات = ctx->horizontal ? چوٽي_2 : کاٻو_2;
	size_t شي_ڳڻپ = سٽ->پهريون + سٽ->ڳڻپ;

	لكل (size_t i = سٽ->پهريون; i < شي_ڳڻپ; i++) {
		بنية flex_شي_معلومات *شي = &ctx->شي.معلومات[i];
		بنية box *b = شي->box;
		صحيح cross_free_space;
		صحيح *box_سائيز_cross;
		صحيح *box_pos_cross;

		box_pos_cross = ctx->horizontal ? &b->y : &b->x;
		box_سائيز_cross = lh_box_سائيز_cross_ptr(ctx->horizontal, b);

		cross_free_space = سٽ->cross_سائيز + extra - *box_سائيز_cross -
				lh__delta_outer_cross(ctx->flex, b);

		بدل (lh__box_align_self(ctx->flex, b)) {
		افتراضي:
		حالة CSS_ALIGN_SELF_STRETCH:
			إذا (lh_box_سائيز_cross_is_auto(ctx->horizontal, b)) {
				*box_سائيز_cross += cross_free_space;

				/* Relayout children for stretch. */
				إذا (!layout_flex_شي(ctx, شي, b->ويڪر)) {
					أرجع;
				}
			}
			fallthrough;
		حالة CSS_ALIGN_SELF_FLEX_شروعات:
			*box_pos_cross = ctx->flex->padding[cross_شروعات] +
					سٽ->pos +
					lh__non_auto_margin(b, cross_شروعات) +
					b->border[cross_شروعات].ويڪر;
			اكسر;

		حالة CSS_ALIGN_SELF_FLEX_پڇاڙي:
			*box_pos_cross = ctx->flex->padding[cross_شروعات] +
					سٽ->pos + cross_free_space +
					lh__non_auto_margin(b, cross_شروعات) +
					b->border[cross_شروعات].ويڪر;
			اكسر;

		حالة CSS_ALIGN_SELF_BASELINE:
		حالة CSS_ALIGN_SELF_CENTER:
			*box_pos_cross = ctx->flex->padding[cross_شروعات] +
					سٽ->pos + cross_free_space / 2 +
					lh__non_auto_margin(b, cross_شروعات) +
					b->border[cross_شروعات].ويڪر;
			اكسر;
		}
	}
}

/**
 * Place the lines and align the items on the line.
 *
 * \param[in] ctx  Flex layout context
 */
ساكن فراغ layout_flex_رکو_سٽون(بنية flex_ctx *ctx)
{
	bool reversed = ctx->wrap == CSS_FLEX_WRAP_WRAP_ابتو;
	صحيح سٽ_pos = reversed ? ctx->cross_سائيز : 0;
	صحيح post_multiplier = reversed ? 0 : 1;
	صحيح pre_multiplier = reversed ? -1 : 0;
	صحيح extra_remainder = 0;
	صحيح extra = 0;

	إذا (ctx->موجود_cross != AUTO &&
	    ctx->موجود_cross > ctx->cross_سائيز &&
	    ctx->سٽ.ڳڻپ > 0) {
		extra = ctx->موجود_cross - ctx->cross_سائيز;

		extra_remainder = extra % ctx->سٽ.ڳڻپ;
		extra /= ctx->سٽ.ڳڻپ;
	}

	لكل (size_t i = 0; i < ctx->سٽ.ڳڻپ; i++) {
		بنية flex_سٽ_معلومات *سٽ = &ctx->سٽ.معلومات[i];

		سٽ_pos += pre_multiplier * سٽ->cross_سائيز;
		سٽ->pos = سٽ_pos;
		سٽ_pos += post_multiplier * سٽ->cross_سائيز +
				extra + extra_remainder;

		layout_flex_رکو_سٽ_شيون_cross(ctx, سٽ,
				extra + extra_remainder);

		إذا (extra_remainder > 0) {
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
bool layout_flex(بنية box *flex, صحيح موجود_ويڪر,
		html_content *content)
{
	صحيح max_اوچائي, min_اوچائي;
	بنية flex_ctx *ctx;
	bool success = false;

	ctx = layout_flex_ctx__create(content, flex);
	إذا (ctx == عدم) {
		أرجع false;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: %s, available_width %i, width: %i",
			flex, ctx->horizontal ? "horizontal" : "vertical",
			موجود_ويڪر, flex->ويڪر);

	layout_ڳوليو_dimensions(
			ctx->unit_len_ctx, موجود_ويڪر, -1,
			flex, flex->style, عدم, &flex->اوچائي,
			عدم, عدم, &max_اوچائي, &min_اوچائي,
			flex->margin, flex->padding, flex->border);

	موجود_ويڪر = min(موجود_ويڪر, flex->ويڪر);

	إذا (ctx->horizontal) {
		ctx->موجود_main = موجود_ويڪر;
		ctx->موجود_cross = ctx->flex->اوچائي;
	} وإلا {
		ctx->موجود_main = ctx->flex->اوچائي;
		ctx->موجود_cross = موجود_ويڪر;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: available_main: %i",
			flex, ctx->موجود_main);
	NSLOG(flex, DEEPDEBUG, "box %p: available_cross: %i",
			flex, ctx->موجود_cross);

	layout_flex_ctx_populate_شي_معلومات(ctx, flex, موجود_ويڪر);

	/* Place items onto lines. */
	success = layout_flex_collect_شيون_into_سٽون(ctx);
	إذا (!success) {
		اذهب cleanup;
	}

	layout_flex_رکو_سٽون(ctx);

	إذا (flex->اوچائي == AUTO) {
		flex->اوچائي = ctx->horizontal ?
				ctx->cross_سائيز :
				ctx->main_سائيز;
	}

	إذا (flex->اوچائي != AUTO) {
		إذا (max_اوچائي >= 0 && flex->اوچائي > max_اوچائي) {
			flex->اوچائي = max_اوچائي;
		}
		إذا (min_اوچائي >  0 && flex->اوچائي < min_اوچائي) {
			flex->اوچائي = min_اوچائي;
		}
	}

	success = true;

cleanup:
	layout_flex_ctx__destroy(ctx);

	NSLOG(flex, DEEPDEBUG, "box %p: %s: w: %i, h: %i", flex,
			success ? "success" : "failure",
			flex->ويڪر, flex->اوچائي);
	أرجع success;
}
