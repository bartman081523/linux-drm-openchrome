/*
 * Copyright 2023 The OpenChrome Project
 * [via_dac_regs.h]
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

#ifndef __VIA_DAC_REGS_H__
#define __VIA_DAC_REGS_H__

/* DAC DPMS states */
#define VIA_DAC_DPMS_ON       0
#define VIA_DAC_DPMS_STANDBY  1
#define VIA_DAC_DPMS_SUSPEND  2
#define VIA_DAC_DPMS_OFF      3

/* Encoder types */
#define VIA_ENCODER_DAC       0
#define VIA_ENCODER_TMDS      1
#define VIA_ENCODER_LVDS      2
#define VIA_ENCODER_TV        3

/* I2C related definitions - use VIA_I2C_* constants from via_drv.h */

/* Function prototypes */
void via_dac_set_dpms_control(void __iomem *regbase, int dpms_state);
void via_dac_set_power(void __iomem *regbase, bool power_on);
void via_dac_set_sync_polarity(void __iomem *regbase, u8 polarity);
void via_dac_set_display_source(void __iomem *regbase, int source);
struct i2c_adapter *via_find_ddc_bus(int ddc_id);
void via_encoder_destroy(struct drm_encoder *encoder);

/* Function prototypes for dac.c exported functions */
void via_dac_probe(struct drm_device *dev);
void via_dac_init(struct drm_device *dev);

#endif /* __VIA_DAC_REGS_H__ */
