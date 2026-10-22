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
بنية flex_توکی_معلومات {
	تعداد css_flex_basis_e basis;
	css_fixed basis_اوږدتیا;
	css_unit basis_unit;
	بنية box *box;

	css_fixed shrink;
	css_fixed grow;

	صحيح min_main;
	صحيح max_main;
	صحيح min_cross;
	صحيح max_cross;

	صحيح موخه_main_اندازه;
	صحيح بنسټ_اندازه;
	صحيح main_اندازه;
	size_t کرښه;

	bool freeze;
	bool min_violation;
	bool max_violation;
};

/**
 * Flex line data
 */
بنية flex_کرښه_معلومات {
	صحيح main_اندازه;
	صحيح cross_اندازه;

	صحيح کارول_شوی_main_اندازه;
	صحيح main_auto_margin_شمېر;

	صحيح pos;

	size_t لومړی;
	size_t شمېر;
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

	صحيح شته_main;
	صحيح شته_cross;

	bool horizontal;
	bool main_reversed;
	تعداد css_flex_wrap_e wrap;

	بنية flex_توکي {
		size_t شمېر;
		بنية flex_توکی_معلومات *معلومات;
	} توکی;

	بنية flex_کرښې {
		size_t شمېر;
		size_t alloc;
		بنية flex_کرښه_معلومات *معلومات;
	} کرښه;
};

/**
 * Destroy a flex layout context
 *
 * \param[in] ctx  Flex layout context
 */
ساكن فراغ layout_flex_ctx__destroy(بنية flex_ctx *ctx)
{
	إذا (ctx != عدم) {
		حرر(ctx->توکی.معلومات);
		حرر(ctx->کرښه.معلومات);
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

	ctx = حافظه_صفر_ورکړه(1, حجم(*ctx));
	إذا (ctx == عدم) {
		أرجع عدم;
	}
	ctx->کرښه.alloc = 1;

	ctx->توکی.شمېر = box_شمېر_children(flex);
	ctx->توکی.معلومات = حافظه_صفر_ورکړه(ctx->توکی.شمېر, حجم(*ctx->توکی.معلومات));
	إذا (ctx->توکی.معلومات == عدم) {
		layout_flex_ctx__destroy(ctx);
		أرجع عدم;
	}

	ctx->کرښه.alloc = 1;
	ctx->کرښه.معلومات = حافظه_صفر_ورکړه(ctx->کرښه.alloc, حجم(*ctx->کرښه.معلومات));
	إذا (ctx->کرښه.معلومات == عدم) {
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
ساكن تعداد box_side layout_flex_main_پیل_side(
		ثابت بنية flex_ctx *ctx)
{
	إذا (ctx->horizontal) {
		أرجع (ctx->main_reversed) ? ښی_2 : کیڼ_2;
	} وإلا {
		أرجع (ctx->main_reversed) ? BOTTOM : سر_2;
	}
}

/**
 * Find box side representing the end of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the end side.
 */
ساكن تعداد box_side layout_flex_main_پای_ټکی_side(
		ثابت بنية flex_ctx *ctx)
{
	إذا (ctx->horizontal) {
		أرجع (ctx->main_reversed) ? کیڼ_2 : ښی_2;
	} وإلا {
		أرجع (ctx->main_reversed) ? سر_2 : BOTTOM;
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
ساكن bool layout_flex_توکی(
		ثابت بنية flex_ctx *ctx,
		ثابت بنية flex_توکی_معلومات *توکی,
		صحيح شته_پلنوالی)
{
	bool success;
	بنية box *b = توکی->box;

	بدل (b->type) {
	حالة BOX_بلاک:
		success = layout_بلاک_context(b, -1, ctx->content);
		اكسر;
	حالة BOX_جدول:
		b->float_container = b->پلار;
		success = layout_جدول(b, شته_پلنوالی, ctx->content);
		b->float_container = عدم;
		اكسر;
	حالة BOX_FLEX:
		b->float_container = b->پلار;
		success = layout_flex(b, شته_پلنوالی, ctx->content);
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
ساكن مضمن bool layout_flex_بنسټ_and_main_sizes(
		ثابت بنية flex_ctx *ctx,
		بنية flex_توکی_معلومات *توکی,
		صحيح شته_پلنوالی)
{
	بنية box *b = توکی->box;
	صحيح content_min_پلنوالی = b->min_پلنوالی;
	صحيح content_max_پلنوالی = b->max_پلنوالی;
	صحيح delta_outer_main = lh__delta_outer_main(ctx->flex, b);

	NSLOG(flex, DEEPDEBUG, "box %p: delta_outer_main: %i",
			b, delta_outer_main);

	إذا (توکی->basis == CSS_FLEX_BASIS_سېټ) {
		إذا (توکی->basis_unit == CSS_UNIT_PCT) {
			توکی->بنسټ_اندازه = FPCT_د_INT_TOINT(
					توکی->basis_اوږدتیا,
					شته_پلنوالی);
		} وإلا {
			توکی->بنسټ_اندازه = FIXTOINT(css_unit_len2device_px(
					b->style, ctx->unit_len_ctx,
					توکی->basis_اوږدتیا,
					توکی->basis_unit));
		}

	} وإلا إذا (توکی->basis == CSS_FLEX_BASIS_AUTO) {
		توکی->بنسټ_اندازه = ctx->horizontal ? b->پلنوالی : b->لوړوالی;
	} وإلا {
		توکی->بنسټ_اندازه = AUTO;
	}

	إذا (ctx->horizontal == false) {
		إذا (b->پلنوالی == AUTO) {
			b->پلنوالی = min(max(content_min_پلنوالی, شته_پلنوالی),
					content_max_پلنوالی);
			b->پلنوالی -= lh_delta_outer_پلنوالی(b);
		}

		إذا (!layout_flex_توکی(ctx, توکی, b->پلنوالی)) {
			أرجع false;
		}
	}

	إذا (توکی->بنسټ_اندازه == AUTO) {
		إذا (ctx->horizontal == false) {
			توکی->بنسټ_اندازه = b->لوړوالی;
		} وإلا {
			توکی->بنسټ_اندازه = content_max_پلنوالی - delta_outer_main;
		}
	}

	توکی->بنسټ_اندازه += delta_outer_main;

	إذا (ctx->horizontal) {
		توکی->بنسټ_اندازه = min(توکی->بنسټ_اندازه, شته_پلنوالی);
		توکی->بنسټ_اندازه = max(توکی->بنسټ_اندازه, content_min_پلنوالی);
	}

	توکی->موخه_main_اندازه = توکی->بنسټ_اندازه;
	توکی->main_اندازه = توکی->بنسټ_اندازه;

	إذا (توکی->max_main > 0 &&
	    توکی->main_اندازه > توکی->max_main + delta_outer_main) {
		توکی->main_اندازه = توکی->max_main + delta_outer_main;
	}

	إذا (توکی->main_اندازه < توکی->min_main + delta_outer_main) {
		توکی->main_اندازه = توکی->min_main + delta_outer_main;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: base_size: %i, main_size %i",
			b, توکی->بنسټ_اندازه, توکی->main_اندازه);

	أرجع true;
}

/**
 * Fill out all item's data in a flex container.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] flex             Flex box
 * \param[in] available_width  Available width in pixels
 */
ساكن فراغ layout_flex_ctx_populate_توکی_معلومات(
		ثابت بنية flex_ctx *ctx,
		ثابت بنية box *flex,
		صحيح شته_پلنوالی)
{
	size_t i = 0;
	bool horizontal = ctx->horizontal;

	لكل (بنية box *b = flex->children; b != عدم; b = b->بل) {
		بنية flex_توکی_معلومات *توکی = &ctx->توکی.معلومات[i++];

		b->float_container = b->پلار;
		layout_پیدا_کړه_dimensions(ctx->unit_len_ctx, شته_پلنوالی, -1,
				b, b->style, &b->پلنوالی, &b->لوړوالی,
				horizontal ? &توکی->max_main : &توکی->max_cross,
				horizontal ? &توکی->min_main : &توکی->min_cross,
				horizontal ? &توکی->max_cross : &توکی->max_main,
				horizontal ? &توکی->min_cross : &توکی->min_main,
				b->margin, b->padding, b->border);
		b->float_container = عدم;

		NSLOG(flex, DEEPDEBUG, "flex-item box: %p: width: %i",
				b, b->پلنوالی);

		توکی->box = b;
		توکی->basis = css_computed_flex_basis(b->style,
				&توکی->basis_اوږدتیا, &توکی->basis_unit);

		css_computed_flex_shrink(b->style, &توکی->shrink);
		css_computed_flex_grow(b->style, &توکی->grow);

		layout_flex_بنسټ_and_main_sizes(ctx, توکی, شته_پلنوالی);
	}
}

/**
 * Ensure context's lines array has a free space
 *
 * \param[in] ctx  Flex layout context
 * \return true on success false on out of memory
 */
ساكن bool layout_flex_ctx_ensure_کرښه(بنية flex_ctx *ctx)
{
	بنية flex_کرښه_معلومات *لنډمهاله;
	size_t کرښه_alloc = ctx->کرښه.alloc * 2;

	إذا (ctx->کرښه.alloc > ctx->کرښه.شمېر) {
		أرجع true;
	}

	لنډمهاله = realloc(ctx->کرښه.معلومات, حجم(*ctx->کرښه.معلومات) * کرښه_alloc);
	إذا (لنډمهاله == عدم) {
		أرجع false;
	}
	ctx->کرښه.معلومات = لنډمهاله;

	memset(ctx->کرښه.معلومات + ctx->کرښه.alloc, 0,
	       حجم(*ctx->کرښه.معلومات) * (کرښه_alloc - ctx->کرښه.alloc));
	ctx->کرښه.alloc = کرښه_alloc;

	أرجع true;
}

/**
 * Assigns flex items to the line and returns the line
 *
 * \param[in] ctx         Flex layout context
 * \param[in] item_index  Index to first item to assign to this line
 * \return Pointer to the new line, or NULL on error.
 */
ساكن بنية flex_کرښه_معلومات *layout_flex_build_کرښه(بنية flex_ctx *ctx,
		size_t توکی_شاخص)
{
	تعداد box_side پیل_side = layout_flex_main_پیل_side(ctx);
	تعداد box_side پای_ټکی_side = layout_flex_main_پای_ټکی_side(ctx);
	بنية flex_کرښه_معلومات *کرښه;
	صحيح کارول_شوی_main = 0;

	إذا (!layout_flex_ctx_ensure_کرښه(ctx)) {
		أرجع عدم;
	}

	کرښه = &ctx->کرښه.معلومات[ctx->کرښه.شمېر];
	کرښه->لومړی = توکی_شاخص;

	NSLOG(flex, DEEPDEBUG, "flex container %p: available main: %i",
			ctx->flex, ctx->شته_main);

	طالما (توکی_شاخص < ctx->توکی.شمېر) {
		بنية flex_توکی_معلومات *توکی = &ctx->توکی.معلومات[توکی_شاخص];
		بنية box *b = توکی->box;
		صحيح pos_main;

		pos_main = ctx->horizontal ?
				توکی->main_اندازه :
				b->لوړوالی + lh__delta_outer_main(ctx->flex, b);

		إذا (ctx->wrap == CSS_FLEX_WRAP_NOWRAP ||
		    pos_main + کارول_شوی_main <= ctx->شته_main ||
		    lh__box_is_absolute(توکی->box) ||
		    ctx->شته_main == AUTO ||
		    کرښه->شمېر == 0 ||
		    pos_main == 0) {
			إذا (lh__box_is_absolute(توکی->box) == false) {
				کرښه->main_اندازه += توکی->main_اندازه;
				کارول_شوی_main += pos_main;

				إذا (b->margin[پیل_side] == AUTO) {
					کرښه->main_auto_margin_شمېر++;
				}
				إذا (b->margin[پای_ټکی_side] == AUTO) {
					کرښه->main_auto_margin_شمېر++;
				}
			}
			توکی->کرښه = ctx->کرښه.شمېر;
			کرښه->شمېر++;
			توکی_شاخص++;
		} وإلا {
			اكسر;
		}
	}

	إذا (کرښه->شمېر > 0) {
		ctx->کرښه.شمېر++;
	} وإلا {
		NSLOG(layout, ERROR, "Failed to fit any flex items");
	}

	أرجع کرښه;
}

/**
 * Freeze an item on a line
 *
 * \param[in] line  Line to containing item
 * \param[in] item  Item to freeze
 */
ساكن مضمن فراغ layout_flex_توکی_freeze(
		بنية flex_کرښه_معلومات *کرښه,
		بنية flex_توکی_معلومات *توکی)
{
	توکی->freeze = true;
	کرښه->frozen++;

	إذا (!lh__box_is_absolute(توکی->box)){
		کرښه->کارول_شوی_main_اندازه += توکی->موخه_main_اندازه;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: "
			"Frozen at target_main_size: %i",
			توکی->box, توکی->موخه_main_اندازه);
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
ساكن مضمن صحيح layout_flex_پاتې_شوی_free_main(
		بنية flex_ctx *ctx,
		بنية flex_کرښه_معلومات *کرښه,
		css_fixed *unfrozen_ضریب_مجموعه,
		صحيح initial_free_main,
		صحيح شته_main,
		bool grow)
{
	صحيح پاتې_شوی_free_main = شته_main;
	size_t توکی_شمېر = کرښه->لومړی + کرښه->شمېر;

	*unfrozen_ضریب_مجموعه = 0;

	لكل (size_t i = کرښه->لومړی; i < توکی_شمېر; i++) {
		بنية flex_توکی_معلومات *توکی = &ctx->توکی.معلومات[i];

		إذا (توکی->freeze) {
			پاتې_شوی_free_main -= توکی->موخه_main_اندازه;
		} وإلا {
			پاتې_شوی_free_main -= توکی->بنسټ_اندازه;

			*unfrozen_ضریب_مجموعه += grow ?
					توکی->grow : توکی->shrink;
		}
	}

	إذا (*unfrozen_ضریب_مجموعه < F_1) {
		صحيح free_space = FIXTOINT(FMUL(INTTOFIX(initial_free_main),
				*unfrozen_ضریب_مجموعه));

		إذا (free_space < پاتې_شوی_free_main) {
			پاتې_شوی_free_main = free_space;
		}
	}

	NSLOG(flex, DEEPDEBUG, "Remaining free space: %i",
			پاتې_شوی_free_main);

	أرجع پاتې_شوی_free_main;
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
		بنية flex_کرښه_معلومات *کرښه)
{

	صحيح ټول_violation = 0;
	size_t توکی_شمېر = کرښه->لومړی + کرښه->شمېر;

	لكل (size_t i = کرښه->لومړی; i < توکی_شمېر; i++) {
		بنية flex_توکی_معلومات *توکی = &ctx->توکی.معلومات[i];
		صحيح موخه_main_اندازه = توکی->موخه_main_اندازه;

		NSLOG(flex, DEEPDEBUG, "item %p: target_main_size: %i",
					توکی->box, موخه_main_اندازه);

		إذا (توکی->freeze) {
			استمر;
		}

		إذا (توکی->max_main > 0 &&
		    موخه_main_اندازه > توکی->max_main) {
			موخه_main_اندازه = توکی->max_main;
			توکی->max_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: max_main: %i",
					توکی->max_main);
		}

		إذا (موخه_main_اندازه < توکی->min_main) {
			موخه_main_اندازه = توکی->min_main;
			توکی->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: min_main: %i",
					توکی->min_main);
		}

		إذا (موخه_main_اندازه < توکی->box->min_پلنوالی) {
			موخه_main_اندازه = توکی->box->min_پلنوالی;
			توکی->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: box min_width: %i",
					توکی->box->min_پلنوالی);
		}

		إذا (موخه_main_اندازه < 0) {
			موخه_main_اندازه = 0;
			توکی->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: less than 0");
		}

		ټول_violation += موخه_main_اندازه - توکی->موخه_main_اندازه;
		توکی->موخه_main_اندازه = موخه_main_اندازه;
	}

	NSLOG(flex, DEEPDEBUG, "Total violation: %i", ټول_violation);

	أرجع ټول_violation;
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
		بنية flex_کرښه_معلومات *کرښه,
		css_fixed unfrozen_ضریب_مجموعه,
		صحيح پاتې_شوی_free_main,
		bool grow)
{
	size_t توکی_شمېر = کرښه->لومړی + کرښه->شمېر;

	إذا (grow) {
		css_fixed remainder = 0;
		لكل (size_t i = کرښه->لومړی; i < توکی_شمېر; i++) {
			بنية flex_توکی_معلومات *توکی = &ctx->توکی.معلومات[i];
			css_fixed پایله;
			css_fixed نسبت;

			إذا (توکی->freeze) {
				استمر;
			}

			نسبت = FDIV(توکی->grow, unfrozen_ضریب_مجموعه);
			پایله = FMUL(INTTOFIX(پاتې_شوی_free_main), نسبت) +
					remainder;

			توکی->موخه_main_اندازه = توکی->بنسټ_اندازه +
					FIXTOINT(پایله);
			remainder = FIXFRAC(پایله);
		}
	} وإلا {
		css_fixed scaled_shrink_ضریب_مجموعه = 0;
		css_fixed remainder = 0;

		لكل (size_t i = کرښه->لومړی; i < توکی_شمېر; i++) {
			بنية flex_توکی_معلومات *توکی = &ctx->توکی.معلومات[i];
			css_fixed scaled_shrink_ضریب;

			إذا (توکی->freeze) {
				استمر;
			}

			scaled_shrink_ضریب = FMUL(
					توکی->shrink,
					INTTOFIX(توکی->بنسټ_اندازه));
			scaled_shrink_ضریب_مجموعه += scaled_shrink_ضریب;
		}

		لكل (size_t i = کرښه->لومړی; i < توکی_شمېر; i++) {
			بنية flex_توکی_معلومات *توکی = &ctx->توکی.معلومات[i];
			css_fixed scaled_shrink_ضریب;
			css_fixed پایله;
			css_fixed نسبت;

			إذا (توکی->freeze) {
				استمر;
			} وإلا إذا (scaled_shrink_ضریب_مجموعه == 0) {
				توکی->موخه_main_اندازه = توکی->main_اندازه;
				layout_flex_توکی_freeze(کرښه, توکی);
				استمر;
			}

			scaled_shrink_ضریب = FMUL(
					توکی->shrink,
					INTTOFIX(توکی->بنسټ_اندازه));
			نسبت = FDIV(scaled_shrink_ضریب,
			             scaled_shrink_ضریب_مجموعه);
			پایله = FMUL(INTTOFIX(مطلق_قیمت(پاتې_شوی_free_main)),
			              نسبت) + remainder;

			توکی->موخه_main_اندازه = توکی->بنسټ_اندازه -
					FIXTOINT(پایله);
			remainder = FIXFRAC(پایله);
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
ساكن bool layout_flex_resolve_کرښه(
		بنية flex_ctx *ctx,
		بنية flex_کرښه_معلومات *کرښه)
{
	size_t توکی_شمېر = کرښه->لومړی + کرښه->شمېر;
	صحيح شته_main = ctx->شته_main;
	صحيح initial_free_main;
	bool grow;

	إذا (شته_main == AUTO) {
		شته_main = لوی_عدد;
	}

	grow = (کرښه->main_اندازه < شته_main);
	initial_free_main = شته_main;

	NSLOG(flex, DEEPDEBUG, "box %p: line %zu: first: %zu, count: %zu",
			ctx->flex, کرښه - ctx->کرښه.معلومات,
			کرښه->لومړی, کرښه->شمېر);
	NSLOG(flex, DEEPDEBUG, "Line main_size: %i, available_main: %i",
			کرښه->main_اندازه, شته_main);

	لكل (size_t i = کرښه->لومړی; i < توکی_شمېر; i++) {
		بنية flex_توکی_معلومات *توکی = &ctx->توکی.معلومات[i];

		/* 3. Size inflexible items */
		إذا (grow) {
			إذا (توکی->grow == 0 ||
			    توکی->بنسټ_اندازه > توکی->main_اندازه) {
				توکی->موخه_main_اندازه = توکی->main_اندازه;
				layout_flex_توکی_freeze(کرښه, توکی);
			}
		} وإلا {
			إذا (توکی->shrink == 0 ||
			    توکی->بنسټ_اندازه < توکی->main_اندازه) {
				توکی->موخه_main_اندازه = توکی->main_اندازه;
				layout_flex_توکی_freeze(کرښه, توکی);
			}
		}

		/* 4. Calculate initial free space */
		إذا (توکی->freeze) {
			initial_free_main -= توکی->موخه_main_اندازه;
		} وإلا {
			initial_free_main -= توکی->بنسټ_اندازه;
		}
	}

	/* 5. Loop */
	طالما (کرښه->frozen < کرښه->شمېر) {
		css_fixed unfrozen_ضریب_مجموعه;
		صحيح پاتې_شوی_free_main;
		صحيح ټول_violation;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: Resolver pass",
				ctx->flex);

		/* b */
		پاتې_شوی_free_main = layout_flex_پاتې_شوی_free_main(ctx,
				کرښه, &unfrozen_ضریب_مجموعه, initial_free_main,
				شته_main, grow);

		/* c */
		إذا (پاتې_شوی_free_main != 0) {
			layout_flex__distribute_free_main(ctx,
					کرښه, unfrozen_ضریب_مجموعه,
					پاتې_شوی_free_main, grow);
		}

		/* d */
		ټول_violation = layout_flex__get_min_max_violations(
				ctx, کرښه);

		/* e */
		لكل (size_t i = کرښه->لومړی; i < توکی_شمېر; i++) {
			بنية flex_توکی_معلومات *توکی = &ctx->توکی.معلومات[i];

			إذا (توکی->freeze) {
				استمر;
			}

			إذا (ټول_violation == 0 ||
			    (ټول_violation > 0 && توکی->min_violation) ||
			    (ټول_violation < 0 && توکی->max_violation)) {
				layout_flex_توکی_freeze(کرښه, توکی);
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
ساكن bool layout_flex_ځای_پرځای_کړه_کرښه_توکي_main(
		بنية flex_ctx *ctx,
		بنية flex_کرښه_معلومات *کرښه)
{
	صحيح main_pos = ctx->flex->padding[layout_flex_main_پیل_side(ctx)];
	صحيح post_multiplier = ctx->main_reversed ? 0 : 1;
	صحيح pre_multiplier = ctx->main_reversed ? -1 : 0;
	size_t توکی_شمېر = کرښه->لومړی + کرښه->شمېر;
	صحيح extra_remainder = 0;
	صحيح extra = 0;

	إذا (ctx->main_reversed) {
		main_pos = lh_box_اندازه_main(ctx->horizontal, ctx->flex) -
				main_pos;
	}

	إذا (ctx->شته_main != AUTO &&
	    ctx->شته_main != UNKNOWN_پلنوالی &&
	    ctx->شته_main > کرښه->کارول_شوی_main_اندازه) {
		إذا (کرښه->main_auto_margin_شمېر > 0) {
			extra = ctx->شته_main - کرښه->کارول_شوی_main_اندازه;

			extra_remainder = extra % کرښه->main_auto_margin_شمېر;
			extra /= کرښه->main_auto_margin_شمېر;
		}
	}

	لكل (size_t i = کرښه->لومړی; i < توکی_شمېر; i++) {
		تعداد box_side main_پای_ټکی = ctx->horizontal ? ښی_2 : BOTTOM;
		تعداد box_side main_پیل = ctx->horizontal ? کیڼ_2 : سر_2;
		بنية flex_توکی_معلومات *توکی = &ctx->توکی.معلومات[i];
		بنية box *b = توکی->box;
		صحيح extra_ټول = 0;
		صحيح extra_post = 0;
		صحيح extra_pre = 0;
		صحيح box_اندازه_main;
		صحيح *box_pos_main;

		إذا (ctx->horizontal) {
			b->پلنوالی = توکی->موخه_main_اندازه -
					lh_delta_outer_پلنوالی(b);

			إذا (!layout_flex_توکی(ctx, توکی, b->پلنوالی)) {
				أرجع false;
			}
		}

		box_اندازه_main = lh_box_اندازه_main(ctx->horizontal, b);
		box_pos_main = ctx->horizontal ? &b->x : &b->y;

		إذا (!lh__box_is_absolute(b)) {
			إذا (b->margin[main_پیل] == AUTO) {
				extra_pre = extra + extra_remainder;
			}
			إذا (b->margin[main_پای_ټکی] == AUTO) {
				extra_post = extra + extra_remainder;
			}
			extra_ټول = extra_pre + extra_post;

			main_pos += pre_multiplier *
					(extra_ټول + box_اندازه_main +
					 lh__delta_outer_main(ctx->flex, b));
		}

		*box_pos_main = main_pos + lh__non_auto_margin(b, main_پیل) +
				extra_pre + b->border[main_پیل].پلنوالی;

		إذا (!lh__box_is_absolute(b)) {
			صحيح cross_اندازه;
			صحيح box_اندازه_cross = lh_box_اندازه_cross(
					ctx->horizontal, b);

			main_pos += post_multiplier *
					(extra_ټول + box_اندازه_main +
					 lh__delta_outer_main(ctx->flex, b));

			cross_اندازه = box_اندازه_cross + lh__delta_outer_cross(
					ctx->flex, b);
			إذا (کرښه->cross_اندازه < cross_اندازه) {
				کرښه->cross_اندازه = cross_اندازه;
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
ساكن bool layout_flex_collect_توکي_into_کرښې(
		بنية flex_ctx *ctx)
{
	size_t pos = 0;

	طالما (pos < ctx->توکی.شمېر) {
		بنية flex_کرښه_معلومات *کرښه;

		کرښه = layout_flex_build_کرښه(ctx, pos);
		إذا (کرښه == عدم) {
			أرجع false;
		}

		pos += کرښه->شمېر;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: "
				"fitted: %zu (total: %zu/%zu)",
				ctx->flex, کرښه->شمېر,
				pos, ctx->توکی.شمېر);

		إذا (!layout_flex_resolve_کرښه(ctx, کرښه)) {
			أرجع false;
		}

		إذا (!layout_flex_ځای_پرځای_کړه_کرښه_توکي_main(ctx, کرښه)) {
			أرجع false;
		}

		ctx->cross_اندازه += کرښه->cross_اندازه;
		إذا (ctx->main_اندازه < کرښه->main_اندازه) {
			ctx->main_اندازه = کرښه->main_اندازه;
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
ساكن فراغ layout_flex_ځای_پرځای_کړه_کرښه_توکي_cross(بنية flex_ctx *ctx,
		بنية flex_کرښه_معلومات *کرښه, صحيح extra)
{
	تعداد box_side cross_پیل = ctx->horizontal ? سر_2 : کیڼ_2;
	size_t توکی_شمېر = کرښه->لومړی + کرښه->شمېر;

	لكل (size_t i = کرښه->لومړی; i < توکی_شمېر; i++) {
		بنية flex_توکی_معلومات *توکی = &ctx->توکی.معلومات[i];
		بنية box *b = توکی->box;
		صحيح cross_free_space;
		صحيح *box_اندازه_cross;
		صحيح *box_pos_cross;

		box_pos_cross = ctx->horizontal ? &b->y : &b->x;
		box_اندازه_cross = lh_box_اندازه_cross_ptr(ctx->horizontal, b);

		cross_free_space = کرښه->cross_اندازه + extra - *box_اندازه_cross -
				lh__delta_outer_cross(ctx->flex, b);

		بدل (lh__box_align_self(ctx->flex, b)) {
		افتراضي:
		حالة CSS_ALIGN_SELF_STRETCH:
			إذا (lh_box_اندازه_cross_is_auto(ctx->horizontal, b)) {
				*box_اندازه_cross += cross_free_space;

				/* Relayout children for stretch. */
				إذا (!layout_flex_توکی(ctx, توکی, b->پلنوالی)) {
					أرجع;
				}
			}
			fallthrough;
		حالة CSS_ALIGN_SELF_FLEX_پیل:
			*box_pos_cross = ctx->flex->padding[cross_پیل] +
					کرښه->pos +
					lh__non_auto_margin(b, cross_پیل) +
					b->border[cross_پیل].پلنوالی;
			اكسر;

		حالة CSS_ALIGN_SELF_FLEX_پای_ټکی:
			*box_pos_cross = ctx->flex->padding[cross_پیل] +
					کرښه->pos + cross_free_space +
					lh__non_auto_margin(b, cross_پیل) +
					b->border[cross_پیل].پلنوالی;
			اكسر;

		حالة CSS_ALIGN_SELF_BASELINE:
		حالة CSS_ALIGN_SELF_CENTER:
			*box_pos_cross = ctx->flex->padding[cross_پیل] +
					کرښه->pos + cross_free_space / 2 +
					lh__non_auto_margin(b, cross_پیل) +
					b->border[cross_پیل].پلنوالی;
			اكسر;
		}
	}
}

/**
 * Place the lines and align the items on the line.
 *
 * \param[in] ctx  Flex layout context
 */
ساكن فراغ layout_flex_ځای_پرځای_کړه_کرښې(بنية flex_ctx *ctx)
{
	bool reversed = ctx->wrap == CSS_FLEX_WRAP_WRAP_سرچپه;
	صحيح کرښه_pos = reversed ? ctx->cross_اندازه : 0;
	صحيح post_multiplier = reversed ? 0 : 1;
	صحيح pre_multiplier = reversed ? -1 : 0;
	صحيح extra_remainder = 0;
	صحيح extra = 0;

	إذا (ctx->شته_cross != AUTO &&
	    ctx->شته_cross > ctx->cross_اندازه &&
	    ctx->کرښه.شمېر > 0) {
		extra = ctx->شته_cross - ctx->cross_اندازه;

		extra_remainder = extra % ctx->کرښه.شمېر;
		extra /= ctx->کرښه.شمېر;
	}

	لكل (size_t i = 0; i < ctx->کرښه.شمېر; i++) {
		بنية flex_کرښه_معلومات *کرښه = &ctx->کرښه.معلومات[i];

		کرښه_pos += pre_multiplier * کرښه->cross_اندازه;
		کرښه->pos = کرښه_pos;
		کرښه_pos += post_multiplier * کرښه->cross_اندازه +
				extra + extra_remainder;

		layout_flex_ځای_پرځای_کړه_کرښه_توکي_cross(ctx, کرښه,
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
bool layout_flex(بنية box *flex, صحيح شته_پلنوالی,
		html_content *content)
{
	صحيح max_لوړوالی, min_لوړوالی;
	بنية flex_ctx *ctx;
	bool success = false;

	ctx = layout_flex_ctx__create(content, flex);
	إذا (ctx == عدم) {
		أرجع false;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: %s, available_width %i, width: %i",
			flex, ctx->horizontal ? "horizontal" : "vertical",
			شته_پلنوالی, flex->پلنوالی);

	layout_پیدا_کړه_dimensions(
			ctx->unit_len_ctx, شته_پلنوالی, -1,
			flex, flex->style, عدم, &flex->لوړوالی,
			عدم, عدم, &max_لوړوالی, &min_لوړوالی,
			flex->margin, flex->padding, flex->border);

	شته_پلنوالی = min(شته_پلنوالی, flex->پلنوالی);

	إذا (ctx->horizontal) {
		ctx->شته_main = شته_پلنوالی;
		ctx->شته_cross = ctx->flex->لوړوالی;
	} وإلا {
		ctx->شته_main = ctx->flex->لوړوالی;
		ctx->شته_cross = شته_پلنوالی;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: available_main: %i",
			flex, ctx->شته_main);
	NSLOG(flex, DEEPDEBUG, "box %p: available_cross: %i",
			flex, ctx->شته_cross);

	layout_flex_ctx_populate_توکی_معلومات(ctx, flex, شته_پلنوالی);

	/* Place items onto lines. */
	success = layout_flex_collect_توکي_into_کرښې(ctx);
	إذا (!success) {
		اذهب cleanup;
	}

	layout_flex_ځای_پرځای_کړه_کرښې(ctx);

	إذا (flex->لوړوالی == AUTO) {
		flex->لوړوالی = ctx->horizontal ?
				ctx->cross_اندازه :
				ctx->main_اندازه;
	}

	إذا (flex->لوړوالی != AUTO) {
		إذا (max_لوړوالی >= 0 && flex->لوړوالی > max_لوړوالی) {
			flex->لوړوالی = max_لوړوالی;
		}
		إذا (min_لوړوالی >  0 && flex->لوړوالی < min_لوړوالی) {
			flex->لوړوالی = min_لوړوالی;
		}
	}

	success = true;

cleanup:
	layout_flex_ctx__destroy(ctx);

	NSLOG(flex, DEEPDEBUG, "box %p: %s: w: %i, h: %i", flex,
			success ? "success" : "failure",
			flex->پلنوالی, flex->لوړوالی);
	أرجع success;
}
