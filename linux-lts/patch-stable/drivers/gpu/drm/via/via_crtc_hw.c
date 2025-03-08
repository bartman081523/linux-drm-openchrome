/*
 * Copyright 2023 The OpenChrome Project
 * [via_crtc_hw.c]
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

#include "via_drv.h"
#include "via_crtc_hw.h"
#include "via_disp_reg.h"
#include "via_regset.h"
#include <drm/drm_print.h>
#include <linux/module.h>

/* Instead of defining these functions here, declare them as extern */
extern void load_register_tables(void __iomem *regbase, struct vga_registers *table);
extern void load_value_to_registers(void __iomem *regbase, const struct vga_registers *regs, u32 value);

/* Fix struct initializations - remove extra fields */
const struct vga_regset iga1_cle266_fifo_depth_select[] = {
    { VIA_PORT_SEQ, 0x17, 0, 6 }
};
EXPORT_SYMBOL_GPL(iga1_cle266_fifo_depth_select);

const struct vga_regset iga1_k8m800_fifo_depth_select[] = {
    { VIA_PORT_SEQ, 0x17, 0, 7 }
};
EXPORT_SYMBOL_GPL(iga1_k8m800_fifo_depth_select);

const struct vga_regset iga2_cle266_fifo_depth_select[] = {
    { VIA_PORT_CRT, 0x68, 4, 7 }
};
EXPORT_SYMBOL_GPL(iga2_cle266_fifo_depth_select);

const struct vga_regset iga2_k8m800_fifo_depth_select[] = {
    { VIA_PORT_CRT, 0x68, 4, 7 }
};
EXPORT_SYMBOL_GPL(iga2_k8m800_fifo_depth_select);

const struct vga_regset iga1_cle266_fifo_threshold_select[] = {
    { VIA_PORT_SEQ, 0x16, 0, 5 }
};
EXPORT_SYMBOL_GPL(iga1_cle266_fifo_threshold_select);

const struct vga_regset iga1_k8m800_fifo_threshold_select[] = {
    { VIA_PORT_SEQ, 0x16, 0, 5 }
};
EXPORT_SYMBOL_GPL(iga1_k8m800_fifo_threshold_select);

const struct vga_regset iga2_cle266_fifo_threshold_select[] = {
    { VIA_PORT_CRT, 0x68, 0, 3 }
};
EXPORT_SYMBOL_GPL(iga2_cle266_fifo_threshold_select);

const struct vga_regset iga2_k8m800_fifo_threshold_select[] = {
    { VIA_PORT_CRT, 0x68, 0, 3 }
};
EXPORT_SYMBOL_GPL(iga2_k8m800_fifo_threshold_select);

const struct vga_regset iga1_cle266_fifo_high_threshold_select[] = {
    { VIA_PORT_SEQ, 0x18, 0, 5 }
};
EXPORT_SYMBOL_GPL(iga1_cle266_fifo_high_threshold_select);

const struct vga_regset iga1_k8m800_fifo_high_threshold_select[] = {
    { VIA_PORT_SEQ, 0x18, 0, 5 }
};
EXPORT_SYMBOL_GPL(iga1_k8m800_fifo_high_threshold_select);

const struct vga_regset iga2_fifo_high_threshold_select[] = {
    { VIA_PORT_CRT, 0x92, 0, 3 }
};
EXPORT_SYMBOL_GPL(iga2_fifo_high_threshold_select);

const struct vga_regset iga1_cle266_display_queue_expire_num[] = {
    { VIA_PORT_SEQ, 0x22, 0, 4 }
};
EXPORT_SYMBOL_GPL(iga1_cle266_display_queue_expire_num);

const struct vga_regset iga1_k8m800_display_queue_expire_num[] = {
    { VIA_PORT_SEQ, 0x22, 0, 4 }
};
EXPORT_SYMBOL_GPL(iga1_k8m800_display_queue_expire_num);

const struct vga_regset iga2_display_queue_expire_num[] = {
    { VIA_PORT_CRT, 0x94, 0, 6 }
};
EXPORT_SYMBOL_GPL(iga2_display_queue_expire_num);

/* IGA2 mode setting functions */
void via_iga2_set_interlace_mode(void __iomem *regbase, bool enable)
{
    /* Implementation stub */
    if (enable) {
        svga_wcrt_mask(regbase, 0x67, 0x80, 0x80);
    } else {
        svga_wcrt_mask(regbase, 0x67, 0x00, 0x80);
    }
}
EXPORT_SYMBOL_GPL(via_iga2_set_interlace_mode);

void via_iga2_set_palette_lut_resolution(void __iomem *regbase, bool is_8bits)
{
    /* Implementation updated to use bool parameter instead of u8 */
    if (is_8bits) {
        svga_wseq_mask(regbase, 0x15, 0x80, 0x80); /* 8-bit */
    } else {
        svga_wseq_mask(regbase, 0x15, 0x00, 0x80); /* 6-bit */
    }
}
EXPORT_SYMBOL_GPL(via_iga2_set_palette_lut_resolution);

/* IGA1 mode setting functions - implementing missing symbols */
void via_iga1_set_interlace_mode(void __iomem *regbase, bool enable)
{
    /* Implementation stub */
    if (enable) {
        /* Set interlace mode for IGA1 */
        svga_wcrt_mask(regbase, 0x17, 0x80, 0x80);
    } else {
        /* Disable interlace mode for IGA1 */
        svga_wcrt_mask(regbase, 0x17, 0x00, 0x80);
    }
}
EXPORT_SYMBOL_GPL(via_iga1_set_interlace_mode);

void via_iga1_set_hsync_shift(void __iomem *regbase, u8 shift_value)
{
    /* Implementation stub */
    /* Set horizontal sync shift value for IGA1 */
    svga_wcrt_mask(regbase, 0x33, shift_value, 0x1F);
}
EXPORT_SYMBOL_GPL(via_iga1_set_hsync_shift);

void via_iga1_set_palette_lut_resolution(void __iomem *regbase, bool is_8bits)
{
    /* Implementation stub */
    if (is_8bits) {
        /* Set 8-bit palette LUT resolution for IGA1 */
        svga_wseq_mask(regbase, 0x15, 0x80, 0x80);
    } else {
        /* Set 6-bit palette LUT resolution for IGA1 */
        svga_wseq_mask(regbase, 0x15, 0x00, 0x80);
    }
}
EXPORT_SYMBOL_GPL(via_iga1_set_palette_lut_resolution);

/* Define the encoder function structures that were referenced but undefined */
const struct drm_encoder_funcs via_crtc_encoder_funcs = {
    .destroy = via_encoder_destroy,
};
EXPORT_SYMBOL_GPL(via_crtc_encoder_funcs);

const struct drm_encoder_helper_funcs via_crtc_helper_funcs = {
    /* Minimal required implementation */
    .disable = NULL,
    .enable = NULL,
};
EXPORT_SYMBOL_GPL(via_crtc_helper_funcs);
