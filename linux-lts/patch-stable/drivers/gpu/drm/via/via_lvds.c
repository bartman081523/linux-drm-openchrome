/*
 * Copyright 2016-2023 The OpenChrome Project
 * [via_lvds.c]
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

#include <linux/delay.h>
#include <linux/pci.h>

#include <drm/drm_atomic_helper.h>
#include <drm/drm_connector.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_edid.h>
#include <drm/drm_modeset_helper_vtables.h>
#include <drm/drm_probe_helper.h>

#include "via_drv.h"
#include "via_dac_regs.h"
#include "via_connector.h" // Include the new header

static int via_lvds_get_modes(struct drm_connector *connector)
{
    struct via_connector *con = to_via_connector(connector);
    struct i2c_adapter *i2c_bus;
    struct edid *edid;
    int count = 0;

    if (con->i2c_bus) {
        i2c_bus = via_find_ddc_bus(0x2c);
        if (i2c_bus) {
            edid = drm_get_edid(&con->base, i2c_bus);
            if (edid) {
                if (edid->input & DRM_EDID_INPUT_DIGITAL) {
                    drm_connector_update_edid_property(connector, edid);
                    count = drm_add_edid_modes(connector, edid);
                }
                kfree(edid);
                return count;
            }
        }
    }
    return 0;
}

/* Remove static keyword to match extern declaration in via_connector.h */
enum drm_connector_status via_lvds_detect(struct drm_connector *connector, bool force)
{
    struct drm_device *dev = connector->dev;
    enum drm_connector_status ret = connector_status_disconnected;

    drm_dbg_kms(dev, "Entered %s.\n", __func__);

    /* For now, always report as connected */
    ret = connector_status_connected;

    drm_dbg_kms(dev, "Exiting %s.\n", __func__);
    return ret;
}

/* Remove static keyword to match extern declaration in via_connector.h */
const struct drm_connector_funcs via_lvds_connector_funcs = {
    .dpms = drm_helper_connector_dpms,
    .detect = via_lvds_detect,
    .fill_modes = drm_helper_probe_single_connector_modes,
    .destroy = via_connector_destroy,
    .reset = drm_atomic_helper_connector_reset,
    .atomic_duplicate_state =
            drm_atomic_helper_connector_duplicate_state,
    .atomic_destroy_state =
            drm_atomic_helper_connector_destroy_state,
};

/* 
 * Fix the declaration of via_lvds_connector_helper_funcs
 * Remove the static keyword since it's declared as external in via_connector.h
 */
const struct drm_connector_helper_funcs via_lvds_connector_helper_funcs = {
    .get_modes = via_lvds_get_modes,
};

/* Declare this as void to match the prototype in header */
void via_lvds_init(struct drm_device *dev)
{
    /* Implementation will be added later */
    drm_dbg_kms(dev, "LVDS init stub\n");
    return;
}

/* Probe LVDS panel presence */
void via_lvds_probe(struct drm_device *dev)
{
    struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
    
    /* For now, stub implementation - assume LVDS exists */
    dev_priv->mapped_i2c_bus |= VIA_I2C_BUS3;
    
    drm_dbg_kms(dev, "LVDS probe stub\n");
    return;
}

int via_lvds_power_on(struct drm_device *dev)
{
    /* Stub implementation */
    drm_dbg_kms(dev, "LVDS power on stub\n");
    return 0;
}

int via_lvds_power_off(struct drm_device *dev)
{
    /* Stub implementation */
    drm_dbg_kms(dev, "LVDS power off stub\n");
    return 0;
}

int via_lvds_set_mode(struct drm_device *dev, struct drm_display_mode *mode)
{
    /* Stub implementation */
    drm_dbg_kms(dev, "LVDS set mode stub\n");
    return 0;
}

/* Add prototype to reduce warnings */
void via_lvds_init_old(struct drm_device *dev);

/* Legacy initialization function - renamed to avoid conflicts */
void via_lvds_init_old(struct drm_device *dev)
{
    struct via_connector *con;
    struct via_encoder *enc;
    struct via_drm_priv *dev_priv = to_via_drm_priv(dev);

    /* Allocate enough memory for encoder and connector */
    enc = kzalloc(sizeof(*enc) + sizeof(*con), GFP_KERNEL);
    if (!enc) {
        drm_err(dev, "Failed to allocate encoder for LVDS\n");
        return;
    }
    
    /* The 'con' field is defined in via_encoder struct */
    con = &enc->con;
    INIT_LIST_HEAD(&con->props);

    /* Initialize connector */
    via_connector_init(&con->base, &via_lvds_connector_funcs, DRM_MODE_CONNECTOR_LVDS);
    drm_connector_helper_add(&con->base, &via_lvds_connector_helper_funcs);
    drm_connector_register(&con->base);
    
    /* Set I2C bus information */
    con->i2c_bus = VIA_I2C_BUS3;
    
    con->base.doublescan_allowed = false;
    con->base.interlace_allowed = false;

    /* Initialize encoder */
    drm_encoder_init(dev, &enc->base, &via_crtc_encoder_funcs,
                    DRM_MODE_ENCODER_LVDS, NULL);
    drm_encoder_helper_add(&enc->base, &via_crtc_helper_funcs);
    
    enc->base.possible_crtcs = BIT(1);
    enc->base.possible_clones = 0;
    enc->encoder_type = VIA_ENCODER_LVDS;
    
    drm_connector_attach_encoder(&con->base, &enc->base);
    
    /* Store LVDS encoder in dev_priv */
    dev_priv->lvds = enc;
}

/* LVDS encoder DPMS control */
static void via_lvds_dpms(struct drm_encoder *encoder, int mode)
{
    struct drm_device *dev = encoder->dev;
    
    drm_dbg_kms(dev, "LVDS encoder DPMS mode: %d\n", mode);
    
    switch (mode) {
    case DRM_MODE_DPMS_ON:
        via_lvds_power_on(dev);
        break;
    case DRM_MODE_DPMS_STANDBY:
    case DRM_MODE_DPMS_SUSPEND:
    case DRM_MODE_DPMS_OFF:
        via_lvds_power_off(dev);
        break;
    }
}

/* LVDS encoder mode fixup */
static bool via_lvds_mode_fixup(struct drm_encoder *encoder,
                             const struct drm_display_mode *mode,
                             struct drm_display_mode *adjusted_mode)
{
    /* For LVDS, we typically need to use the panel's fixed mode */
    /* This is just a stub for now */
    return true;
}

/* LVDS encoder prepare */
static void via_lvds_prepare(struct drm_encoder *encoder)
{
    struct drm_device *dev = encoder->dev;
    
    drm_dbg_kms(dev, "Preparing LVDS encoder\n");
    
    /* Turn off LVDS before mode setting */
    via_lvds_dpms(encoder, DRM_MODE_DPMS_OFF);
}

/* LVDS encoder commit */
static void via_lvds_commit(struct drm_encoder *encoder)
{
    struct drm_device *dev = encoder->dev;
    
    drm_dbg_kms(dev, "Committing LVDS encoder\n");
    
    /* Turn on LVDS after mode setting */
    via_lvds_dpms(encoder, DRM_MODE_DPMS_ON);
}

/* LVDS encoder mode set */
static void via_lvds_mode_set(struct drm_encoder *encoder,
                           struct drm_display_mode *mode,
                           struct drm_display_mode *adjusted_mode)
{
    struct drm_device *dev = encoder->dev;
    
    drm_dbg_kms(dev, "LVDS encoder mode set\n");
    
    /* Configure LVDS for the requested mode */
    via_lvds_set_mode(dev, adjusted_mode);
}

/* LVDS encoder disable */
static void via_lvds_disable(struct drm_encoder *encoder)
{
    struct drm_device *dev = encoder->dev;
    
    drm_dbg_kms(dev, "Disabling LVDS encoder\n");
    
    /* Turn off LVDS */
    via_lvds_dpms(encoder, DRM_MODE_DPMS_OFF);
}

/* LVDS encoder helper functions - fixed the previously undefined symbol issue */
const struct drm_encoder_helper_funcs via_lvds_encoder_helper_funcs = {
    .dpms = via_lvds_dpms,
    .mode_fixup = via_lvds_mode_fixup,
    .prepare = via_lvds_prepare,
    .commit = via_lvds_commit,
    .mode_set = via_lvds_mode_set,
    .disable = via_lvds_disable
};
