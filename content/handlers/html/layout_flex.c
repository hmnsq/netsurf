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

#شامل <string.h>

#شامل "utils/log.h"
#شامل "utils/utils.h"

#شامل "html/box.h"
#شامل "html/html.h"
#شامل "html/private.h"
#شامل "html/box_inspect.h"
#شامل "html/layout_internal.h"

/**
 * Flex item data
 */
ساخت flex_شے_ڈیٹا {
	شمار css_flex_basis_e basis;
	css_fixed basis_لمبائی;
	css_unit basis_unit;
	ساخت box *box;

	css_fixed shrink;
	css_fixed grow;

	صحیح min_main;
	صحیح max_main;
	صحیح min_cross;
	صحیح max_cross;

	صحیح ہدف_main_سائز;
	صحیح بنیاد_سائز;
	صحیح main_سائز;
	size_t سطر;

	bool freeze;
	bool min_violation;
	bool max_violation;
};

/**
 * Flex line data
 */
ساخت flex_سطر_ڈیٹا {
	صحیح main_سائز;
	صحیح cross_سائز;

	صحیح مستعمل_main_سائز;
	صحیح main_auto_margin_گنتی;

	صحیح pos;

	size_t پہلا;
	size_t گنتی;
	size_t frozen;
};

/**
 * Flex layout context
 */
ساخت flex_ctx {
	html_content *content;
	ثابت ساخت box *flex;
	ثابت css_unit_ctx *unit_len_ctx;

	صحیح main_سائز;
	صحیح cross_سائز;

	صحیح دستیاب_main;
	صحیح دستیاب_cross;

	bool horizontal;
	bool main_reversed;
	شمار css_flex_wrap_e wrap;

	ساخت flex_اشیاء {
		size_t گنتی;
		ساخت flex_شے_ڈیٹا *ڈیٹا;
	} شے;

	ساخت flex_سطریں {
		size_t گنتی;
		size_t alloc;
		ساخت flex_سطر_ڈیٹا *ڈیٹا;
	} سطر;
};

/**
 * Destroy a flex layout context
 *
 * \param[in] ctx  Flex layout context
 */
ساکن خالی layout_flex_ctx__destroy(ساخت flex_ctx *ctx)
{
	اگر (ctx != تہی) {
		آزاد(ctx->شے.ڈیٹا);
		آزاد(ctx->سطر.ڈیٹا);
		آزاد(ctx);
	}
}

/**
 * Create a flex layout context
 *
 * \param[in] content  HTML content containing flex box
 * \param[in] flex     Box to create layout context for
 * \return flex layout context or NULL on error
 */
ساکن ساخت flex_ctx *layout_flex_ctx__create(
		html_content *content,
		ثابت ساخت box *flex)
{
	ساخت flex_ctx *ctx;

	ctx = صفر_حافظہ_دو(1, حجم(*ctx));
	اگر (ctx == تہی) {
		واپس تہی;
	}
	ctx->سطر.alloc = 1;

	ctx->شے.گنتی = box_گنتی_children(flex);
	ctx->شے.ڈیٹا = صفر_حافظہ_دو(ctx->شے.گنتی, حجم(*ctx->شے.ڈیٹا));
	اگر (ctx->شے.ڈیٹا == تہی) {
		layout_flex_ctx__destroy(ctx);
		واپس تہی;
	}

	ctx->سطر.alloc = 1;
	ctx->سطر.ڈیٹا = صفر_حافظہ_دو(ctx->سطر.alloc, حجم(*ctx->سطر.ڈیٹا));
	اگر (ctx->سطر.ڈیٹا == تہی) {
		layout_flex_ctx__destroy(ctx);
		واپس تہی;
	}

	ctx->flex = flex;
	ctx->content = content;
	ctx->unit_len_ctx = &content->unit_len_ctx;

	ctx->wrap = css_computed_flex_wrap(flex->style);
	ctx->horizontal = lh__flex_main_is_horizontal(flex);
	ctx->main_reversed = lh__flex_direction_reversed(flex);

	واپس ctx;
}

/**
 * Find box side representing the start of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the start side.
 */
ساکن شمار box_side layout_flex_main_آغاز_side(
		ثابت ساخت flex_ctx *ctx)
{
	اگر (ctx->horizontal) {
		واپس (ctx->main_reversed) ? دایاں_2 : بایاں_2;
	} ورنہ {
		واپس (ctx->main_reversed) ? BOTTOM : چوٹی_2;
	}
}

/**
 * Find box side representing the end of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the end side.
 */
ساکن شمار box_side layout_flex_main_انتہا_side(
		ثابت ساخت flex_ctx *ctx)
{
	اگر (ctx->horizontal) {
		واپس (ctx->main_reversed) ? بایاں_2 : دایاں_2;
	} ورنہ {
		واپس (ctx->main_reversed) ? چوٹی_2 : BOTTOM;
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
ساکن bool layout_flex_شے(
		ثابت ساخت flex_ctx *ctx,
		ثابت ساخت flex_شے_ڈیٹا *شے,
		صحیح دستیاب_چوڑائی)
{
	bool success;
	ساخت box *b = شے->box;

	انتخاب (b->type) {
	حالت BOX_بلاک:
		success = layout_بلاک_context(b, -1, ctx->content);
		توڑو;
	حالت BOX_جدول:
		b->float_container = b->والد;
		success = layout_جدول(b, دستیاب_چوڑائی, ctx->content);
		b->float_container = تہی;
		توڑو;
	حالت BOX_FLEX:
		b->float_container = b->والد;
		success = layout_flex(b, دستیاب_چوڑائی, ctx->content);
		b->float_container = تہی;
		توڑو;
	طےشدہ:
		أكد(0 && "Bad flex item back type");
		success = false;
		توڑو;
	}

	اگر (!success) {
		NSLOG(flex, ERROR, "box %p: layout failed", b);
	}

	واپس success;
}

/**
 * Calculate an item's base and target main sizes.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] item             Item to get sizes of
 * \param[in] available_width  Available width in pixels
 * \return true on success false on failure
 */
ساکن اندرونی bool layout_flex_بنیاد_and_main_sizes(
		ثابت ساخت flex_ctx *ctx,
		ساخت flex_شے_ڈیٹا *شے,
		صحیح دستیاب_چوڑائی)
{
	ساخت box *b = شے->box;
	صحیح content_min_چوڑائی = b->min_چوڑائی;
	صحیح content_max_چوڑائی = b->max_چوڑائی;
	صحیح delta_outer_main = lh__delta_outer_main(ctx->flex, b);

	NSLOG(flex, DEEPDEBUG, "box %p: delta_outer_main: %i",
			b, delta_outer_main);

	اگر (شے->basis == CSS_FLEX_BASIS_سیٹ) {
		اگر (شے->basis_unit == CSS_UNIT_PCT) {
			شے->بنیاد_سائز = FPCT_کا_INT_TOINT(
					شے->basis_لمبائی,
					دستیاب_چوڑائی);
		} ورنہ {
			شے->بنیاد_سائز = FIXTOINT(css_unit_len2device_px(
					b->style, ctx->unit_len_ctx,
					شے->basis_لمبائی,
					شے->basis_unit));
		}

	} ورنہ اگر (شے->basis == CSS_FLEX_BASIS_AUTO) {
		شے->بنیاد_سائز = ctx->horizontal ? b->چوڑائی : b->اونچائی;
	} ورنہ {
		شے->بنیاد_سائز = AUTO;
	}

	اگر (ctx->horizontal == false) {
		اگر (b->چوڑائی == AUTO) {
			b->چوڑائی = min(max(content_min_چوڑائی, دستیاب_چوڑائی),
					content_max_چوڑائی);
			b->چوڑائی -= lh_delta_outer_چوڑائی(b);
		}

		اگر (!layout_flex_شے(ctx, شے, b->چوڑائی)) {
			واپس false;
		}
	}

	اگر (شے->بنیاد_سائز == AUTO) {
		اگر (ctx->horizontal == false) {
			شے->بنیاد_سائز = b->اونچائی;
		} ورنہ {
			شے->بنیاد_سائز = content_max_چوڑائی - delta_outer_main;
		}
	}

	شے->بنیاد_سائز += delta_outer_main;

	اگر (ctx->horizontal) {
		شے->بنیاد_سائز = min(شے->بنیاد_سائز, دستیاب_چوڑائی);
		شے->بنیاد_سائز = max(شے->بنیاد_سائز, content_min_چوڑائی);
	}

	شے->ہدف_main_سائز = شے->بنیاد_سائز;
	شے->main_سائز = شے->بنیاد_سائز;

	اگر (شے->max_main > 0 &&
	    شے->main_سائز > شے->max_main + delta_outer_main) {
		شے->main_سائز = شے->max_main + delta_outer_main;
	}

	اگر (شے->main_سائز < شے->min_main + delta_outer_main) {
		شے->main_سائز = شے->min_main + delta_outer_main;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: base_size: %i, main_size %i",
			b, شے->بنیاد_سائز, شے->main_سائز);

	واپس true;
}

/**
 * Fill out all item's data in a flex container.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] flex             Flex box
 * \param[in] available_width  Available width in pixels
 */
ساکن خالی layout_flex_ctx_populate_شے_ڈیٹا(
		ثابت ساخت flex_ctx *ctx,
		ثابت ساخت box *flex,
		صحیح دستیاب_چوڑائی)
{
	size_t i = 0;
	bool horizontal = ctx->horizontal;

	برائے (ساخت box *b = flex->children; b != تہی; b = b->اگلا) {
		ساخت flex_شے_ڈیٹا *شے = &ctx->شے.ڈیٹا[i++];

		b->float_container = b->والد;
		layout_ڈھونڈو_dimensions(ctx->unit_len_ctx, دستیاب_چوڑائی, -1,
				b, b->style, &b->چوڑائی, &b->اونچائی,
				horizontal ? &شے->max_main : &شے->max_cross,
				horizontal ? &شے->min_main : &شے->min_cross,
				horizontal ? &شے->max_cross : &شے->max_main,
				horizontal ? &شے->min_cross : &شے->min_main,
				b->margin, b->padding, b->border);
		b->float_container = تہی;

		NSLOG(flex, DEEPDEBUG, "flex-item box: %p: width: %i",
				b, b->چوڑائی);

		شے->box = b;
		شے->basis = css_computed_flex_basis(b->style,
				&شے->basis_لمبائی, &شے->basis_unit);

		css_computed_flex_shrink(b->style, &شے->shrink);
		css_computed_flex_grow(b->style, &شے->grow);

		layout_flex_بنیاد_and_main_sizes(ctx, شے, دستیاب_چوڑائی);
	}
}

/**
 * Ensure context's lines array has a free space
 *
 * \param[in] ctx  Flex layout context
 * \return true on success false on out of memory
 */
ساکن bool layout_flex_ctx_ensure_سطر(ساخت flex_ctx *ctx)
{
	ساخت flex_سطر_ڈیٹا *عارضی;
	size_t سطر_alloc = ctx->سطر.alloc * 2;

	اگر (ctx->سطر.alloc > ctx->سطر.گنتی) {
		واپس true;
	}

	عارضی = realloc(ctx->سطر.ڈیٹا, حجم(*ctx->سطر.ڈیٹا) * سطر_alloc);
	اگر (عارضی == تہی) {
		واپس false;
	}
	ctx->سطر.ڈیٹا = عارضی;

	memset(ctx->سطر.ڈیٹا + ctx->سطر.alloc, 0,
	       حجم(*ctx->سطر.ڈیٹا) * (سطر_alloc - ctx->سطر.alloc));
	ctx->سطر.alloc = سطر_alloc;

	واپس true;
}

/**
 * Assigns flex items to the line and returns the line
 *
 * \param[in] ctx         Flex layout context
 * \param[in] item_index  Index to first item to assign to this line
 * \return Pointer to the new line, or NULL on error.
 */
ساکن ساخت flex_سطر_ڈیٹا *layout_flex_build_سطر(ساخت flex_ctx *ctx,
		size_t شے_اشاریہ)
{
	شمار box_side آغاز_side = layout_flex_main_آغاز_side(ctx);
	شمار box_side انتہا_side = layout_flex_main_انتہا_side(ctx);
	ساخت flex_سطر_ڈیٹا *سطر;
	صحیح مستعمل_main = 0;

	اگر (!layout_flex_ctx_ensure_سطر(ctx)) {
		واپس تہی;
	}

	سطر = &ctx->سطر.ڈیٹا[ctx->سطر.گنتی];
	سطر->پہلا = شے_اشاریہ;

	NSLOG(flex, DEEPDEBUG, "flex container %p: available main: %i",
			ctx->flex, ctx->دستیاب_main);

	جب (شے_اشاریہ < ctx->شے.گنتی) {
		ساخت flex_شے_ڈیٹا *شے = &ctx->شے.ڈیٹا[شے_اشاریہ];
		ساخت box *b = شے->box;
		صحیح pos_main;

		pos_main = ctx->horizontal ?
				شے->main_سائز :
				b->اونچائی + lh__delta_outer_main(ctx->flex, b);

		اگر (ctx->wrap == CSS_FLEX_WRAP_NOWRAP ||
		    pos_main + مستعمل_main <= ctx->دستیاب_main ||
		    lh__box_is_absolute(شے->box) ||
		    ctx->دستیاب_main == AUTO ||
		    سطر->گنتی == 0 ||
		    pos_main == 0) {
			اگر (lh__box_is_absolute(شے->box) == false) {
				سطر->main_سائز += شے->main_سائز;
				مستعمل_main += pos_main;

				اگر (b->margin[آغاز_side] == AUTO) {
					سطر->main_auto_margin_گنتی++;
				}
				اگر (b->margin[انتہا_side] == AUTO) {
					سطر->main_auto_margin_گنتی++;
				}
			}
			شے->سطر = ctx->سطر.گنتی;
			سطر->گنتی++;
			شے_اشاریہ++;
		} ورنہ {
			توڑو;
		}
	}

	اگر (سطر->گنتی > 0) {
		ctx->سطر.گنتی++;
	} ورنہ {
		NSLOG(layout, ERROR, "Failed to fit any flex items");
	}

	واپس سطر;
}

/**
 * Freeze an item on a line
 *
 * \param[in] line  Line to containing item
 * \param[in] item  Item to freeze
 */
ساکن اندرونی خالی layout_flex_شے_freeze(
		ساخت flex_سطر_ڈیٹا *سطر,
		ساخت flex_شے_ڈیٹا *شے)
{
	شے->freeze = true;
	سطر->frozen++;

	اگر (!lh__box_is_absolute(شے->box)){
		سطر->مستعمل_main_سائز += شے->ہدف_main_سائز;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: "
			"Frozen at target_main_size: %i",
			شے->box, شے->ہدف_main_سائز);
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
ساکن اندرونی صحیح layout_flex_بقایا_free_main(
		ساخت flex_ctx *ctx,
		ساخت flex_سطر_ڈیٹا *سطر,
		css_fixed *unfrozen_ضربی_عدد_مجموع,
		صحیح initial_free_main,
		صحیح دستیاب_main,
		bool grow)
{
	صحیح بقایا_free_main = دستیاب_main;
	size_t شے_گنتی = سطر->پہلا + سطر->گنتی;

	*unfrozen_ضربی_عدد_مجموع = 0;

	برائے (size_t i = سطر->پہلا; i < شے_گنتی; i++) {
		ساخت flex_شے_ڈیٹا *شے = &ctx->شے.ڈیٹا[i];

		اگر (شے->freeze) {
			بقایا_free_main -= شے->ہدف_main_سائز;
		} ورنہ {
			بقایا_free_main -= شے->بنیاد_سائز;

			*unfrozen_ضربی_عدد_مجموع += grow ?
					شے->grow : شے->shrink;
		}
	}

	اگر (*unfrozen_ضربی_عدد_مجموع < F_1) {
		صحیح free_space = FIXTOINT(FMUL(INTTOFIX(initial_free_main),
				*unfrozen_ضربی_عدد_مجموع));

		اگر (free_space < بقایا_free_main) {
			بقایا_free_main = free_space;
		}
	}

	NSLOG(flex, DEEPDEBUG, "Remaining free space: %i",
			بقایا_free_main);

	واپس بقایا_free_main;
}

/**
 * Clamp flex item target main size and get min/max violations
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to align items on
 * return total violation in pixels
 */
ساکن اندرونی صحیح layout_flex__get_min_max_violations(
		ساخت flex_ctx *ctx,
		ساخت flex_سطر_ڈیٹا *سطر)
{

	صحیح کل_violation = 0;
	size_t شے_گنتی = سطر->پہلا + سطر->گنتی;

	برائے (size_t i = سطر->پہلا; i < شے_گنتی; i++) {
		ساخت flex_شے_ڈیٹا *شے = &ctx->شے.ڈیٹا[i];
		صحیح ہدف_main_سائز = شے->ہدف_main_سائز;

		NSLOG(flex, DEEPDEBUG, "item %p: target_main_size: %i",
					شے->box, ہدف_main_سائز);

		اگر (شے->freeze) {
			جاری;
		}

		اگر (شے->max_main > 0 &&
		    ہدف_main_سائز > شے->max_main) {
			ہدف_main_سائز = شے->max_main;
			شے->max_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: max_main: %i",
					شے->max_main);
		}

		اگر (ہدف_main_سائز < شے->min_main) {
			ہدف_main_سائز = شے->min_main;
			شے->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: min_main: %i",
					شے->min_main);
		}

		اگر (ہدف_main_سائز < شے->box->min_چوڑائی) {
			ہدف_main_سائز = شے->box->min_چوڑائی;
			شے->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: box min_width: %i",
					شے->box->min_چوڑائی);
		}

		اگر (ہدف_main_سائز < 0) {
			ہدف_main_سائز = 0;
			شے->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: less than 0");
		}

		کل_violation += ہدف_main_سائز - شے->ہدف_main_سائز;
		شے->ہدف_main_سائز = ہدف_main_سائز;
	}

	NSLOG(flex, DEEPDEBUG, "Total violation: %i", کل_violation);

	واپس کل_violation;
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
ساکن اندرونی خالی layout_flex__distribute_free_main(
		ساخت flex_ctx *ctx,
		ساخت flex_سطر_ڈیٹا *سطر,
		css_fixed unfrozen_ضربی_عدد_مجموع,
		صحیح بقایا_free_main,
		bool grow)
{
	size_t شے_گنتی = سطر->پہلا + سطر->گنتی;

	اگر (grow) {
		css_fixed remainder = 0;
		برائے (size_t i = سطر->پہلا; i < شے_گنتی; i++) {
			ساخت flex_شے_ڈیٹا *شے = &ctx->شے.ڈیٹا[i];
			css_fixed نتیجہ;
			css_fixed تناسب;

			اگر (شے->freeze) {
				جاری;
			}

			تناسب = FDIV(شے->grow, unfrozen_ضربی_عدد_مجموع);
			نتیجہ = FMUL(INTTOFIX(بقایا_free_main), تناسب) +
					remainder;

			شے->ہدف_main_سائز = شے->بنیاد_سائز +
					FIXTOINT(نتیجہ);
			remainder = FIXFRAC(نتیجہ);
		}
	} ورنہ {
		css_fixed scaled_shrink_ضربی_عدد_مجموع = 0;
		css_fixed remainder = 0;

		برائے (size_t i = سطر->پہلا; i < شے_گنتی; i++) {
			ساخت flex_شے_ڈیٹا *شے = &ctx->شے.ڈیٹا[i];
			css_fixed scaled_shrink_ضربی_عدد;

			اگر (شے->freeze) {
				جاری;
			}

			scaled_shrink_ضربی_عدد = FMUL(
					شے->shrink,
					INTTOFIX(شے->بنیاد_سائز));
			scaled_shrink_ضربی_عدد_مجموع += scaled_shrink_ضربی_عدد;
		}

		برائے (size_t i = سطر->پہلا; i < شے_گنتی; i++) {
			ساخت flex_شے_ڈیٹا *شے = &ctx->شے.ڈیٹا[i];
			css_fixed scaled_shrink_ضربی_عدد;
			css_fixed نتیجہ;
			css_fixed تناسب;

			اگر (شے->freeze) {
				جاری;
			} ورنہ اگر (scaled_shrink_ضربی_عدد_مجموع == 0) {
				شے->ہدف_main_سائز = شے->main_سائز;
				layout_flex_شے_freeze(سطر, شے);
				جاری;
			}

			scaled_shrink_ضربی_عدد = FMUL(
					شے->shrink,
					INTTOFIX(شے->بنیاد_سائز));
			تناسب = FDIV(scaled_shrink_ضربی_عدد,
			             scaled_shrink_ضربی_عدد_مجموع);
			نتیجہ = FMUL(INTTOFIX(مطلق_قدر(بقایا_free_main)),
			              تناسب) + remainder;

			شے->ہدف_main_سائز = شے->بنیاد_سائز -
					FIXTOINT(نتیجہ);
			remainder = FIXFRAC(نتیجہ);
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
ساکن bool layout_flex_resolve_سطر(
		ساخت flex_ctx *ctx,
		ساخت flex_سطر_ڈیٹا *سطر)
{
	size_t شے_گنتی = سطر->پہلا + سطر->گنتی;
	صحیح دستیاب_main = ctx->دستیاب_main;
	صحیح initial_free_main;
	bool grow;

	اگر (دستیاب_main == AUTO) {
		دستیاب_main = زیادہ_سے_زیادہ_عدد;
	}

	grow = (سطر->main_سائز < دستیاب_main);
	initial_free_main = دستیاب_main;

	NSLOG(flex, DEEPDEBUG, "box %p: line %zu: first: %zu, count: %zu",
			ctx->flex, سطر - ctx->سطر.ڈیٹا,
			سطر->پہلا, سطر->گنتی);
	NSLOG(flex, DEEPDEBUG, "Line main_size: %i, available_main: %i",
			سطر->main_سائز, دستیاب_main);

	برائے (size_t i = سطر->پہلا; i < شے_گنتی; i++) {
		ساخت flex_شے_ڈیٹا *شے = &ctx->شے.ڈیٹا[i];

		/* 3. Size inflexible items */
		اگر (grow) {
			اگر (شے->grow == 0 ||
			    شے->بنیاد_سائز > شے->main_سائز) {
				شے->ہدف_main_سائز = شے->main_سائز;
				layout_flex_شے_freeze(سطر, شے);
			}
		} ورنہ {
			اگر (شے->shrink == 0 ||
			    شے->بنیاد_سائز < شے->main_سائز) {
				شے->ہدف_main_سائز = شے->main_سائز;
				layout_flex_شے_freeze(سطر, شے);
			}
		}

		/* 4. Calculate initial free space */
		اگر (شے->freeze) {
			initial_free_main -= شے->ہدف_main_سائز;
		} ورنہ {
			initial_free_main -= شے->بنیاد_سائز;
		}
	}

	/* 5. Loop */
	جب (سطر->frozen < سطر->گنتی) {
		css_fixed unfrozen_ضربی_عدد_مجموع;
		صحیح بقایا_free_main;
		صحیح کل_violation;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: Resolver pass",
				ctx->flex);

		/* b */
		بقایا_free_main = layout_flex_بقایا_free_main(ctx,
				سطر, &unfrozen_ضربی_عدد_مجموع, initial_free_main,
				دستیاب_main, grow);

		/* c */
		اگر (بقایا_free_main != 0) {
			layout_flex__distribute_free_main(ctx,
					سطر, unfrozen_ضربی_عدد_مجموع,
					بقایا_free_main, grow);
		}

		/* d */
		کل_violation = layout_flex__get_min_max_violations(
				ctx, سطر);

		/* e */
		برائے (size_t i = سطر->پہلا; i < شے_گنتی; i++) {
			ساخت flex_شے_ڈیٹا *شے = &ctx->شے.ڈیٹا[i];

			اگر (شے->freeze) {
				جاری;
			}

			اگر (کل_violation == 0 ||
			    (کل_violation > 0 && شے->min_violation) ||
			    (کل_violation < 0 && شے->max_violation)) {
				layout_flex_شے_freeze(سطر, شے);
			}
		}
	}

	واپس true;
}

/**
 * Position items along a line
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to resolve
 * \return true on success, false on failure.
 */
ساکن bool layout_flex_رکھو_سطر_اشیاء_main(
		ساخت flex_ctx *ctx,
		ساخت flex_سطر_ڈیٹا *سطر)
{
	صحیح main_pos = ctx->flex->padding[layout_flex_main_آغاز_side(ctx)];
	صحیح post_multiplier = ctx->main_reversed ? 0 : 1;
	صحیح pre_multiplier = ctx->main_reversed ? -1 : 0;
	size_t شے_گنتی = سطر->پہلا + سطر->گنتی;
	صحیح extra_remainder = 0;
	صحیح extra = 0;

	اگر (ctx->main_reversed) {
		main_pos = lh_box_سائز_main(ctx->horizontal, ctx->flex) -
				main_pos;
	}

	اگر (ctx->دستیاب_main != AUTO &&
	    ctx->دستیاب_main != UNKNOWN_چوڑائی &&
	    ctx->دستیاب_main > سطر->مستعمل_main_سائز) {
		اگر (سطر->main_auto_margin_گنتی > 0) {
			extra = ctx->دستیاب_main - سطر->مستعمل_main_سائز;

			extra_remainder = extra % سطر->main_auto_margin_گنتی;
			extra /= سطر->main_auto_margin_گنتی;
		}
	}

	برائے (size_t i = سطر->پہلا; i < شے_گنتی; i++) {
		شمار box_side main_انتہا = ctx->horizontal ? دایاں_2 : BOTTOM;
		شمار box_side main_آغاز = ctx->horizontal ? بایاں_2 : چوٹی_2;
		ساخت flex_شے_ڈیٹا *شے = &ctx->شے.ڈیٹا[i];
		ساخت box *b = شے->box;
		صحیح extra_کل = 0;
		صحیح extra_post = 0;
		صحیح extra_pre = 0;
		صحیح box_سائز_main;
		صحیح *box_pos_main;

		اگر (ctx->horizontal) {
			b->چوڑائی = شے->ہدف_main_سائز -
					lh_delta_outer_چوڑائی(b);

			اگر (!layout_flex_شے(ctx, شے, b->چوڑائی)) {
				واپس false;
			}
		}

		box_سائز_main = lh_box_سائز_main(ctx->horizontal, b);
		box_pos_main = ctx->horizontal ? &b->x : &b->y;

		اگر (!lh__box_is_absolute(b)) {
			اگر (b->margin[main_آغاز] == AUTO) {
				extra_pre = extra + extra_remainder;
			}
			اگر (b->margin[main_انتہا] == AUTO) {
				extra_post = extra + extra_remainder;
			}
			extra_کل = extra_pre + extra_post;

			main_pos += pre_multiplier *
					(extra_کل + box_سائز_main +
					 lh__delta_outer_main(ctx->flex, b));
		}

		*box_pos_main = main_pos + lh__non_auto_margin(b, main_آغاز) +
				extra_pre + b->border[main_آغاز].چوڑائی;

		اگر (!lh__box_is_absolute(b)) {
			صحیح cross_سائز;
			صحیح box_سائز_cross = lh_box_سائز_cross(
					ctx->horizontal, b);

			main_pos += post_multiplier *
					(extra_کل + box_سائز_main +
					 lh__delta_outer_main(ctx->flex, b));

			cross_سائز = box_سائز_cross + lh__delta_outer_cross(
					ctx->flex, b);
			اگر (سطر->cross_سائز < cross_سائز) {
				سطر->cross_سائز = cross_سائز;
			}
		}
	}

	واپس true;
}

/**
 * Collect items onto lines and place items along the lines
 *
 * \param[in] ctx   Flex layout context
 * \return true on success, false on failure.
 */
ساکن bool layout_flex_collect_اشیاء_into_سطریں(
		ساخت flex_ctx *ctx)
{
	size_t pos = 0;

	جب (pos < ctx->شے.گنتی) {
		ساخت flex_سطر_ڈیٹا *سطر;

		سطر = layout_flex_build_سطر(ctx, pos);
		اگر (سطر == تہی) {
			واپس false;
		}

		pos += سطر->گنتی;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: "
				"fitted: %zu (total: %zu/%zu)",
				ctx->flex, سطر->گنتی,
				pos, ctx->شے.گنتی);

		اگر (!layout_flex_resolve_سطر(ctx, سطر)) {
			واپس false;
		}

		اگر (!layout_flex_رکھو_سطر_اشیاء_main(ctx, سطر)) {
			واپس false;
		}

		ctx->cross_سائز += سطر->cross_سائز;
		اگر (ctx->main_سائز < سطر->main_سائز) {
			ctx->main_سائز = سطر->main_سائز;
		}
	}

	واپس true;
}

/**
 * Align items on a line.
 *
 * \param[in] ctx    Flex layout context
 * \param[in] line   Line to align items on
 * \param[in] extra  Extra line width in pixels
 */
ساکن خالی layout_flex_رکھو_سطر_اشیاء_cross(ساخت flex_ctx *ctx,
		ساخت flex_سطر_ڈیٹا *سطر, صحیح extra)
{
	شمار box_side cross_آغاز = ctx->horizontal ? چوٹی_2 : بایاں_2;
	size_t شے_گنتی = سطر->پہلا + سطر->گنتی;

	برائے (size_t i = سطر->پہلا; i < شے_گنتی; i++) {
		ساخت flex_شے_ڈیٹا *شے = &ctx->شے.ڈیٹا[i];
		ساخت box *b = شے->box;
		صحیح cross_free_space;
		صحیح *box_سائز_cross;
		صحیح *box_pos_cross;

		box_pos_cross = ctx->horizontal ? &b->y : &b->x;
		box_سائز_cross = lh_box_سائز_cross_ptr(ctx->horizontal, b);

		cross_free_space = سطر->cross_سائز + extra - *box_سائز_cross -
				lh__delta_outer_cross(ctx->flex, b);

		انتخاب (lh__box_align_self(ctx->flex, b)) {
		طےشدہ:
		حالت CSS_ALIGN_SELF_STRETCH:
			اگر (lh_box_سائز_cross_is_auto(ctx->horizontal, b)) {
				*box_سائز_cross += cross_free_space;

				/* Relayout children for stretch. */
				اگر (!layout_flex_شے(ctx, شے, b->چوڑائی)) {
					واپس;
				}
			}
			fallthrough;
		حالت CSS_ALIGN_SELF_FLEX_آغاز:
			*box_pos_cross = ctx->flex->padding[cross_آغاز] +
					سطر->pos +
					lh__non_auto_margin(b, cross_آغاز) +
					b->border[cross_آغاز].چوڑائی;
			توڑو;

		حالت CSS_ALIGN_SELF_FLEX_انتہا:
			*box_pos_cross = ctx->flex->padding[cross_آغاز] +
					سطر->pos + cross_free_space +
					lh__non_auto_margin(b, cross_آغاز) +
					b->border[cross_آغاز].چوڑائی;
			توڑو;

		حالت CSS_ALIGN_SELF_BASELINE:
		حالت CSS_ALIGN_SELF_CENTER:
			*box_pos_cross = ctx->flex->padding[cross_آغاز] +
					سطر->pos + cross_free_space / 2 +
					lh__non_auto_margin(b, cross_آغاز) +
					b->border[cross_آغاز].چوڑائی;
			توڑو;
		}
	}
}

/**
 * Place the lines and align the items on the line.
 *
 * \param[in] ctx  Flex layout context
 */
ساکن خالی layout_flex_رکھو_سطریں(ساخت flex_ctx *ctx)
{
	bool reversed = ctx->wrap == CSS_FLEX_WRAP_WRAP_الٹ;
	صحیح سطر_pos = reversed ? ctx->cross_سائز : 0;
	صحیح post_multiplier = reversed ? 0 : 1;
	صحیح pre_multiplier = reversed ? -1 : 0;
	صحیح extra_remainder = 0;
	صحیح extra = 0;

	اگر (ctx->دستیاب_cross != AUTO &&
	    ctx->دستیاب_cross > ctx->cross_سائز &&
	    ctx->سطر.گنتی > 0) {
		extra = ctx->دستیاب_cross - ctx->cross_سائز;

		extra_remainder = extra % ctx->سطر.گنتی;
		extra /= ctx->سطر.گنتی;
	}

	برائے (size_t i = 0; i < ctx->سطر.گنتی; i++) {
		ساخت flex_سطر_ڈیٹا *سطر = &ctx->سطر.ڈیٹا[i];

		سطر_pos += pre_multiplier * سطر->cross_سائز;
		سطر->pos = سطر_pos;
		سطر_pos += post_multiplier * سطر->cross_سائز +
				extra + extra_remainder;

		layout_flex_رکھو_سطر_اشیاء_cross(ctx, سطر,
				extra + extra_remainder);

		اگر (extra_remainder > 0) {
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
bool layout_flex(ساخت box *flex, صحیح دستیاب_چوڑائی,
		html_content *content)
{
	صحیح max_اونچائی, min_اونچائی;
	ساخت flex_ctx *ctx;
	bool success = false;

	ctx = layout_flex_ctx__create(content, flex);
	اگر (ctx == تہی) {
		واپس false;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: %s, available_width %i, width: %i",
			flex, ctx->horizontal ? "horizontal" : "vertical",
			دستیاب_چوڑائی, flex->چوڑائی);

	layout_ڈھونڈو_dimensions(
			ctx->unit_len_ctx, دستیاب_چوڑائی, -1,
			flex, flex->style, تہی, &flex->اونچائی,
			تہی, تہی, &max_اونچائی, &min_اونچائی,
			flex->margin, flex->padding, flex->border);

	دستیاب_چوڑائی = min(دستیاب_چوڑائی, flex->چوڑائی);

	اگر (ctx->horizontal) {
		ctx->دستیاب_main = دستیاب_چوڑائی;
		ctx->دستیاب_cross = ctx->flex->اونچائی;
	} ورنہ {
		ctx->دستیاب_main = ctx->flex->اونچائی;
		ctx->دستیاب_cross = دستیاب_چوڑائی;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: available_main: %i",
			flex, ctx->دستیاب_main);
	NSLOG(flex, DEEPDEBUG, "box %p: available_cross: %i",
			flex, ctx->دستیاب_cross);

	layout_flex_ctx_populate_شے_ڈیٹا(ctx, flex, دستیاب_چوڑائی);

	/* Place items onto lines. */
	success = layout_flex_collect_اشیاء_into_سطریں(ctx);
	اگر (!success) {
		جاؤ cleanup;
	}

	layout_flex_رکھو_سطریں(ctx);

	اگر (flex->اونچائی == AUTO) {
		flex->اونچائی = ctx->horizontal ?
				ctx->cross_سائز :
				ctx->main_سائز;
	}

	اگر (flex->اونچائی != AUTO) {
		اگر (max_اونچائی >= 0 && flex->اونچائی > max_اونچائی) {
			flex->اونچائی = max_اونچائی;
		}
		اگر (min_اونچائی >  0 && flex->اونچائی < min_اونچائی) {
			flex->اونچائی = min_اونچائی;
		}
	}

	success = true;

cleanup:
	layout_flex_ctx__destroy(ctx);

	NSLOG(flex, DEEPDEBUG, "box %p: %s: w: %i, h: %i", flex,
			success ? "success" : "failure",
			flex->چوڑائی, flex->اونچائی);
	واپس success;
}
