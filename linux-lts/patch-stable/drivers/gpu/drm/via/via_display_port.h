/*
 * Copyright 2023 The OpenChrome Project
 * [via_display_port.h]
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef __VIA_DISPLAY_PORT_H__
#define __VIA_DISPLAY_PORT_H__

/* Include via_hdmi_regs.h to get shared definitions */
#include "via_hdmi_regs.h"

/* Display Interface Port definitions - using values compatible with via_hdmi_regs.h */
/* VIA_DI_PORT_NONE already defined in via_hdmi_regs.h (0) */
#define VIA_DI_PORT_DVP0     0x02  /* Digital Video Port 0 (most basic) */
#define VIA_DI_PORT_DVP1     0x03  /* Digital Video Port 1 */
#define VIA_DI_PORT_DFPLOW   0x04  /* DFP Low (Digital Flat Panel, usually DVI) */
#define VIA_DI_PORT_DFPHIGH  0x05  /* DFP High */
#define VIA_DI_PORT_LVDS     0x06  /* LVDS (Low-Voltage Differential Signaling) for laptop panels */
/* VIA_DI_PORT_HDMI already defined in via_hdmi_regs.h (1) */

#endif /* __VIA_DISPLAY_PORT_H__ */
