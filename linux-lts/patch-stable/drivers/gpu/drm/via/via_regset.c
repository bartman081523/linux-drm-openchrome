/*
 * Copyright 2016 Kevin Brace
 * Copyright 2016 The OpenChrome Project
 * [via_regset.c]
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

#include "via_drv.h"
#include "via_regset.h"

#include <drm/drm_print.h>
#include <linux/module.h>

/* Static array for vpit_table to reference */
static const struct vga_regset vpit_regs[] = {
    { VIA_PORT_SEQ, 0x00, 0x00, 0x00 }
};

/* VGA Initialization Table */
struct vga_registers vpit_table[] = {
    { 1, vpit_regs },
    { 0, NULL } /* Terminate with a 0 count entry */
};
EXPORT_SYMBOL_GPL(vpit_table);

/* Define register sets with at least one element */
/* Use extern for symbols already defined in via_crtc_hw.c */
extern const struct vga_regset iga1_cle266_fifo_depth_select[];
extern const struct vga_regset iga1_k8m800_fifo_depth_select[];
extern const struct vga_regset iga2_cle266_fifo_depth_select[];
extern const struct vga_regset iga2_k8m800_fifo_depth_select[];

/* Define minimum implementations for all required arrays */
const struct vga_regset lcd_hor_scaling[] = {
    { VIA_PORT_CRT, 0xA2, 0, 7 }
};
EXPORT_SYMBOL_GPL(lcd_hor_scaling);

const struct vga_regset lcd_ver_scaling[] = {
    { VIA_PORT_CRT, 0xA2, 0, 3 }
};
EXPORT_SYMBOL_GPL(lcd_ver_scaling);

const struct vga_regset iga1_hor_total[] = {
    { VIA_PORT_CRT, 0x00, 0, 7 }
};
EXPORT_SYMBOL_GPL(iga1_hor_total);

const struct vga_regset iga1_hor_addr[] = {
    { VIA_PORT_CRT, 0x01, 0, 7 }
};
EXPORT_SYMBOL_GPL(iga1_hor_addr);

const struct vga_regset iga1_hor_blank_start[] = {
    { VIA_PORT_CRT, 0x02, 0, 7 }
};
EXPORT_SYMBOL_GPL(iga1_hor_blank_start);

const struct vga_regset iga1_hor_blank_end[] = {
    { VIA_PORT_CRT, 0x03, 0, 7 }
};
EXPORT_SYMBOL_GPL(iga1_hor_blank_end);

const struct vga_regset iga1_hor_sync_start[] = {
    { VIA_PORT_CRT, 0x04, 0, 7 }
};
EXPORT_SYMBOL_GPL(iga1_hor_sync_start);

const struct vga_regset iga1_hor_sync_end[] = {
    { VIA_PORT_CRT, 0x05, 0, 7 }
};
EXPORT_SYMBOL_GPL(iga1_hor_sync_end);

const struct vga_regset iga1_ver_total[] = {
    { VIA_PORT_CRT, 0x06, 0, 7 }
};
EXPORT_SYMBOL_GPL(iga1_ver_total);

const struct vga_regset iga1_ver_addr[] = {
    { VIA_PORT_CRT, 0x12, 0, 7 }
};
EXPORT_SYMBOL_GPL(iga1_ver_addr);

const struct vga_regset iga1_ver_blank_start[] = {
    { VIA_PORT_CRT, 0x15, 0, 7 }
};
EXPORT_SYMBOL_GPL(iga1_ver_blank_start);

const struct vga_regset iga1_ver_blank_end[] = {
    { VIA_PORT_CRT, 0x16, 0, 7 }
};
EXPORT_SYMBOL_GPL(iga1_ver_blank_end);

const struct vga_regset iga1_ver_sync_start[] = {
    { VIA_PORT_CRT, 0x10, 0, 7 }
};
EXPORT_SYMBOL_GPL(iga1_ver_sync_start);

const struct vga_regset iga1_ver_sync_end[] = {
    { VIA_PORT_CRT, 0x11, 0, 7 }
};
EXPORT_SYMBOL_GPL(iga1_ver_sync_end);

const struct vga_regset iga1_fetch_count[] = {
    { VIA_PORT_CRT, 0x1C, 0, 7 }
};
EXPORT_SYMBOL_GPL(iga1_fetch_count);

const struct vga_regset iga1_offset[] = {
    { VIA_PORT_CRT, 0x13, 0, 7 }
};
EXPORT_SYMBOL_GPL(iga1_offset);

const struct vga_regset iga2_hor_total[] = {
    { VIA_PORT_CRT, 0x50, 0, 7 }
};
EXPORT_SYMBOL_GPL(iga2_hor_total);

const struct vga_regset iga2_hor_addr[] = {
    { VIA_PORT_CRT, 0x51, 0, 7 }
};
EXPORT_SYMBOL_GPL(iga2_hor_addr);

const struct vga_regset iga2_hor_blank_start[] = {
    { VIA_PORT_CRT, 0x52, 0, 7 }
};
EXPORT_SYMBOL_GPL(iga2_hor_blank_start);

const struct vga_regset iga2_hor_blank_end[] = {
    { VIA_PORT_CRT, 0x53, 0, 7 }
};
EXPORT_SYMBOL_GPL(iga2_hor_blank_end);

const struct vga_regset iga2_hor_sync_start[] = {
    { VIA_PORT_CRT, 0x54, 0, 7 }
};
EXPORT_SYMBOL_GPL(iga2_hor_sync_start);

const struct vga_regset iga2_hor_sync_end[] = {
    { VIA_PORT_CRT, 0x55, 0, 7 }
};
EXPORT_SYMBOL_GPL(iga2_hor_sync_end);

const struct vga_regset iga2_ver_total[] = {
    { VIA_PORT_CRT, 0x56, 0, 7 }
};
EXPORT_SYMBOL_GPL(iga2_ver_total);

const struct vga_regset iga2_ver_addr[] = {
    { VIA_PORT_CRT, 0x57, 0, 7 }
};
EXPORT_SYMBOL_GPL(iga2_ver_addr);

const struct vga_regset iga2_ver_blank_start[] = {
    { VIA_PORT_CRT, 0x58, 0, 7 }
};
EXPORT_SYMBOL_GPL(iga2_ver_blank_start);

const struct vga_regset iga2_ver_blank_end[] = {
    { VIA_PORT_CRT, 0x59, 0, 7 }
};
EXPORT_SYMBOL_GPL(iga2_ver_blank_end);

const struct vga_regset iga2_ver_sync_start[] = {
    { VIA_PORT_CRT, 0x5A, 0, 7 }
};
EXPORT_SYMBOL_GPL(iga2_ver_sync_start);

const struct vga_regset iga2_ver_sync_end[] = {
    { VIA_PORT_CRT, 0x5B, 0, 7 }
};
EXPORT_SYMBOL_GPL(iga2_ver_sync_end);

const struct vga_regset iga2_fetch_count[] = {
    { VIA_PORT_CRT, 0x65, 0, 7 }
};
EXPORT_SYMBOL_GPL(iga2_fetch_count);

const struct vga_regset iga2_offset[] = {
    { VIA_PORT_CRT, 0x66, 0, 7 }
};
EXPORT_SYMBOL_GPL(iga2_offset);

/* Define remaining required arrays with at least one element - use extern for already defined symbols */
extern const struct vga_regset iga1_cle266_fifo_threshold_select[];
extern const struct vga_regset iga1_k8m800_fifo_threshold_select[];
extern const struct vga_regset iga2_cle266_fifo_threshold_select[];
extern const struct vga_regset iga2_k8m800_fifo_threshold_select[];
extern const struct vga_regset iga1_cle266_fifo_high_threshold_select[];
extern const struct vga_regset iga1_k8m800_fifo_high_threshold_select[];
extern const struct vga_regset iga2_fifo_high_threshold_select[];
extern const struct vga_regset iga1_cle266_display_queue_expire_num[];
extern const struct vga_regset iga1_k8m800_display_queue_expire_num[];
extern const struct vga_regset iga2_display_queue_expire_num[];

/* These functions are already defined in via_vgahw.c - use extern declarations */
extern void load_value_to_registers(void __iomem *regbase, const struct vga_registers *regs, unsigned int value);
extern void load_register_tables(void __iomem *regbase, struct vga_registers *table);

/* Adapter function to handle different struct types */
void via_load_regset(void __iomem *regbase, struct vga_registers *regs, unsigned int value)
{
    /* Call the original function - structure should be compatible now */
    load_value_to_registers(regbase, regs, value);
}
EXPORT_SYMBOL_GPL(via_load_regset);