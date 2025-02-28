/*
 * Copyright © 2019-2020 Kevin Brace.
 * Copyright 2012 James Simmons. All Rights Reserved.
 * Copyright 1998-2009 VIA Technologies, Inc. All Rights Reserved.
 * Copyright 2001-2009 S3 Graphics, Inc. All Rights Reserved.
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
 * Author(s):
 * Kevin Brace <kevinbrace@bracecomputerlab.com>
 * James Simmons <jsimmons@infradead.org>
 */

#include <linux/pci.h>
#include <linux/pci_ids.h>

#include <drm/drm_atomic.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_atomic_state_helper.h>
#include <drm/drm_crtc.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_fourcc.h>
#include <drm/drm_framebuffer.h>
#include <drm/drm_gem.h>
#include <drm/drm_mode.h>
#include <drm/drm_modeset_helper_vtables.h>
#include <drm/drm_plane.h>
#include <drm/drm_print.h>

#include <drm/ttm/ttm_bo.h>

#include "via_drv.h"
#include "via_disp_reg.h"
#include "via_crtc_hw.h"

/* Forward Declarations (without static, to fix "follows non-static" errors) */
void via_dump_iga1_regs(struct drm_device *dev);
void via_dump_iga2_regs(struct drm_device *dev);
void via_dump_crtc_info(struct drm_crtc *crtc, const char *label);
static void via_load_vpit_regs(struct via_drm_priv *dev_priv);
static int via_iga1_display_fifo_regs(struct drm_device *dev,
					struct via_crtc *iga,
					struct drm_display_mode *mode,
					struct drm_framebuffer *fb);
static int via_iga2_display_fifo_regs(struct drm_device *dev,
					struct via_crtc *iga,
					struct drm_display_mode *mode,
					struct drm_framebuffer *fb);
void via_load_crtc_pixel_timing(struct drm_crtc *crtc,
		struct drm_display_mode *mode);
static void via_load_crtc_timing(struct via_crtc *iga, struct drm_display_mode *mode);
static int via_gamma_init(struct drm_crtc *crtc);
static void via_mode_set_nofb(struct drm_crtc *crtc);
static void via_crtc_helper_atomic_enable(struct drm_crtc *crtc,
					struct drm_atomic_state *state);
static void via_crtc_helper_atomic_disable(struct drm_crtc *crtc,
					struct drm_atomic_state *state);
static void via_set_scale_path(struct drm_crtc *crtc, u32 scale_type);
static void via_disable_iga_scaling(struct drm_crtc *crtc);
static bool via_set_iga_scale_function(struct drm_crtc *crtc, u32 scale_type);

static bool via_load_iga_scale_factor_regs(struct via_drm_priv *dev_priv,
                    struct drm_display_mode *mode,
                    struct drm_display_mode *adjusted_mode,
                    u32 scale_type, u32 is_hor_or_ver);
static void via_set_iga2_downscale_source_timing(struct drm_crtc *crtc,
                struct drm_display_mode *mode,
                struct drm_display_mode *adjusted_mode);
void via_dump_iga1_regs(struct drm_device *dev)
{
    struct via_drm_priv *dev_priv = to_via_drm_priv(dev); // Correctly get dev_priv

    drm_dbg_kms(dev, "IGA1 Register Dump:\n");
    drm_dbg_kms(dev, "  CR00 (HTotal): 0x%02X\n", vga_rcrt(VGABASE, 0x00));
    drm_dbg_kms(dev, "  CR01 (HDisplay): 0x%02X\n", vga_rcrt(VGABASE, 0x01));
    drm_dbg_kms(dev, "  CR02 (HBlank Start): 0x%02X\n", vga_rcrt(VGABASE, 0x02));
    drm_dbg_kms(dev, "  CR03 (HBlank End): 0x%02X\n", vga_rcrt(VGABASE, 0x03));
    drm_dbg_kms(dev, "  CR04 (HSync Start): 0x%02X\n", vga_rcrt(VGABASE, 0x04));
    drm_dbg_kms(dev, "  CR05 (HSync End): 0x%02X\n", vga_rcrt(VGABASE, 0x05));
    drm_dbg_kms(dev, "  CR06 (VTotal): 0x%02X\n", vga_rcrt(VGABASE, 0x06));
    drm_dbg_kms(dev, "  CR07 (Overflow): 0x%02X\n", vga_rcrt(VGABASE, 0x07));
    drm_dbg_kms(dev, "  CR09 (Max Scan Line): 0x%02X\n", vga_rcrt(VGABASE, 0x09));
    drm_dbg_kms(dev, "  CR10 (VSync Start): 0x%02X\n", vga_rcrt(VGABASE, 0x10));
    drm_dbg_kms(dev, "  CR11 (VSync End): 0x%02X\n", vga_rcrt(VGABASE, 0x11));
    drm_dbg_kms(dev, "  CR12 (VDisplay): 0x%02X\n", vga_rcrt(VGABASE, 0x12));
    drm_dbg_kms(dev, "  CR13 (Offset): 0x%02X\n", vga_rcrt(VGABASE, 0x13));
    drm_dbg_kms(dev, "  CR15 (VBlank Start): 0x%02X\n", vga_rcrt(VGABASE, 0x15));
    drm_dbg_kms(dev, "  CR16 (VBlank End): 0x%02X\n", vga_rcrt(VGABASE, 0x16));
    drm_dbg_kms(dev, "  CR32: 0x%02X\n", vga_rcrt(VGABASE, 0x32));
    drm_dbg_kms(dev, "  CR33 (IGA1 Control): 0x%02X\n", vga_rcrt(VGABASE, 0x33));
    drm_dbg_kms(dev, "  CR34 (IGA1 Base Addr High): 0x%02X\n", vga_rcrt(VGABASE, 0x34));
    drm_dbg_kms(dev, "  CR47: 0x%02X\n", vga_rcrt(VGABASE, 0x47));
    drm_dbg_kms(dev, "  CR48 (IGA1 Base Addr High Ext): 0x%02X\n", vga_rcrt(VGABASE, 0x48));
    drm_dbg_kms(dev, "  CRFD (IGA Common): 0x%02X\n", vga_rcrt(VGABASE, 0xFD));
    drm_dbg_kms(dev, "  SR15: 0x%02X\n", vga_rseq(VGABASE, 0x15));
    drm_dbg_kms(dev, "  SR16: 0x%02X\n", vga_rseq(VGABASE, 0x16));
    drm_dbg_kms(dev, "  SR17: 0x%02X\n", vga_rseq(VGABASE, 0x17));
    drm_dbg_kms(dev, "  SR18: 0x%02X\n", vga_rseq(VGABASE, 0x18));
    drm_dbg_kms(dev, "  SR22: 0x%02X\n", vga_rseq(VGABASE, 0x22));

    // Add more IGA1 registers as needed from via_disp_reg.h and the programming manual.
}

void via_dump_iga2_regs(struct drm_device *dev)
{
    struct via_drm_priv *dev_priv = to_via_drm_priv(dev); // Correctly get dev_priv
    drm_dbg_kms(dev, "IGA2 Register Dump:\n");
    drm_dbg_kms(dev, "  CR50: 0x%02X\n", vga_rcrt(VGABASE, 0x50));
    drm_dbg_kms(dev, "  CR51: 0x%02X\n", vga_rcrt(VGABASE, 0x51));
    drm_dbg_kms(dev, "  CR52: 0x%02X\n", vga_rcrt(VGABASE, 0x52));
    drm_dbg_kms(dev, "  CR53: 0x%02X\n", vga_rcrt(VGABASE, 0x53));
    drm_dbg_kms(dev, "  CR54: 0x%02X\n", vga_rcrt(VGABASE, 0x54));
    drm_dbg_kms(dev, "  CR55: 0x%02X\n", vga_rcrt(VGABASE, 0x55));
    drm_dbg_kms(dev, "  CR56: 0x%02X\n", vga_rcrt(VGABASE, 0x56));
    drm_dbg_kms(dev, "  CR57: 0x%02X\n", vga_rcrt(VGABASE, 0x57));
    drm_dbg_kms(dev, "  CR58: 0x%02X\n", vga_rcrt(VGABASE, 0x58));
    drm_dbg_kms(dev, "  CR59: 0x%02X\n", vga_rcrt(VGABASE, 0x59));
    drm_dbg_kms(dev, "  CR5A: 0x%02X\n", vga_rcrt(VGABASE, 0x5A));
    drm_dbg_kms(dev, "  CR5B: 0x%02X\n", vga_rcrt(VGABASE, 0x5B));
    drm_dbg_kms(dev, "  CR5C: 0x%02X\n", vga_rcrt(VGABASE, 0x5C));
    drm_dbg_kms(dev, "  CR5D: 0x%02X\n", vga_rcrt(VGABASE, 0x5D));
    drm_dbg_kms(dev, "  CR5E: 0x%02X\n", vga_rcrt(VGABASE, 0x5E));
    drm_dbg_kms(dev, "  CR5F: 0x%02X\n", vga_rcrt(VGABASE, 0x5F));
    drm_dbg_kms(dev, "  CR62: 0x%02X\n", vga_rcrt(VGABASE, 0x62));
    drm_dbg_kms(dev, "  CR63: 0x%02X\n", vga_rcrt(VGABASE, 0x63));
    drm_dbg_kms(dev, "  CR64: 0x%02X\n", vga_rcrt(VGABASE, 0x64));
    drm_dbg_kms(dev, "  CR65: 0x%02X\n", vga_rcrt(VGABASE, 0x65));
    drm_dbg_kms(dev, "  CR66: 0x%02X\n", vga_rcrt(VGABASE, 0x66));
    drm_dbg_kms(dev, "  CR67: 0x%02X\n", vga_rcrt(VGABASE, 0x67));
    drm_dbg_kms(dev, "  CR68: 0x%02X\n", vga_rcrt(VGABASE, 0x68));
    drm_dbg_kms(dev, "  CR6A: 0x%02X\n", vga_rcrt(VGABASE, 0x6A));
    drm_dbg_kms(dev, "  CR6B: 0x%02X\n", vga_rcrt(VGABASE, 0x6B));
    drm_dbg_kms(dev, "  CR71: 0x%02X\n", vga_rcrt(VGABASE, 0x71));
    drm_dbg_kms(dev, "  CR79: 0x%02X\n", vga_rcrt(VGABASE, 0x79));
    drm_dbg_kms(dev, "  CR89: 0x%02X\n", vga_rcrt(VGABASE, 0x89));
    drm_dbg_kms(dev, "  CR92: 0x%02X\n", vga_rcrt(VGABASE, 0x92));
    drm_dbg_kms(dev, "  CR94: 0x%02X\n", vga_rcrt(VGABASE, 0x94));
    drm_dbg_kms(dev, "  CR95: 0x%02X\n", vga_rcrt(VGABASE, 0x95));
    drm_dbg_kms(dev, "  CRA2: 0x%02X\n", vga_rcrt(VGABASE, 0xA2));
    drm_dbg_kms(dev, "  CRA3: 0x%02X\n", vga_rcrt(VGABASE, 0xA3));
    drm_dbg_kms(dev, "  CRFD: 0x%02X\n", vga_rcrt(VGABASE, 0xFD));

    // Add more IGA2 registers as needed.
}


void via_dump_crtc_info(struct drm_crtc *crtc, const char *label)
{
    struct via_crtc *iga = container_of(crtc, struct via_crtc, base);
	struct drm_device *dev = crtc->dev;

    drm_dbg_kms(dev, "%s:\n", label);
    drm_dbg_kms(dev, "  CRTC Index: %u\n", iga->index);
    drm_dbg_kms(dev, "  Scaling Mode: 0x%08X\n", iga->scaling_mode);
    drm_dbg_kms(dev, "  Enabled: %s\n", crtc->enabled ? "yes" : "no");

	if (iga->index == 0)
		via_dump_iga1_regs(dev);
	else
		via_dump_iga2_regs(dev);
}


static struct vga_regset vpit_table[] = {
	{VGA_SEQ_I, 0x01, 0xFF, 0x01 },
	{VGA_SEQ_I, 0x02, 0xFF, 0x0F },
	{VGA_SEQ_I, 0x03, 0xFF, 0x00 },
	{VGA_SEQ_I, 0x04, 0xFF, 0x0E },
	{VGA_GFX_I, 0x00, 0xFF, 0x00 },
	{VGA_GFX_I, 0x01, 0xFF, 0x00 },
	{VGA_GFX_I, 0x02, 0xFF, 0x00 },
	{VGA_GFX_I, 0x03, 0xFF, 0x00 },
	{VGA_GFX_I, 0x04, 0xFF, 0x00 },
	{VGA_GFX_I, 0x05, 0xFF, 0x00 },
	{VGA_GFX_I, 0x06, 0xFF, 0x05 },
	{VGA_GFX_I, 0x07, 0xFF, 0x0F },
	{VGA_GFX_I, 0x08, 0xFF, 0xFF }
};

static void via_iga_common_init(struct drm_device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev->dev);
	struct via_drm_priv *dev_priv = to_via_drm_priv(dev);

	drm_dbg_kms(dev, "Entered %s.\n", __func__);

	/* Be careful with 3C5.15[5] - Wrap Around Disable.
	 * It must be set to 1 for proper operation. */
	/* 3C5.15[5]   - Wrap Around Disable
	 *               0: Disable (For Mode 0-13)
	 *               1: Enable
	 * 3C5.15[1]   - Extended Display Mode Enable
	 *               0: Disable
	 *               1: Enable */
	svga_wseq_mask(VGABASE, 0x15, BIT(5) | BIT(1), BIT(5) | BIT(1));
    drm_dbg_kms(dev, "  SR15: 0x%02X\n", vga_rseq(VGABASE, 0x15)); //Log

	/*
	 * It was observed on NeoWare CA10 thin client with DVI that not
	 * resetting CR55[7] to 0 causes the screen driven by IGA2 to get
	 * distorted.
     * Added CLE266 too.
	 */
	if (pdev->device == PCI_DEVICE_ID_VIA_CLE266_GFX) {
		svga_wcrt_mask(VGABASE, 0x55, 0x00, BIT(7));
        drm_dbg_kms(dev, "  CR55: 0x%02X\n", vga_rcrt(VGABASE, 0x55));
	}

	/*
	 * Disable simultaneous display.
	 * Turning this on causes IGA1 to have a display issue.
	 */
	/*
	 * 3X5.6B[3]   - Simultaneous Display Enable
	 *               0: Disable
	 *               1: Enable
	 */
	svga_wcrt_mask(VGABASE, 0x6B, 0x00, BIT(3));
    drm_dbg_kms(dev, "  CR6B: 0x%02X\n", vga_rcrt(VGABASE, 0x6b));

	drm_dbg_kms(dev, "Exiting %s.\n", __func__);
}

static void via_iga1_set_color_depth(struct drm_device *dev,
					u8 cpp, u8 depth)
{
	struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
	u8 data;

	drm_dbg_kms(dev, "Entered %s.\n", __func__);
    drm_dbg_kms(dev, "cpp: %d, depth: %d\n", cpp, depth);

	data = 0x00;

	/* Set the color depth for IGA1. */
	switch (cpp) {
	case 1:
		data |= BIT(4);
		break;
	case 2:
		data = (depth == 15) ? BIT(2) : BIT(4) | BIT(2);
		break;
	case 4:
		data = (depth == 30) ? BIT(3) : BIT(3) | BIT(2);
		data |= BIT(4);
		break;
	default:
		break;
	}

	/*
	 * 3C5.15[4]   - Hi Color Mode Select
	 *               0: 555
	 *               1: 565
	 * 3C5.15[3:2] - Display Color Depth Select
	 *               00: 8bpp
	 *               01: 16bpp
	 *               10: 30bpp
	 *               11: 32bpp
	 */
	svga_wseq_mask(VGABASE, 0x15, data, BIT(4) | BIT(3) | BIT(2));
    drm_dbg_kms(dev, "SR15: 0x%02X\n", vga_rseq(VGABASE, 0x15));

	drm_dbg_kms(dev, "Exiting %s.\n", __func__);
}

static void via_iga2_set_color_depth(struct drm_device *dev,
					u8 cpp, u8 depth)
{
	struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
	u8 data;

	drm_dbg_kms(dev, "Entered %s.\n", __func__);
    drm_dbg_kms(dev, "cpp: %d, depth: %d\n", cpp, depth);

	data = 0x00;

	/* Set the color depth for IGA2. */
	switch (cpp) {
	case 1:
		break;
	case 2:
		data = BIT(6);
		break;
	case 4:
		data = (depth == 30) ? BIT(7) : BIT(7) | BIT(6);
		break;
	default:
		break;
	}

	/*
	 * 3X5.67[7:6] - Display Color Depth Select
	 *               00: 8bpp
	 *               01: 16bpp
	 *               10: 30bpp
	 *               11: 32bpp
	 */
	svga_wcrt_mask(VGABASE, 0x67, data, BIT(7) | BIT(6));
    drm_dbg_kms(dev, "CR67: 0x%02X\n", vga_rcrt(VGABASE, 0x67));

	drm_dbg_kms(dev, "Exiting %s.\n", __func__);
}

static int via_gamma_set(struct drm_crtc *crtc,
				u16 *r, u16 *g, u16 *b,
				uint32_t size,
				struct drm_modeset_acquire_ctx *ctx)
{
	struct drm_device *dev = crtc->dev;
	struct pci_dev *pdev = to_pci_dev(dev->dev);
	struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
	struct via_crtc *iga = container_of(crtc,
						struct via_crtc, base);
	int end = (size > 256) ? 256 : size, i;
	u8 val = 0;
	int ret = 0;

	drm_dbg_kms(dev, "Entered %s.\n", __func__);

	if ((!crtc->enabled) || (!crtc->primary->fb)) {
		ret = -EINVAL;
		goto exit;
	}

	if (!iga->index) {
		/*
		 * Access IGA1's pallette LUT.
		 */
		svga_wseq_mask(VGABASE, 0x1A, 0x00, BIT(0));

		/*
		 * Is it an 8-bit color mode?
		 */
		if (crtc->primary->fb->format->cpp[0] == 1) {
			/* Change to Primary Display's LUT */
			val = vga_rseq(VGABASE, 0x1B);
			vga_wseq(VGABASE, 0x1B, val);
			val = vga_rcrt(VGABASE, 0x67);
			vga_wcrt(VGABASE, 0x67, val);

			/* Fill in IGA1's LUT */
			for (i = 0; i < end; i++) {
				/* Bit mask of palette */
				vga_w(VGABASE, VGA_PEL_MSK, 0xFF);
				vga_w(VGABASE, VGA_PEL_IW, i);
				vga_w(VGABASE, VGA_PEL_D, r[i] >> 8);
				vga_w(VGABASE, VGA_PEL_D, g[i] >> 8);
				vga_w(VGABASE, VGA_PEL_D, b[i] >> 8);
			}
			/* enable LUT */
			svga_wseq_mask(VGABASE, 0x1B, 0x00, BIT(0));
			/*
			 * Disable gamma in case it was enabled
			 * previously
			 */
			svga_wcrt_mask(VGABASE, 0x33, 0x00, BIT(7));
		} else {
			/* Enable Gamma */
			svga_wcrt_mask(VGABASE, 0x33, BIT(7), BIT(7));

			/* Fill in IGA1's gamma */
			for (i = 0; i < end; i++) {
				/* bit mask of palette */
				vga_w(VGABASE, VGA_PEL_MSK, 0xFF);
				vga_w(VGABASE, VGA_PEL_IW, i);
				vga_w(VGABASE, VGA_PEL_D, r[i] >> 8);
				vga_w(VGABASE, VGA_PEL_D, g[i] >> 8);
				vga_w(VGABASE, VGA_PEL_D, b[i] >> 8);
			}
		}
	} else {
		/*
		 * Access IGA2's pallette LUT.
		 */
		svga_wseq_mask(VGABASE, 0x1A, BIT(0), BIT(0));

		/*
		 * Is it an 8-bit color mode?
		 */
		if (crtc->primary->fb->format->cpp[0] == 1) {
			/* Enable Secondary Display Engine */
			svga_wseq_mask(VGABASE, 0x1B, BIT(7), BIT(7));
			/* Second Display Color Depth, 8bpp */
			svga_wcrt_mask(VGABASE, 0x67, 0x3F, 0x3F);

			/*
			 * Enable second display channel just in case.
			 */
			if (!(vga_rcrt(VGABASE, 0x6A) & BIT(7)))
				svga_wcrt_mask(VGABASE, 0x6A,
						BIT(7), BIT(7));

			/* Fill in IGA2's LUT */
			for (i = 0; i < end; i++) {
				/* Bit mask of palette */
				vga_w(VGABASE, VGA_PEL_MSK, 0xFF);
				vga_w(VGABASE, VGA_PEL_IW, i);
				vga_w(VGABASE, VGA_PEL_D, r[i] >> 8);
				vga_w(VGABASE, VGA_PEL_D, g[i] >> 8);
				vga_w(VGABASE, VGA_PEL_D, b[i] >> 8);
			}
			/*
			 * Disable gamma in case it was enabled
			 * previously
			 */
			svga_wcrt_mask(VGABASE, 0x6A, 0x00, BIT(1));
		} else {
			u8 reg_bits = BIT(1);

			/* Bit 1 enables gamma */
			svga_wcrt_mask(VGABASE, 0x6A, BIT(1), BIT(1));

			/* Old platforms LUT are 6 bits in size.
			 * Newer it is 8 bits. */
			switch (pdev->device) {
			case PCI_DEVICE_ID_VIA_CLE266_GFX:
			case PCI_DEVICE_ID_VIA_KM400_GFX:
			case PCI_DEVICE_ID_VIA_K8M800_GFX:
			case PCI_DEVICE_ID_VIA_PM800_GFX:
				break;

			default:
				reg_bits |= BIT(5);
				break;
			}
			svga_wcrt_mask(VGABASE, 0x6A, reg_bits,
					reg_bits);

			/*
			 * Before we fill the second LUT, we have to
			 * enable second display channel. If it's
			 * enabled before, we don't need to do that,
			 * or else the secondary display will be dark
			 * for about 1 sec and then be turned on
			 * again.
			 */
			if (!(vga_rcrt(VGABASE, 0x6A) & BIT(7)))
				svga_wcrt_mask(VGABASE, 0x6A,
						BIT(7), BIT(7));

			/* Fill in IGA2's gamma */
			for (i = 0; i < end; i++) {
				/* bit mask of palette */
				vga_w(VGABASE, VGA_PEL_MSK, 0xFF);
				vga_w(VGABASE, VGA_PEL_IW, i);
				vga_w(VGABASE, VGA_PEL_D, r[i] >> 8);
				vga_w(VGABASE, VGA_PEL_D, g[i] >> 8);
				vga_w(VGABASE, VGA_PEL_D, b[i] >> 8);
			}
		}
	}

exit:
	drm_dbg_kms(dev, "Exiting %s.\n", __func__);
	return ret;
}

static void via_crtc_destroy(struct drm_crtc *crtc)
{
	struct via_crtc *iga = container_of(crtc, struct via_crtc, base);

	drm_crtc_cleanup(&iga->base);
	kfree(iga);
}
static const struct drm_crtc_helper_funcs via_drm_crtc_helper_funcs = {
	.mode_set_nofb = via_mode_set_nofb,
	.atomic_enable = via_crtc_helper_atomic_enable,
	.atomic_disable = via_crtc_helper_atomic_disable,
};
static const struct drm_crtc_funcs via_drm_crtc_funcs = {
	.reset = drm_atomic_helper_crtc_reset,
	.gamma_set = via_gamma_set,
	.set_config = drm_atomic_helper_set_config,
	.destroy = via_crtc_destroy,
	.page_flip = drm_atomic_helper_page_flip,
	.atomic_duplicate_state = drm_atomic_helper_crtc_duplicate_state,
	.atomic_destroy_state = drm_atomic_helper_crtc_destroy_state,
};

static void via_load_vpit_regs(struct via_drm_priv *dev_priv)
{
    struct drm_device *dev = &dev_priv->dev;
    u8 ar[] = {0x00, 0x01, 0x02, 0x03,
            0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0A, 0x0B,
            0x0C, 0x0D, 0x0E, 0x0F,
            0x01, 0x00, 0x0F, 0x00};
    unsigned int i = 0;
    u8 reg_value = 0;

    drm_dbg_kms(dev, "Entered %s.\n", __func__);

    /* Enable changing the palette registers */
    reg_value = vga_r(VGABASE, VGA_IS1_RC);
    drm_dbg_kms(dev, "  VGA_IS1_RC: 0x%02X\n", reg_value); //log the value
    vga_w(VGABASE, VGA_ATT_W, 0x00);

    /* Write Misc register */
    vga_w(VGABASE, VGA_MIS_W, 0xCF);
    drm_dbg_kms(dev, "  VGA_MIS_W: 0x%02X\n", vga_r(VGABASE, VGA_MIS_R)); //log the value

    /* Fill VPIT registers */
    /* Instead of using load_register_tables. */
    svga_wseq_mask(VGABASE, 0x01, 0x01, 0xFF);
    drm_dbg_kms(dev, "  SEQ_I 0x01: 0x%02X\n", vga_rseq(VGABASE, 0x01));
    svga_wseq_mask(VGABASE, 0x02, 0x0F, 0xFF);
    drm_dbg_kms(dev, "  SEQ_I 0x02: 0x%02X\n", vga_rseq(VGABASE, 0x02));
    svga_wseq_mask(VGABASE, 0x03, 0x00, 0xFF);
    drm_dbg_kms(dev, "  SEQ_I 0x03: 0x%02X\n", vga_rseq(VGABASE, 0x03));
    svga_wseq_mask(VGABASE, 0x04, 0x0E, 0xFF);
    drm_dbg_kms(dev, "  SEQ_I 0x04: 0x%02X\n", vga_rseq(VGABASE, 0x04));

    svga_wcrt_mask(VGABASE, 0x00, 0x00, 0xFF);
    drm_dbg_kms(dev, "  GFX_I 0x00: 0x%02X\n", vga_rcrt(VGABASE, 0x00));
    svga_wcrt_mask(VGABASE, 0x01, 0x00, 0xFF);
    drm_dbg_kms(dev, "  GFX_I 0x01: 0x%02X\n", vga_rcrt(VGABASE, 0x01));
    svga_wcrt_mask(VGABASE, 0x02, 0x00, 0xFF);
    drm_dbg_kms(dev, "  GFX_I 0x02: 0x%02X\n", vga_rcrt(VGABASE, 0x02));
    svga_wcrt_mask(VGABASE, 0x03, 0x00, 0xFF);
    drm_dbg_kms(dev, "  GFX_I 0x03: 0x%02X\n", vga_rcrt(VGABASE, 0x03));
    svga_wcrt_mask(VGABASE, 0x04, 0x00, 0xFF);
    drm_dbg_kms(dev, "  GFX_I 0x04: 0x%02X\n", vga_rcrt(VGABASE, 0x04));
    svga_wcrt_mask(VGABASE, 0x05, 0x00, 0xFF);
    drm_dbg_kms(dev, "  GFX_I 0x05: 0x%02X\n", vga_rcrt(VGABASE, 0x05));
    svga_wcrt_mask(VGABASE, 0x06, 0x05, 0xFF);
    drm_dbg_kms(dev, "  GFX_I 0x06: 0x%02X\n", vga_rcrt(VGABASE, 0x06));
    svga_wcrt_mask(VGABASE, 0x07, 0x0F, 0xFF);
    drm_dbg_kms(dev, "  GFX_I 0x07: 0x%02X\n", vga_rcrt(VGABASE, 0x07));
    svga_wcrt_mask(VGABASE, 0x08, 0xFF, 0xFF);
    drm_dbg_kms(dev, "  GFX_I 0x08: 0x%02X\n", vga_rcrt(VGABASE, 0x08));

	/* Write Attribute Controller */
	for (i = 0; i < 0x14; i++) {
		reg_value = vga_r(VGABASE, VGA_IS1_RC);
		vga_w(VGABASE, VGA_ATT_W, i);
		vga_w(VGABASE, VGA_ATT_W, ar[i]);
        drm_dbg_kms(dev, "  VGA_ATT_W[%d]: 0x%02X\n", i, ar[i]);

	}

	/* Disable changing the palette registers */
	reg_value = vga_r(VGABASE, VGA_IS1_RC);
	vga_w(VGABASE, VGA_ATT_W, BIT(5));

    drm_dbg_kms(&dev_priv->dev, "Exiting %s.\n", __func__);
}



static int via_iga1_display_fifo_regs(struct drm_device *dev,
					struct via_crtc *iga,
					struct drm_display_mode *mode,
					struct drm_framebuffer *fb)
{
	struct pci_dev *pdev = to_pci_dev(dev->dev);
	struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
	u32 reg_value;
	unsigned int fifo_max_depth = 0;
	unsigned int fifo_threshold = 0;
	unsigned int fifo_high_threshold = 0;
	unsigned int display_queue_expire_num = 0;
	bool enable_extended_display_fifo = false;
	int ret = 0;

	drm_dbg_kms(dev, "Entered %s.\n", __func__);

	switch (pdev->device) {
	case PCI_DEVICE_ID_VIA_CLE266_GFX:
		if (dev_priv->revision == CLE266_REVISION_AX) {
			if (mode->hdisplay > 1024) {
				/* SR17[6:0] */
				fifo_max_depth = 96;

				/* SR16[5:0] */
				fifo_threshold = 92;

				/* SR18[5:0] */
				fifo_high_threshold = 92;

				enable_extended_display_fifo = true;
			} else {
				/* SR17[6:0] */
				fifo_max_depth = 64;

				/* SR16[5:0] */
				fifo_threshold = 32;

				/* SR18[5:0] */
				fifo_high_threshold = 56;

				enable_extended_display_fifo = false;
			}

			if (dev_priv->vram_type <= VIA_MEM_DDR_200) {
				if (fb->format->depth == 24) {
					if (mode->hdisplay > 1024) {
						if (mode->vdisplay > 768) {
							/* SR22[4:0] */
							display_queue_expire_num = 16;
						} else {
							/* SR22[4:0] */
							display_queue_expire_num = 12;
						}
					} else if (mode->hdisplay > 640) {
						/* SR22[4:0] */
						display_queue_expire_num = 40;
					} else {
						/* SR22[4:0] */
						display_queue_expire_num = 124;
					}
				} else if (fb->format->depth == 16){
					if (mode->hdisplay > 1400) {
						/* SR22[4:0] */
						display_queue_expire_num = 16;
					} else {
						/* SR22[4:0] */
						display_queue_expire_num = 12;
					}
				} else {
					/* SR22[4:0] */
					display_queue_expire_num = 124;
				}
			} else {
				if (mode->hdisplay > 1280) {
					/* SR22[4:0] */
					display_queue_expire_num = 16;
				} else if (mode->hdisplay > 1024) {
					/* SR22[4:0] */
					display_queue_expire_num = 12;
				} else {
					/* SR22[4:0] */
					display_queue_expire_num = 124;
				}
			}
		/* dev_priv->revision == CLE266_REVISION_CX */
		} else {
			if (mode->hdisplay >= 1024) {
				/* SR17[6:0] */
				fifo_max_depth = 128;

				/* SR16[5:0] */
				fifo_threshold = 112;

				/* SR18[5:0] */
				fifo_high_threshold = 92;

				enable_extended_display_fifo = false;
			} else {
				/* SR17[6:0] */
				fifo_max_depth = 64;

				/* SR16[5:0] */
				fifo_threshold = 32;

				/* SR18[5:0] */
				fifo_high_threshold = 56;

				enable_extended_display_fifo = false;
			}

			if (dev_priv->vram_type <= VIA_MEM_DDR_200) {
				if (mode->hdisplay > 1024) {
					if (mode->vdisplay > 768) {
						/* SR22[4:0] */
						display_queue_expire_num = 16;
					} else {
						/* SR22[4:0] */
						display_queue_expire_num = 12;
					}
				} else if (mode->hdisplay > 640) {
					/* SR22[4:0] */
					display_queue_expire_num = 40;
				} else {
					/* SR22[4:0] */
					display_queue_expire_num = 124;
				}
			} else {
				if (mode->hdisplay >= 1280) {
					/* SR22[4:0] */
					display_queue_expire_num = 16;
				} else {
					/* SR22[4:0] */
					display_queue_expire_num = 124;
				}
			}
		}
		break;

	case PCI_DEVICE_ID_VIA_KM400_GFX:
		if ((mode->hdisplay >= 1600) &&
			(dev_priv->vram_type <= VIA_MEM_DDR_200)) {
			/* SR17[6:0] */
			fifo_max_depth = 58;

			/* SR16[5:0] */
			fifo_threshold = 24;

			/* SR18[5:0] */
			fifo_high_threshold = 92;
		} else {
			/* SR17[6:0] */
			fifo_max_depth = 128;

			/* SR16[5:0] */
			fifo_threshold = 112;

			/* SR18[5:0] */
			fifo_high_threshold = 92;
		}

		enable_extended_display_fifo = false;

		if (dev_priv->vram_type <= VIA_MEM_DDR_200) {
			if (mode->hdisplay >= 1600) {
				/* SR22[4:0] */
				display_queue_expire_num = 16;
			} else {
				/* SR22[4:0] */
				display_queue_expire_num = 8;
			}
		} else {
			if (mode->hdisplay >= 1600) {
				/* SR22[4:0] */
				display_queue_expire_num = 40;
			} else {
				/* SR22[4:0] */
				display_queue_expire_num = 36;
			}
		}

		break;
	case PCI_DEVICE_ID_VIA_K8M800_GFX:
		/* SR17[7:0] */
		fifo_max_depth = 384;

		/* SR16[7], SR16[5:0] */
		fifo_threshold = 328;

		/* SR18[7], SR18[5:0] */
		fifo_high_threshold = 296;

		if ((fb->format->depth == 24) &&
			(mode->hdisplay >= 1400)) {
			/* SR22[4:0] */
			display_queue_expire_num = 64;
		} else {
			/* SR22[4:0] */
			display_queue_expire_num = 128;
		}

		break;
	case PCI_DEVICE_ID_VIA_PM800_GFX:
		/* SR17[7:0] */
		fifo_max_depth = 192;

		/* SR16[7], SR16[5:0] */
		fifo_threshold = 128;

		/* SR18[7], SR18[5:0] */
		fifo_high_threshold = 64;

		if ((fb->format->depth == 24) &&
				(mode->hdisplay >= 1400)) {
			/* SR22[4:0] */
			display_queue_expire_num = 64;
		} else {
			/* SR22[4:0] */
			display_queue_expire_num = 124;
		}

		break;
	case PCI_DEVICE_ID_VIA_P4M800_PRO_GFX:
		/* SR17[7:0] */
		fifo_max_depth = CN700_IGA1_FIFO_MAX_DEPTH;

		/* SR16[7], SR16[5:0] */
		fifo_threshold = CN700_IGA1_FIFO_THRESHOLD;

		/* SR18[7], SR18[5:0] */
		fifo_high_threshold = CN700_IGA1_FIFO_HIGH_THRESHOLD;

		/* SR22[4:0] */
		display_queue_expire_num = CN700_IGA1_DISPLAY_QUEUE_EXPIRE_NUM;
		break;
		/* CX700 */
	case PCI_DEVICE_ID_VIA_UNICHROME_PRO_II:
		/* SR17[7:0] */
		fifo_max_depth = CX700_IGA1_FIFO_MAX_DEPTH;

		/* SR16[7], SR16[5:0] */
		fifo_threshold = CX700_IGA1_FIFO_THRESHOLD;

		/* SR18[7], SR18[5:0] */
		fifo_high_threshold = CX700_IGA1_FIFO_HIGH_THRESHOLD;

		/* SR22[4:0] */
		display_queue_expire_num = CX700_IGA1_DISPLAY_QUEUE_EXPIRE_NUM;
		break;

		/* K8M890 */
	case PCI_DEVICE_ID_VIA_CHROME9:
		/* SR17[7:0] */
		fifo_max_depth = K8M890_IGA1_FIFO_MAX_DEPTH;

		/* SR16[7], SR16[5:0] */
		fifo_threshold = K8M890_IGA1_FIFO_THRESHOLD;

		/* SR18[7], SR18[5:0] */
		fifo_high_threshold = K8M890_IGA1_FIFO_HIGH_THRESHOLD;

		/* SR22[4:0] */
		display_queue_expire_num = K8M890_IGA1_DISPLAY_QUEUE_EXPIRE_NUM;
		break;
		/* P4M890 */
	case PCI_DEVICE_ID_VIA_P4M890_GFX:
		/* SR17[7:0] */
		fifo_max_depth = P4M890_IGA1_FIFO_MAX_DEPTH;

		/* SR16[7], SR16[5:0] */
		fifo_threshold = P4M890_IGA1_FIFO_THRESHOLD;

		/* SR18[7], SR18[5:0] */
		fifo_high_threshold = P4M890_IGA1_FIFO_HIGH_THRESHOLD;

		/* SR22[4:0] */
		display_queue_expire_num = P4M890_IGA1_DISPLAY_QUEUE_EXPIRE_NUM;
		break;
		/* P4M900 */
	case PCI_DEVICE_ID_VIA_CHROME9_HC:
		/* SR17[7:0] */
		fifo_max_depth = P4M900_IGA1_FIFO_MAX_DEPTH;

		/* SR16[7], SR16[5:0] */
		fifo_threshold = P4M900_IGA1_FIFO_THRESHOLD;

		/* SR18[7], SR18[5:0] */
		fifo_high_threshold = P4M900_IGA1_FIFO_HIGH_THRESHOLD;

		/* SR22[4:0] */
		display_queue_expire_num = P4M900_IGA1_DISPLAY_QUEUE_EXPIRE_NUM;
		break;
		/* VX800 */
	case PCI_DEVICE_ID_VIA_CHROME9_HC3:
		/* SR17[7:0] */
		fifo_max_depth = VX800_IGA1_FIFO_MAX_DEPTH;

		/* SR16[7], SR16[5:0] */
		fifo_threshold = VX800_IGA1_FIFO_THRESHOLD;

		/* SR18[7], SR18[5:0] */
		fifo_high_threshold = VX800_IGA1_FIFO_HIGH_THRESHOLD;

		/* SR22[4:0] */
		display_queue_expire_num = VX800_IGA1_DISPLAY_QUEUE_EXPIRE_NUM;
		break;
		/* VX855 */
	case PCI_DEVICE_ID_VIA_CHROME9_HCM:
		/* SR17[7:0] */
		fifo_max_depth = VX855_IGA1_FIFO_MAX_DEPTH;

		/* SR16[7], SR16[5:0] */
		fifo_threshold = VX855_IGA1_FIFO_THRESHOLD;

		/* SR18[7], SR18[5:0] */
		fifo_high_threshold = VX855_IGA1_FIFO_HIGH_THRESHOLD;

		/* SR22[4:0] */
		display_queue_expire_num = VX855_IGA1_DISPLAY_QUEUE_EXPIRE_NUM;
		break;
		/* VX900 */
	case PCI_DEVICE_ID_VIA_CHROME9_HD:
		/* SR17[7:0] */
		fifo_max_depth = VX900_IGA1_FIFO_MAX_DEPTH;

		/* SR16[7], SR16[5:0] */
		fifo_threshold = VX900_IGA1_FIFO_THRESHOLD;

		/* SR18[7], SR18[5:0] */
		fifo_high_threshold = VX900_IGA1_FIFO_HIGH_THRESHOLD;

		/* SR22[4:0] */
		display_queue_expire_num = VX900_IGA1_DISPLAY_QUEUE_EXPIRE_NUM;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	if (ret) {
		goto exit;
	}

	if ((pdev->device == PCI_DEVICE_ID_VIA_CLE266_GFX) ||
		(pdev->device == PCI_DEVICE_ID_VIA_KM400_GFX) ||
		(pdev->device == PCI_DEVICE_ID_VIA_K8M800_GFX) ||
		(pdev->device == PCI_DEVICE_ID_VIA_PM800_GFX) ||
		(pdev->device == PCI_DEVICE_ID_VIA_P4M800_PRO_GFX) ||
		(pdev->device == PCI_DEVICE_ID_VIA_UNICHROME_PRO_II)) {
		/* Force PREQ to be always higher than TREQ. */
		svga_wseq_mask(VGABASE, 0x18, BIT(6), BIT(6));
        drm_dbg_kms(dev, "  SR18 after forcing PREQ > TREQ: 0x%02X\n", vga_rseq(VGABASE, 0x18));
	} else {
		svga_wseq_mask(VGABASE, 0x18, 0x00, BIT(6));
        drm_dbg_kms(dev, "  SR18 after forcing PREQ > TREQ: 0x%02X\n", vga_rseq(VGABASE, 0x18));
	}

	if ((pdev->device == PCI_DEVICE_ID_VIA_CLE266_GFX) ||
		(pdev->device == PCI_DEVICE_ID_VIA_KM400_GFX)) {
		if (enable_extended_display_fifo) {
			u32 reg_value = VIA_READ(0x0298);
			VIA_WRITE(0x0298, reg_value | 0x20000000);

			/* Turn on IGA1 extended display FIFO. */
			reg_value = VIA_READ(0x0230);
			VIA_WRITE(0x0230, reg_value | 0x00200000);

			reg_value = VIA_READ(0x0298);
			VIA_WRITE(0x0298, reg_value & (~0x20000000));
		} else {
			u32 reg_value = VIA_READ(0x0298);
			VIA_WRITE(0x0298, reg_value | 0x20000000);

			/* Turn off IGA1 extended display FIFO. */
			reg_value = VIA_READ(0x0230);
			VIA_WRITE(0x0230, reg_value & (~0x00200000));

			reg_value = VIA_READ(0x0298);
			VIA_WRITE(0x0298, reg_value & (~0x20000000));

		}
	}

	/* Set IGA1 Display FIFO Depth Select */
    reg_value = (fifo_max_depth >> 1) - 1;
    svga_wseq_mask(VGABASE, 0x17, reg_value & 0x7F, 0x7F); //SR17, bit[6:0]
    drm_dbg_kms(dev, "  SR17 (FIFO Depth): 0x%02X\n", vga_rseq(VGABASE, 0x17));


	/* Set Display FIFO Threshold Select */
    reg_value = fifo_threshold/4;
    svga_wseq_mask(VGABASE, 0x16, (reg_value & 0x3F) | (fifo_threshold & 0x40) << 1, 0xBF);  //SR16 bit[5:0]
    drm_dbg_kms(dev, "  SR16 (FIFO Threshold): 0x%02X\n", vga_rseq(VGABASE, 0x16));

	/* Set FIFO High Threshold Select */
    reg_value = fifo_high_threshold / 4;
    svga_wseq_mask(VGABASE, 0x18, (reg_value & 0x3F) | (fifo_high_threshold & 0x40) << 1, 0xBF); //SR18 bit[5:0]
    drm_dbg_kms(dev, "  SR18 (FIFO High Threshold): 0x%02X\n", vga_rseq(VGABASE, 0x18));
    

	/* Set Display Queue Expire Num */
    reg_value = display_queue_expire_num / 4;
    svga_wseq_mask(VGABASE, 0x22, reg_value & 0x1F, 0x1F); //SR22 bit[4:0]
    drm_dbg_kms(dev, "  SR22 (Display Queue Expire): 0x%02X\n", vga_rseq(VGABASE, 0x22));

exit:
	drm_dbg_kms(dev, "Exiting %s.\n", __func__);
	return ret;
}

static int via_iga2_display_fifo_regs(struct drm_device *dev,
					struct via_crtc *iga,
					struct drm_display_mode *mode,
					struct drm_framebuffer *fb)
{
	struct pci_dev *pdev = to_pci_dev(dev->dev);
	struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
	u32 reg_value;
	unsigned int fifo_max_depth = 0;
	unsigned int fifo_threshold = 0;
	unsigned int fifo_high_threshold = 0;
	unsigned int display_queue_expire_num = 0;
	bool enable_extended_display_fifo = false;
	int ret = 0;

	drm_dbg_kms(dev, "Entered %s.\n", __func__);

	switch (pdev->device) {
	case PCI_DEVICE_ID_VIA_CLE266_GFX:
		if (dev_priv->revision == CLE266_REVISION_AX) {
			if (((dev_priv->vram_type <= VIA_MEM_DDR_200) &&
				(fb->format->depth > 16) &&
				(mode->vdisplay > 768)) ||
				((dev_priv->vram_type <= VIA_MEM_DDR_266) &&
				(fb->format->depth > 16) &&
				(mode->hdisplay > 1280))) {
				/* CR68[7:4] */
				fifo_max_depth = 88;

				/* CR68[3:0] */
				fifo_threshold = 44;

				enable_extended_display_fifo = true;
			} else {
				/* CR68[7:4] */
				fifo_max_depth = 56;

				/* CR68[3:0] */
				fifo_threshold = 28;

				enable_extended_display_fifo = false;
			}
		/* dev_priv->revision == CLE266_REVISION_CX */
		} else {
			if (mode->hdisplay >= 1024) {
				/* CR68[7:4] */
				fifo_max_depth = 88;

				/* CR68[3:0] */
				fifo_threshold = 44;

				enable_extended_display_fifo = false;
			} else {
				/* CR68[7:4] */
				fifo_max_depth = 56;

				/* CR68[3:0] */
				fifo_threshold = 28;

				enable_extended_display_fifo = false;
			}
		}

		break;
	case PCI_DEVICE_ID_VIA_KM400_GFX:
		if (mode->hdisplay >= 1600) {
			/* CR68[7:4] */
			fifo_max_depth = 120;

			/* CR68[3:0] */
			fifo_threshold = 44;

			enable_extended_display_fifo = true;
		} else if (((mode->hdisplay > 1024) &&
			(fb->format->depth == 24) &&
			(dev_priv->vram_type <= VIA_MEM_DDR_333)) ||
			((mode->hdisplay == 1024) &&
			(fb->format->depth == 24) &&
			(dev_priv->vram_type <= VIA_MEM_DDR_200))) {
			/* CR68[7:4] */
			fifo_max_depth = 104;

			/* CR68[3:0] */
			fifo_threshold = 28;

			enable_extended_display_fifo = true;
		} else if (((mode->hdisplay > 1280) &&
			(fb->format->depth == 16) &&
			(dev_priv->vram_type <= VIA_MEM_DDR_333)) ||
			((mode->hdisplay == 1280) &&
			(fb->format->depth == 16) &&
			(dev_priv->vram_type <= VIA_MEM_DDR_200))) {
			/* CR68[7:4] */
			fifo_max_depth = 88;

			/* CR68[3:0] */
			fifo_threshold = 44;

			enable_extended_display_fifo = true;
		} else {
			/* CR68[7:4] */
			fifo_max_depth = 56;

			/* CR68[3:0] */
			fifo_threshold = 28;

			enable_extended_display_fifo = false;
		}

		break;
	case PCI_DEVICE_ID_VIA_K8M800_GFX:
		/* CR95[7], CR94[7], CR68[7:4] */
		fifo_max_depth = 376;

		/* CR95[6:4], CR68[3:0] */
		fifo_threshold = 328;

		/* CR95[2:0], CR92[3:0] */
		fifo_high_threshold = 296;

		if ((fb->format->depth == 24) &&
			(mode->hdisplay >= 1400)) {
			/* CR94[6:0] */
			display_queue_expire_num = 64;
		} else {
			/* CR94[6:0] */
			display_queue_expire_num = 128;
		}

		break;
	case PCI_DEVICE_ID_VIA_PM800_GFX:
		/* CR95[7], CR94[7], CR68[7:4] */
		fifo_max_depth = 96;

		/* CR95[6:4], CR68[3:0] */
		fifo_threshold = 64;

		/* CR95[2:0], CR92[3:0] */
		fifo_high_threshold = 32;

		if ((fb->format->depth == 24) &&
				(mode->hdisplay >= 1400)) {
			/* CR94[6:0] */
			display_queue_expire_num = 64;
		} else {
			/* CR94[6:0] */
			display_queue_expire_num = 124;
		}

		break;
	case PCI_DEVICE_ID_VIA_P4M800_PRO_GFX:
		/* CR95[7], CR94[7], CR68[7:4] */
		fifo_max_depth = CN700_IGA2_FIFO_MAX_DEPTH;

		/* CR95[6:4], CR68[3:0] */
		fifo_threshold = CN700_IGA2_FIFO_THRESHOLD;

		/* CR95[2:0], CR92[3:0] */
		fifo_high_threshold = CN700_IGA2_FIFO_HIGH_THRESHOLD;

		/* CR94[6:0] */
		display_queue_expire_num = CN700_IGA2_DISPLAY_QUEUE_EXPIRE_NUM;
		break;
		/* CX700 */
	case PCI_DEVICE_ID_VIA_UNICHROME_PRO_II:
		/* CR95[7], CR94[7], CR68[7:4] */
		fifo_max_depth = CX700_IGA2_FIFO_MAX_DEPTH;

		/* CR95[6:4], CR68[3:0] */
		fifo_threshold = CX700_IGA2_FIFO_THRESHOLD;

		/* CR95[2:0], CR92[3:0] */
		fifo_high_threshold = CX700_IGA2_FIFO_HIGH_THRESHOLD;

		/* CR94[6:0] */
		display_queue_expire_num = CX700_IGA2_DISPLAY_QUEUE_EXPIRE_NUM;
		break;

		/* K8M890 */
	case PCI_DEVICE_ID_VIA_CHROME9:
		/* CR95[7], CR94[7], CR68[7:4] */
		fifo_max_depth = K8M890_IGA2_FIFO_MAX_DEPTH;

		/* CR95[6:4], CR68[3:0] */
		fifo_threshold = K8M890_IGA2_FIFO_THRESHOLD;

		/* CR95[2:0], CR92[3:0] */
		fifo_high_threshold = K8M890_IGA2_FIFO_HIGH_THRESHOLD;

		/* CR94[6:0] */
		display_queue_expire_num = K8M890_IGA2_DISPLAY_QUEUE_EXPIRE_NUM;
		break;
		/* P4M890 */
	case PCI_DEVICE_ID_VIA_P4M890_GFX:
		/* CR95[7], CR94[7], CR68[7:4] */
		fifo_max_depth = P4M890_IGA2_FIFO_MAX_DEPTH;

		/* CR95[6:4], CR68[3:0] */
		fifo_threshold = P4M890_IGA2_FIFO_THRESHOLD;

		/* CR95[2:0], CR92[3:0] */
		fifo_high_threshold = P4M890_IGA2_FIFO_HIGH_THRESHOLD;

		/* CR94[6:0] */
		display_queue_expire_num = P4M890_IGA2_DISPLAY_QUEUE_EXPIRE_NUM;
		break;
		/* P4M900 */
	case PCI_DEVICE_ID_VIA_CHROME9_HC:
		/* CR95[7], CR94[7], CR68[7:4] */
		fifo_max_depth = P4M900_IGA2_FIFO_MAX_DEPTH;

		/* CR95[7], CR94[7], CR68[7:4] */
		fifo_threshold = P4M900_IGA2_FIFO_THRESHOLD;

		/* CR95[2:0], CR92[3:0] */
		fifo_high_threshold = P4M900_IGA2_FIFO_HIGH_THRESHOLD;

		/* CR94[6:0] */
		display_queue_expire_num = P4M900_IGA2_DISPLAY_QUEUE_EXPIRE_NUM;
		break;
		/* VX800 */
	case PCI_DEVICE_ID_VIA_CHROME9_HC3:
		/* CR95[7], CR94[7], CR68[7:4] */
		fifo_max_depth = VX800_IGA2_FIFO_MAX_DEPTH;

		/* CR95[6:4], CR68[3:0] */
		fifo_threshold = VX800_IGA2_FIFO_THRESHOLD;

		/* CR95[2:0], CR92[3:0] */
		fifo_high_threshold = VX800_IGA2_FIFO_HIGH_THRESHOLD;

		/* CR94[6:0] */
		display_queue_expire_num = VX800_IGA2_DISPLAY_QUEUE_EXPIRE_NUM;
		break;
		/* VX855 */
	case PCI_DEVICE_ID_VIA_CHROME9_HCM:
		/* CR95[7], CR94[7], CR68[7:4] */
		fifo_max_depth = VX855_IGA2_FIFO_MAX_DEPTH;

		/* CR95[6:4], CR68[3:0] */
		fifo_threshold = VX855_IGA2_FIFO_THRESHOLD;

		/* CR95[2:0], CR92[3:0] */
		fifo_high_threshold = VX855_IGA2_FIFO_HIGH_THRESHOLD;

		/* CR94[6:0] */
		display_queue_expire_num = VX855_IGA2_DISPLAY_QUEUE_EXPIRE_NUM;
		break;
		/* VX900 */
	case PCI_DEVICE_ID_VIA_CHROME9_HD:
		/* CR95[7], CR94[7], CR68[7:4] */
		fifo_max_depth = VX900_IGA2_FIFO_MAX_DEPTH;

		/* CR95[6:4], CR68[3:0] */
		fifo_threshold = VX900_IGA2_FIFO_THRESHOLD;

		/* CR95[2:0], CR92[3:0] */
		fifo_high_threshold = VX900_IGA2_FIFO_HIGH_THRESHOLD;

        /* CR94[6:0] */
		display_queue_expire_num = VX900_IGA2_DISPLAY_QUEUE_EXPIRE_NUM;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	if (ret) {
		goto exit;
	}

	if ((pdev->device == PCI_DEVICE_ID_VIA_CLE266_GFX) ||
		(pdev->device == PCI_DEVICE_ID_VIA_KM400_GFX)) {
		if (enable_extended_display_fifo) {
			/* Enable IGA2 extended display FIFO. */
			svga_wcrt_mask(VGABASE, 0x6a, BIT(5), BIT(5));
            drm_dbg_kms(dev, "  CR6A after enabling extended display FIFO: 0x%02X\n", vga_rcrt(VGABASE, 0x6A));
		} else {
			/* Disable IGA2 extended display FIFO. */
			svga_wcrt_mask(VGABASE, 0x6a, 0x00, BIT(5));
            drm_dbg_kms(dev, "  CR6A after disabling extended display FIFO: 0x%02X\n", vga_rcrt(VGABASE, 0x6A));
		}
	}

	if ((pdev->device == PCI_DEVICE_ID_VIA_CLE266_GFX) ||
		(pdev->device == PCI_DEVICE_ID_VIA_KM400_GFX)) {
		/* Set IGA2 Display FIFO Depth Select */
		reg_value = IGA2_FIFO_DEPTH_SELECT_FORMULA(fifo_max_depth);
        svga_wcrt_mask(VGABASE, 0x68, (reg_value << 4) & 0xF0, 0xF0); //CR68 bits[7:4]
		drm_dbg_kms(dev, "  CR68 (FIFO Depth): 0x%02X\n", vga_rcrt(VGABASE, 0x68));

		/* Set Display FIFO Threshold Select */
		reg_value = fifo_threshold / 4;
        svga_wcrt_mask(VGABASE, 0x68, reg_value & 0xF, 0x0F); //CR68 bits [3:0]
		drm_dbg_kms(dev, "  CR68 (FIFO Threshold): 0x%02X\n", vga_rcrt(VGABASE, 0x68));

	} else {
		/* Set IGA2 Display FIFO Depth Select */

        /* CR95[7], CR94[7], CR68[7:4] */
        reg_value = (fifo_max_depth >> 3) - 1;
        svga_wcrt_mask(VGABASE, 0x68, (reg_value << 4) & 0xF0, 0xF0);
        svga_wcrt_mask(VGABASE, 0x94, (reg_value >> 3) & 0x80, 0x80);
        svga_wcrt_mask(VGABASE, 0x95, (reg_value >> 4) & 0x80, 0x80);
        drm_dbg_kms(dev, "  CR68 (FIFO Depth): 0x%02X\n", vga_rcrt(VGABASE, 0x68));
        drm_dbg_kms(dev, "  CR94 (FIFO Depth): 0x%02X\n", vga_rcrt(VGABASE, 0x94));
        drm_dbg_kms(dev, "  CR95 (FIFO Depth): 0x%02X\n", vga_rcrt(VGABASE, 0x95));

		/* Set Display FIFO Threshold Select */

        /* CR95[6:4], CR68[3:0] */
        reg_value = fifo_threshold / 4;
        svga_wcrt_mask(VGABASE, 0x68, reg_value & 0x0F, 0x0F);
        svga_wcrt_mask(VGABASE, 0x95, (reg_value << 1) & 0x70, 0x70);
        drm_dbg_kms(dev, "  CR68 (FIFO Threshold): 0x%02X\n", vga_rcrt(VGABASE, 0x68));
        drm_dbg_kms(dev, "  CR95 (FIFO Threshold): 0x%02X\n", vga_rcrt(VGABASE, 0x95));
		/* Set FIFO High Threshold Select */

        /* CR95[2:0], CR92[3:0] */
        reg_value = fifo_high_threshold / 4;
        svga_wcrt_mask(VGABASE, 0x92, reg_value & 0x0F, 0x0F);
        svga_wcrt_mask(VGABASE, 0x95, (reg_value << 4) & 0x07, 0x07);
        drm_dbg_kms(dev, "  CR92 (FIFO High Threshold): 0x%02X\n", vga_rcrt(VGABASE, 0x92));
        drm_dbg_kms(dev, "  CR95 (FIFO High Threshold): 0x%02X\n", vga_rcrt(VGABASE, 0x95));

		/* Set Display Queue Expire Num */
        reg_value = display_queue_expire_num /4;
        svga_wcrt_mask(VGABASE, 0x94, reg_value & 0x7F, 0x7F);
        drm_dbg_kms(dev, "  CR94 (Display Queue Expire): 0x%02X\n", vga_rcrt(VGABASE, 0x94));
	}

exit:
	drm_dbg_kms(dev, "Exiting %s.\n", __func__);
	return ret;
}

/* Load CRTC Pixel Timing registers */
 void via_load_crtc_pixel_timing(struct drm_crtc *crtc,
				struct drm_display_mode *mode)
{
    struct drm_device *dev = crtc->dev;
    //struct via_crtc *iga = container_of(crtc, struct via_crtc, base); // Removed unused variable.
    struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
    u32 reg_value = 0;

    drm_dbg_kms(dev, "Entered %s.\n", __func__);

    reg_value = IGA1_PIXELTIMING_HOR_TOTAL_FORMULA(mode->crtc_htotal);
    VIA_WRITE_MASK(IGA1_PIX_H_TOTAL_REG, reg_value, IGA1_PIX_H_TOTAL_MASK);
    drm_dbg_kms(dev, "  IGA1_PIX_H_TOTAL_REG: 0x%08X\n", VIA_READ(IGA1_PIX_H_TOTAL_REG));


    reg_value = IGA1_PIXELTIMING_HOR_ADDR_FORMULA(mode->crtc_hdisplay) << 16;
    VIA_WRITE_MASK(IGA1_PIX_H_ADDR_REG, reg_value, IGA1_PIX_H_ADDR_MASK);
    drm_dbg_kms(dev, "  IGA1_PIX_H_ADDR_REG: 0x%08X\n", VIA_READ(IGA1_PIX_H_ADDR_REG));

    reg_value = IGA1_PIXELTIMING_HOR_BLANK_START_FORMULA(
			        mode->crtc_hblank_start);
    VIA_WRITE_MASK(IGA1_PIX_H_BNK_ST_REG, reg_value, IGA1_PIX_H_BNK_ST_MASK);
    drm_dbg_kms(dev, "  IGA1_PIX_H_BNK_ST_REG: 0x%08X\n", VIA_READ(IGA1_PIX_H_BNK_ST_REG));

    reg_value = IGA1_PIXELTIMING_HOR_BLANK_END_FORMULA(mode->crtc_hblank_end) << 16;
    VIA_WRITE_MASK(IGA1_PIX_H_BNK_END_REG, reg_value, IGA1_PIX_H_BNK_END_MASK);
    drm_dbg_kms(dev, "  IGA1_PIX_H_BNK_END_REG: 0x%08X\n", VIA_READ(IGA1_PIX_H_BNK_END_REG));

    reg_value = IGA1_PIXELTIMING_HOR_SYNC_START_FORMULA(mode->crtc_hsync_start);
    VIA_WRITE_MASK(IGA1_PIX_H_SYNC_ST_REG, reg_value, IGA1_PIX_H_SYNC_ST_MASK);
    drm_dbg_kms(dev, "  IGA1_PIX_H_SYNC_ST_REG: 0x%08X\n", VIA_READ(IGA1_PIX_H_SYNC_ST_REG));

    reg_value = IGA1_PIXELTIMING_HOR_SYNC_END_FORMULA(mode->crtc_hsync_end) << 16;
    VIA_WRITE_MASK(IGA1_PIX_H_SYNC_END_REG, reg_value, IGA1_PIX_H_SYNC_END_MASK);
    drm_dbg_kms(dev, "  IGA1_PIX_H_SYNC_END_REG: 0x%08X\n", VIA_READ(IGA1_PIX_H_SYNC_END_REG));

    reg_value = IGA1_PIXELTIMING_VER_TOTAL_FORMULA(mode->crtc_vtotal);
    VIA_WRITE_MASK(IGA1_PIX_V_TOTAL_REG, reg_value, IGA1_PIX_V_TOTAL_MASK);
    drm_dbg_kms(dev, "  IGA1_PIX_V_TOTAL_REG: 0x%08X\n", VIA_READ(IGA1_PIX_V_TOTAL_REG));

    reg_value = IGA1_PIXELTIMING_VER_ADDR_FORMULA(mode->crtc_vdisplay) << 16;
    VIA_WRITE_MASK(IGA1_PIX_V_ADDR_REG, reg_value, IGA1_PIX_V_ADDR_MASK);
    drm_dbg_kms(dev, "  IGA1_PIX_V_ADDR_REG: 0x%08X\n", VIA_READ(IGA1_PIX_V_ADDR_REG));

    reg_value = IGA1_PIXELTIMING_VER_BLANK_START_FORMULA(
			        mode->crtc_vblank_start);
    VIA_WRITE_MASK(IGA1_PIX_V_BNK_ST_REG, reg_value, IGA1_PIX_V_BNK_ST_MASK);
    drm_dbg_kms(dev, "  IGA1_PIX_V_BNK_ST_REG: 0x%08X\n", VIA_READ(IGA1_PIX_V_BNK_ST_REG));

    reg_value = IGA1_PIXELTIMING_VER_BLANK_END_FORMULA(mode->crtc_vblank_end) << 16;
    VIA_WRITE_MASK(IGA1_PIX_V_BNK_END_REG, reg_value, IGA1_PIX_V_BNK_END_MASK);
    drm_dbg_kms(dev, "  IGA1_PIX_V_BNK_END_REG: 0x%08X\n", VIA_READ(IGA1_PIX_V_BNK_END_REG));

    reg_value = IGA1_PIXELTIMING_VER_SYNC_START_FORMULA(mode->crtc_vsync_start);
    VIA_WRITE_MASK(IGA1_PIX_V_SYNC_ST_REG, reg_value, IGA1_PIX_V_SYNC_ST_MASK);
    drm_dbg_kms(dev, "  IGA1_PIX_V_SYNC_ST_REG: 0x%08X\n", VIA_READ(IGA1_PIX_V_SYNC_ST_REG));

    reg_value = IGA1_PIXELTIMING_VER_SYNC_END_FORMULA(mode->crtc_vsync_end) << 12;
    VIA_WRITE_MASK(IGA1_PIX_V_SYNC_END_REG, reg_value, IGA1_PIX_V_SYNC_END_MASK);
    drm_dbg_kms(dev, "  IGA1_PIX_V_SYNC_END_REG: 0x%08X\n", VIA_READ(IGA1_PIX_V_SYNC_END_REG));

    /* Removed interlace and half-line adjustments for now */

	svga_wcrt_mask(VGABASE, 0xFD, BIT(5), BIT(5));
    drm_dbg_kms(dev, "  CRFD after setting bit 5: 0x%02X\n", vga_rcrt(VGABASE, 0xFD));
    drm_dbg_kms(dev, "Exiting %s.\n", __func__);
}


/* Load CRTC timing registers */
static void via_load_crtc_timing(struct via_crtc *iga, struct drm_display_mode *mode)
{
    struct drm_device *dev = iga->base.dev;
    struct pci_dev *pdev = to_pci_dev(dev->dev);
    struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
    //u32 reg_value = 0; //Removed as unused.

    drm_dbg_kms(dev, "Entered %s.\n", __func__);

	if (!iga->index) {
		if (pdev->device == PCI_DEVICE_ID_VIA_CHROME9_HD) {
			/* Disable IGA1 shadow timing */
			svga_wcrt_mask(VGABASE, 0x45, 0x00, BIT(0));

			/* Disable IGA1 pixel timing */
			svga_wcrt_mask(VGABASE, 0xFD, 0x00, BIT(5));
		}
        /* IGA1 Horizontal Total */
		u16 htotal = IGA1_HOR_TOTAL_FORMULA(mode->crtc_htotal);
        svga_wcrt_mask(VGABASE, 0x00, htotal & 0xFF, 0xFF);
        svga_wcrt_mask(VGABASE, 0x36, (htotal >> 8) & 0x08, 0x08);
        drm_dbg_kms(dev, "  IGA1 CR00 (HTotal): 0x%02X\n", vga_rcrt(VGABASE, 0x00));
        drm_dbg_kms(dev, "  IGA1 CR36 (HTotal High): 0x%02X\n", vga_rcrt(VGABASE, 0x36));

		/* IGA1 Horizontal Display End */
        u16 hdisplay = IGA1_HOR_ADDR_FORMULA(mode->crtc_hdisplay);
        svga_wcrt_mask(VGABASE, 0x01, hdisplay & 0xFF, 0xFF);
        svga_wcrt_mask(VGABASE, 0x45, (hdisplay >> 7) & 0x02, 0x02);
        drm_dbg_kms(dev, "  IGA1 CR01 (HDisplay): 0x%02X\n", vga_rcrt(VGABASE, 0x01));
        drm_dbg_kms(dev, "  IGA1 CR45 (HDisplay High): 0x%02X\n", vga_rcrt(VGABASE, 0x45));


		/* IGA1 Horizontal Blank Start */
        u16 hblank_start = IGA1_HOR_BLANK_START_FORMULA(mode->crtc_hblank_start);
        svga_wcrt_mask(VGABASE, 0x02, hblank_start & 0xFF, 0xFF);
        svga_wcrt_mask(VGABASE, 0x45, (hblank_start >> 6) & 0x04, 0x04);
        drm_dbg_kms(dev, "  IGA1 CR02 (HBlank Start): 0x%02X\n", vga_rcrt(VGABASE, 0x02));
        drm_dbg_kms(dev, "  IGA1 CR45 (HBlank Start High): 0x%02X\n", vga_rcrt(VGABASE, 0x45));

		/* IGA1 Horizontal Blank End */
        u8 hblank_end = IGA1_HOR_BLANK_END_FORMULA(mode->crtc_hblank_end);
        svga_wcrt_mask(VGABASE, 0x03, hblank_end & 0x1F, 0x1F);
        svga_wcrt_mask(VGABASE, 0x05, (hblank_end & 0x20) << 2, 0x80);
        svga_wcrt_mask(VGABASE, 0x33, (hblank_end & 0x40) << 1, 0x20);
        drm_dbg_kms(dev, "  IGA1 CR03 (HBlank End): 0x%02X\n", vga_rcrt(VGABASE, 0x03));
        drm_dbg_kms(dev, "  IGA1 CR05 (HBlank End High): 0x%02X\n", vga_rcrt(VGABASE, 0x05));
        drm_dbg_kms(dev, "  IGA1 CR33 (HBlank End High): 0x%02X\n", vga_rcrt(VGABASE, 0x33));


		/* IGA1 Horizontal Sync Start */
        u16 hsync_start = IGA1_HOR_SYNC_START_FORMULA(mode->crtc_hsync_start);
        svga_wcrt_mask(VGABASE, 0x04, hsync_start & 0xFF, 0xFF);
        svga_wcrt_mask(VGABASE, 0x33, (hsync_start >> 6) & 0x10, 0x10);
        drm_dbg_kms(dev, "  IGA1 CR04 (HSync Start): 0x%02X\n", vga_rcrt(VGABASE, 0x04));
        drm_dbg_kms(dev, "  IGA1 CR33 (HSync Start High): 0x%02X\n", vga_rcrt(VGABASE, 0x33));

		/* IGA1 Horizontal Sync End */
        u8 hsync_end = IGA1_HOR_SYNC_END_FORMULA(mode->crtc_hsync_end);
        svga_wcrt_mask(VGABASE, 0x05, hsync_end & 0x1F, 0x1F);
        drm_dbg_kms(dev, "  IGA1 CR05 (HSync End): 0x%02X\n", vga_rcrt(VGABASE, 0x05));


		/* IGA1 Vertical Total */
        u16 vtotal = IGA1_VER_TOTAL_FORMULA(mode->crtc_vtotal);
        svga_wcrt_mask(VGABASE, 0x06, vtotal & 0xFF, 0xFF);
        svga_wcrt_mask(VGABASE, 0x07, vtotal & 0x0100 ? 0x01 : 0x00, 0x01);
        svga_wcrt_mask(VGABASE, 0x07, vtotal & 0x0200 ? 0x20 : 0x00, 0x20);
        svga_wcrt_mask(VGABASE, 0x35, vtotal & 0x0400 ? 0x01 : 0x00, 0x01);
        drm_dbg_kms(dev, "  IGA1 CR06 (VTotal): 0x%02X\n", vga_rcrt(VGABASE, 0x06));
        drm_dbg_kms(dev, "  IGA1 CR07 (VTotal High): 0x%02X\n", vga_rcrt(VGABASE, 0x07));
        drm_dbg_kms(dev, "  IGA1 CR35 (VTotal High): 0x%02X\n", vga_rcrt(VGABASE, 0x35));


		/* IGA1 Vertical Display End */
        u16 vdisplay = IGA1_VER_ADDR_FORMULA(mode->crtc_vdisplay);
        svga_wcrt_mask(VGABASE, 0x12, vdisplay & 0xFF, 0xFF);
        svga_wcrt_mask(VGABASE, 0x07, vdisplay & 0x0100 ? 0x02 : 0x00, 0x02);
        svga_wcrt_mask(VGABASE, 0x07, vdisplay & 0x0200 ? 0x40 : 0x00, 0x40);
        svga_wcrt_mask(VGABASE, 0x35, vdisplay & 0x0400 ? 0x04 : 0x00, 0x04);
        drm_dbg_kms(dev, "  IGA1 CR12 (VDisplay): 0x%02X\n", vga_rcrt(VGABASE, 0x12));
        drm_dbg_kms(dev, "  IGA1 CR07 (VDisplay High): 0x%02X\n", vga_rcrt(VGABASE, 0x07));
        drm_dbg_kms(dev, "  IGA1 CR35 (VDisplay High): 0x%02X\n", vga_rcrt(VGABASE, 0x35));

		/* IGA1 Vertical Blank Start */
        u16 vblank_start = IGA1_VER_BLANK_START_FORMULA(mode->crtc_vblank_start);
        svga_wcrt_mask(VGABASE, 0x15, vblank_start & 0xFF, 0xFF);
        svga_wcrt_mask(VGABASE, 0x07, vblank_start & 0x0100 ? 0x08 : 0x00, 0x08);
        svga_wcrt_mask(VGABASE, 0x09, vblank_start & 0x0200 ? 0x20 : 0x00, 0x20);
        svga_wcrt_mask(VGABASE, 0x35, vblank_start & 0x0400 ? 0x08: 0x00, 0x08);
        drm_dbg_kms(dev, "  IGA1 CR15 (VBlank Start): 0x%02X\n", vga_rcrt(VGABASE, 0x15));
        drm_dbg_kms(dev, "  IGA1 CR07 (VBlank Start High): 0x%02X\n", vga_rcrt(VGABASE, 0x07));
        drm_dbg_kms(dev, "  IGA1 CR09 (VBlank Start High): 0x%02X\n", vga_rcrt(VGABASE, 0x09));
        drm_dbg_kms(dev, "  IGA1 CR35 (VBlank Start High): 0x%02X\n", vga_rcrt(VGABASE, 0x35));

		/* IGA1 Vertical Blank End */
        u8 vblank_end = IGA1_VER_BLANK_END_FORMULA(mode->crtc_vblank_end);
        svga_wcrt_mask(VGABASE, 0x16, vblank_end & 0xFF, 0xFF);
        drm_dbg_kms(dev, "  IGA1 CR16 (VBlank End): 0x%02X\n", vga_rcrt(VGABASE, 0x16));

		/* IGA1 Vertical Sync Start */
        u16 vsync_start = IGA1_VER_SYNC_START_FORMULA(mode->crtc_vsync_start);
        svga_wcrt_mask(VGABASE, 0x10, vsync_start & 0xFF, 0xFF);
        svga_wcrt_mask(VGABASE, 0x07, vsync_start & 0x0100 ? 0x04 : 0x00, 0x04);
        svga_wcrt_mask(VGABASE, 0x07, vsync_start & 0x0200 ? 0x80 : 0x00, 0x80);
        svga_wcrt_mask(VGABASE, 0x35, vsync_start & 0x0400 ? 0x02 : 0x00, 0x02);
        drm_dbg_kms(dev, "  IGA1 CR10 (VSync Start): 0x%02X\n", vga_rcrt(VGABASE, 0x10));
        drm_dbg_kms(dev, "  IGA1 CR07 (VSync Start High): 0x%02X\n", vga_rcrt(VGABASE, 0x07));
        drm_dbg_kms(dev, "  IGA1 CR35 (VSync Start High): 0x%02X\n", vga_rcrt(VGABASE, 0x35));


		/* IGA1 Vertical Sync End */
        u8 vsync_end = IGA1_VER_SYNC_END_FORMULA(mode->crtc_vsync_end);
        svga_wcrt_mask(VGABASE, 0x11, vsync_end & 0x0F, 0x0F);
        drm_dbg_kms(dev, "  IGA1 CR11 (VSync End): 0x%02X\n", vga_rcrt(VGABASE, 0x11));

} else {
	/* IGA2 Horizontal Total */
	u16 htotal = IGA2_HOR_TOTAL_FORMULA(mode->crtc_htotal);
    svga_wcrt_mask(VGABASE, 0x50, (u8)(htotal & 0xFF), 0xFF); //CR50
    drm_dbg_kms(dev, "  IGA2 CR50 (H Total) set to: 0x%02X\n", vga_rcrt(VGABASE, 0x50));
    svga_wcrt_mask(VGABASE, 0x55, (u8)((htotal >> 8) & 0x0F), 0x0F); //CR55 Bits 0-3
    drm_dbg_kms(dev, "  IGA2 CR55 (H Total bits) set to: 0x%02X\n", vga_rcrt(VGABASE, 0x55));

	/* IGA2 Horizontal Display End */
	u16 hdisplay = IGA2_HOR_ADDR_FORMULA(mode->crtc_hdisplay);
    svga_wcrt_mask(VGABASE, 0x51, (u8)(hdisplay & 0xFF), 0xFF); //CR51
    drm_dbg_kms(dev, "  IGA2 CR51 (H Display End) set to: 0x%02X\n", vga_rcrt(VGABASE, 0x51));
	u8 cr55 = (vga_rcrt(VGABASE, 0x55) & 0x8F);
    svga_wcrt_mask(VGABASE, 0x55, cr55 | (((u8)(hdisplay >> 8)) & 0x07) << 4, 0x70); //CR55 bit 4,5,6,
    drm_dbg_kms(dev, "  IGA2 CR55 (H Display End) set to: 0x%02X\n", vga_rcrt(VGABASE, 0x55));


	/* IGA2 Horizontal Blank Start */
	u16 hblank_start = IGA2_HOR_BLANK_START_FORMULA(mode->crtc_hblank_start);
    svga_wcrt_mask(VGABASE, 0x52, hblank_start & 0xFF, 0xFF);
    drm_dbg_kms(dev, "  IGA2 CR52 (H Blank Start) set to: 0x%02X\n", vga_rcrt(VGABASE, 0x52));
    svga_wcrt_mask(VGABASE, 0x54, (hblank_start >> 8) & 0x07, 0x07);
    drm_dbg_kms(dev, "  IGA2 CR54 (H Blank Start) set to: 0x%02X\n", vga_rcrt(VGABASE, 0x54));


	/* IGA2 Horizontal Blank End */
    u8 hblank_end = IGA2_HOR_BLANK_END_FORMULA(mode->crtc_hblank_end);
    svga_wcrt_mask(VGABASE, 0x53, hblank_end & 0xFF, 0xFF);
    drm_dbg_kms(dev, "  IGA2 CR53 (H Blank End) set to: 0x%02X\n", vga_rcrt(VGABASE, 0x53));
    svga_wcrt_mask(VGABASE, 0x54, (hblank_end >> 5) & 0x38, 0x38);
    drm_dbg_kms(dev, "  IGA2 CR54 (H Blank End) set to: 0x%02X\n", vga_rcrt(VGABASE, 0x54));
    svga_wcrt_mask(VGABASE, 0x5D, (hblank_end >> 3) & 0x40, 0x40);
    drm_dbg_kms(dev, "  IGA2 CR5D (H Blank End) set to: 0x%02X\n", vga_rcrt(VGABASE, 0x5D));

	/* IGA2 Horizontal Sync Start */
    u16 hsync_start = IGA2_HOR_SYNC_START_FORMULA(mode->crtc_hsync_start);
    svga_wcrt_mask(VGABASE, 0x56, hsync_start & 0xFF, 0xFF);
    drm_dbg_kms(dev, "  IGA2 CR56 (H Sync Start) set to: 0x%02X\n", vga_rcrt(VGABASE, 0x56));
    svga_wcrt_mask(VGABASE, 0x54, (hsync_start >> 2) & 0xC0, 0xC0);
    drm_dbg_kms(dev, "  IGA2 CR54 (H Sync Start) set to: 0x%02X\n", vga_rcrt(VGABASE, 0x54));
    svga_wcrt_mask(VGABASE, 0x5C, (hsync_start >> 8) & 0x80, 0x80);
    drm_dbg_kms(dev, "  IGA2 CR5C (H Sync Start) set to: 0x%02X\n", vga_rcrt(VGABASE, 0x5C));
	svga_wcrt_mask(VGABASE, 0x5D, (hsync_start >> 1) & 0x80, 0x80);
    drm_dbg_kms(dev, "  IGA2 CR5D (H Sync Start) set to: 0x%02X\n", vga_rcrt(VGABASE, 0x5D));


	/* IGA2 Horizontal Sync End */
    u8 hsync_end = IGA2_HOR_SYNC_END_FORMULA(mode->crtc_hsync_end);
    svga_wcrt_mask(VGABASE, 0x57, hsync_end & 0xFF, 0xFF);
    drm_dbg_kms(dev, "  IGA2 CR57 (H Sync End) set to: 0x%02X\n", vga_rcrt(VGABASE, 0x57));
    svga_wcrt_mask(VGABASE, 0x5C, (hsync_end >> 2) & 0x40, 0x40);
    drm_dbg_kms(dev, "  IGA2 CR5C (H Sync End) set to: 0x%02X\n", vga_rcrt(VGABASE, 0x5C));


	/* IGA2 Vertical Total */
    u16 vtotal = IGA2_VER_TOTAL_FORMULA(mode->crtc_vtotal);
    svga_wcrt_mask(VGABASE, 0x58, vtotal & 0xFF, 0xFF);
    drm_dbg_kms(dev, "  IGA2 CR58 (V Total) set to: 0x%02X\n", vga_rcrt(VGABASE, 0x58));
    svga_wcrt_mask(VGABASE, 0x5D, (vtotal >> 8) & 0x07, 0x07);
    drm_dbg_kms(dev, "  IGA2 CR5D (V Total) set to: 0x%02X\n", vga_rcrt(VGABASE, 0x5D));

	/* IGA2 Vertical Display End */
    u16 vdisplay = IGA2_VER_ADDR_FORMULA(mode->crtc_vdisplay);
    svga_wcrt_mask(VGABASE, 0x59, vdisplay & 0xFF, 0xFF);
    drm_dbg_kms(dev, "  IGA2 CR59 (V Display End) set to: 0x%02X\n", vga_rcrt(VGABASE, 0x59));
    svga_wcrt_mask(VGABASE, 0x5D, (vdisplay >> 5) & 0x38, 0x38);
    drm_dbg_kms(dev, "  IGA2 CR5D (V Display End) set to: 0x%02X\n", vga_rcrt(VGABASE, 0x5D));

	/* IGA2 Vertical Blank Start */
    u16 vblank_start = IGA2_VER_BLANK_START_FORMULA(mode->crtc_vblank_start);
    svga_wcrt_mask(VGABASE, 0x5A, vblank_start & 0xFF, 0xFF);
    drm_dbg_kms(dev, "  IGA2 CR5A (V Blank Start) set to: 0x%02X\n", vga_rcrt(VGABASE, 0x5A));
    svga_wcrt_mask(VGABASE, 0x5C, (vblank_start >> 8) & 0x07, 0x07);
    drm_dbg_kms(dev, "  IGA2 CR5C (V Blank Start) set to: 0x%02X\n", vga_rcrt(VGABASE, 0x5C));

	/* IGA2 Vertical Blank End */
    u8 vblank_end = IGA2_VER_BLANK_END_FORMULA(mode->crtc_vblank_end);
    svga_wcrt_mask(VGABASE, 0x5B, vblank_end & 0xFF, 0xFF);
    drm_dbg_kms(dev, "  IGA2 CR5B (V Blank End) set to: 0x%02X\n", vga_rcrt(VGABASE, 0x5B));
    svga_wcrt_mask(VGABASE, 0x5C, (vblank_end >> 3) & 0x38, 0x38);
    drm_dbg_kms(dev, "  IGA2 CR5C (V Blank End) set to: 0x%02X\n", vga_rcrt(VGABASE, 0x5C));


	/* IGA2 Vertical Sync Start */
    u16 vsync_start = IGA2_VER_SYNC_START_FORMULA(mode->crtc_vsync_start);
    svga_wcrt_mask(VGABASE, 0x5E, vsync_start & 0xFF, 0xFF);
    drm_dbg_kms(dev, "  IGA2 CR5E (V Sync Start) set to: 0x%02X\n", vga_rcrt(VGABASE, 0x5E));
    svga_wcrt_mask(VGABASE, 0x5F, (vsync_start >> 5) & 0xE0, 0xE0);
    drm_dbg_kms(dev, "  IGA2 CR5F (V Sync Start) set to: 0x%02X\n", vga_rcrt(VGABASE, 0x5F));

	/* IGA2 Vertical Sync End */
    u8 vsync_end = IGA2_VER_SYNC_END_FORMULA(mode->crtc_vsync_end);
    svga_wcrt_mask(VGABASE, 0x5F, vsync_end & 0x1F, 0x1F);
    drm_dbg_kms(dev, "  IGA2 CR5F (V Sync End) set to: 0x%02X\n", vga_rcrt(VGABASE, 0x5F));
	}
    drm_dbg_kms(dev, "Exiting %s.\n", __func__);
}

static void via_mode_set_nofb(struct drm_crtc *crtc)
{
	struct drm_device *dev = crtc->dev;
	struct pci_dev *pdev = to_pci_dev(dev->dev);
	struct drm_display_mode *mode = &crtc->state->mode;
	struct drm_display_mode *adjusted_mode =
					&crtc->state->adjusted_mode;
	struct via_crtc *iga = container_of(crtc,
						struct via_crtc, base);
	struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
	u8 reg_value = 0;
	int ret;

	drm_dbg_kms(dev, "Entered %s.\n", __func__);

    drm_dbg_kms(dev, "Mode: %dx%d@%dHz\n",
        mode->hdisplay, mode->vdisplay, drm_mode_vrefresh(mode));


	/* Load standard registers */
	via_load_vpit_regs(dev_priv);

	/*
	 * For VX855 and VX900 chipsets, CRTC unlock register is
	 * CR47[4].  For all others, CR47[0].
	 */
	if ((pdev->device == PCI_DEVICE_ID_VIA_CHROME9_HCM) ||
		(pdev->device == PCI_DEVICE_ID_VIA_CHROME9_HD)) {
		reg_value = BIT(4);
	} else {
		reg_value = BIT(0);
	}

	/* Unlock CRTC registers. */
	svga_wcrt_mask(VGABASE, 0x11, 0x00, BIT(7));
	svga_wcrt_mask(VGABASE, 0x47, 0x00, reg_value);

	if (!iga->index) {
		/* IGA1 reset */
		vga_wcrt(VGABASE, 0x09, 0x00); /* initial CR09=0 */
		svga_wcrt_mask(VGABASE, 0x11, 0x00, BIT(6));

        /* disable IGA scales first */
		//via_disable_iga_scaling(crtc);

		/*
		 * when not down scaling, we only need load one
		 * timing.
		 */
		via_load_crtc_timing(iga, adjusted_mode);

        /* Horizontal Total Ending */
		switch (adjusted_mode->crtc_htotal % 8) {
		case 0:
		default:
			break;
		case 2:
			reg_value = BIT(7);
			break;
		case 4:
			reg_value = BIT(6);
			break;
		case 6:
			reg_value = BIT(3);
			break;
		}

		svga_wcrt_mask(VGABASE, 0x47,
				reg_value, BIT(7) | BIT(6) | BIT(3));
	} else {
		/* disable IGA scales first */
		//via_disable_iga_scaling(crtc);

		/* Load crtc timing and IGA scaling */
		if (iga->scaling_mode & VIA_SHRINK) {

            /*
             * IGA2 does not have a shadow timing, because it's
             * a newer IGP that has pixel-level timing registers
             * that can be used by both scaling engines.
             */
            via_load_crtc_pixel_timing(crtc,
                adjusted_mode);

		} else {
			/*
			 * when not down scaling, we only need load
			 * one timing.
			 */
			via_load_crtc_timing(iga, adjusted_mode);

			/* II. up scaling */
			if (iga->scaling_mode & VIA_EXPAND) {
		
			}
		}
	}

	if (!iga->index) {
		/* Set non-interlace / interlace mode. */
		//via_iga1_set_interlace_mode(VGABASE,
		//			adjusted_mode->flags &
		//			DRM_MODE_FLAG_INTERLACE);

		/* No HSYNC shift. */
		//via_iga1_set_hsync_shift(VGABASE, 0x05);

		/* Load display FIFO. */
		ret = via_iga1_display_fifo_regs(dev, iga, adjusted_mode,
							crtc->primary->fb);
		if (ret) {
			goto exit;
		}

		/* Set PLL */
		if (adjusted_mode->clock) {
			u32 clock = adjusted_mode->clock * 1000;
			u32 pll_regs;

			if (iga->scaling_mode & VIA_SHRINK)
				clock *= 2;
			pll_regs = via_get_clk_value(crtc->dev, clock);
			via_set_vclock(crtc, pll_regs);
		}

		via_iga_common_init(dev);

		/* Set palette LUT to 8-bit mode. */
		via_iga1_set_palette_lut_resolution(VGABASE, true);
	} else {
		/* Set non-interlace / interlace mode. */
		//via_iga2_set_interlace_mode(VGABASE,
		//			adjusted_mode->flags &
		//			DRM_MODE_FLAG_INTERLACE);

		/* Load display FIFO. */
		ret = via_iga2_display_fifo_regs(dev, iga, adjusted_mode,
							crtc->primary->fb);
		if (ret) {
			goto exit;
		}

		/* Set PLL */
		if (adjusted_mode->clock) {
			u32 clock = adjusted_mode->clock * 1000;
			u32 pll_regs;

			if (iga->scaling_mode & VIA_SHRINK)
				clock *= 2;
			pll_regs = via_get_clk_value(crtc->dev, clock);
			via_set_vclock(crtc, pll_regs);
		}

		via_iga_common_init(dev);

		/* Set palette LUT to 8-bit mode. */
		via_iga2_set_palette_lut_resolution(VGABASE, true);

		svga_wcrt_mask(VGABASE, 0x6A, BIT(7), BIT(7));
	}
exit:
	drm_dbg_kms(dev, "Exiting %s.\n", __func__);
}

static void via_crtc_helper_atomic_enable(struct drm_crtc *crtc,
					struct drm_atomic_state *state)
{
    struct drm_device *dev = crtc->dev;
	struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
	struct via_crtc *iga = container_of(crtc,
						struct via_crtc, base);

    drm_dbg_kms(dev, "Entered %s.\n", __func__);
    via_dump_crtc_info(crtc, "Before via_mode_set_nofb"); // Add this
    via_mode_set_nofb(crtc);
    	if (!iga->index) {
    	svga_wseq_mask(VGABASE, 0x01, 0x00, BIT(5));
    	} else {
    	svga_wcrt_mask(VGABASE, 0x6B, 0x00, BIT(2));
    	}
    via_dump_crtc_info(crtc, "After via_mode_set_nofb"); // Add this

    	drm_dbg_kms(dev, "Exiting %s.\n", __func__);

}


static void via_crtc_helper_atomic_disable(struct drm_crtc *crtc,
					struct drm_atomic_state *state)
{
	via_dump_crtc_info(crtc, "Before Disable"); // Add this
	struct drm_device *dev = crtc->dev;
	struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
	struct via_crtc *iga = container_of(crtc,
						struct via_crtc, base);

	drm_dbg_kms(dev, "Entered %s.\n", __func__);

	if (!iga->index) {
		svga_wseq_mask(VGABASE, 0x01, BIT(5), BIT(5));
	} else {
		svga_wcrt_mask(VGABASE, 0x6B, BIT(2), BIT(2));
	}
    via_dump_crtc_info(crtc, "After Disable"); // Add this

	drm_dbg_kms(dev, "Exiting %s.\n", __func__);
}


/*
 * Scaling functions.  Temporarily disabled for initial debugging.
 */

static void via_set_scale_path(struct drm_crtc *crtc, u32 scale_type)
{
    return;
}

static void via_disable_iga_scaling(struct drm_crtc *crtc)
{
    return;
}

static bool via_set_iga_scale_function(struct drm_crtc *crtc, u32 scale_type)
{
    return false;
}


static bool via_load_iga_scale_factor_regs(struct via_drm_priv *dev_priv,
					struct drm_display_mode *mode,
					struct drm_display_mode *adjusted_mode,
					u32 scale_type, u32 is_hor_or_ver)
{
    return false;
}


static void via_set_iga2_downscale_source_timing(struct drm_crtc *crtc,
				struct drm_display_mode *mode,
				struct drm_display_mode *adjusted_mode)
{
	return;
}

static int via_primary_atomic_check(struct drm_plane *plane,
					struct drm_atomic_state *state)
{
	struct drm_plane_state *new_plane_state =
			drm_atomic_get_new_plane_state(state, plane);
	struct drm_crtc_state *new_crtc_state;
	struct drm_device *dev = plane->dev;
	struct drm_framebuffer *fb = new_plane_state->fb;
	struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
	uint32_t frame_buffer_size;
	int ret = 0;

	drm_dbg_kms(dev, "Entered %s.\n", __func__);

	if ((!new_plane_state->crtc) || (!new_plane_state->visible)) {
		goto exit;
	}

	frame_buffer_size = (fb->width * fb->format->cpp[0]) *
				fb->height;
	if (frame_buffer_size > dev_priv->vram_size) {
		ret = -ENOMEM;
		goto exit;
	}

	if ((fb->width > dev->mode_config.max_width) ||
		(fb->width < dev->mode_config.min_width)) {
		ret = -EINVAL;
		goto exit;
	}

	new_crtc_state = drm_atomic_get_new_crtc_state(state,
						new_plane_state->crtc);
	ret = drm_atomic_helper_check_plane_state(
					new_plane_state,
					new_crtc_state,
					DRM_PLANE_NO_SCALING,
					DRM_PLANE_NO_SCALING,
					false, true);
exit:
	drm_dbg_kms(dev, "Exiting %s.\n", __func__);
	return ret;
}

static void via_primary_atomic_disable(struct drm_plane *plane,
					struct drm_atomic_state *state)
{
	struct drm_device *dev = plane->dev;

	drm_dbg_kms(dev, "Entered %s.\n", __func__);

	drm_dbg_kms(dev, "Exiting %s.\n", __func__);
	return;
}

static void via_primary_atomic_update(struct drm_plane *plane,
					struct drm_atomic_state *state)
{
	struct drm_plane_state *new_state =
			drm_atomic_get_new_plane_state(state, plane);
	struct drm_crtc *crtc = new_state->crtc;
	struct drm_framebuffer *fb = new_state->fb;
	uint32_t pitch = (new_state->crtc_y * fb->pitches[0]) +
			(new_state->crtc_x * fb->format->cpp[0]);
	uint32_t addr;
	struct via_crtc *iga = container_of(crtc, struct via_crtc, base);
	struct drm_device *dev = crtc->dev;
	struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
	struct drm_gem_object *gem;
	struct ttm_buffer_object *ttm_bo;
	struct via_bo *bo;

	drm_dbg_kms(dev, "Entered %s.\n", __func__);

	gem = fb->obj[0];
	ttm_bo = container_of(gem, struct ttm_buffer_object, base);
	bo = to_ttm_bo(ttm_bo);

	if (!iga->index) {
		via_iga1_set_color_depth(dev,
						fb->format->cpp[0],
						fb->format->depth);

		/* Set the framebuffer offset */
		addr = round_up((ttm_bo->resource->start << PAGE_SHIFT) +
				pitch, 16) >> 1;

        /* IGA1 */
		vga_wcrt(VGABASE, 0x0D, addr & 0xFF);
        drm_dbg_kms(dev, "  CR0D: 0x%02X\n", vga_rcrt(VGABASE, 0x0D));
		vga_wcrt(VGABASE, 0x0C, (addr >> 8) & 0xFF);
        drm_dbg_kms(dev, "  CR0C: 0x%02X\n", vga_rcrt(VGABASE, 0x0C));
		/* Yes order of setting these registers matters on some hardware */
		svga_wcrt_mask(VGABASE, 0x48, ((addr >> 24) & 0x1F), 0x1F);
        drm_dbg_kms(dev, "  CR48: 0x%02X\n", vga_rcrt(VGABASE, 0x48));
		vga_wcrt(VGABASE, 0x34, (addr >> 16) & 0xFF);
        drm_dbg_kms(dev, "  CR34: 0x%02X\n", vga_rcrt(VGABASE, 0x34));


		/* Load fetch count registers */
		pitch = ALIGN(crtc->mode.hdisplay * fb->format->cpp[0],	16);
		//load_value_to_registers(VGABASE, &iga->fetch, pitch >> 4);
        svga_wseq_mask(VGABASE, 0x1C, (pitch >> 4) & 0xFF, 0xFF);
        svga_wseq_mask(VGABASE, 0x1D, (pitch >> 12) & 0x03, 0x03);
        drm_dbg_kms(dev, "  SR1C: 0x%02X\n", vga_rseq(VGABASE, 0x1C));
        drm_dbg_kms(dev, "  SR1D: 0x%02X\n", vga_rseq(VGABASE, 0x1D));
		/* Set the primary pitch */
		pitch = ALIGN(fb->pitches[0], 16);
		/* Spec does not say that first adapter skips 3 bits but old
		 * code did it and seems to be reasonable in analogy to
		 * second adapter */
		//load_value_to_registers(VGABASE, &iga->offset, pitch >> 3);
        svga_wcrt_mask(VGABASE, 0x13, (pitch >> 3) & 0xFF, 0xFF);
        svga_wcrt_mask(VGABASE, 0x35, (pitch >> 11) & 0x07, 0x07);
        drm_dbg_kms(dev, "  CR13: 0x%02X\n", vga_rcrt(VGABASE, 0x13));
        drm_dbg_kms(dev, "  CR35: 0x%02X\n", vga_rcrt(VGABASE, 0x35));
	} else {
		via_iga2_set_color_depth(dev,
						fb->format->cpp[0],
						fb->format->depth);

		/* Set the framebuffer offset */
		addr = round_up((ttm_bo->resource->start << PAGE_SHIFT) +
				pitch, 16);
		/* Bits 9 to 3 of the frame buffer go into bits 7 to 1
		 * of the register. Bit 0 is for setting tile mode or
		 * linear mode. A value of zero sets it to linear mode */
		vga_wcrt(VGABASE, 0x62, ((addr >> 3) & 0x7F) << 1);
		vga_wcrt(VGABASE, 0x63, (addr >> 10) & 0xFF);
		vga_wcrt(VGABASE, 0x64, (addr >> 18) & 0xFF);
		svga_wcrt_mask(VGABASE, 0xA3, ((addr >> 26) & 0x07), 0x07);
        drm_dbg_kms(dev, "  CR62: 0x%02X\n", vga_rcrt(VGABASE, 0x62));
        drm_dbg_kms(dev, "  CR63: 0x%02X\n", vga_rcrt(VGABASE, 0x63));
        drm_dbg_kms(dev, "  CR64: 0x%02X\n", vga_rcrt(VGABASE, 0x64));
        drm_dbg_kms(dev, "  CRA3: 0x%02X\n", vga_rcrt(VGABASE, 0xA3));

		/* Load fetch count registers */
		pitch = ALIGN(crtc->mode.hdisplay * fb->format->cpp[0], 16);
		//load_value_to_registers(VGABASE, &iga->fetch, pitch >> 4);
        svga_wcrt_mask(VGABASE, 0x65, (pitch >> 4) & 0xFF, 0xFF);
        svga_wcrt_mask(VGABASE, 0x67, (pitch >> 12) & 0x03, 0x03);
        drm_dbg_kms(dev, "  CR65: 0x%02X\n", vga_rcrt(VGABASE, 0x65));
        drm_dbg_kms(dev, "  CR67: 0x%02X\n", vga_rcrt(VGABASE, 0x67));

		/* Set secondary pitch */
		pitch = ALIGN(fb->pitches[0], 16);
		//load_value_to_registers(VGABASE, &iga->offset, pitch >> 3);
        svga_wcrt_mask(VGABASE, 0x66, (pitch >> 3) & 0xFF, 0xFF);
        svga_wcrt_mask(VGABASE, 0x67, (pitch >> 11) & 0x03, 0x03);
        svga_wcrt_mask(VGABASE, 0x71, (pitch >> 7) & 0x80, 0x80);
        drm_dbg_kms(dev, "  CR66: 0x%02X\n", vga_rcrt(VGABASE, 0x66));
        drm_dbg_kms(dev, "  CR67: 0x%02X\n", vga_rcrt(VGABASE, 0x67));
        drm_dbg_kms(dev, "  CR71: 0x%02X\n", vga_rcrt(VGABASE, 0x71));
	}

	drm_dbg_kms(dev, "Exiting %s.\n", __func__);
}

static int via_primary_prepare_fb(struct drm_plane *plane,
				struct drm_plane_state *new_state)
{
	struct drm_device *dev = plane->dev;
	struct drm_gem_object *gem;
	struct ttm_buffer_object *ttm_bo;
	struct via_bo *bo;
	int ret = 0;

	drm_dbg_kms(dev, "Entered %s.\n", __func__);

	if (!new_state->fb) {
		goto exit;
	}

	gem = new_state->fb->obj[0];
	ttm_bo = container_of(gem, struct ttm_buffer_object, base);
	bo = to_ttm_bo(ttm_bo);

	ret = ttm_bo_reserve(ttm_bo, true, false, NULL);
	if (ret) {
		goto exit;
	}

	ret = via_bo_pin(bo, TTM_PL_VRAM);
	ttm_bo_unreserve(ttm_bo);
exit:
	drm_dbg_kms(dev, "Exiting %s.\n", __func__);
	return ret;
}

static void via_primary_cleanup_fb(struct drm_plane *plane,
				struct drm_plane_state *old_state)
{
	struct drm_device *dev = plane->dev;
	struct drm_gem_object *gem;
	struct ttm_buffer_object *ttm_bo;
	struct via_bo *bo;
	int ret;

	drm_dbg_kms(dev, "Entered %s.\n", __func__);

	if (!old_state->fb) {
		goto exit;
	}

	gem = old_state->fb->obj[0];
	ttm_bo = container_of(gem, struct ttm_buffer_object, base);
	bo = to_ttm_bo(ttm_bo);

	ret = ttm_bo_reserve(ttm_bo, true, false, NULL);
	if (ret) {
		goto exit;
	}

	via_bo_unpin(bo);
	ttm_bo_unreserve(ttm_bo);
exit:
	drm_dbg_kms(dev, "Exiting %s.\n", __func__);
}

static const struct drm_plane_helper_funcs
via_primary_drm_plane_helper_funcs = {
	.prepare_fb = via_primary_prepare_fb,
	.cleanup_fb = via_primary_cleanup_fb,
	.atomic_check = via_primary_atomic_check,
	.atomic_update = via_primary_atomic_update,
	.atomic_disable = via_primary_atomic_disable,
};

static const struct drm_plane_funcs via_primary_drm_plane_funcs = {
	.update_plane	= drm_atomic_helper_update_plane,
	.disable_plane = drm_atomic_helper_disable_plane,
	.destroy = drm_plane_cleanup,
	.reset = drm_atomic_helper_plane_reset,
	.atomic_duplicate_state =
			drm_atomic_helper_plane_duplicate_state,
	.atomic_destroy_state =
			drm_atomic_helper_plane_destroy_state,
};

static void via_crtc_param_init(struct via_drm_priv *dev_priv,
					struct drm_crtc *crtc,
					uint32_t index)
{
	struct drm_device *dev = &dev_priv->dev;
	struct pci_dev *pdev = to_pci_dev(dev->dev);
	struct via_crtc *iga = container_of(crtc,
						struct via_crtc, base);

    drm_dbg_kms(dev, "Entered %s.\n", __func__);

	if (iga->index) {
		iga->timings.htotal.count = ARRAY_SIZE(iga2_hor_total);
		iga->timings.htotal.regs = iga2_hor_total;

		iga->timings.hdisplay.count = ARRAY_SIZE(iga2_hor_addr);
		iga->timings.hdisplay.regs = iga2_hor_addr;
		if (pdev->device != PCI_DEVICE_ID_VIA_CHROME9_HD)
			iga->timings.hdisplay.count--;

		iga->timings.hblank_start.count = ARRAY_SIZE(iga2_hor_blank_start);
		iga->timings.hblank_start.regs = iga2_hor_blank_start;
		if (pdev->device != PCI_DEVICE_ID_VIA_CHROME9_HD)
			iga->timings.hblank_start.count--;

		iga->timings.hblank_end.count = ARRAY_SIZE(iga2_hor_blank_end);
		iga->timings.hblank_end.regs = iga2_hor_blank_end;

		iga->timings.hsync_start.count = ARRAY_SIZE(iga2_hor_sync_start);
		iga->timings.hsync_start.regs = iga2_hor_sync_start;
		if (pdev->device == PCI_DEVICE_ID_VIA_CLE266_GFX
			|| pdev->device == PCI_DEVICE_ID_VIA_KM400_GFX)
			iga->timings.hsync_start.count--;

		iga->timings.hsync_end.count = ARRAY_SIZE(iga2_hor_sync_end);
		iga->timings.hsync_end.regs = iga2_hor_sync_end;

		iga->timings.vtotal.count = ARRAY_SIZE(iga2_ver_total);
		iga->timings.vtotal.regs = iga2_ver_total;

		iga->timings.vdisplay.count = ARRAY_SIZE(iga2_ver_addr);
		iga->timings.vdisplay.regs = iga2_ver_addr;

		iga->timings.vblank_start.count = ARRAY_SIZE(iga2_ver_blank_start);
		iga->timings.vblank_start.regs = iga2_ver_blank_start;

		iga->timings.vblank_end.count = ARRAY_SIZE(iga2_ver_blank_end);
		iga->timings.vblank_end.regs = iga2_ver_blank_end;

		iga->timings.vsync_start.count = ARRAY_SIZE(iga2_ver_sync_start);
		iga->timings.vsync_start.regs = iga2_ver_sync_start;

		iga->timings.vsync_end.count = ARRAY_SIZE(iga2_ver_sync_end);
		iga->timings.vsync_end.regs = iga2_ver_sync_end;

		/* Secondary FIFO setup */
		if ((pdev->device == PCI_DEVICE_ID_VIA_CLE266_GFX) ||
			(pdev->device == PCI_DEVICE_ID_VIA_KM400_GFX)) {
			iga->fifo_depth.count =
				ARRAY_SIZE(iga2_cle266_fifo_depth_select);
			iga->fifo_depth.regs = iga2_cle266_fifo_depth_select;

			iga->threshold.count =
				ARRAY_SIZE(iga2_cle266_fifo_threshold_select);
			iga->threshold.regs = iga2_cle266_fifo_threshold_select;
		} else {
			iga->fifo_depth.count = ARRAY_SIZE(iga2_k8m800_fifo_depth_select);
			iga->fifo_depth.regs = iga2_k8m800_fifo_depth_select;

			iga->threshold.count = ARRAY_SIZE(iga2_k8m800_fifo_threshold_select);
			iga->threshold.regs = iga2_k8m800_fifo_threshold_select;

			iga->high_threshold.count = ARRAY_SIZE(iga2_fifo_high_threshold_select);
			iga->high_threshold.regs = iga2_fifo_high_threshold_select;

			iga->display_queue.count = ARRAY_SIZE(iga2_display_queue_expire_num);
			iga->display_queue.regs = iga2_display_queue_expire_num;
		}

		iga->fetch.count = ARRAY_SIZE(iga2_fetch_count);
		iga->fetch.regs = iga2_fetch_count;

		/* Older hardware only uses 12 bits */
		iga->offset.count = ARRAY_SIZE(iga2_offset) - 1;
		iga->offset.regs = iga2_offset;
	} else {
		iga->timings.htotal.count = ARRAY_SIZE(iga1_hor_total);
		iga->timings.htotal.regs = iga1_hor_total;

		iga->timings.hdisplay.count = ARRAY_SIZE(iga1_hor_addr);
		iga->timings.hdisplay.regs = iga1_hor_addr;
		if (pdev->device != PCI_DEVICE_ID_VIA_CHROME9_HD)
			iga->timings.hdisplay.count--;

		iga->timings.hblank_start.count = ARRAY_SIZE(iga1_hor_blank_start);
		iga->timings.hblank_start.regs = iga1_hor_blank_start;
		if (pdev->device != PCI_DEVICE_ID_VIA_CHROME9_HD)
			iga->timings.hblank_start.count--;

		iga->timings.hblank_end.count = ARRAY_SIZE(iga1_hor_blank_end);
		iga->timings.hblank_end.regs = iga1_hor_blank_end;

		iga->timings.hsync_start.count = ARRAY_SIZE(iga1_hor_sync_start);
		iga->timings.hsync_start.regs = iga1_hor_sync_start;

		iga->timings.hsync_end.count = ARRAY_SIZE(iga1_hor_sync_end);
		iga->timings.hsync_end.regs = iga1_hor_sync_end;

		iga->timings.vtotal.count = ARRAY_SIZE(iga1_ver_total);
		iga->timings.vtotal.regs = iga1_ver_total;

		iga->timings.vdisplay.count = ARRAY_SIZE(iga1_ver_addr);
		iga->timings.vdisplay.regs = iga1_ver_addr;

		iga->timings.vblank_start.count = ARRAY_SIZE(iga1_ver_blank_start);
		iga->timings.vblank_start.regs = iga1_ver_blank_start;

		iga->timings.vblank_end.count = ARRAY_SIZE(iga1_ver_blank_end);
		iga->timings.vblank_end.regs = iga1_ver_blank_end;

		iga->timings.vsync_start.count = ARRAY_SIZE(iga1_ver_sync_start);
		iga->timings.vsync_start.regs = iga1_ver_sync_start;

		iga->timings.vsync_end.count = ARRAY_SIZE(iga1_ver_sync_end);
		iga->timings.vsync_end.regs = iga1_ver_sync_end;

		/* Primary FIFO setup */
		if ((pdev->device == PCI_DEVICE_ID_VIA_CLE266_GFX) ||
			(pdev->device == PCI_DEVICE_ID_VIA_KM400_GFX)) {
			iga->fifo_depth.count = ARRAY_SIZE(iga1_cle266_fifo_depth_select);
			iga->fifo_depth.regs = iga1_cle266_fifo_depth_select;

			iga->threshold.count = ARRAY_SIZE(iga1_cle266_fifo_threshold_select);
			iga->threshold.regs = iga1_cle266_fifo_threshold_select;

			iga->high_threshold.count = ARRAY_SIZE(iga1_cle266_fifo_high_threshold_select);
			iga->high_threshold.regs = iga1_cle266_fifo_high_threshold_select;

			iga->display_queue.count = ARRAY_SIZE(iga1_cle266_display_queue_expire_num);
			iga->display_queue.regs = iga1_cle266_display_queue_expire_num;
		} else {
			iga->fifo_depth.count = ARRAY_SIZE(iga1_k8m800_fifo_depth_select);
			iga->fifo_depth.regs = iga1_k8m800_fifo_depth_select;

			iga->threshold.count = ARRAY_SIZE(iga1_k8m800_fifo_threshold_select);
			iga->threshold.regs = iga1_k8m800_fifo_threshold_select;

			iga->high_threshold.count = ARRAY_SIZE(iga1_k8m800_fifo_high_threshold_select);
			iga->high_threshold.regs = iga1_k8m800_fifo_high_threshold_select;

			iga->display_queue.count = ARRAY_SIZE(iga1_k8m800_display_queue_expire_num);
			iga->display_queue.regs = iga1_k8m800_display_queue_expire_num;
		}

		iga->fetch.count = ARRAY_SIZE(iga1_fetch_count);
		iga->fetch.regs = iga1_fetch_count;

		iga->offset.count = ARRAY_SIZE(iga1_offset);
		iga->offset.regs = iga1_offset;
	}
    drm_dbg_kms(dev, "Exiting %s.\n", __func__);
}

static int via_gamma_init(struct drm_crtc *crtc)
{
	struct drm_device *dev = crtc->dev;
	u16 *gamma;
	uint32_t i;
	int ret;

	drm_dbg_kms(dev, "Entered %s.\n", __func__);

	ret = drm_mode_crtc_set_gamma_size(crtc, 256);
	if (ret) {
		drm_err(dev, "Failed to set gamma size!\n");
		goto exit;
	}

	gamma = crtc->gamma_store;
	for (i = 0; i < 256; i++) {
		gamma[i] = i << 8 | i;
		gamma[i + 256] = i << 8 | i;
		gamma[i + 512] = i << 8 | i;
	}

exit:
	drm_dbg_kms(dev, "Exiting %s.\n", __func__);
	return ret;
}

static const uint32_t via_primary_formats[] = {
	DRM_FORMAT_XRGB8888,
	DRM_FORMAT_RGB565,
	DRM_FORMAT_C8,
};

int via_crtc_init(struct via_drm_priv *dev_priv, uint32_t index)
{
	struct drm_device *dev = &dev_priv->dev;
	struct via_crtc *iga;
	struct drm_plane *primary;
	struct drm_plane *cursor;
	uint32_t possible_crtcs;
	int ret;

    drm_dbg_kms(dev, "Entered %s.\n", __func__);

	possible_crtcs = 1 << index;

	primary = kzalloc(sizeof(struct drm_plane), GFP_KERNEL);
	if (!primary) {
		ret = -ENOMEM;
		drm_err(dev, "Failed to allocate a primary plane.\n");
		goto exit;
	}

	drm_plane_helper_add(primary,
			&via_primary_drm_plane_helper_funcs);
	ret = drm_universal_plane_init(dev, primary, possible_crtcs,
			&via_primary_drm_plane_funcs,
			via_primary_formats,
			ARRAY_SIZE(via_primary_formats),
			NULL, DRM_PLANE_TYPE_PRIMARY, NULL);
	if (ret) {
		drm_err(dev, "Failed to initialize a primary "
				"plane.\n");
		goto free_primary;
	}

	cursor = kzalloc(sizeof(struct drm_plane), GFP_KERNEL);
	if (!cursor) {
		ret = -ENOMEM;
		drm_err(dev, "Failed to allocate a cursor plane.\n");
		goto cleanup_primary;
	}

	drm_plane_helper_add(cursor,
			&via_cursor_drm_plane_helper_funcs);
	ret = drm_universal_plane_init(dev, cursor, possible_crtcs,
			&via_cursor_drm_plane_funcs,
			via_cursor_formats,
			via_cursor_formats_size,
			NULL, DRM_PLANE_TYPE_CURSOR, NULL);
	if (ret) {
		drm_err(dev, "Failed to initialize a cursor "
				"plane.\n");
		goto free_cursor;
	}

	iga = kzalloc(sizeof(struct via_crtc), GFP_KERNEL);
	if (!iga) {
		ret = -ENOMEM;
		drm_err(dev, "Failed to allocate CRTC storage.\n");
		goto cleanup_cursor;
	}

	drm_crtc_helper_add(&iga->base,
			&via_drm_crtc_helper_funcs);
	ret = drm_crtc_init_with_planes(dev, &iga->base,
					primary, cursor,
					&via_drm_crtc_funcs,
					NULL);
	if (ret) {
		drm_err(dev, "Failed to initialize CRTC!\n");
		goto free_crtc;
	}

	iga->index = index;

	via_crtc_param_init(dev_priv, &iga->base, index);
	ret = via_gamma_init(&iga->base);
	if (ret) {
		goto free_crtc;
	}

	goto exit;
free_crtc:
	kfree(iga);
cleanup_cursor:
	drm_plane_cleanup(cursor);
free_cursor:
	kfree(cursor);
cleanup_primary:
	drm_plane_cleanup(primary);
free_primary:
	kfree(primary);
exit:
	drm_dbg_kms(dev, "Exiting %s.\n", __func__);
	return ret;
}