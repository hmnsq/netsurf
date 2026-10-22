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

#అన్తర్భావ <string.h>

#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/utils.h"

#అన్తర్భావ "html/box.h"
#అన్తర్భావ "html/html.h"
#అన్తర్భావ "html/private.h"
#అన్తర్భావ "html/box_inspect.h"
#అన్తర్భావ "html/layout_internal.h"

/**
 * Flex item data
 */
నిర్మాణం flex_అంశం_దత్తాంశం {
	గణన css_flex_basis_e basis;
	css_fixed basis_నిడివి;
	css_unit basis_unit;
	నిర్మాణం box *box;

	css_fixed shrink;
	css_fixed grow;

	పూర్ణాంకం min_main;
	పూర్ణాంకం max_main;
	పూర్ణాంకం min_cross;
	పూర్ణాంకం max_cross;

	పూర్ణాంకం లక్ష్యం_main_కొలత;
	పూర్ణాంకం పునాది_కొలత;
	పూర్ణాంకం main_కొలత;
	size_t పంక్తి;

	bool freeze;
	bool min_violation;
	bool max_violation;
};

/**
 * Flex line data
 */
నిర్మాణం flex_పంక్తి_దత్తాంశం {
	పూర్ణాంకం main_కొలత;
	పూర్ణాంకం cross_కొలత;

	పూర్ణాంకం వాడిన_main_కొలత;
	పూర్ణాంకం main_auto_margin_లెక్క;

	పూర్ణాంకం pos;

	size_t మొదటి;
	size_t లెక్క;
	size_t frozen;
};

/**
 * Flex layout context
 */
నిర్మాణం flex_ctx {
	html_content *content;
	స్థిరరాశి నిర్మాణం box *flex;
	స్థిరరాశి css_unit_ctx *unit_len_ctx;

	పూర్ణాంకం main_కొలత;
	పూర్ణాంకం cross_కొలత;

	పూర్ణాంకం అందుబాటు_main;
	పూర్ణాంకం అందుబాటు_cross;

	bool horizontal;
	bool main_reversed;
	గణన css_flex_wrap_e wrap;

	నిర్మాణం flex_వస్తువులు {
		size_t లెక్క;
		నిర్మాణం flex_అంశం_దత్తాంశం *దత్తాంశం;
	} అంశం;

	నిర్మాణం flex_పంక్తులు {
		size_t లెక్క;
		size_t alloc;
		నిర్మాణం flex_పంక్తి_దత్తాంశం *దత్తాంశం;
	} పంక్తి;
};

/**
 * Destroy a flex layout context
 *
 * \param[in] ctx  Flex layout context
 */
స్థిర శూన్య layout_flex_ctx__destroy(నిర్మాణం flex_ctx *ctx)
{
	ఒకవేళ (ctx != ఖాళీ) {
		విడుదల(ctx->అంశం.దత్తాంశం);
		విడుదల(ctx->పంక్తి.దత్తాంశం);
		విడుదల(ctx);
	}
}

/**
 * Create a flex layout context
 *
 * \param[in] content  HTML content containing flex box
 * \param[in] flex     Box to create layout context for
 * \return flex layout context or NULL on error
 */
స్థిర నిర్మాణం flex_ctx *layout_flex_ctx__create(
		html_content *content,
		స్థిరరాశి నిర్మాణం box *flex)
{
	నిర్మాణం flex_ctx *ctx;

	ctx = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(*ctx));
	ఒకవేళ (ctx == ఖాళీ) {
		తిరిగి ఖాళీ;
	}
	ctx->పంక్తి.alloc = 1;

	ctx->అంశం.లెక్క = box_లెక్క_children(flex);
	ctx->అంశం.దత్తాంశం = సున్నా_జ్ఞాపకం_ఇవ్వు(ctx->అంశం.లెక్క, పరిమాణం(*ctx->అంశం.దత్తాంశం));
	ఒకవేళ (ctx->అంశం.దత్తాంశం == ఖాళీ) {
		layout_flex_ctx__destroy(ctx);
		తిరిగి ఖాళీ;
	}

	ctx->పంక్తి.alloc = 1;
	ctx->పంక్తి.దత్తాంశం = సున్నా_జ్ఞాపకం_ఇవ్వు(ctx->పంక్తి.alloc, పరిమాణం(*ctx->పంక్తి.దత్తాంశం));
	ఒకవేళ (ctx->పంక్తి.దత్తాంశం == ఖాళీ) {
		layout_flex_ctx__destroy(ctx);
		తిరిగి ఖాళీ;
	}

	ctx->flex = flex;
	ctx->content = content;
	ctx->unit_len_ctx = &content->unit_len_ctx;

	ctx->wrap = css_computed_flex_wrap(flex->style);
	ctx->horizontal = lh__flex_main_is_horizontal(flex);
	ctx->main_reversed = lh__flex_direction_reversed(flex);

	తిరిగి ctx;
}

/**
 * Find box side representing the start of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the start side.
 */
స్థిర గణన box_side layout_flex_main_ప్రారంభం_side(
		స్థిరరాశి నిర్మాణం flex_ctx *ctx)
{
	ఒకవేళ (ctx->horizontal) {
		తిరిగి (ctx->main_reversed) ? కుడి_2 : ఎడమ_2;
	} లేకపోతే {
		తిరిగి (ctx->main_reversed) ? BOTTOM : శిఖరం_2;
	}
}

/**
 * Find box side representing the end of flex container in main direction.
 *
 * \param[in] ctx   Flex layout context.
 * \return the end side.
 */
స్థిర గణన box_side layout_flex_main_చివర_side(
		స్థిరరాశి నిర్మాణం flex_ctx *ctx)
{
	ఒకవేళ (ctx->horizontal) {
		తిరిగి (ctx->main_reversed) ? ఎడమ_2 : కుడి_2;
	} లేకపోతే {
		తిరిగి (ctx->main_reversed) ? శిఖరం_2 : BOTTOM;
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
స్థిర bool layout_flex_అంశం(
		స్థిరరాశి నిర్మాణం flex_ctx *ctx,
		స్థిరరాశి నిర్మాణం flex_అంశం_దత్తాంశం *అంశం,
		పూర్ణాంకం అందుబాటు_వెడల్పు)
{
	bool success;
	నిర్మాణం box *b = అంశం->box;

	ఎంపిక (b->type) {
	స్థితి BOX_ఖండం:
		success = layout_ఖండం_context(b, -1, ctx->content);
		ఆపు;
	స్థితి BOX_పట్టిక:
		b->float_container = b->మాతృ;
		success = layout_పట్టిక(b, అందుబాటు_వెడల్పు, ctx->content);
		b->float_container = ఖాళీ;
		ఆపు;
	స్థితి BOX_FLEX:
		b->float_container = b->మాతృ;
		success = layout_flex(b, అందుబాటు_వెడల్పు, ctx->content);
		b->float_container = ఖాళీ;
		ఆపు;
	అప్రమేయ:
		ప్రతిజ్ఞా(0 && "Bad flex item back type");
		success = false;
		ఆపు;
	}

	ఒకవేళ (!success) {
		NSLOG(flex, ERROR, "box %p: layout failed", b);
	}

	తిరిగి success;
}

/**
 * Calculate an item's base and target main sizes.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] item             Item to get sizes of
 * \param[in] available_width  Available width in pixels
 * \return true on success false on failure
 */
స్థిర అంతఃవరుస bool layout_flex_పునాది_and_main_sizes(
		స్థిరరాశి నిర్మాణం flex_ctx *ctx,
		నిర్మాణం flex_అంశం_దత్తాంశం *అంశం,
		పూర్ణాంకం అందుబాటు_వెడల్పు)
{
	నిర్మాణం box *b = అంశం->box;
	పూర్ణాంకం content_min_వెడల్పు = b->min_వెడల్పు;
	పూర్ణాంకం content_max_వెడల్పు = b->max_వెడల్పు;
	పూర్ణాంకం delta_outer_main = lh__delta_outer_main(ctx->flex, b);

	NSLOG(flex, DEEPDEBUG, "box %p: delta_outer_main: %i",
			b, delta_outer_main);

	ఒకవేళ (అంశం->basis == CSS_FLEX_BASIS_సమితి) {
		ఒకవేళ (అంశం->basis_unit == CSS_UNIT_PCT) {
			అంశం->పునాది_కొలత = FPCT_యొక్క_INT_TOINT(
					అంశం->basis_నిడివి,
					అందుబాటు_వెడల్పు);
		} లేకపోతే {
			అంశం->పునాది_కొలత = FIXTOINT(css_unit_len2device_px(
					b->style, ctx->unit_len_ctx,
					అంశం->basis_నిడివి,
					అంశం->basis_unit));
		}

	} లేకపోతే ఒకవేళ (అంశం->basis == CSS_FLEX_BASIS_AUTO) {
		అంశం->పునాది_కొలత = ctx->horizontal ? b->వెడల్పు : b->ఎత్తు;
	} లేకపోతే {
		అంశం->పునాది_కొలత = AUTO;
	}

	ఒకవేళ (ctx->horizontal == false) {
		ఒకవేళ (b->వెడల్పు == AUTO) {
			b->వెడల్పు = min(max(content_min_వెడల్పు, అందుబాటు_వెడల్పు),
					content_max_వెడల్పు);
			b->వెడల్పు -= lh_delta_outer_వెడల్పు(b);
		}

		ఒకవేళ (!layout_flex_అంశం(ctx, అంశం, b->వెడల్పు)) {
			తిరిగి false;
		}
	}

	ఒకవేళ (అంశం->పునాది_కొలత == AUTO) {
		ఒకవేళ (ctx->horizontal == false) {
			అంశం->పునాది_కొలత = b->ఎత్తు;
		} లేకపోతే {
			అంశం->పునాది_కొలత = content_max_వెడల్పు - delta_outer_main;
		}
	}

	అంశం->పునాది_కొలత += delta_outer_main;

	ఒకవేళ (ctx->horizontal) {
		అంశం->పునాది_కొలత = min(అంశం->పునాది_కొలత, అందుబాటు_వెడల్పు);
		అంశం->పునాది_కొలత = max(అంశం->పునాది_కొలత, content_min_వెడల్పు);
	}

	అంశం->లక్ష్యం_main_కొలత = అంశం->పునాది_కొలత;
	అంశం->main_కొలత = అంశం->పునాది_కొలత;

	ఒకవేళ (అంశం->max_main > 0 &&
	    అంశం->main_కొలత > అంశం->max_main + delta_outer_main) {
		అంశం->main_కొలత = అంశం->max_main + delta_outer_main;
	}

	ఒకవేళ (అంశం->main_కొలత < అంశం->min_main + delta_outer_main) {
		అంశం->main_కొలత = అంశం->min_main + delta_outer_main;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: base_size: %i, main_size %i",
			b, అంశం->పునాది_కొలత, అంశం->main_కొలత);

	తిరిగి true;
}

/**
 * Fill out all item's data in a flex container.
 *
 * \param[in] ctx              Flex layout context
 * \param[in] flex             Flex box
 * \param[in] available_width  Available width in pixels
 */
స్థిర శూన్య layout_flex_ctx_populate_అంశం_దత్తాంశం(
		స్థిరరాశి నిర్మాణం flex_ctx *ctx,
		స్థిరరాశి నిర్మాణం box *flex,
		పూర్ణాంకం అందుబాటు_వెడల్పు)
{
	size_t i = 0;
	bool horizontal = ctx->horizontal;

	కొరకు (నిర్మాణం box *b = flex->children; b != ఖాళీ; b = b->తదుపరి) {
		నిర్మాణం flex_అంశం_దత్తాంశం *అంశం = &ctx->అంశం.దత్తాంశం[i++];

		b->float_container = b->మాతృ;
		layout_కనుగొను_dimensions(ctx->unit_len_ctx, అందుబాటు_వెడల్పు, -1,
				b, b->style, &b->వెడల్పు, &b->ఎత్తు,
				horizontal ? &అంశం->max_main : &అంశం->max_cross,
				horizontal ? &అంశం->min_main : &అంశం->min_cross,
				horizontal ? &అంశం->max_cross : &అంశం->max_main,
				horizontal ? &అంశం->min_cross : &అంశం->min_main,
				b->margin, b->padding, b->border);
		b->float_container = ఖాళీ;

		NSLOG(flex, DEEPDEBUG, "flex-item box: %p: width: %i",
				b, b->వెడల్పు);

		అంశం->box = b;
		అంశం->basis = css_computed_flex_basis(b->style,
				&అంశం->basis_నిడివి, &అంశం->basis_unit);

		css_computed_flex_shrink(b->style, &అంశం->shrink);
		css_computed_flex_grow(b->style, &అంశం->grow);

		layout_flex_పునాది_and_main_sizes(ctx, అంశం, అందుబాటు_వెడల్పు);
	}
}

/**
 * Ensure context's lines array has a free space
 *
 * \param[in] ctx  Flex layout context
 * \return true on success false on out of memory
 */
స్థిర bool layout_flex_ctx_ensure_పంక్తి(నిర్మాణం flex_ctx *ctx)
{
	నిర్మాణం flex_పంక్తి_దత్తాంశం *తాత్కాలిక;
	size_t పంక్తి_alloc = ctx->పంక్తి.alloc * 2;

	ఒకవేళ (ctx->పంక్తి.alloc > ctx->పంక్తి.లెక్క) {
		తిరిగి true;
	}

	తాత్కాలిక = realloc(ctx->పంక్తి.దత్తాంశం, పరిమాణం(*ctx->పంక్తి.దత్తాంశం) * పంక్తి_alloc);
	ఒకవేళ (తాత్కాలిక == ఖాళీ) {
		తిరిగి false;
	}
	ctx->పంక్తి.దత్తాంశం = తాత్కాలిక;

	memset(ctx->పంక్తి.దత్తాంశం + ctx->పంక్తి.alloc, 0,
	       పరిమాణం(*ctx->పంక్తి.దత్తాంశం) * (పంక్తి_alloc - ctx->పంక్తి.alloc));
	ctx->పంక్తి.alloc = పంక్తి_alloc;

	తిరిగి true;
}

/**
 * Assigns flex items to the line and returns the line
 *
 * \param[in] ctx         Flex layout context
 * \param[in] item_index  Index to first item to assign to this line
 * \return Pointer to the new line, or NULL on error.
 */
స్థిర నిర్మాణం flex_పంక్తి_దత్తాంశం *layout_flex_build_పంక్తి(నిర్మాణం flex_ctx *ctx,
		size_t అంశం_సూచీ)
{
	గణన box_side ప్రారంభం_side = layout_flex_main_ప్రారంభం_side(ctx);
	గణన box_side చివర_side = layout_flex_main_చివర_side(ctx);
	నిర్మాణం flex_పంక్తి_దత్తాంశం *పంక్తి;
	పూర్ణాంకం వాడిన_main = 0;

	ఒకవేళ (!layout_flex_ctx_ensure_పంక్తి(ctx)) {
		తిరిగి ఖాళీ;
	}

	పంక్తి = &ctx->పంక్తి.దత్తాంశం[ctx->పంక్తి.లెక్క];
	పంక్తి->మొదటి = అంశం_సూచీ;

	NSLOG(flex, DEEPDEBUG, "flex container %p: available main: %i",
			ctx->flex, ctx->అందుబాటు_main);

	అయితే (అంశం_సూచీ < ctx->అంశం.లెక్క) {
		నిర్మాణం flex_అంశం_దత్తాంశం *అంశం = &ctx->అంశం.దత్తాంశం[అంశం_సూచీ];
		నిర్మాణం box *b = అంశం->box;
		పూర్ణాంకం pos_main;

		pos_main = ctx->horizontal ?
				అంశం->main_కొలత :
				b->ఎత్తు + lh__delta_outer_main(ctx->flex, b);

		ఒకవేళ (ctx->wrap == CSS_FLEX_WRAP_NOWRAP ||
		    pos_main + వాడిన_main <= ctx->అందుబాటు_main ||
		    lh__box_is_absolute(అంశం->box) ||
		    ctx->అందుబాటు_main == AUTO ||
		    పంక్తి->లెక్క == 0 ||
		    pos_main == 0) {
			ఒకవేళ (lh__box_is_absolute(అంశం->box) == false) {
				పంక్తి->main_కొలత += అంశం->main_కొలత;
				వాడిన_main += pos_main;

				ఒకవేళ (b->margin[ప్రారంభం_side] == AUTO) {
					పంక్తి->main_auto_margin_లెక్క++;
				}
				ఒకవేళ (b->margin[చివర_side] == AUTO) {
					పంక్తి->main_auto_margin_లెక్క++;
				}
			}
			అంశం->పంక్తి = ctx->పంక్తి.లెక్క;
			పంక్తి->లెక్క++;
			అంశం_సూచీ++;
		} లేకపోతే {
			ఆపు;
		}
	}

	ఒకవేళ (పంక్తి->లెక్క > 0) {
		ctx->పంక్తి.లెక్క++;
	} లేకపోతే {
		NSLOG(layout, ERROR, "Failed to fit any flex items");
	}

	తిరిగి పంక్తి;
}

/**
 * Freeze an item on a line
 *
 * \param[in] line  Line to containing item
 * \param[in] item  Item to freeze
 */
స్థిర అంతఃవరుస శూన్య layout_flex_అంశం_freeze(
		నిర్మాణం flex_పంక్తి_దత్తాంశం *పంక్తి,
		నిర్మాణం flex_అంశం_దత్తాంశం *అంశం)
{
	అంశం->freeze = true;
	పంక్తి->frozen++;

	ఒకవేళ (!lh__box_is_absolute(అంశం->box)){
		పంక్తి->వాడిన_main_కొలత += అంశం->లక్ష్యం_main_కొలత;
	}

	NSLOG(flex, DEEPDEBUG, "flex-item box: %p: "
			"Frozen at target_main_size: %i",
			అంశం->box, అంశం->లక్ష్యం_main_కొలత);
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
స్థిర అంతఃవరుస పూర్ణాంకం layout_flex_మిగిలిన_free_main(
		నిర్మాణం flex_ctx *ctx,
		నిర్మాణం flex_పంక్తి_దత్తాంశం *పంక్తి,
		css_fixed *unfrozen_గుణకం_కూడిక,
		పూర్ణాంకం initial_free_main,
		పూర్ణాంకం అందుబాటు_main,
		bool grow)
{
	పూర్ణాంకం మిగిలిన_free_main = అందుబాటు_main;
	size_t అంశం_లెక్క = పంక్తి->మొదటి + పంక్తి->లెక్క;

	*unfrozen_గుణకం_కూడిక = 0;

	కొరకు (size_t i = పంక్తి->మొదటి; i < అంశం_లెక్క; i++) {
		నిర్మాణం flex_అంశం_దత్తాంశం *అంశం = &ctx->అంశం.దత్తాంశం[i];

		ఒకవేళ (అంశం->freeze) {
			మిగిలిన_free_main -= అంశం->లక్ష్యం_main_కొలత;
		} లేకపోతే {
			మిగిలిన_free_main -= అంశం->పునాది_కొలత;

			*unfrozen_గుణకం_కూడిక += grow ?
					అంశం->grow : అంశం->shrink;
		}
	}

	ఒకవేళ (*unfrozen_గుణకం_కూడిక < F_1) {
		పూర్ణాంకం free_space = FIXTOINT(FMUL(INTTOFIX(initial_free_main),
				*unfrozen_గుణకం_కూడిక));

		ఒకవేళ (free_space < మిగిలిన_free_main) {
			మిగిలిన_free_main = free_space;
		}
	}

	NSLOG(flex, DEEPDEBUG, "Remaining free space: %i",
			మిగిలిన_free_main);

	తిరిగి మిగిలిన_free_main;
}

/**
 * Clamp flex item target main size and get min/max violations
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to align items on
 * return total violation in pixels
 */
స్థిర అంతఃవరుస పూర్ణాంకం layout_flex__get_min_max_violations(
		నిర్మాణం flex_ctx *ctx,
		నిర్మాణం flex_పంక్తి_దత్తాంశం *పంక్తి)
{

	పూర్ణాంకం మొత్తం_violation = 0;
	size_t అంశం_లెక్క = పంక్తి->మొదటి + పంక్తి->లెక్క;

	కొరకు (size_t i = పంక్తి->మొదటి; i < అంశం_లెక్క; i++) {
		నిర్మాణం flex_అంశం_దత్తాంశం *అంశం = &ctx->అంశం.దత్తాంశం[i];
		పూర్ణాంకం లక్ష్యం_main_కొలత = అంశం->లక్ష్యం_main_కొలత;

		NSLOG(flex, DEEPDEBUG, "item %p: target_main_size: %i",
					అంశం->box, లక్ష్యం_main_కొలత);

		ఒకవేళ (అంశం->freeze) {
			కొనసాగు;
		}

		ఒకవేళ (అంశం->max_main > 0 &&
		    లక్ష్యం_main_కొలత > అంశం->max_main) {
			లక్ష్యం_main_కొలత = అంశం->max_main;
			అంశం->max_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: max_main: %i",
					అంశం->max_main);
		}

		ఒకవేళ (లక్ష్యం_main_కొలత < అంశం->min_main) {
			లక్ష్యం_main_కొలత = అంశం->min_main;
			అంశం->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: min_main: %i",
					అంశం->min_main);
		}

		ఒకవేళ (లక్ష్యం_main_కొలత < అంశం->box->min_వెడల్పు) {
			లక్ష్యం_main_కొలత = అంశం->box->min_వెడల్పు;
			అంశం->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: box min_width: %i",
					అంశం->box->min_వెడల్పు);
		}

		ఒకవేళ (లక్ష్యం_main_కొలత < 0) {
			లక్ష్యం_main_కొలత = 0;
			అంశం->min_violation = true;
			NSLOG(flex, DEEPDEBUG, "Violation: less than 0");
		}

		మొత్తం_violation += లక్ష్యం_main_కొలత - అంశం->లక్ష్యం_main_కొలత;
		అంశం->లక్ష్యం_main_కొలత = లక్ష్యం_main_కొలత;
	}

	NSLOG(flex, DEEPDEBUG, "Total violation: %i", మొత్తం_violation);

	తిరిగి మొత్తం_violation;
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
స్థిర అంతఃవరుస శూన్య layout_flex__distribute_free_main(
		నిర్మాణం flex_ctx *ctx,
		నిర్మాణం flex_పంక్తి_దత్తాంశం *పంక్తి,
		css_fixed unfrozen_గుణకం_కూడిక,
		పూర్ణాంకం మిగిలిన_free_main,
		bool grow)
{
	size_t అంశం_లెక్క = పంక్తి->మొదటి + పంక్తి->లెక్క;

	ఒకవేళ (grow) {
		css_fixed remainder = 0;
		కొరకు (size_t i = పంక్తి->మొదటి; i < అంశం_లెక్క; i++) {
			నిర్మాణం flex_అంశం_దత్తాంశం *అంశం = &ctx->అంశం.దత్తాంశం[i];
			css_fixed ఫలితం;
			css_fixed నిష్పత్తి;

			ఒకవేళ (అంశం->freeze) {
				కొనసాగు;
			}

			నిష్పత్తి = FDIV(అంశం->grow, unfrozen_గుణకం_కూడిక);
			ఫలితం = FMUL(INTTOFIX(మిగిలిన_free_main), నిష్పత్తి) +
					remainder;

			అంశం->లక్ష్యం_main_కొలత = అంశం->పునాది_కొలత +
					FIXTOINT(ఫలితం);
			remainder = FIXFRAC(ఫలితం);
		}
	} లేకపోతే {
		css_fixed scaled_shrink_గుణకం_కూడిక = 0;
		css_fixed remainder = 0;

		కొరకు (size_t i = పంక్తి->మొదటి; i < అంశం_లెక్క; i++) {
			నిర్మాణం flex_అంశం_దత్తాంశం *అంశం = &ctx->అంశం.దత్తాంశం[i];
			css_fixed scaled_shrink_గుణకం;

			ఒకవేళ (అంశం->freeze) {
				కొనసాగు;
			}

			scaled_shrink_గుణకం = FMUL(
					అంశం->shrink,
					INTTOFIX(అంశం->పునాది_కొలత));
			scaled_shrink_గుణకం_కూడిక += scaled_shrink_గుణకం;
		}

		కొరకు (size_t i = పంక్తి->మొదటి; i < అంశం_లెక్క; i++) {
			నిర్మాణం flex_అంశం_దత్తాంశం *అంశం = &ctx->అంశం.దత్తాంశం[i];
			css_fixed scaled_shrink_గుణకం;
			css_fixed ఫలితం;
			css_fixed నిష్పత్తి;

			ఒకవేళ (అంశం->freeze) {
				కొనసాగు;
			} లేకపోతే ఒకవేళ (scaled_shrink_గుణకం_కూడిక == 0) {
				అంశం->లక్ష్యం_main_కొలత = అంశం->main_కొలత;
				layout_flex_అంశం_freeze(పంక్తి, అంశం);
				కొనసాగు;
			}

			scaled_shrink_గుణకం = FMUL(
					అంశం->shrink,
					INTTOFIX(అంశం->పునాది_కొలత));
			నిష్పత్తి = FDIV(scaled_shrink_గుణకం,
			             scaled_shrink_గుణకం_కూడిక);
			ఫలితం = FMUL(INTTOFIX(నిరపేక్ష(మిగిలిన_free_main)),
			              నిష్పత్తి) + remainder;

			అంశం->లక్ష్యం_main_కొలత = అంశం->పునాది_కొలత -
					FIXTOINT(ఫలితం);
			remainder = FIXFRAC(ఫలితం);
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
స్థిర bool layout_flex_resolve_పంక్తి(
		నిర్మాణం flex_ctx *ctx,
		నిర్మాణం flex_పంక్తి_దత్తాంశం *పంక్తి)
{
	size_t అంశం_లెక్క = పంక్తి->మొదటి + పంక్తి->లెక్క;
	పూర్ణాంకం అందుబాటు_main = ctx->అందుబాటు_main;
	పూర్ణాంకం initial_free_main;
	bool grow;

	ఒకవేళ (అందుబాటు_main == AUTO) {
		అందుబాటు_main = గరిష్ఠ_పూర్ణాంకం;
	}

	grow = (పంక్తి->main_కొలత < అందుబాటు_main);
	initial_free_main = అందుబాటు_main;

	NSLOG(flex, DEEPDEBUG, "box %p: line %zu: first: %zu, count: %zu",
			ctx->flex, పంక్తి - ctx->పంక్తి.దత్తాంశం,
			పంక్తి->మొదటి, పంక్తి->లెక్క);
	NSLOG(flex, DEEPDEBUG, "Line main_size: %i, available_main: %i",
			పంక్తి->main_కొలత, అందుబాటు_main);

	కొరకు (size_t i = పంక్తి->మొదటి; i < అంశం_లెక్క; i++) {
		నిర్మాణం flex_అంశం_దత్తాంశం *అంశం = &ctx->అంశం.దత్తాంశం[i];

		/* 3. Size inflexible items */
		ఒకవేళ (grow) {
			ఒకవేళ (అంశం->grow == 0 ||
			    అంశం->పునాది_కొలత > అంశం->main_కొలత) {
				అంశం->లక్ష్యం_main_కొలత = అంశం->main_కొలత;
				layout_flex_అంశం_freeze(పంక్తి, అంశం);
			}
		} లేకపోతే {
			ఒకవేళ (అంశం->shrink == 0 ||
			    అంశం->పునాది_కొలత < అంశం->main_కొలత) {
				అంశం->లక్ష్యం_main_కొలత = అంశం->main_కొలత;
				layout_flex_అంశం_freeze(పంక్తి, అంశం);
			}
		}

		/* 4. Calculate initial free space */
		ఒకవేళ (అంశం->freeze) {
			initial_free_main -= అంశం->లక్ష్యం_main_కొలత;
		} లేకపోతే {
			initial_free_main -= అంశం->పునాది_కొలత;
		}
	}

	/* 5. Loop */
	అయితే (పంక్తి->frozen < పంక్తి->లెక్క) {
		css_fixed unfrozen_గుణకం_కూడిక;
		పూర్ణాంకం మిగిలిన_free_main;
		పూర్ణాంకం మొత్తం_violation;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: Resolver pass",
				ctx->flex);

		/* b */
		మిగిలిన_free_main = layout_flex_మిగిలిన_free_main(ctx,
				పంక్తి, &unfrozen_గుణకం_కూడిక, initial_free_main,
				అందుబాటు_main, grow);

		/* c */
		ఒకవేళ (మిగిలిన_free_main != 0) {
			layout_flex__distribute_free_main(ctx,
					పంక్తి, unfrozen_గుణకం_కూడిక,
					మిగిలిన_free_main, grow);
		}

		/* d */
		మొత్తం_violation = layout_flex__get_min_max_violations(
				ctx, పంక్తి);

		/* e */
		కొరకు (size_t i = పంక్తి->మొదటి; i < అంశం_లెక్క; i++) {
			నిర్మాణం flex_అంశం_దత్తాంశం *అంశం = &ctx->అంశం.దత్తాంశం[i];

			ఒకవేళ (అంశం->freeze) {
				కొనసాగు;
			}

			ఒకవేళ (మొత్తం_violation == 0 ||
			    (మొత్తం_violation > 0 && అంశం->min_violation) ||
			    (మొత్తం_violation < 0 && అంశం->max_violation)) {
				layout_flex_అంశం_freeze(పంక్తి, అంశం);
			}
		}
	}

	తిరిగి true;
}

/**
 * Position items along a line
 *
 * \param[in] ctx   Flex layout context
 * \param[in] line  Line to resolve
 * \return true on success, false on failure.
 */
స్థిర bool layout_flex_ఉంచు_పంక్తి_వస్తువులు_main(
		నిర్మాణం flex_ctx *ctx,
		నిర్మాణం flex_పంక్తి_దత్తాంశం *పంక్తి)
{
	పూర్ణాంకం main_pos = ctx->flex->padding[layout_flex_main_ప్రారంభం_side(ctx)];
	పూర్ణాంకం post_multiplier = ctx->main_reversed ? 0 : 1;
	పూర్ణాంకం pre_multiplier = ctx->main_reversed ? -1 : 0;
	size_t అంశం_లెక్క = పంక్తి->మొదటి + పంక్తి->లెక్క;
	పూర్ణాంకం extra_remainder = 0;
	పూర్ణాంకం extra = 0;

	ఒకవేళ (ctx->main_reversed) {
		main_pos = lh_box_కొలత_main(ctx->horizontal, ctx->flex) -
				main_pos;
	}

	ఒకవేళ (ctx->అందుబాటు_main != AUTO &&
	    ctx->అందుబాటు_main != UNKNOWN_వెడల్పు &&
	    ctx->అందుబాటు_main > పంక్తి->వాడిన_main_కొలత) {
		ఒకవేళ (పంక్తి->main_auto_margin_లెక్క > 0) {
			extra = ctx->అందుబాటు_main - పంక్తి->వాడిన_main_కొలత;

			extra_remainder = extra % పంక్తి->main_auto_margin_లెక్క;
			extra /= పంక్తి->main_auto_margin_లెక్క;
		}
	}

	కొరకు (size_t i = పంక్తి->మొదటి; i < అంశం_లెక్క; i++) {
		గణన box_side main_చివర = ctx->horizontal ? కుడి_2 : BOTTOM;
		గణన box_side main_ప్రారంభం = ctx->horizontal ? ఎడమ_2 : శిఖరం_2;
		నిర్మాణం flex_అంశం_దత్తాంశం *అంశం = &ctx->అంశం.దత్తాంశం[i];
		నిర్మాణం box *b = అంశం->box;
		పూర్ణాంకం extra_మొత్తం = 0;
		పూర్ణాంకం extra_post = 0;
		పూర్ణాంకం extra_pre = 0;
		పూర్ణాంకం box_కొలత_main;
		పూర్ణాంకం *box_pos_main;

		ఒకవేళ (ctx->horizontal) {
			b->వెడల్పు = అంశం->లక్ష్యం_main_కొలత -
					lh_delta_outer_వెడల్పు(b);

			ఒకవేళ (!layout_flex_అంశం(ctx, అంశం, b->వెడల్పు)) {
				తిరిగి false;
			}
		}

		box_కొలత_main = lh_box_కొలత_main(ctx->horizontal, b);
		box_pos_main = ctx->horizontal ? &b->x : &b->y;

		ఒకవేళ (!lh__box_is_absolute(b)) {
			ఒకవేళ (b->margin[main_ప్రారంభం] == AUTO) {
				extra_pre = extra + extra_remainder;
			}
			ఒకవేళ (b->margin[main_చివర] == AUTO) {
				extra_post = extra + extra_remainder;
			}
			extra_మొత్తం = extra_pre + extra_post;

			main_pos += pre_multiplier *
					(extra_మొత్తం + box_కొలత_main +
					 lh__delta_outer_main(ctx->flex, b));
		}

		*box_pos_main = main_pos + lh__non_auto_margin(b, main_ప్రారంభం) +
				extra_pre + b->border[main_ప్రారంభం].వెడల్పు;

		ఒకవేళ (!lh__box_is_absolute(b)) {
			పూర్ణాంకం cross_కొలత;
			పూర్ణాంకం box_కొలత_cross = lh_box_కొలత_cross(
					ctx->horizontal, b);

			main_pos += post_multiplier *
					(extra_మొత్తం + box_కొలత_main +
					 lh__delta_outer_main(ctx->flex, b));

			cross_కొలత = box_కొలత_cross + lh__delta_outer_cross(
					ctx->flex, b);
			ఒకవేళ (పంక్తి->cross_కొలత < cross_కొలత) {
				పంక్తి->cross_కొలత = cross_కొలత;
			}
		}
	}

	తిరిగి true;
}

/**
 * Collect items onto lines and place items along the lines
 *
 * \param[in] ctx   Flex layout context
 * \return true on success, false on failure.
 */
స్థిర bool layout_flex_collect_వస్తువులు_into_పంక్తులు(
		నిర్మాణం flex_ctx *ctx)
{
	size_t pos = 0;

	అయితే (pos < ctx->అంశం.లెక్క) {
		నిర్మాణం flex_పంక్తి_దత్తాంశం *పంక్తి;

		పంక్తి = layout_flex_build_పంక్తి(ctx, pos);
		ఒకవేళ (పంక్తి == ఖాళీ) {
			తిరిగి false;
		}

		pos += పంక్తి->లెక్క;

		NSLOG(flex, DEEPDEBUG, "flex-container: %p: "
				"fitted: %zu (total: %zu/%zu)",
				ctx->flex, పంక్తి->లెక్క,
				pos, ctx->అంశం.లెక్క);

		ఒకవేళ (!layout_flex_resolve_పంక్తి(ctx, పంక్తి)) {
			తిరిగి false;
		}

		ఒకవేళ (!layout_flex_ఉంచు_పంక్తి_వస్తువులు_main(ctx, పంక్తి)) {
			తిరిగి false;
		}

		ctx->cross_కొలత += పంక్తి->cross_కొలత;
		ఒకవేళ (ctx->main_కొలత < పంక్తి->main_కొలత) {
			ctx->main_కొలత = పంక్తి->main_కొలత;
		}
	}

	తిరిగి true;
}

/**
 * Align items on a line.
 *
 * \param[in] ctx    Flex layout context
 * \param[in] line   Line to align items on
 * \param[in] extra  Extra line width in pixels
 */
స్థిర శూన్య layout_flex_ఉంచు_పంక్తి_వస్తువులు_cross(నిర్మాణం flex_ctx *ctx,
		నిర్మాణం flex_పంక్తి_దత్తాంశం *పంక్తి, పూర్ణాంకం extra)
{
	గణన box_side cross_ప్రారంభం = ctx->horizontal ? శిఖరం_2 : ఎడమ_2;
	size_t అంశం_లెక్క = పంక్తి->మొదటి + పంక్తి->లెక్క;

	కొరకు (size_t i = పంక్తి->మొదటి; i < అంశం_లెక్క; i++) {
		నిర్మాణం flex_అంశం_దత్తాంశం *అంశం = &ctx->అంశం.దత్తాంశం[i];
		నిర్మాణం box *b = అంశం->box;
		పూర్ణాంకం cross_free_space;
		పూర్ణాంకం *box_కొలత_cross;
		పూర్ణాంకం *box_pos_cross;

		box_pos_cross = ctx->horizontal ? &b->y : &b->x;
		box_కొలత_cross = lh_box_కొలత_cross_ptr(ctx->horizontal, b);

		cross_free_space = పంక్తి->cross_కొలత + extra - *box_కొలత_cross -
				lh__delta_outer_cross(ctx->flex, b);

		ఎంపిక (lh__box_align_self(ctx->flex, b)) {
		అప్రమేయ:
		స్థితి CSS_ALIGN_SELF_STRETCH:
			ఒకవేళ (lh_box_కొలత_cross_is_auto(ctx->horizontal, b)) {
				*box_కొలత_cross += cross_free_space;

				/* Relayout children for stretch. */
				ఒకవేళ (!layout_flex_అంశం(ctx, అంశం, b->వెడల్పు)) {
					తిరిగి;
				}
			}
			fallthrough;
		స్థితి CSS_ALIGN_SELF_FLEX_ప్రారంభం:
			*box_pos_cross = ctx->flex->padding[cross_ప్రారంభం] +
					పంక్తి->pos +
					lh__non_auto_margin(b, cross_ప్రారంభం) +
					b->border[cross_ప్రారంభం].వెడల్పు;
			ఆపు;

		స్థితి CSS_ALIGN_SELF_FLEX_చివర:
			*box_pos_cross = ctx->flex->padding[cross_ప్రారంభం] +
					పంక్తి->pos + cross_free_space +
					lh__non_auto_margin(b, cross_ప్రారంభం) +
					b->border[cross_ప్రారంభం].వెడల్పు;
			ఆపు;

		స్థితి CSS_ALIGN_SELF_BASELINE:
		స్థితి CSS_ALIGN_SELF_CENTER:
			*box_pos_cross = ctx->flex->padding[cross_ప్రారంభం] +
					పంక్తి->pos + cross_free_space / 2 +
					lh__non_auto_margin(b, cross_ప్రారంభం) +
					b->border[cross_ప్రారంభం].వెడల్పు;
			ఆపు;
		}
	}
}

/**
 * Place the lines and align the items on the line.
 *
 * \param[in] ctx  Flex layout context
 */
స్థిర శూన్య layout_flex_ఉంచు_పంక్తులు(నిర్మాణం flex_ctx *ctx)
{
	bool reversed = ctx->wrap == CSS_FLEX_WRAP_WRAP_తిరగేయి;
	పూర్ణాంకం పంక్తి_pos = reversed ? ctx->cross_కొలత : 0;
	పూర్ణాంకం post_multiplier = reversed ? 0 : 1;
	పూర్ణాంకం pre_multiplier = reversed ? -1 : 0;
	పూర్ణాంకం extra_remainder = 0;
	పూర్ణాంకం extra = 0;

	ఒకవేళ (ctx->అందుబాటు_cross != AUTO &&
	    ctx->అందుబాటు_cross > ctx->cross_కొలత &&
	    ctx->పంక్తి.లెక్క > 0) {
		extra = ctx->అందుబాటు_cross - ctx->cross_కొలత;

		extra_remainder = extra % ctx->పంక్తి.లెక్క;
		extra /= ctx->పంక్తి.లెక్క;
	}

	కొరకు (size_t i = 0; i < ctx->పంక్తి.లెక్క; i++) {
		నిర్మాణం flex_పంక్తి_దత్తాంశం *పంక్తి = &ctx->పంక్తి.దత్తాంశం[i];

		పంక్తి_pos += pre_multiplier * పంక్తి->cross_కొలత;
		పంక్తి->pos = పంక్తి_pos;
		పంక్తి_pos += post_multiplier * పంక్తి->cross_కొలత +
				extra + extra_remainder;

		layout_flex_ఉంచు_పంక్తి_వస్తువులు_cross(ctx, పంక్తి,
				extra + extra_remainder);

		ఒకవేళ (extra_remainder > 0) {
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
bool layout_flex(నిర్మాణం box *flex, పూర్ణాంకం అందుబాటు_వెడల్పు,
		html_content *content)
{
	పూర్ణాంకం max_ఎత్తు, min_ఎత్తు;
	నిర్మాణం flex_ctx *ctx;
	bool success = false;

	ctx = layout_flex_ctx__create(content, flex);
	ఒకవేళ (ctx == ఖాళీ) {
		తిరిగి false;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: %s, available_width %i, width: %i",
			flex, ctx->horizontal ? "horizontal" : "vertical",
			అందుబాటు_వెడల్పు, flex->వెడల్పు);

	layout_కనుగొను_dimensions(
			ctx->unit_len_ctx, అందుబాటు_వెడల్పు, -1,
			flex, flex->style, ఖాళీ, &flex->ఎత్తు,
			ఖాళీ, ఖాళీ, &max_ఎత్తు, &min_ఎత్తు,
			flex->margin, flex->padding, flex->border);

	అందుబాటు_వెడల్పు = min(అందుబాటు_వెడల్పు, flex->వెడల్పు);

	ఒకవేళ (ctx->horizontal) {
		ctx->అందుబాటు_main = అందుబాటు_వెడల్పు;
		ctx->అందుబాటు_cross = ctx->flex->ఎత్తు;
	} లేకపోతే {
		ctx->అందుబాటు_main = ctx->flex->ఎత్తు;
		ctx->అందుబాటు_cross = అందుబాటు_వెడల్పు;
	}

	NSLOG(flex, DEEPDEBUG, "box %p: available_main: %i",
			flex, ctx->అందుబాటు_main);
	NSLOG(flex, DEEPDEBUG, "box %p: available_cross: %i",
			flex, ctx->అందుబాటు_cross);

	layout_flex_ctx_populate_అంశం_దత్తాంశం(ctx, flex, అందుబాటు_వెడల్పు);

	/* Place items onto lines. */
	success = layout_flex_collect_వస్తువులు_into_పంక్తులు(ctx);
	ఒకవేళ (!success) {
		వెళ్ళు cleanup;
	}

	layout_flex_ఉంచు_పంక్తులు(ctx);

	ఒకవేళ (flex->ఎత్తు == AUTO) {
		flex->ఎత్తు = ctx->horizontal ?
				ctx->cross_కొలత :
				ctx->main_కొలత;
	}

	ఒకవేళ (flex->ఎత్తు != AUTO) {
		ఒకవేళ (max_ఎత్తు >= 0 && flex->ఎత్తు > max_ఎత్తు) {
			flex->ఎత్తు = max_ఎత్తు;
		}
		ఒకవేళ (min_ఎత్తు >  0 && flex->ఎత్తు < min_ఎత్తు) {
			flex->ఎత్తు = min_ఎత్తు;
		}
	}

	success = true;

cleanup:
	layout_flex_ctx__destroy(ctx);

	NSLOG(flex, DEEPDEBUG, "box %p: %s: w: %i, h: %i", flex,
			success ? "success" : "failure",
			flex->వెడల్పు, flex->ఎత్తు);
	తిరిగి success;
}
