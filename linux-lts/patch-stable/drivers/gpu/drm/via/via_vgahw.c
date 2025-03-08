/*
 * Copyright 2005 Thomas Hellström. All Rights Reserved.
 * Copyright 2020-2021 Kevin Brace. All Rights Reserved.
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
 *
 * Author: Thomas Hellström 2005.
 */

#include <video/vga.h>
#include "via_vgahw.h"
#include "via_drv.h"
#include <drm/drm_gem_ttm_helper.h>
#include <linux/module.h>
#include "via_ttm_mod.h"

void
svga_wcrt_mask(void __iomem *vgabase, u8 index, u8 value, u8 mask)
{
	u8 tmp = vga_rcrt(vgabase, index) & ~mask;
	vga_wcrt(vgabase, index, tmp | (value & mask));
}

void
svga_wseq_mask(void __iomem *vgabase, u8 index, u8 value, u8 mask)
{
	u8 tmp = vga_rseq(vgabase, index) & ~mask;
	vga_wseq(vgabase, index, tmp | (value & mask));
}

void
svga_wgfx_mask(void __iomem *vgabase, u8 index, u8 value, u8 mask)
{
	u8 tmp = vga_rgfx(vgabase, index) & ~mask;
	vga_wgfx(vgabase, index, tmp | (value & mask));
}

void
svga_wmisc_mask(void __iomem *vgabase, u8 value, u8 mask)
{
	u8 tmp = vga_r(vgabase, VGA_MIS_R) & ~mask;
	vga_w(vgabase, VGA_MIS_W, tmp | (value & mask));
}

/* Helper function to load values into registers */
void 
load_value_to_registers(void __iomem *regbase, const struct vga_registers *regs, unsigned int value)
{
    int i;
    enum via_vga_port_type port_type;
    
    if (!regs || !regs->regs)
        return;
        
    for (i = 0; i < regs->count; i++) {
        /* Calculate mask based on the high and low values */
        u8 mask_bits = (1 << (regs->regs[i].high - regs->regs[i].low + 1)) - 1;
        u8 mask = mask_bits << regs->regs[i].low;
        u8 shift_right = regs->regs[i].low;
        
        /* Use type field for port selection */
        port_type = regs->regs[i].type;

        switch (port_type) {
        case VIA_PORT_SEQ:
            svga_wseq_mask(regbase, regs->regs[i].index, 
                          ((value >> shift_right) & mask_bits) << regs->regs[i].low, 
                          mask);
            break;
            
        case VIA_PORT_CRT:
            svga_wcrt_mask(regbase, regs->regs[i].index, 
                          ((value >> shift_right) & mask_bits) << regs->regs[i].low, 
                          mask);
            break;
            
        case VIA_PORT_GFX:
            svga_wgfx_mask(regbase, regs->regs[i].index, 
                          ((value >> shift_right) & mask_bits) << regs->regs[i].low, 
                          mask);
            break;
            
        default:
            /* Handle other register types if needed */
            break;
        }
    }
}
EXPORT_SYMBOL_GPL(load_value_to_registers);

/* Load a table of register values */
void load_register_tables(void __iomem *regbase, struct vga_registers *table)
{
    int i;
    enum via_vga_port_type port_type;
    
    if (!table || !table->regs)
        return;
    
    for (i = 0; i < table->count; i++) {
        /* Calculate mask based on the high and low bit positions */
        u8 mask_bits = (1 << (table->regs[i].high - table->regs[i].low + 1)) - 1;
        u8 mask = mask_bits << table->regs[i].low;
        
        /* Use type field for port selection */
        port_type = table->regs[i].type;

        switch (port_type) {
        case VIA_PORT_SEQ:
            svga_wseq_mask(regbase, table->regs[i].index, 
                          (table->regs[i].low << 0) & mask,
                          mask);
            break;
            
        case VIA_PORT_CRT:
            svga_wcrt_mask(regbase, table->regs[i].index, 
                          (table->regs[i].low << 0) & mask,
                          mask);
            break;
            
        /* Add cases for other port types as needed */
        
        default:
            break;
        }
    }
}
EXPORT_SYMBOL_GPL(load_register_tables);
