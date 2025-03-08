/*
 * Copyright 2023 The OpenChrome Project
 * [via_hdmi_regs.h]
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

#ifndef __VIA_HDMI_REGS_H__
#define __VIA_HDMI_REGS_H__

/* DisplayPort/HDMI register definitions */
#define DP_EPHY_PLL_REG             0xC280
#define DP_DATA_PASS_ENABLE_REG     0xC640
#define DP_EPHY_MISC_PWR_REG        0xC288

/* DisplayPort/HDMI IRQ definitions */
#define VIA_IRQ_DP_HOT_UNPLUG       0x00000001

/* Digital Interface port types */
#define VIA_DI_PORT_NONE            0
#define VIA_DI_PORT_HDMI            1
#define VIA_DI_PORT_DP              2

/* Function prototypes - update to match implementation */
void via_hdmi_init(struct drm_device *dev, u32 di_port);
enum drm_connector_status via_hdmi_detect(struct drm_connector *connector, bool force);
struct edid *via_hdmi_get_edid(struct drm_connector *connector, struct i2c_adapter *i2c);

#endif /* __VIA_HDMI_REGS_H__ */
