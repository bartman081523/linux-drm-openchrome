/*
 * Copyright 2023 The OpenChrome Project
 * [via_tx.c]
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

#include <linux/delay.h>

#include "via_drv.h"
#include "via_tx_regs.h"

void via_transmitter_io_pad_state(struct drm_device *dev,
                              u32 di_port, bool enable)
{
    struct via_drm_priv *dev_priv = to_via_drm_priv(dev);

    drm_dbg_kms(dev, "Entered %s.\n", __func__);

    switch (di_port) {
        case VIA_DI_PORT_DIP0:
                via_dip0_set_io_pad_state(VGABASE,
                                     enable);
                break;
        case VIA_DI_PORT_DIP1:
                via_dip1_set_io_pad_state(VGABASE,
                                     enable);
                break;
        case VIA_DI_PORT_DVP0:
                via_dvp0_set_io_pad_state(VGABASE,
                                     enable);
                break;
        case VIA_DI_PORT_DVP1:
                via_dvp1_set_io_pad_state(VGABASE,
                                     enable);
                break;
        case VIA_DI_PORT_FPDPLOW:
                via_fpdp_low_set_io_pad_state(VGABASE,
                                         enable);
                break;
        case VIA_DI_PORT_FPDPHIGH:
                via_fpdp_high_set_io_pad_state(VGABASE,
                                          enable);
                break;
        case VIA_DI_PORT_DFPLOW:
        case VIA_DI_PORT_DFPHIGH:
                /* Do nothing */
                break;
        case VIA_DI_PORT_LVDS1:
                via_lvds1_set_io_pad_setting(VGABASE,
                                        enable);
                break;
        case VIA_DI_PORT_LVDS2:
                via_lvds2_set_io_pad_setting(VGABASE,
                                        enable);
                break;
        default:
                break;
    }

    drm_dbg_kms(dev, "Exiting %s.\n", __func__);
}

void via_output_enable(struct drm_device *dev,
                   u32 di_port, bool output_enable)
{
    struct via_drm_priv *dev_priv = to_via_drm_priv(dev);

    drm_dbg_kms(dev, "Entered %s.\n", __func__);

    switch (di_port) {
        case VIA_DI_PORT_DIP0:
                via_dip0_set_output_enable(VGABASE, output_enable);
                break;
        case VIA_DI_PORT_DIP1:
                via_dip1_set_output_enable(VGABASE, output_enable);
                break;
        default:
                break;
    }

    drm_dbg_kms(dev, "Exiting %s.\n", __func__);
}

void via_clock_source(struct drm_device *dev,
                  u32 di_port, bool clock_source)
{
    struct via_drm_priv *dev_priv = to_via_drm_priv(dev);

    drm_dbg_kms(dev, "Entered %s.\n", __func__);

    switch (di_port) {
        case VIA_DI_PORT_DIP0:
                via_dip0_set_clock_source(VGABASE, clock_source);
                break;
        case VIA_DI_PORT_DIP1:
                via_dip1_set_clock_source(VGABASE, clock_source);
                break;
        default:
                break;
    }

    drm_dbg_kms(dev, "Exiting %s.\n", __func__);
}

void via_transmitter_clock_drive_strength(struct drm_device *dev,
                                      u32 di_port,
                                      u8 clock_drive_strength)
{
    struct via_drm_priv *dev_priv = to_via_drm_priv(dev);

    drm_dbg_kms(dev, "Entered %s.\n", __func__);

    switch (di_port) {
        case VIA_DI_PORT_DVP0:
                via_dvp0_set_clock_drive_strength(VGABASE,
                                             clock_drive_strength);
                break;
        case VIA_DI_PORT_DVP1:
                via_dvp1_set_clock_drive_strength(VGABASE,
                                             clock_drive_strength);
                break;
        default:
                break;
    }

    drm_dbg_kms(dev, "Exiting %s.\n", __func__);
}

void via_transmitter_data_drive_strength(struct drm_device *dev,
                                     u32 di_port, u8 data_drive_strength)
{
    struct via_drm_priv *dev_priv = to_via_drm_priv(dev);

    drm_dbg_kms(dev, "Entered %s.\n", __func__);

    switch (di_port) {
        case VIA_DI_PORT_DVP0:
                via_dvp0_set_data_drive_strength(VGABASE,
                                            data_drive_strength);
                break;
        case VIA_DI_PORT_DVP1:
                via_dvp1_set_data_drive_strength(VGABASE,
                                            data_drive_strength);
                break;
        default:
                break;
    }

    drm_dbg_kms(dev, "Exiting %s.\n", __func__);
}

void via_transmitter_display_source(struct drm_device *dev,
                                u32 di_port, int display_source)
{
    struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
    
    drm_dbg_kms(dev, "Entered %s.\n", __func__);

    switch (di_port) {
        case VIA_DI_PORT_DIP0:
                via_dip0_set_display_source(VGABASE, display_source);
                break;
        case VIA_DI_PORT_DIP1:
                via_dip1_set_display_source(VGABASE, display_source);
                break;
        case VIA_DI_PORT_DVP0:
                via_dvp0_set_display_source(VGABASE, display_source);
                break;
        case VIA_DI_PORT_DVP1:
                via_dvp1_set_display_source(VGABASE, display_source);
                break;
        case VIA_DI_PORT_FPDPLOW:
                via_fpdp_low_set_display_source(VGABASE, display_source);
                break;

        case VIA_DI_PORT_FPDPHIGH:
                via_fpdp_high_set_display_source(VGABASE, display_source);
                break;
        case VIA_DI_PORT_DFPLOW:
        case VIA_DI_PORT_DFPHIGH:
                /* Do nothing */
                break;

        case VIA_DI_PORT_LVDS1:
                via_lvds1_set_display_source(VGABASE, display_source);
                break;
        case VIA_DI_PORT_LVDS2:
                via_lvds2_set_display_source(VGABASE, display_source);
                break;
        default:
                break;
    }

    drm_dbg_kms(dev, "Exiting %s.\n", __func__);
}

/* Stub implementations of low-level functions */

/* DIP port functions */
void via_dip0_set_io_pad_state(void __iomem *base, bool enable) {}
void via_dip1_set_io_pad_state(void __iomem *base, bool enable) {}
void via_dip0_set_output_enable(void __iomem *base, bool enable) {}
void via_dip1_set_output_enable(void __iomem *base, bool enable) {}
void via_dip0_set_clock_source(void __iomem *base, bool enable) {}
void via_dip1_set_clock_source(void __iomem *base, bool enable) {}
void via_dip0_set_display_source(void __iomem *base, int source) {}
void via_dip1_set_display_source(void __iomem *base, int source) {}

/* DVP port functions */
void via_dvp0_set_io_pad_state(void __iomem *base, bool enable) {}
void via_dvp1_set_io_pad_state(void __iomem *base, bool enable) {}
void via_dvp0_set_clock_drive_strength(void __iomem *base, u8 strength) {}
void via_dvp1_set_clock_drive_strength(void __iomem *base, u8 strength) {}
void via_dvp0_set_data_drive_strength(void __iomem *base, u8 strength) {}
void via_dvp1_set_data_drive_strength(void __iomem *base, u8 strength) {}
void via_dvp0_set_display_source(void __iomem *base, int source) {}
void via_dvp1_set_display_source(void __iomem *base, int source) {}

/* FPDP port functions */
void via_fpdp_low_set_io_pad_state(void __iomem *base, bool enable) {}
void via_fpdp_high_set_io_pad_state(void __iomem *base, bool enable) {}
void via_fpdp_low_set_display_source(void __iomem *base, int source) {}
void via_fpdp_high_set_display_source(void __iomem *base, int source) {}

/* LVDS port functions */
void via_lvds1_set_io_pad_setting(void __iomem *base, bool enable) {}
void via_lvds2_set_io_pad_setting(void __iomem *base, bool enable) {}
void via_lvds1_set_display_source(void __iomem *base, int source) {}
void via_lvds2_set_display_source(void __iomem *base, int source) {}
