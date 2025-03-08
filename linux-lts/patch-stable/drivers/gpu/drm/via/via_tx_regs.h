/*
 * Copyright 2023 The OpenChrome Project
 * [via_tx_regs.h]
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

#ifndef __VIA_TX_REGS_H__
#define __VIA_TX_REGS_H__

/* Extended Digital Interface port types */
#define VIA_DI_PORT_NONE       0x00
#define VIA_DI_PORT_DIP0       0x05
#define VIA_DI_PORT_DIP1       0x06
#define VIA_DI_PORT_DVP0       0x07
#define VIA_DI_PORT_DVP1       0x08
#define VIA_DI_PORT_FPDPLOW    0x09
#define VIA_DI_PORT_FPDPHIGH   0x0A
#define VIA_DI_PORT_LVDS1      0x0B
#define VIA_DI_PORT_LVDS2      0x0C
/* Add the constants from via_tmds_regs.h for compatibility */
#define VIA_DI_PORT_DFPLOW     0x03
#define VIA_DI_PORT_DFPHIGH    0x04

/* Function prototypes for DIP port functions */
void via_dip0_set_io_pad_state(void __iomem *base, bool enable);
void via_dip1_set_io_pad_state(void __iomem *base, bool enable);
void via_dip0_set_output_enable(void __iomem *base, bool enable);
void via_dip1_set_output_enable(void __iomem *base, bool enable);
void via_dip0_set_clock_source(void __iomem *base, bool enable);
void via_dip1_set_clock_source(void __iomem *base, bool enable);
void via_dip0_set_display_source(void __iomem *base, int source);
void via_dip1_set_display_source(void __iomem *base, int source);

/* Function prototypes for DVP port functions */
void via_dvp0_set_io_pad_state(void __iomem *base, bool enable);
void via_dvp1_set_io_pad_state(void __iomem *base, bool enable);
void via_dvp0_set_clock_drive_strength(void __iomem *base, u8 strength);
void via_dvp1_set_clock_drive_strength(void __iomem *base, u8 strength);
void via_dvp0_set_data_drive_strength(void __iomem *base, u8 strength);
void via_dvp1_set_data_drive_strength(void __iomem *base, u8 strength);
void via_dvp0_set_display_source(void __iomem *base, int source);
void via_dvp1_set_display_source(void __iomem *base, int source);

/* Function prototypes for FPDP port functions */
void via_fpdp_low_set_io_pad_state(void __iomem *base, bool enable);
void via_fpdp_high_set_io_pad_state(void __iomem *base, bool enable);
void via_fpdp_low_set_display_source(void __iomem *base, int source);
void via_fpdp_high_set_display_source(void __iomem *base, int source);

/* Function prototypes for LVDS port functions */
void via_lvds1_set_io_pad_setting(void __iomem *base, bool enable);
void via_lvds2_set_io_pad_setting(void __iomem *base, bool enable);
void via_lvds1_set_display_source(void __iomem *base, int source);
void via_lvds2_set_display_source(void __iomem *base, int source);

/* Exported function prototypes */
void via_transmitter_io_pad_state(struct drm_device *dev, u32 di_port, bool enable);
void via_output_enable(struct drm_device *dev, u32 di_port, bool enable);
void via_clock_source(struct drm_device *dev, u32 di_port, bool enable);
void via_transmitter_clock_drive_strength(struct drm_device *dev, u32 di_port, u8 strength);
void via_transmitter_data_drive_strength(struct drm_device *dev, u32 di_port, u8 strength);
void via_transmitter_display_source(struct drm_device *dev, u32 di_port, int index);

#endif /* __VIA_TX_REGS_H__ */
