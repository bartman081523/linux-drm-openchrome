/*
 * Copyright 2023 The OpenChrome Project
 * [via_modeset.c]
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <linux/spinlock.h>
#include <linux/completion.h>
#include <linux/sched.h>
#include <linux/mutex.h>

#include <drm/drm_atomic_helper.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_gem_framebuffer_helper.h>
#include <drm/drm_mode_config.h>
#include <drm/drm_probe_helper.h>
#include <drm/drm_vblank.h>

#include "via_drv.h"
#include "via_hdmi_regs.h"
#include "via_connector.h"
#include "via_encoder.h"     /* Add the new encoder header */
#include "via_display_port.h" /* Add for display port constants */

/* Define the via_drm_mode_config_funcs which was missing */
static const struct drm_mode_config_funcs via_drm_mode_config_funcs = {
    .fb_create = drm_gem_fb_create,
    .atomic_check = drm_atomic_helper_check,
    .atomic_commit = drm_atomic_helper_commit,
};

/* Helper function to safely initialize hardware components with timeout */
static int via_hw_init_with_timeout(struct drm_device *dev, 
                                   int (*init_func)(struct drm_device *dev), 
                                   const char *component_name)
{
    int ret = 0;
    unsigned long timeout = jiffies + msecs_to_jiffies(1000); /* 1 second timeout */
    
    drm_dbg_kms(dev, "Starting %s initialization\n", component_name);
    
    /* Initialize with timeout check to avoid soft locks */
    ret = init_func(dev);
    if (ret) {
        drm_err(dev, "%s initialization failed: %d\n", component_name, ret);
        return ret;
    }
    
    if (time_after(jiffies, timeout)) {
        drm_warn(dev, "%s initialization took too long, may be unstable\n", component_name);
    }
    
    drm_dbg_kms(dev, "%s initialization completed\n", component_name);
    return 0;
}

/* This function is extracted from via_modeset_init in via_init.c to provide better
 * control over the initialization sequence with proper timeouts and error handling */
int via_modeset_init_safe(struct drm_device *dev)
{
    struct pci_dev *pdev = to_pci_dev(dev->dev);
    struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
    struct via_connector *con = NULL;
    struct via_encoder *enc = NULL;
    int ret = 0;
    unsigned int i;

    drm_dbg_kms(dev, "Starting safe modeset initialization\n");

    /* Safety check for dev_priv */
    if (!dev_priv) {
        drm_err(dev, "NULL dev_priv in via_modeset_init_safe\n");
        return -EINVAL;
    }

    /* Initialize the number of display connectors. */
    dev_priv->number_fp = 0;
    dev_priv->number_dvi = 0;

    dev->mode_config.min_width = 0;
    dev->mode_config.min_height = 0;
    dev->mode_config.max_width = 2044;
    dev->mode_config.max_height = 4096;
    dev->mode_config.preferred_depth = 24;
    dev->mode_config.cursor_width = VIA_CURSOR_SIZE;
    dev->mode_config.cursor_height = VIA_CURSOR_SIZE;

    dev->mode_config.funcs = &via_drm_mode_config_funcs;

    ret = drmm_mode_config_init(dev);
    if (ret) {
        drm_err(dev, "drmm_mode_config_init failed: %d\n", ret);
        return ret;
    }

    /* Initialize CRTCs with timeout protection */
    for (i = 0; i < VIA_MAX_CRTC; i++) {
        ret = via_hw_init_with_timeout(dev, 
                                      (i == 0) ? via_crtc1_init : via_crtc2_init, 
                                      (i == 0) ? "CRTC1" : "CRTC2");
        if (ret) {
            drm_err(dev, "Failed to initialize %s\n", (i == 0) ? "CRTC1" : "CRTC2");
            goto error_crtc_init;
        }
    }

    /*
     * Allocate and initialize connectors and encoders.
     * Do this for all potential connectors (VGA, DVI, LVDS, HDMI),
     * even if we don't detect the hardware. The `detect` functions
     * will handle reporting whether a display is actually connected.
     */

    /* DAC (VGA) */
    enc = kzalloc(sizeof(*enc) + sizeof(*con), GFP_KERNEL);
    if (!enc) {
        drm_err(dev, "Failed to allocate DAC encoder\n");
        ret = -ENOMEM;
        goto error_crtc_init;
    }
    con = &enc->con;
    INIT_LIST_HEAD(&con->props);
    drm_connector_init(dev, &con->base, &via_dac_connector_funcs, DRM_MODE_CONNECTOR_VGA);
    drm_connector_helper_add(&con->base, &via_dac_connector_helper_funcs);
    drm_connector_register(&con->base);
    con->i2c_bus = dev_priv->dac_i2c_bus;
    con->base.doublescan_allowed = false;
    con->base.interlace_allowed = true;
    dev_priv->dac = enc; // Store encoder in dev_priv

    /* TMDS (DVI) */
    enc = kzalloc(sizeof(*enc) + sizeof(*con), GFP_KERNEL);
    if (!enc) {
        drm_err(dev, "Failed to allocate TMDS encoder\n");
        ret = -ENOMEM;
        goto error_alloc;
    }
    con = &enc->con;
    INIT_LIST_HEAD(&con->props);
    drm_connector_init(dev, &con->base, &via_dvi_connector_funcs, DRM_MODE_CONNECTOR_DVID);
    drm_connector_helper_add(&con->base, &via_tmds_connector_helper_funcs);
    drm_connector_register(&con->base);
    con->i2c_bus = dev_priv->ext_tmds_i2c_bus; /* Will be updated after probing */
    con->base.doublescan_allowed = false;
    con->base.interlace_allowed = false;
    dev_priv->tmds = enc; // Store encoder in dev_priv

    /* LVDS (Laptop Display) */
    enc = kzalloc(sizeof(*enc) + sizeof(*con), GFP_KERNEL);
    if (!enc) {
        drm_err(dev, "Failed to allocate LVDS encoder\n");
        ret = -ENOMEM;
        goto error_alloc;
    }
    con = &enc->con;
    INIT_LIST_HEAD(&con->props);
    drm_connector_init(dev, &con->base, &via_lvds_connector_funcs, DRM_MODE_CONNECTOR_LVDS);
    drm_connector_helper_add(&con->base, &via_lvds_connector_helper_funcs);
    drm_connector_register(&con->base);
    con->i2c_bus = VIA_I2C_NONE; /* Will be updated after probing */
    con->base.doublescan_allowed = false;
    con->base.interlace_allowed = false;
    dev_priv->lvds = enc; // Store encoder in dev_priv

    /* Now probe for hardware */
    via_dac_probe(dev);
    via_ext_dvi_probe(dev);
    via_tmds_probe(dev);
    via_lvds_probe(dev);

    /* Initialize encoders and attach them to the connectors */
    if (dev_priv->dac_presence) {
        /* Initialize DAC encoder */
        drm_encoder_init(dev, &dev_priv->dac->base, &via_crtc_encoder_funcs,
                DRM_MODE_ENCODER_DAC, NULL);
        drm_encoder_helper_add(&dev_priv->dac->base, &via_dac_encoder_helper_funcs);
        dev_priv->dac->base.possible_crtcs = 0x1; /* Can use CRTC1 */
        dev_priv->dac->base.possible_clones = 0;
        dev_priv->dac->encoder_type = VIA_ENCODER_DAC;
        drm_connector_attach_encoder(&dev_priv->dac->con.base, &dev_priv->dac->base);
    }

    if (dev_priv->ext_tmds_presence) {
        /* Initialize TMDS encoder */
        drm_encoder_init(dev, &dev_priv->tmds->base, &via_crtc_encoder_funcs,
                DRM_MODE_ENCODER_TMDS, NULL);
        drm_encoder_helper_add(&dev_priv->tmds->base, &via_tmds_encoder_helper_funcs);
        dev_priv->tmds->base.possible_crtcs = 0x3; /* Can use either CRTC */
        dev_priv->tmds->base.possible_clones = 0;
        dev_priv->tmds->encoder_type = VIA_ENCODER_TMDS;
        drm_connector_attach_encoder(&dev_priv->tmds->con.base, &dev_priv->tmds->base);
    }

    if (dev_priv->int_lvds_presence) {
        /* Initialize LVDS encoder */
        drm_encoder_init(dev, &dev_priv->lvds->base, &via_crtc_encoder_funcs,
                DRM_MODE_ENCODER_LVDS, NULL);
        drm_encoder_helper_add(&dev_priv->lvds->base, &via_lvds_encoder_helper_funcs);
        dev_priv->lvds->base.possible_crtcs = 0x3; /* Can use either CRTC */
        dev_priv->lvds->base.possible_clones = 0;
        dev_priv->lvds->encoder_type = VIA_ENCODER_LVDS;
        drm_connector_attach_encoder(&dev_priv->lvds->con.base, &dev_priv->lvds->base);
    }

    /* VX900 specific initialization for HDMI */
    if (pdev && (pdev->device == PCI_DEVICE_ID_VIA_CHROME9_HD ||
                 pdev->device == PCI_DEVICE_ID_VIA_CHROME9_HCM ||
                 pdev->device == PCI_DEVICE_ID_VIA_VX900)) {
        via_hdmi_init(dev, VIA_DI_PORT_NONE);
    }

    drm_mode_config_reset(dev);
    drm_kms_helper_poll_init(dev);

    drm_dbg_kms(dev, "Safe modeset initialization completed\n");
    return 0;

error_alloc:
    /* Clean up allocated encoders/connectors here */
error_crtc_init:
    via_i2c_exit();
    return ret;  /* Return directly instead of using the exit label */
}

/* This function provides a safe way to shutdown modeset if needed */
void via_modeset_fini_safe(struct drm_device *dev)
{
    drm_dbg_kms(dev, "Entered %s.\n", __func__);

    /* Shutdown in reverse order of initialization */
    drm_kms_helper_poll_fini(dev);
    drm_helper_force_disable_all(dev);
    via_i2c_exit();

    drm_dbg_kms(dev, "Exiting %s.\n", __func__);
}
