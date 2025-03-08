/*
 * Copyright 2023 The OpenChrome Project
 * [via_tmds_regs.h]
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

#ifndef __VIA_TMDS_REGS_H__
#define __VIA_TMDS_REGS_H__

#include <linux/types.h>
#include <drm/drm_device.h>
#include <linux/i2c.h>

/* TMDS transmitter types */
#define VIA_TMDS_SII164      0x01
#define VIA_TMDS_VT1632      0x02

/* Digital Interface port types */
#define VIA_DI_PORT_NONE     0x00
#define VIA_DI_PORT_DVP0     0x01
#define VIA_DI_PORT_DVP1     0x02
#define VIA_DI_PORT_DFPLOW   0x03
#define VIA_DI_PORT_DFPHIGH  0x04

/* Function prototypes for TMDS transmitter functions */
void via_transmitter_io_pad_state(struct drm_device *dev, u32 di_port, bool enable);
void via_transmitter_clock_drive_strength(struct drm_device *dev, u32 di_port, u8 strength);
void via_transmitter_data_drive_strength(struct drm_device *dev, u32 di_port, u8 strength);
void via_transmitter_display_source(struct drm_device *dev, u32 di_port, int index);
void via_clock_source(struct drm_device *dev, u32 di_port, bool enable);
void via_output_enable(struct drm_device *dev, u32 di_port, bool enable);

bool via_vt1632_probe(struct drm_device *dev, struct i2c_adapter *i2c_bus);
void via_vt1632_init(struct drm_device *dev);
bool via_sii164_probe(struct drm_device *dev, u32 i2c_bus_id, u32 di_port);
void via_sii164_init(struct drm_device *dev);

#endif /* __VIA_TMDS_REGS_H__ */
