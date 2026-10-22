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
بنية flex_شے_انکڑے {
	تعداد css_flex_basis_e basis;
	css_fixed basis_لمائی;
	css_unit basis_unit;
	بنية box *box;

	css_fixed shrink;
	css_fixed grow;

	صحيح min_main;
	صحيح max_main;
	صحيح min_cross;
	صحيح max_cross;

	صحيح نشانہ_main_ناپ;
	صحيح ادھار_ناپ;
	صحيح main_ناپ;
	size_t سطر;

	bool freeze;
	bool min_violation;
	bool max_violation;
};

/**
 * Flex line data
 */
بنية flex_سطر_انکڑے {
	صحيح main_ناپ;
	صحيح cross_ناپ;

	صحيح ورتیا_main_ناپ;
	صحيح main_auto_margin_گنتی;

	صحيح pos;

	size_t پہلا;
	size_t گنتی;
	size_t frozen;
};

/**
 * Flex layout context
 */
بنية flex_ctx {
	html_content *content;
	ثابت بنية box *flex;
	ثابت css_unit_ctx *unit_len_ctx;

	صحيح main_ناپ;
	صحيح cross_ناپ;

	صحيح دستیاب_main;
	صحيح دستیاب_cross;

	bool horizontal;
	bool main_reversed;
	تعداد css_flex_wrap_e wrap;

	بنية flex_اشیاء {
		size_t گنتی;
		بنية flex_شے_انکڑے *انکڑے;
	} شے;

	بنية flex_سطریں {
		size_t گنتی;
		size_t alloc;
		بنية flex_سطر_انکڑے *انکڑے;
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
		حرر(ctx->شے.انکڑے);
		حرر(ctx->سطر.انکڑے);
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

	ctx = صفر_حافظہ_دیو(1, حجم(*ctx));
	إذا (ctx == عدم) {
		أرجع عدم;
	}
	ctx->سطر.alloc = 1;

	ctx->شے.گنتی = box_گنتی_children(flex);
	ctx->شے.انکڑے = صفر_حافظہ_دیو(ctx->شے.گنتی, حجم(*ctx->شے.انکڑے));
	إذا (ctx->شے.انکڑے == عدم) {
		layout_flex_ctx__destroy(ctx);
		أرجع عدم;
	}

	ctx->سطر.alloc = 1;
	ctx->سطر.انکڑے = صفر_حافظہ_دیو(ctx->سطر.alloc, حجم(*ctx->سطر.انکڑے));
	إذا (ctx->سطر.انکڑے == عدم) {
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
		أرجع (ctx->main_reversed) ? دایاں_2 : بایاں_2;
	} وإلا {
		أرجع (ctx->main_reversed) ? BOTTOM : چوٹی_2;
	}
}

/**
 * Find box side representing the end of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the end side.
 */
ساكن تعداد box_side layout_flex_main_سرا_side(
		ثابت بنية flex_ctx *ctx)
{
	إذا (ctx->horizontal) {
		أرجع (ctx->main_reversed) ? بایاں_2 : دایاں_2;
	} وإلا {
		أرجع (ctx->main_reversed) ? چوٹی_2 : BOTTOM;
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
ساكن bool layout_flex_شے(
		ثابت بنية flex_ctx *ctx,
		ثابت بنية flex_شے_انکڑے *شے,
		صحيح دستیاب_چوڑائی)
{
	bool success;
	بنية box *b = شے->box;

	بدل (b->type) {
	حالة BOX_بلاک:
		success = layout_بلاک_context(b, -1, ctx->content);
		اكسر;
	حالة BOX_جدول:
		b->float_container = b->ماپا;
		success = layout_جدول(b, دستیاب_چوڑائی, ctx->content);
		b->float_container = عدم;
		اكسر;
	حالة BOX_FLEX:
		b->float_container = b->ماپا;
		success = layout_flex(b, دستیاب_چوڑائی, ctx->content);
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
ساكن مضمن bool layout_flex_ادھار_and_main_sizes(
		ثابت بنية flex_ctx *ctx,
		بنية flex_شے_انکڑے *شے,
		صحيح دستیاب_چوڑائی)
{
	بنية box *b = شے->box;
	صحيح content_min_چوڑائی = b->min_چوڑائی;
	صحيح content_max_چوڑائی = b->max_چوڑائی;
	صحيح delta_outer_main = lh__delta_outer_main(ctx->flex, b);

	NSLOG(flex, DEEPDEBUG, "box %p: delta_outer_main: %i",
			b, delta_outer_main);

	إذا (شے->basis == CSS_FLEX_BASIS_سیٹ) {
		إذا (شے->basis_unit == CSS_UNIT_PCT) {
			شے->ادھار_ناپ = FPCT_دا_INT_TOINT(
					شے->basis_لمائی,
					دستیاب_چوڑائی);
		} وإلا {
			شے->ادھار_ناپ = FIXTOINT(css_unit_len2device_px(
					b->style, ctx->unit_len_ctx,
					شے->basis_لمائی,
					شے->basis_unit));
		}

	} وإلا إذا (شے->basis == CSS_FLEX_BASIS_AUTO) {
		شے->ادھار_ناپ = ctx->horizontal ? b->چوڑائی : b->اچائی;
	} وإلا {
		شے->ادھار_ناپ = AUTO;
	}

	إذا (ctx->horizontal == false) {
		إذا (b->چوڑائی == AUTO) {
			b->چوڑائی = min(max(content_min_چوڑائی, دستیاب_چوڑائی),
					content_max_چوڑائی);
			b->چوڑائی -= lh_delta_outer_چوڑائی(b);
		}

		إذا (!layout_flex_شے(ctx, شے, b->چوڑائی)) {
			أرجع false;
		}
	}

	إذا (شے->ادھار_ناپ == AUTO) {
		إذا (ctx->horizontal == false) {
			شے->ادھار_ناپ = b->اچائی;
		} وإلا {
			شے->ادھار_ناپ = content_max_چوڑائی - delta_outer_main;
		}
	}

	شے->ادھار_ناپ += delta_outer_main;

	إذا (ctx->horizontal) {
		شے->ادھار_ناپ = min(شے->ادھار_ناپ, دستیاب_چوڑائی);
		شے->ادھار_ناپ = max(شے->ادھار_ناپ, content_min_چوڑائی);
	}

	شے->نشانہ_main_ناپ = شے->ادھار_ناپ;
	شے->main_ناپ = شے->ادھار_ناپ;

	إذا (شے->max_main > 0 &&
	    شے->main_ناپ > شے->max_main + delta_outer_main) {
		شے->main_ناپ = شے->max_main + delta_outer_main;
	}

	إذا (شے->main_ناپ < شے->min_main + delta_outer_main) {
		شے->main_ناپ = شے->min_main + delta_outer_main;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: base_size: %i, main_size %i",
			b, شے->ادھار_ناپ, شے->main_ناپ);

	أرجع true;
}

/**
 * Fill out all item's data in a flex container.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] flex             Flex box
 * \param[in] available_width  Available width in pixels
 */
ساكن فراغ layout_flex_ctx_populate_شے_انکڑے(
		ثابت بنية flex_ctx *ctx,
		ثابت بنية box *flex,
		صحيح دستیاب_چوڑائی)
{
	size_t i = 0;
	bool horizontal = ctx->horizontal;

	لكل (بنية box *b = flex->children; b != عدم; b = b->اگلا) {
		بنية flex_شے_انکڑے *شے = &ctx->شے.انکڑے[i++];

		b->float_container = b->ماپا;
		layout_لبھو_dimensions(ctx->unit_len_ctx, دستیاب_چوڑائی, -1,
				b, b->style, &b->چوڑائی, &b->اچائی,
				horizontal ? &شے->max_main : &شے->max_cross,
				horizontal ? &شے->min_main : &شے->min_cross,
				horizontal ? &شے->max_cross : &شے->max_main,
				horizontal ? &شے->min_cross : &شے->min_main,
				b->margin, b->padding, b->border);
		b->float_container = عدم;

		NSLOG(flex, DEEPDEBUG, "flex-item box: %p: width: %i",
				b, b->چوڑائی);

		شے->box = b;
		شے->basis = css_computed_flex_basis(b->style,
				&شے->basis_لمائی, &شے->basis_unit);

		css_computed_flex_shrink(b->style, &شے->shrink);
		css_computed_flex_grow(b->style, &شے->grow);

		layout_flex_ادھار_and_main_sizes(ctx, شے, دستیاب_چوڑائی);
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
	بنية flex_سطر_انکڑے *عارضی;
	size_t سطر_alloc = ctx->سطر.alloc * 2;

	إذا (ctx->سطر.alloc > ctx->سطر.گنتی) {
		أرجع true;
	}

	عارضی = realloc(ctx->سطر.انکڑے, حجم(*ctx->سطر.انکڑے) * سطر_alloc);
	إذا (عارضی == عدم) {
		أرجع false;
	}
	ctx->سطر.انکڑے = عارضی;

	memset(ctx->سطر.انکڑے + ctx->سطر.alloc, 0,
	       حجم(*ctx->سطر.انکڑے) * (سطر_alloc - ctx->سطر.alloc));
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
ساكن بنية flex_سطر_انکڑے *layout_flex_build_سطر(بنية flex_ctx *ctx,
		size_t شے_اشاریہ)
{
	تعداد box_side شروع_side = layout_flex_main_شروع_side(ctx);
	تعداد box_side سرا_side = layout_flex_main_سرا_side(ctx);
	بنية flex_سطر_انکڑے *سطر;
	صحيح ورتیا_main = 0;

	إذا (!layout_flex_ctx_ensure_سطر(ctx)) {
		أرجع عدم;
	}

	سطر = &ctx->سطر.انکڑے[ctx->سطر.گنتی];
	سطر->پہلا = شے_اشاریہ;

	NSLOG(flex, DEEPDEBUG, "flex container %p: available main: %i",
			ctx->flex, ctx->دستیاب_main);

	طالما (شے_اشاریہ < ctx->شے.گنتی) {
		بنية flex_شے_انکڑے *شے = &ctx->شے.انکڑے[شے_اشاریہ];
		بنية box *b = شے->box;
		صحيح pos_main;

		pos_main = ctx->horizontal ?
				شے->main_ناپ :
				b->اچائی + lh__delta_outer_main(ctx->flex, b);

		إذا (ctx->wrap == CSS_FLEX_WRAP_NOWRAP ||
		    pos_main + ورتیا_main <= ctx->دستیاب_main ||
		    lh__box_is_absolute(شے->box) ||
		    ctx->دستیاب_main == AUTO ||
		    سطر->گنتی == 0 ||
		    pos_main == 0) {
			إذا (lh__box_is_absolute(شے->box) == false) {
				سطر->main_ناپ += شے->main_ناپ;
				ورتیا_main += pos_main;

				إذا (b->margin[شروع_side] == AUTO) {
					سطر->main_auto_margin_گنتی++;
				}
				إذا (b->margin[سرا_side] == AUTO) {
					سطر->main_auto_margin_گنتی++;
				}
			}
			شے->سطر = ctx->سطر.گنتی;
			سطر->گنتی++;
			شے_اشاریہ++;
		} وإلا {
			اكسر;
		}
	}

	إذا (سطر->گنتی > 0) {
		ctx->سطر.گنتی++;
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
ساكن مضمن فراغ layout_flex_شے_freeze(
		بنية flex_سطر_انکڑے *سطر,
		بنية flex_شے_انکڑے *شے)
{
	شے->freeze = true;
	سطر->frozen++;

	إذا (!lh__box_is_absolute(شے->box)){
		سطر->ورتیا_main_ناپ += شے->نشانہ_main_ناپ;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: "
			"Frozen at target_main_size: %i",
			شے->box, شے->نشانہ_main_ناپ);
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
ساكن مضمن صحيح layout_flex_بچیا_free_main(
		بنية flex_ctx *ctx,
		بنية flex_سطر_انکڑے *سطر,
		css_fixed *unfrozen_ضارب_جوڑ,
		صحيح initial_free_main,
		صحيح دستیاب_main,
		bool grow)
{
	صحيح بچیا_free_main = دستیاب_main;
	size_t شے_گنتی = سطر->پہلا + سطر->گنتی;

	*unfrozen_ضارب_جوڑ = 0;

	لكل (size_t i = سطر->پہلا; i < شے_گنتی; i++) {
		بنية flex_شے_انکڑے *شے = &ctx->شے.انکڑے[i];

		إذا (شے->freeze) {
			بچیا_free_main -= شے->نشانہ_main_ناپ;
		} وإلا {
			بچیا_free_main -= شے->ادھار_ناپ;

			*unfrozen_ضارب_جوڑ += grow ?
					شے->grow : شے->shrink;
		}
	}

	إذا (*unfrozen_ضارب_جوڑ < F_1) {
		صحيح free_space = FIXTOINT(FMUL(INTTOFIX(initial_free_main),
				*unfrozen_ضارب_جوڑ));

		إذا (free_space < بچیا_free_main) {
			بچیا_free_main = free_space;
		}
	}

	NSLOG(flex, DEEPDEBUG, "Remaining free space: %i",
			بچیا_free_main);

	أرجع بچیا_free_main;
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
		بنية flex_سطر_انکڑے *سطر)
{

	صحيح کل_violation = 0;
	size_t شے_گنتی = سطر->پہلا + سطر->گنتی;

	لكل (size_t i = سطر->پہلا; i < شے_گنتی; i++) {
		بنية flex_شے_انکڑے *شے = &ctx->شے.انکڑے[i];
		صحيح نشانہ_main_ناپ = شے->نشانہ_main_ناپ;

		NSLOG(flex, DEEPDEBUG, "item %p: target_main_size: %i",
					شے->box, نشانہ_main_ناپ);

		إذا (شے->freeze) {
			استمر;
		}

		إذا (شے->max_main > 0 &&
		    نشانہ_main_ناپ > شے->max_main) {
			نشانہ_main_ناپ = شے->max_main;
			شے->max_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: max_main: %i",
					شے->max_main);
		}

		إذا (نشانہ_main_ناپ < شے->min_main) {
			نشانہ_main_ناپ = شے->min_main;
			شے->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: min_main: %i",
					شے->min_main);
		}

		إذا (نشانہ_main_ناپ < شے->box->min_چوڑائی) {
			نشانہ_main_ناپ = شے->box->min_چوڑائی;
			شے->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: box min_width: %i",
					شے->box->min_چوڑائی);
		}

		إذا (نشانہ_main_ناپ < 0) {
			نشانہ_main_ناپ = 0;
			شے->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: less than 0");
		}

		کل_violation += نشانہ_main_ناپ - شے->نشانہ_main_ناپ;
		شے->نشانہ_main_ناپ = نشانہ_main_ناپ;
	}

	NSLOG(flex, DEEPDEBUG, "Total violation: %i", کل_violation);

	أرجع کل_violation;
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
		بنية flex_سطر_انکڑے *سطر,
		css_fixed unfrozen_ضارب_جوڑ,
		صحيح بچیا_free_main,
		bool grow)
{
	size_t شے_گنتی = سطر->پہلا + سطر->گنتی;

	إذا (grow) {
		css_fixed remainder = 0;
		لكل (size_t i = سطر->پہلا; i < شے_گنتی; i++) {
			بنية flex_شے_انکڑے *شے = &ctx->شے.انکڑے[i];
			css_fixed نتیجہ;
			css_fixed تناسب;

			إذا (شے->freeze) {
				استمر;
			}

			تناسب = FDIV(شے->grow, unfrozen_ضارب_جوڑ);
			نتیجہ = FMUL(INTTOFIX(بچیا_free_main), تناسب) +
					remainder;

			شے->نشانہ_main_ناپ = شے->ادھار_ناپ +
					FIXTOINT(نتیجہ);
			remainder = FIXFRAC(نتیجہ);
		}
	} وإلا {
		css_fixed scaled_shrink_ضارب_جوڑ = 0;
		css_fixed remainder = 0;

		لكل (size_t i = سطر->پہلا; i < شے_گنتی; i++) {
			بنية flex_شے_انکڑے *شے = &ctx->شے.انکڑے[i];
			css_fixed scaled_shrink_ضارب;

			إذا (شے->freeze) {
				استمر;
			}

			scaled_shrink_ضارب = FMUL(
					شے->shrink,
					INTTOFIX(شے->ادھار_ناپ));
			scaled_shrink_ضارب_جوڑ += scaled_shrink_ضارب;
		}

		لكل (size_t i = سطر->پہلا; i < شے_گنتی; i++) {
			بنية flex_شے_انکڑے *شے = &ctx->شے.انکڑے[i];
			css_fixed scaled_shrink_ضارب;
			css_fixed نتیجہ;
			css_fixed تناسب;

			إذا (شے->freeze) {
				استمر;
			} وإلا إذا (scaled_shrink_ضارب_جوڑ == 0) {
				شے->نشانہ_main_ناپ = شے->main_ناپ;
				layout_flex_شے_freeze(سطر, شے);
				استمر;
			}

			scaled_shrink_ضارب = FMUL(
					شے->shrink,
					INTTOFIX(شے->ادھار_ناپ));
			تناسب = FDIV(scaled_shrink_ضارب,
			             scaled_shrink_ضارب_جوڑ);
			نتیجہ = FMUL(INTTOFIX(مطلق_قدر(بچیا_free_main)),
			              تناسب) + remainder;

			شے->نشانہ_main_ناپ = شے->ادھار_ناپ -
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
ساكن bool layout_flex_resolve_سطر(
		بنية flex_ctx *ctx,
		بنية flex_سطر_انکڑے *سطر)
{
	size_t شے_گنتی = سطر->پہلا + سطر->گنتی;
	صحيح دستیاب_main = ctx->دستیاب_main;
	صحيح initial_free_main;
	bool grow;

	إذا (دستیاب_main == AUTO) {
		دستیاب_main = ودھ_توں_ودھ_عدد;
	}

	grow = (سطر->main_ناپ < دستیاب_main);
	initial_free_main = دستیاب_main;

	NSLOG(flex, DEEPDEBUG, "box %p: line %zu: first: %zu, count: %zu",
			ctx->flex, سطر - ctx->سطر.انکڑے,
			سطر->پہلا, سطر->گنتی);
	NSLOG(flex, DEEPDEBUG, "Line main_size: %i, available_main: %i",
			سطر->main_ناپ, دستیاب_main);

	لكل (size_t i = سطر->پہلا; i < شے_گنتی; i++) {
		بنية flex_شے_انکڑے *شے = &ctx->شے.انکڑے[i];

		/* 3. Size inflexible items */
		إذا (grow) {
			إذا (شے->grow == 0 ||
			    شے->ادھار_ناپ > شے->main_ناپ) {
				شے->نشانہ_main_ناپ = شے->main_ناپ;
				layout_flex_شے_freeze(سطر, شے);
			}
		} وإلا {
			إذا (شے->shrink == 0 ||
			    شے->ادھار_ناپ < شے->main_ناپ) {
				شے->نشانہ_main_ناپ = شے->main_ناپ;
				layout_flex_شے_freeze(سطر, شے);
			}
		}

		/* 4. Calculate initial free space */
		إذا (شے->freeze) {
			initial_free_main -= شے->نشانہ_main_ناپ;
		} وإلا {
			initial_free_main -= شے->ادھار_ناپ;
		}
	}

	/* 5. Loop */
	طالما (سطر->frozen < سطر->گنتی) {
		css_fixed unfrozen_ضارب_جوڑ;
		صحيح بچیا_free_main;
		صحيح کل_violation;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: Resolver pass",
				ctx->flex);

		/* b */
		بچیا_free_main = layout_flex_بچیا_free_main(ctx,
				سطر, &unfrozen_ضارب_جوڑ, initial_free_main,
				دستیاب_main, grow);

		/* c */
		إذا (بچیا_free_main != 0) {
			layout_flex__distribute_free_main(ctx,
					سطر, unfrozen_ضارب_جوڑ,
					بچیا_free_main, grow);
		}

		/* d */
		کل_violation = layout_flex__get_min_max_violations(
				ctx, سطر);

		/* e */
		لكل (size_t i = سطر->پہلا; i < شے_گنتی; i++) {
			بنية flex_شے_انکڑے *شے = &ctx->شے.انکڑے[i];

			إذا (شے->freeze) {
				استمر;
			}

			إذا (کل_violation == 0 ||
			    (کل_violation > 0 && شے->min_violation) ||
			    (کل_violation < 0 && شے->max_violation)) {
				layout_flex_شے_freeze(سطر, شے);
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
ساكن bool layout_flex_رکھو_سطر_اشیاء_main(
		بنية flex_ctx *ctx,
		بنية flex_سطر_انکڑے *سطر)
{
	صحيح main_pos = ctx->flex->padding[layout_flex_main_شروع_side(ctx)];
	صحيح post_multiplier = ctx->main_reversed ? 0 : 1;
	صحيح pre_multiplier = ctx->main_reversed ? -1 : 0;
	size_t شے_گنتی = سطر->پہلا + سطر->گنتی;
	صحيح extra_remainder = 0;
	صحيح extra = 0;

	إذا (ctx->main_reversed) {
		main_pos = lh_box_ناپ_main(ctx->horizontal, ctx->flex) -
				main_pos;
	}

	إذا (ctx->دستیاب_main != AUTO &&
	    ctx->دستیاب_main != UNKNOWN_چوڑائی &&
	    ctx->دستیاب_main > سطر->ورتیا_main_ناپ) {
		إذا (سطر->main_auto_margin_گنتی > 0) {
			extra = ctx->دستیاب_main - سطر->ورتیا_main_ناپ;

			extra_remainder = extra % سطر->main_auto_margin_گنتی;
			extra /= سطر->main_auto_margin_گنتی;
		}
	}

	لكل (size_t i = سطر->پہلا; i < شے_گنتی; i++) {
		تعداد box_side main_سرا = ctx->horizontal ? دایاں_2 : BOTTOM;
		تعداد box_side main_شروع = ctx->horizontal ? بایاں_2 : چوٹی_2;
		بنية flex_شے_انکڑے *شے = &ctx->شے.انکڑے[i];
		بنية box *b = شے->box;
		صحيح extra_کل = 0;
		صحيح extra_post = 0;
		صحيح extra_pre = 0;
		صحيح box_ناپ_main;
		صحيح *box_pos_main;

		إذا (ctx->horizontal) {
			b->چوڑائی = شے->نشانہ_main_ناپ -
					lh_delta_outer_چوڑائی(b);

			إذا (!layout_flex_شے(ctx, شے, b->چوڑائی)) {
				أرجع false;
			}
		}

		box_ناپ_main = lh_box_ناپ_main(ctx->horizontal, b);
		box_pos_main = ctx->horizontal ? &b->x : &b->y;

		إذا (!lh__box_is_absolute(b)) {
			إذا (b->margin[main_شروع] == AUTO) {
				extra_pre = extra + extra_remainder;
			}
			إذا (b->margin[main_سرا] == AUTO) {
				extra_post = extra + extra_remainder;
			}
			extra_کل = extra_pre + extra_post;

			main_pos += pre_multiplier *
					(extra_کل + box_ناپ_main +
					 lh__delta_outer_main(ctx->flex, b));
		}

		*box_pos_main = main_pos + lh__non_auto_margin(b, main_شروع) +
				extra_pre + b->border[main_شروع].چوڑائی;

		إذا (!lh__box_is_absolute(b)) {
			صحيح cross_ناپ;
			صحيح box_ناپ_cross = lh_box_ناپ_cross(
					ctx->horizontal, b);

			main_pos += post_multiplier *
					(extra_کل + box_ناپ_main +
					 lh__delta_outer_main(ctx->flex, b));

			cross_ناپ = box_ناپ_cross + lh__delta_outer_cross(
					ctx->flex, b);
			إذا (سطر->cross_ناپ < cross_ناپ) {
				سطر->cross_ناپ = cross_ناپ;
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
ساكن bool layout_flex_collect_اشیاء_into_سطریں(
		بنية flex_ctx *ctx)
{
	size_t pos = 0;

	طالما (pos < ctx->شے.گنتی) {
		بنية flex_سطر_انکڑے *سطر;

		سطر = layout_flex_build_سطر(ctx, pos);
		إذا (سطر == عدم) {
			أرجع false;
		}

		pos += سطر->گنتی;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: "
				"fitted: %zu (total: %zu/%zu)",
				ctx->flex, سطر->گنتی,
				pos, ctx->شے.گنتی);

		إذا (!layout_flex_resolve_سطر(ctx, سطر)) {
			أرجع false;
		}

		إذا (!layout_flex_رکھو_سطر_اشیاء_main(ctx, سطر)) {
			أرجع false;
		}

		ctx->cross_ناپ += سطر->cross_ناپ;
		إذا (ctx->main_ناپ < سطر->main_ناپ) {
			ctx->main_ناپ = سطر->main_ناپ;
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
ساكن فراغ layout_flex_رکھو_سطر_اشیاء_cross(بنية flex_ctx *ctx,
		بنية flex_سطر_انکڑے *سطر, صحيح extra)
{
	تعداد box_side cross_شروع = ctx->horizontal ? چوٹی_2 : بایاں_2;
	size_t شے_گنتی = سطر->پہلا + سطر->گنتی;

	لكل (size_t i = سطر->پہلا; i < شے_گنتی; i++) {
		بنية flex_شے_انکڑے *شے = &ctx->شے.انکڑے[i];
		بنية box *b = شے->box;
		صحيح cross_free_space;
		صحيح *box_ناپ_cross;
		صحيح *box_pos_cross;

		box_pos_cross = ctx->horizontal ? &b->y : &b->x;
		box_ناپ_cross = lh_box_ناپ_cross_ptr(ctx->horizontal, b);

		cross_free_space = سطر->cross_ناپ + extra - *box_ناپ_cross -
				lh__delta_outer_cross(ctx->flex, b);

		بدل (lh__box_align_self(ctx->flex, b)) {
		افتراضي:
		حالة CSS_ALIGN_SELF_STRETCH:
			إذا (lh_box_ناپ_cross_is_auto(ctx->horizontal, b)) {
				*box_ناپ_cross += cross_free_space;

				/* Relayout children for stretch. */
				إذا (!layout_flex_شے(ctx, شے, b->چوڑائی)) {
					أرجع;
				}
			}
			fallthrough;
		حالة CSS_ALIGN_SELF_FLEX_شروع:
			*box_pos_cross = ctx->flex->padding[cross_شروع] +
					سطر->pos +
					lh__non_auto_margin(b, cross_شروع) +
					b->border[cross_شروع].چوڑائی;
			اكسر;

		حالة CSS_ALIGN_SELF_FLEX_سرا:
			*box_pos_cross = ctx->flex->padding[cross_شروع] +
					سطر->pos + cross_free_space +
					lh__non_auto_margin(b, cross_شروع) +
					b->border[cross_شروع].چوڑائی;
			اكسر;

		حالة CSS_ALIGN_SELF_BASELINE:
		حالة CSS_ALIGN_SELF_CENTER:
			*box_pos_cross = ctx->flex->padding[cross_شروع] +
					سطر->pos + cross_free_space / 2 +
					lh__non_auto_margin(b, cross_شروع) +
					b->border[cross_شروع].چوڑائی;
			اكسر;
		}
	}
}

/**
 * Place the lines and align the items on the line.
 *
 * \param[in] ctx  Flex layout context
 */
ساكن فراغ layout_flex_رکھو_سطریں(بنية flex_ctx *ctx)
{
	bool reversed = ctx->wrap == CSS_FLEX_WRAP_WRAP_الٹ;
	صحيح سطر_pos = reversed ? ctx->cross_ناپ : 0;
	صحيح post_multiplier = reversed ? 0 : 1;
	صحيح pre_multiplier = reversed ? -1 : 0;
	صحيح extra_remainder = 0;
	صحيح extra = 0;

	إذا (ctx->دستیاب_cross != AUTO &&
	    ctx->دستیاب_cross > ctx->cross_ناپ &&
	    ctx->سطر.گنتی > 0) {
		extra = ctx->دستیاب_cross - ctx->cross_ناپ;

		extra_remainder = extra % ctx->سطر.گنتی;
		extra /= ctx->سطر.گنتی;
	}

	لكل (size_t i = 0; i < ctx->سطر.گنتی; i++) {
		بنية flex_سطر_انکڑے *سطر = &ctx->سطر.انکڑے[i];

		سطر_pos += pre_multiplier * سطر->cross_ناپ;
		سطر->pos = سطر_pos;
		سطر_pos += post_multiplier * سطر->cross_ناپ +
				extra + extra_remainder;

		layout_flex_رکھو_سطر_اشیاء_cross(ctx, سطر,
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
bool layout_flex(بنية box *flex, صحيح دستیاب_چوڑائی,
		html_content *content)
{
	صحيح max_اچائی, min_اچائی;
	بنية flex_ctx *ctx;
	bool success = false;

	ctx = layout_flex_ctx__create(content, flex);
	إذا (ctx == عدم) {
		أرجع false;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: %s, available_width %i, width: %i",
			flex, ctx->horizontal ? "horizontal" : "vertical",
			دستیاب_چوڑائی, flex->چوڑائی);

	layout_لبھو_dimensions(
			ctx->unit_len_ctx, دستیاب_چوڑائی, -1,
			flex, flex->style, عدم, &flex->اچائی,
			عدم, عدم, &max_اچائی, &min_اچائی,
			flex->margin, flex->padding, flex->border);

	دستیاب_چوڑائی = min(دستیاب_چوڑائی, flex->چوڑائی);

	إذا (ctx->horizontal) {
		ctx->دستیاب_main = دستیاب_چوڑائی;
		ctx->دستیاب_cross = ctx->flex->اچائی;
	} وإلا {
		ctx->دستیاب_main = ctx->flex->اچائی;
		ctx->دستیاب_cross = دستیاب_چوڑائی;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: available_main: %i",
			flex, ctx->دستیاب_main);
	NSLOG(flex, DEEPDEBUG, "box %p: available_cross: %i",
			flex, ctx->دستیاب_cross);

	layout_flex_ctx_populate_شے_انکڑے(ctx, flex, دستیاب_چوڑائی);

	/* Place items onto lines. */
	success = layout_flex_collect_اشیاء_into_سطریں(ctx);
	إذا (!success) {
		اذهب cleanup;
	}

	layout_flex_رکھو_سطریں(ctx);

	إذا (flex->اچائی == AUTO) {
		flex->اچائی = ctx->horizontal ?
				ctx->cross_ناپ :
				ctx->main_ناپ;
	}

	إذا (flex->اچائی != AUTO) {
		إذا (max_اچائی >= 0 && flex->اچائی > max_اچائی) {
			flex->اچائی = max_اچائی;
		}
		إذا (min_اچائی >  0 && flex->اچائی < min_اچائی) {
			flex->اچائی = min_اچائی;
		}
	}

	success = true;

cleanup:
	layout_flex_ctx__destroy(ctx);

	NSLOG(flex, DEEPDEBUG, "box %p: %s: w: %i, h: %i", flex,
			success ? "success" : "failure",
			flex->چوڑائی, flex->اچائی);
	أرجع success;
}
