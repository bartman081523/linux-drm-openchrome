/*
 * Copyright 2023 The OpenChrome Project
 * [via_crtc_hw.h]
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

#ifndef __VIA_CRTC_HW_H__
#define __VIA_CRTC_HW_H__

/* Include the main header first to ensure struct definitions are available */
#include "via_drv.h"

/* Scaling mode definitions */
#define VIA_NO_SCALING      0x00000000
#define VIA_HOR_EXPAND      0x00000001
#define VIA_VER_EXPAND      0x00000002
#define VIA_EXPAND          (VIA_HOR_EXPAND | VIA_VER_EXPAND)
#define VIA_SHRINK          0x00000004

/* Scaling constants */
#define HOR_SCALE           0x00000001
#define VER_SCALE           0x00000002
#define HOR_VER_SCALE       (HOR_SCALE | VER_SCALE)

/* Hardware constants */
#define VX800_IGA1_FIFO_MAX_DEPTH            384
#define VX800_IGA1_FIFO_THRESHOLD            328
#define VX800_IGA1_FIFO_HIGH_THRESHOLD       296
#define VX800_IGA1_DISPLAY_QUEUE_EXPIRE_NUM  124

#define VX800_IGA2_FIFO_MAX_DEPTH            384
#define VX800_IGA2_FIFO_THRESHOLD            328
#define VX800_IGA2_FIFO_HIGH_THRESHOLD       296
#define VX800_IGA2_DISPLAY_QUEUE_EXPIRE_NUM  124

/* Helper macros for FIFO threshold calculation - comment out conflicting macros */
/* #define IGA1_FIFO_THRESHOLD_FORMULA(x)           ((x >> 1) - 1) */
/* #define IGA1_FIFO_HIGH_THRESHOLD_FORMULA(x)      ((x * 3 / 4) - 1) */
/* #define IGA1_FIFO_LOW_THRESHOLD_SELECT_FORMULA(x)   ((x >> 2) - 1) */

/* External register arrays declarations */
extern const struct vga_regset iga1_cle266_fifo_depth_select[];
extern const struct vga_regset iga1_k8m800_fifo_depth_select[];
extern const struct vga_regset iga2_k8m800_fifo_depth_select[];
extern const struct vga_regset iga1_cle266_fetch_count[];
extern const struct vga_regset iga1_k8m800_fetch_count[];
extern const struct vga_regset iga1_fifo_depth[];
extern const struct vga_regset iga1_fifo_threshold_select[];
extern const struct vga_regset iga1_display_queue_expire_num[];
extern const struct vga_regset iga2_fifo_depth[];
extern const struct vga_regset iga2_fifo_threshold[];
extern const struct vga_regset lcd_starting_address[];
extern const struct vga_regset iga1_starting_address[];
extern const struct vga_regset iga2_starting_address[];
extern const struct vga_regset iga1_offset[];
extern const struct vga_regset iga2_offset[];
extern const struct vga_regset iga1_fetch_count[];
extern const struct vga_regset iga2_fetch_count[];
extern const struct vga_regset burst_count[];
extern const struct vga_regset iga2_fifo_max_depth[];
extern const struct vga_regset lcd_enable_polarity[];
extern const struct vga_regset lcd_panel_type[];
extern const struct vga_regset lcd_mode[];
extern const struct vga_regset hor_scaling_factor[];
extern const struct vga_regset ver_scaling_factor[];
extern const struct vga_regset hor_scaling_enable[];
extern const struct vga_regset ver_scaling_enable[];
extern const struct vga_regset lcd_ver_scaling[];
extern const struct vga_regset iga2_cle266_fifo_depth_select[];

/* Function prototypes */

/* via_crtc.c */
void load_register_tables(void __iomem *regbase, struct vga_registers *table);
void via_iga1_set_palette_lut_resolution(void __iomem *regbase, bool is_8bit);
void via_iga2_set_palette_lut_resolution(void __iomem *regbase, bool is_8bit);
void via_iga1_set_interlace_mode(void __iomem *regbase, bool is_interlace);
void via_iga2_set_interlace_mode(void __iomem *regbase, bool is_interlace);
void via_iga1_set_hsync_shift(void __iomem *regbase, u8 value);
u32 via_get_clk_value(struct drm_device *dev, u32 clk);
void via_set_vclock(struct drm_crtc *crtc, u32 clk_value);
void via_load_crtc_pixel_timing(struct drm_crtc *crtc, struct drm_display_mode *mode);

#endif /* __VIA_CRTC_HW_H__ */
