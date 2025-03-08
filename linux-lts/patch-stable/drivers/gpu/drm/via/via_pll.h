/*
 * Copyright 2023 The OpenChrome Project
 * [via_pll.h]
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) OR COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef __VIA_PLL_H__
#define __VIA_PLL_H__

/* VIA PLL limits */
#define VIA_MIN_CHIP_CLOCK      25000   /* kHz */
#define VIA_MAX_CHIP_CLOCK      1000000 /* kHz */

/* VIA PLL calculation evaluation targets */
#define VIA_VCLK_OK             2000 /* kHz */
#define VIA_VCLK_FAIL           2500 /* kHz */

/* Function prototypes */
u32 via_get_clk_value(struct drm_device *dev, u32 freq);
void via_set_vclock(struct drm_crtc *crtc, u32 clk);

#endif /* __VIA_PLL_H__ */
