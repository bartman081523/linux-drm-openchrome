/*
 * Copyright 2023 The OpenChrome Project
 * [via_vgahw.h]
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
 * THE AUTHOR(S) OR COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef __VIA_VGAHW_H__
#define __VIA_VGAHW_H__

#include <video/vga.h>
#include "via_drv.h"

/* Function declarations (prototypes) instead of implementations */
void svga_wcrt_mask(void __iomem *vgabase, u8 index, u8 value, u8 mask);
void svga_wseq_mask(void __iomem *vgabase, u8 index, u8 value, u8 mask);
void svga_wgfx_mask(void __iomem *vgabase, u8 index, u8 value, u8 mask);
void svga_wmisc_mask(void __iomem *vgabase, u8 value, u8 mask);

/* Helper function to load values into registers */
void load_value_to_registers(void __iomem *regbase, const struct vga_registers *regs, unsigned int value);
void load_register_tables(void __iomem *regbase, struct vga_registers *table);

#endif /* __VIA_VGAHW_H__ */
