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
بنية flex_قلم_داده {
	تعداد css_flex_basis_e basis;
	css_fixed basis_درازی;
	css_unit basis_unit;
	بنية box *box;

	css_fixed shrink;
	css_fixed grow;

	صحيح min_main;
	صحيح max_main;
	صحيح min_cross;
	صحيح max_cross;

	صحيح هدف_main_اندازه;
	صحيح پایه_اندازه;
	صحيح main_اندازه;
	size_t سطر;

	bool freeze;
	bool min_violation;
	bool max_violation;
};

/**
 * Flex line data
 */
بنية flex_سطر_داده {
	صحيح main_اندازه;
	صحيح cross_اندازه;

	صحيح استفاده_شده_main_اندازه;
	صحيح main_auto_margin_شمار;

	صحيح pos;

	size_t اولی;
	size_t شمار;
	size_t frozen;
};

/**
 * Flex layout context
 */
بنية flex_ctx {
	html_content *content;
	ثابت بنية box *flex;
	ثابت css_unit_ctx *unit_len_ctx;

	صحيح main_اندازه;
	صحيح cross_اندازه;

	صحيح موجود_main;
	صحيح موجود_cross;

	bool horizontal;
	bool main_reversed;
	تعداد css_flex_wrap_e wrap;

	بنية flex_اقلام {
		size_t شمار;
		بنية flex_قلم_داده *داده;
	} قلم;

	بنية flex_سطرها {
		size_t شمار;
		size_t alloc;
		بنية flex_سطر_داده *داده;
	} سطر;
};

/**
 * Destroy a flex layout context
 *
 * \param[in] ctx  Flex layout context
 */
ساكن فراغ layout_flex_ctx__destroy(بنية flex_ctx *ctx)
{
	إذا (ctx != عدم) {
		حرر(ctx->قلم.داده);
		حرر(ctx->سطر.داده);
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

	ctx = حافظه_صفر_بده(1, حجم(*ctx));
	إذا (ctx == عدم) {
		أرجع عدم;
	}
	ctx->سطر.alloc = 1;

	ctx->قلم.شمار = box_شمار_children(flex);
	ctx->قلم.داده = حافظه_صفر_بده(ctx->قلم.شمار, حجم(*ctx->قلم.داده));
	إذا (ctx->قلم.داده == عدم) {
		layout_flex_ctx__destroy(ctx);
		أرجع عدم;
	}

	ctx->سطر.alloc = 1;
	ctx->سطر.داده = حافظه_صفر_بده(ctx->سطر.alloc, حجم(*ctx->سطر.داده));
	إذا (ctx->سطر.داده == عدم) {
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
ساكن تعداد box_side layout_flex_main_شروع_side(
		ثابت بنية flex_ctx *ctx)
{
	إذا (ctx->horizontal) {
		أرجع (ctx->main_reversed) ? راست_2 : چپ_2;
	} وإلا {
		أرجع (ctx->main_reversed) ? BOTTOM : بالاترین_2;
	}
}

/**
 * Find box side representing the end of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the end side.
 */
ساكن تعداد box_side layout_flex_main_انتها_side(
		ثابت بنية flex_ctx *ctx)
{
	إذا (ctx->horizontal) {
		أرجع (ctx->main_reversed) ? چپ_2 : راست_2;
	} وإلا {
		أرجع (ctx->main_reversed) ? بالاترین_2 : BOTTOM;
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
ساكن bool layout_flex_قلم(
		ثابت بنية flex_ctx *ctx,
		ثابت بنية flex_قلم_داده *قلم,
		صحيح موجود_عرض)
{
	bool success;
	بنية box *b = قلم->box;

	بدل (b->type) {
	حالة BOX_بلوک:
		success = layout_بلوک_context(b, -1, ctx->content);
		اكسر;
	حالة BOX_جدول:
		b->float_container = b->والد;
		success = layout_جدول(b, موجود_عرض, ctx->content);
		b->float_container = عدم;
		اكسر;
	حالة BOX_FLEX:
		b->float_container = b->والد;
		success = layout_flex(b, موجود_عرض, ctx->content);
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
ساكن مضمن bool layout_flex_پایه_and_main_sizes(
		ثابت بنية flex_ctx *ctx,
		بنية flex_قلم_داده *قلم,
		صحيح موجود_عرض)
{
	بنية box *b = قلم->box;
	صحيح content_min_عرض = b->min_عرض;
	صحيح content_max_عرض = b->max_عرض;
	صحيح delta_outer_main = lh__delta_outer_main(ctx->flex, b);

	NSLOG(flex, DEEPDEBUG, "box %p: delta_outer_main: %i",
			b, delta_outer_main);

	إذا (قلم->basis == CSS_FLEX_BASIS_مجموعه) {
		إذا (قلم->basis_unit == CSS_UNIT_PCT) {
			قلم->پایه_اندازه = FPCT_از_INT_TOINT(
					قلم->basis_درازی,
					موجود_عرض);
		} وإلا {
			قلم->پایه_اندازه = FIXTOINT(css_unit_len2device_px(
					b->style, ctx->unit_len_ctx,
					قلم->basis_درازی,
					قلم->basis_unit));
		}

	} وإلا إذا (قلم->basis == CSS_FLEX_BASIS_AUTO) {
		قلم->پایه_اندازه = ctx->horizontal ? b->عرض : b->ارتفاع;
	} وإلا {
		قلم->پایه_اندازه = AUTO;
	}

	إذا (ctx->horizontal == false) {
		إذا (b->عرض == AUTO) {
			b->عرض = min(max(content_min_عرض, موجود_عرض),
					content_max_عرض);
			b->عرض -= lh_delta_outer_عرض(b);
		}

		إذا (!layout_flex_قلم(ctx, قلم, b->عرض)) {
			أرجع false;
		}
	}

	إذا (قلم->پایه_اندازه == AUTO) {
		إذا (ctx->horizontal == false) {
			قلم->پایه_اندازه = b->ارتفاع;
		} وإلا {
			قلم->پایه_اندازه = content_max_عرض - delta_outer_main;
		}
	}

	قلم->پایه_اندازه += delta_outer_main;

	إذا (ctx->horizontal) {
		قلم->پایه_اندازه = min(قلم->پایه_اندازه, موجود_عرض);
		قلم->پایه_اندازه = max(قلم->پایه_اندازه, content_min_عرض);
	}

	قلم->هدف_main_اندازه = قلم->پایه_اندازه;
	قلم->main_اندازه = قلم->پایه_اندازه;

	إذا (قلم->max_main > 0 &&
	    قلم->main_اندازه > قلم->max_main + delta_outer_main) {
		قلم->main_اندازه = قلم->max_main + delta_outer_main;
	}

	إذا (قلم->main_اندازه < قلم->min_main + delta_outer_main) {
		قلم->main_اندازه = قلم->min_main + delta_outer_main;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: base_size: %i, main_size %i",
			b, قلم->پایه_اندازه, قلم->main_اندازه);

	أرجع true;
}

/**
 * Fill out all item's data in a flex container.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] flex             Flex box
 * \param[in] available_width  Available width in pixels
 */
ساكن فراغ layout_flex_ctx_populate_قلم_داده(
		ثابت بنية flex_ctx *ctx,
		ثابت بنية box *flex,
		صحيح موجود_عرض)
{
	size_t i = 0;
	bool horizontal = ctx->horizontal;

	لكل (بنية box *b = flex->children; b != عدم; b = b->بعدی) {
		بنية flex_قلم_داده *قلم = &ctx->قلم.داده[i++];

		b->float_container = b->والد;
		layout_پیدا_کن_dimensions(ctx->unit_len_ctx, موجود_عرض, -1,
				b, b->style, &b->عرض, &b->ارتفاع,
				horizontal ? &قلم->max_main : &قلم->max_cross,
				horizontal ? &قلم->min_main : &قلم->min_cross,
				horizontal ? &قلم->max_cross : &قلم->max_main,
				horizontal ? &قلم->min_cross : &قلم->min_main,
				b->margin, b->padding, b->border);
		b->float_container = عدم;

		NSLOG(flex, DEEPDEBUG, "flex-item box: %p: width: %i",
				b, b->عرض);

		قلم->box = b;
		قلم->basis = css_computed_flex_basis(b->style,
				&قلم->basis_درازی, &قلم->basis_unit);

		css_computed_flex_shrink(b->style, &قلم->shrink);
		css_computed_flex_grow(b->style, &قلم->grow);

		layout_flex_پایه_and_main_sizes(ctx, قلم, موجود_عرض);
	}
}

/**
 * Ensure context's lines array has a free space
 *
 * \param[in] ctx  Flex layout context
 * \return true on success false on out of memory
 */
ساكن bool layout_flex_ctx_ensure_سطر(بنية flex_ctx *ctx)
{
	بنية flex_سطر_داده *موقت;
	size_t سطر_alloc = ctx->سطر.alloc * 2;

	إذا (ctx->سطر.alloc > ctx->سطر.شمار) {
		أرجع true;
	}

	موقت = realloc(ctx->سطر.داده, حجم(*ctx->سطر.داده) * سطر_alloc);
	إذا (موقت == عدم) {
		أرجع false;
	}
	ctx->سطر.داده = موقت;

	memset(ctx->سطر.داده + ctx->سطر.alloc, 0,
	       حجم(*ctx->سطر.داده) * (سطر_alloc - ctx->سطر.alloc));
	ctx->سطر.alloc = سطر_alloc;

	أرجع true;
}

/**
 * Assigns flex items to the line and returns the line
 *
 * \param[in] ctx         Flex layout context
 * \param[in] item_index  Index to first item to assign to this line
 * \return Pointer to the new line, or NULL on error.
 */
ساكن بنية flex_سطر_داده *layout_flex_build_سطر(بنية flex_ctx *ctx,
		size_t قلم_شاخص)
{
	تعداد box_side شروع_side = layout_flex_main_شروع_side(ctx);
	تعداد box_side انتها_side = layout_flex_main_انتها_side(ctx);
	بنية flex_سطر_داده *سطر;
	صحيح استفاده_شده_main = 0;

	إذا (!layout_flex_ctx_ensure_سطر(ctx)) {
		أرجع عدم;
	}

	سطر = &ctx->سطر.داده[ctx->سطر.شمار];
	سطر->اولی = قلم_شاخص;

	NSLOG(flex, DEEPDEBUG, "flex container %p: available main: %i",
			ctx->flex, ctx->موجود_main);

	طالما (قلم_شاخص < ctx->قلم.شمار) {
		بنية flex_قلم_داده *قلم = &ctx->قلم.داده[قلم_شاخص];
		بنية box *b = قلم->box;
		صحيح pos_main;

		pos_main = ctx->horizontal ?
				قلم->main_اندازه :
				b->ارتفاع + lh__delta_outer_main(ctx->flex, b);

		إذا (ctx->wrap == CSS_FLEX_WRAP_NOWRAP ||
		    pos_main + استفاده_شده_main <= ctx->موجود_main ||
		    lh__box_is_absolute(قلم->box) ||
		    ctx->موجود_main == AUTO ||
		    سطر->شمار == 0 ||
		    pos_main == 0) {
			إذا (lh__box_is_absolute(قلم->box) == false) {
				سطر->main_اندازه += قلم->main_اندازه;
				استفاده_شده_main += pos_main;

				إذا (b->margin[شروع_side] == AUTO) {
					سطر->main_auto_margin_شمار++;
				}
				إذا (b->margin[انتها_side] == AUTO) {
					سطر->main_auto_margin_شمار++;
				}
			}
			قلم->سطر = ctx->سطر.شمار;
			سطر->شمار++;
			قلم_شاخص++;
		} وإلا {
			اكسر;
		}
	}

	إذا (سطر->شمار > 0) {
		ctx->سطر.شمار++;
	} وإلا {
		NSLOG(layout, ERROR, "Failed to fit any flex items");
	}

	أرجع سطر;
}

/**
 * Freeze an item on a line
 *
 * \param[in] line  Line to containing item
 * \param[in] item  Item to freeze
 */
ساكن مضمن فراغ layout_flex_قلم_freeze(
		بنية flex_سطر_داده *سطر,
		بنية flex_قلم_داده *قلم)
{
	قلم->freeze = true;
	سطر->frozen++;

	إذا (!lh__box_is_absolute(قلم->box)){
		سطر->استفاده_شده_main_اندازه += قلم->هدف_main_اندازه;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: "
			"Frozen at target_main_size: %i",
			قلم->box, قلم->هدف_main_اندازه);
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
ساكن مضمن صحيح layout_flex_مانده_free_main(
		بنية flex_ctx *ctx,
		بنية flex_سطر_داده *سطر,
		css_fixed *unfrozen_ضریب_جمع,
		صحيح initial_free_main,
		صحيح موجود_main,
		bool grow)
{
	صحيح مانده_free_main = موجود_main;
	size_t قلم_شمار = سطر->اولی + سطر->شمار;

	*unfrozen_ضریب_جمع = 0;

	لكل (size_t i = سطر->اولی; i < قلم_شمار; i++) {
		بنية flex_قلم_داده *قلم = &ctx->قلم.داده[i];

		إذا (قلم->freeze) {
			مانده_free_main -= قلم->هدف_main_اندازه;
		} وإلا {
			مانده_free_main -= قلم->پایه_اندازه;

			*unfrozen_ضریب_جمع += grow ?
					قلم->grow : قلم->shrink;
		}
	}

	إذا (*unfrozen_ضریب_جمع < F_1) {
		صحيح free_space = FIXTOINT(FMUL(INTTOFIX(initial_free_main),
				*unfrozen_ضریب_جمع));

		إذا (free_space < مانده_free_main) {
			مانده_free_main = free_space;
		}
	}

	NSLOG(flex, DEEPDEBUG, "Remaining free space: %i",
			مانده_free_main);

	أرجع مانده_free_main;
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
		بنية flex_سطر_داده *سطر)
{

	صحيح مجموع_violation = 0;
	size_t قلم_شمار = سطر->اولی + سطر->شمار;

	لكل (size_t i = سطر->اولی; i < قلم_شمار; i++) {
		بنية flex_قلم_داده *قلم = &ctx->قلم.داده[i];
		صحيح هدف_main_اندازه = قلم->هدف_main_اندازه;

		NSLOG(flex, DEEPDEBUG, "item %p: target_main_size: %i",
					قلم->box, هدف_main_اندازه);

		إذا (قلم->freeze) {
			استمر;
		}

		إذا (قلم->max_main > 0 &&
		    هدف_main_اندازه > قلم->max_main) {
			هدف_main_اندازه = قلم->max_main;
			قلم->max_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: max_main: %i",
					قلم->max_main);
		}

		إذا (هدف_main_اندازه < قلم->min_main) {
			هدف_main_اندازه = قلم->min_main;
			قلم->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: min_main: %i",
					قلم->min_main);
		}

		إذا (هدف_main_اندازه < قلم->box->min_عرض) {
			هدف_main_اندازه = قلم->box->min_عرض;
			قلم->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: box min_width: %i",
					قلم->box->min_عرض);
		}

		إذا (هدف_main_اندازه < 0) {
			هدف_main_اندازه = 0;
			قلم->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: less than 0");
		}

		مجموع_violation += هدف_main_اندازه - قلم->هدف_main_اندازه;
		قلم->هدف_main_اندازه = هدف_main_اندازه;
	}

	NSLOG(flex, DEEPDEBUG, "Total violation: %i", مجموع_violation);

	أرجع مجموع_violation;
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
		بنية flex_سطر_داده *سطر,
		css_fixed unfrozen_ضریب_جمع,
		صحيح مانده_free_main,
		bool grow)
{
	size_t قلم_شمار = سطر->اولی + سطر->شمار;

	إذا (grow) {
		css_fixed remainder = 0;
		لكل (size_t i = سطر->اولی; i < قلم_شمار; i++) {
			بنية flex_قلم_داده *قلم = &ctx->قلم.داده[i];
			css_fixed نتیجه;
			css_fixed نسبت;

			إذا (قلم->freeze) {
				استمر;
			}

			نسبت = FDIV(قلم->grow, unfrozen_ضریب_جمع);
			نتیجه = FMUL(INTTOFIX(مانده_free_main), نسبت) +
					remainder;

			قلم->هدف_main_اندازه = قلم->پایه_اندازه +
					FIXTOINT(نتیجه);
			remainder = FIXFRAC(نتیجه);
		}
	} وإلا {
		css_fixed scaled_shrink_ضریب_جمع = 0;
		css_fixed remainder = 0;

		لكل (size_t i = سطر->اولی; i < قلم_شمار; i++) {
			بنية flex_قلم_داده *قلم = &ctx->قلم.داده[i];
			css_fixed scaled_shrink_ضریب;

			إذا (قلم->freeze) {
				استمر;
			}

			scaled_shrink_ضریب = FMUL(
					قلم->shrink,
					INTTOFIX(قلم->پایه_اندازه));
			scaled_shrink_ضریب_جمع += scaled_shrink_ضریب;
		}

		لكل (size_t i = سطر->اولی; i < قلم_شمار; i++) {
			بنية flex_قلم_داده *قلم = &ctx->قلم.داده[i];
			css_fixed scaled_shrink_ضریب;
			css_fixed نتیجه;
			css_fixed نسبت;

			إذا (قلم->freeze) {
				استمر;
			} وإلا إذا (scaled_shrink_ضریب_جمع == 0) {
				قلم->هدف_main_اندازه = قلم->main_اندازه;
				layout_flex_قلم_freeze(سطر, قلم);
				استمر;
			}

			scaled_shrink_ضریب = FMUL(
					قلم->shrink,
					INTTOFIX(قلم->پایه_اندازه));
			نسبت = FDIV(scaled_shrink_ضریب,
			             scaled_shrink_ضریب_جمع);
			نتیجه = FMUL(INTTOFIX(قدر_مطلق(مانده_free_main)),
			              نسبت) + remainder;

			قلم->هدف_main_اندازه = قلم->پایه_اندازه -
					FIXTOINT(نتیجه);
			remainder = FIXFRAC(نتیجه);
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
ساكن bool layout_flex_resolve_سطر(
		بنية flex_ctx *ctx,
		بنية flex_سطر_داده *سطر)
{
	size_t قلم_شمار = سطر->اولی + سطر->شمار;
	صحيح موجود_main = ctx->موجود_main;
	صحيح initial_free_main;
	bool grow;

	إذا (موجود_main == AUTO) {
		موجود_main = بیشترین_صحیح;
	}

	grow = (سطر->main_اندازه < موجود_main);
	initial_free_main = موجود_main;

	NSLOG(flex, DEEPDEBUG, "box %p: line %zu: first: %zu, count: %zu",
			ctx->flex, سطر - ctx->سطر.داده,
			سطر->اولی, سطر->شمار);
	NSLOG(flex, DEEPDEBUG, "Line main_size: %i, available_main: %i",
			سطر->main_اندازه, موجود_main);

	لكل (size_t i = سطر->اولی; i < قلم_شمار; i++) {
		بنية flex_قلم_داده *قلم = &ctx->قلم.داده[i];

		/* 3. Size inflexible items */
		إذا (grow) {
			إذا (قلم->grow == 0 ||
			    قلم->پایه_اندازه > قلم->main_اندازه) {
				قلم->هدف_main_اندازه = قلم->main_اندازه;
				layout_flex_قلم_freeze(سطر, قلم);
			}
		} وإلا {
			إذا (قلم->shrink == 0 ||
			    قلم->پایه_اندازه < قلم->main_اندازه) {
				قلم->هدف_main_اندازه = قلم->main_اندازه;
				layout_flex_قلم_freeze(سطر, قلم);
			}
		}

		/* 4. Calculate initial free space */
		إذا (قلم->freeze) {
			initial_free_main -= قلم->هدف_main_اندازه;
		} وإلا {
			initial_free_main -= قلم->پایه_اندازه;
		}
	}

	/* 5. Loop */
	طالما (سطر->frozen < سطر->شمار) {
		css_fixed unfrozen_ضریب_جمع;
		صحيح مانده_free_main;
		صحيح مجموع_violation;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: Resolver pass",
				ctx->flex);

		/* b */
		مانده_free_main = layout_flex_مانده_free_main(ctx,
				سطر, &unfrozen_ضریب_جمع, initial_free_main,
				موجود_main, grow);

		/* c */
		إذا (مانده_free_main != 0) {
			layout_flex__distribute_free_main(ctx,
					سطر, unfrozen_ضریب_جمع,
					مانده_free_main, grow);
		}

		/* d */
		مجموع_violation = layout_flex__get_min_max_violations(
				ctx, سطر);

		/* e */
		لكل (size_t i = سطر->اولی; i < قلم_شمار; i++) {
			بنية flex_قلم_داده *قلم = &ctx->قلم.داده[i];

			إذا (قلم->freeze) {
				استمر;
			}

			إذا (مجموع_violation == 0 ||
			    (مجموع_violation > 0 && قلم->min_violation) ||
			    (مجموع_violation < 0 && قلم->max_violation)) {
				layout_flex_قلم_freeze(سطر, قلم);
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
ساكن bool layout_flex_قراربده_سطر_اقلام_main(
		بنية flex_ctx *ctx,
		بنية flex_سطر_داده *سطر)
{
	صحيح main_pos = ctx->flex->padding[layout_flex_main_شروع_side(ctx)];
	صحيح post_multiplier = ctx->main_reversed ? 0 : 1;
	صحيح pre_multiplier = ctx->main_reversed ? -1 : 0;
	size_t قلم_شمار = سطر->اولی + سطر->شمار;
	صحيح extra_remainder = 0;
	صحيح extra = 0;

	إذا (ctx->main_reversed) {
		main_pos = lh_box_اندازه_main(ctx->horizontal, ctx->flex) -
				main_pos;
	}

	إذا (ctx->موجود_main != AUTO &&
	    ctx->موجود_main != UNKNOWN_عرض &&
	    ctx->موجود_main > سطر->استفاده_شده_main_اندازه) {
		إذا (سطر->main_auto_margin_شمار > 0) {
			extra = ctx->موجود_main - سطر->استفاده_شده_main_اندازه;

			extra_remainder = extra % سطر->main_auto_margin_شمار;
			extra /= سطر->main_auto_margin_شمار;
		}
	}

	لكل (size_t i = سطر->اولی; i < قلم_شمار; i++) {
		تعداد box_side main_انتها = ctx->horizontal ? راست_2 : BOTTOM;
		تعداد box_side main_شروع = ctx->horizontal ? چپ_2 : بالاترین_2;
		بنية flex_قلم_داده *قلم = &ctx->قلم.داده[i];
		بنية box *b = قلم->box;
		صحيح extra_مجموع = 0;
		صحيح extra_post = 0;
		صحيح extra_pre = 0;
		صحيح box_اندازه_main;
		صحيح *box_pos_main;

		إذا (ctx->horizontal) {
			b->عرض = قلم->هدف_main_اندازه -
					lh_delta_outer_عرض(b);

			إذا (!layout_flex_قلم(ctx, قلم, b->عرض)) {
				أرجع false;
			}
		}

		box_اندازه_main = lh_box_اندازه_main(ctx->horizontal, b);
		box_pos_main = ctx->horizontal ? &b->x : &b->y;

		إذا (!lh__box_is_absolute(b)) {
			إذا (b->margin[main_شروع] == AUTO) {
				extra_pre = extra + extra_remainder;
			}
			إذا (b->margin[main_انتها] == AUTO) {
				extra_post = extra + extra_remainder;
			}
			extra_مجموع = extra_pre + extra_post;

			main_pos += pre_multiplier *
					(extra_مجموع + box_اندازه_main +
					 lh__delta_outer_main(ctx->flex, b));
		}

		*box_pos_main = main_pos + lh__non_auto_margin(b, main_شروع) +
				extra_pre + b->border[main_شروع].عرض;

		إذا (!lh__box_is_absolute(b)) {
			صحيح cross_اندازه;
			صحيح box_اندازه_cross = lh_box_اندازه_cross(
					ctx->horizontal, b);

			main_pos += post_multiplier *
					(extra_مجموع + box_اندازه_main +
					 lh__delta_outer_main(ctx->flex, b));

			cross_اندازه = box_اندازه_cross + lh__delta_outer_cross(
					ctx->flex, b);
			إذا (سطر->cross_اندازه < cross_اندازه) {
				سطر->cross_اندازه = cross_اندازه;
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
ساكن bool layout_flex_collect_اقلام_into_سطرها(
		بنية flex_ctx *ctx)
{
	size_t pos = 0;

	طالما (pos < ctx->قلم.شمار) {
		بنية flex_سطر_داده *سطر;

		سطر = layout_flex_build_سطر(ctx, pos);
		إذا (سطر == عدم) {
			أرجع false;
		}

		pos += سطر->شمار;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: "
				"fitted: %zu (total: %zu/%zu)",
				ctx->flex, سطر->شمار,
				pos, ctx->قلم.شمار);

		إذا (!layout_flex_resolve_سطر(ctx, سطر)) {
			أرجع false;
		}

		إذا (!layout_flex_قراربده_سطر_اقلام_main(ctx, سطر)) {
			أرجع false;
		}

		ctx->cross_اندازه += سطر->cross_اندازه;
		إذا (ctx->main_اندازه < سطر->main_اندازه) {
			ctx->main_اندازه = سطر->main_اندازه;
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
ساكن فراغ layout_flex_قراربده_سطر_اقلام_cross(بنية flex_ctx *ctx,
		بنية flex_سطر_داده *سطر, صحيح extra)
{
	تعداد box_side cross_شروع = ctx->horizontal ? بالاترین_2 : چپ_2;
	size_t قلم_شمار = سطر->اولی + سطر->شمار;

	لكل (size_t i = سطر->اولی; i < قلم_شمار; i++) {
		بنية flex_قلم_داده *قلم = &ctx->قلم.داده[i];
		بنية box *b = قلم->box;
		صحيح cross_free_space;
		صحيح *box_اندازه_cross;
		صحيح *box_pos_cross;

		box_pos_cross = ctx->horizontal ? &b->y : &b->x;
		box_اندازه_cross = lh_box_اندازه_cross_ptr(ctx->horizontal, b);

		cross_free_space = سطر->cross_اندازه + extra - *box_اندازه_cross -
				lh__delta_outer_cross(ctx->flex, b);

		بدل (lh__box_align_self(ctx->flex, b)) {
		افتراضي:
		حالة CSS_ALIGN_SELF_STRETCH:
			إذا (lh_box_اندازه_cross_is_auto(ctx->horizontal, b)) {
				*box_اندازه_cross += cross_free_space;

				/* Relayout children for stretch. */
				إذا (!layout_flex_قلم(ctx, قلم, b->عرض)) {
					أرجع;
				}
			}
			fallthrough;
		حالة CSS_ALIGN_SELF_FLEX_شروع:
			*box_pos_cross = ctx->flex->padding[cross_شروع] +
					سطر->pos +
					lh__non_auto_margin(b, cross_شروع) +
					b->border[cross_شروع].عرض;
			اكسر;

		حالة CSS_ALIGN_SELF_FLEX_انتها:
			*box_pos_cross = ctx->flex->padding[cross_شروع] +
					سطر->pos + cross_free_space +
					lh__non_auto_margin(b, cross_شروع) +
					b->border[cross_شروع].عرض;
			اكسر;

		حالة CSS_ALIGN_SELF_BASELINE:
		حالة CSS_ALIGN_SELF_CENTER:
			*box_pos_cross = ctx->flex->padding[cross_شروع] +
					سطر->pos + cross_free_space / 2 +
					lh__non_auto_margin(b, cross_شروع) +
					b->border[cross_شروع].عرض;
			اكسر;
		}
	}
}

/**
 * Place the lines and align the items on the line.
 *
 * \param[in] ctx  Flex layout context
 */
ساكن فراغ layout_flex_قراربده_سطرها(بنية flex_ctx *ctx)
{
	bool reversed = ctx->wrap == CSS_FLEX_WRAP_WRAP_معکوس;
	صحيح سطر_pos = reversed ? ctx->cross_اندازه : 0;
	صحيح post_multiplier = reversed ? 0 : 1;
	صحيح pre_multiplier = reversed ? -1 : 0;
	صحيح extra_remainder = 0;
	صحيح extra = 0;

	إذا (ctx->موجود_cross != AUTO &&
	    ctx->موجود_cross > ctx->cross_اندازه &&
	    ctx->سطر.شمار > 0) {
		extra = ctx->موجود_cross - ctx->cross_اندازه;

		extra_remainder = extra % ctx->سطر.شمار;
		extra /= ctx->سطر.شمار;
	}

	لكل (size_t i = 0; i < ctx->سطر.شمار; i++) {
		بنية flex_سطر_داده *سطر = &ctx->سطر.داده[i];

		سطر_pos += pre_multiplier * سطر->cross_اندازه;
		سطر->pos = سطر_pos;
		سطر_pos += post_multiplier * سطر->cross_اندازه +
				extra + extra_remainder;

		layout_flex_قراربده_سطر_اقلام_cross(ctx, سطر,
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
bool layout_flex(بنية box *flex, صحيح موجود_عرض,
		html_content *content)
{
	صحيح max_ارتفاع, min_ارتفاع;
	بنية flex_ctx *ctx;
	bool success = false;

	ctx = layout_flex_ctx__create(content, flex);
	إذا (ctx == عدم) {
		أرجع false;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: %s, available_width %i, width: %i",
			flex, ctx->horizontal ? "horizontal" : "vertical",
			موجود_عرض, flex->عرض);

	layout_پیدا_کن_dimensions(
			ctx->unit_len_ctx, موجود_عرض, -1,
			flex, flex->style, عدم, &flex->ارتفاع,
			عدم, عدم, &max_ارتفاع, &min_ارتفاع,
			flex->margin, flex->padding, flex->border);

	موجود_عرض = min(موجود_عرض, flex->عرض);

	إذا (ctx->horizontal) {
		ctx->موجود_main = موجود_عرض;
		ctx->موجود_cross = ctx->flex->ارتفاع;
	} وإلا {
		ctx->موجود_main = ctx->flex->ارتفاع;
		ctx->موجود_cross = موجود_عرض;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: available_main: %i",
			flex, ctx->موجود_main);
	NSLOG(flex, DEEPDEBUG, "box %p: available_cross: %i",
			flex, ctx->موجود_cross);

	layout_flex_ctx_populate_قلم_داده(ctx, flex, موجود_عرض);

	/* Place items onto lines. */
	success = layout_flex_collect_اقلام_into_سطرها(ctx);
	إذا (!success) {
		اذهب cleanup;
	}

	layout_flex_قراربده_سطرها(ctx);

	إذا (flex->ارتفاع == AUTO) {
		flex->ارتفاع = ctx->horizontal ?
				ctx->cross_اندازه :
				ctx->main_اندازه;
	}

	إذا (flex->ارتفاع != AUTO) {
		إذا (max_ارتفاع >= 0 && flex->ارتفاع > max_ارتفاع) {
			flex->ارتفاع = max_ارتفاع;
		}
		إذا (min_ارتفاع >  0 && flex->ارتفاع < min_ارتفاع) {
			flex->ارتفاع = min_ارتفاع;
		}
	}

	success = true;

cleanup:
	layout_flex_ctx__destroy(ctx);

	NSLOG(flex, DEEPDEBUG, "box %p: %s: w: %i, h: %i", flex,
			success ? "success" : "failure",
			flex->عرض, flex->ارتفاع);
	أرجع success;
}
