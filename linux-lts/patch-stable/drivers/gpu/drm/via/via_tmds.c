/*
 * Copyright 2023 The OpenChrome Project
 * [via_tmds.c]
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
#include "via_i2c.h"
#include "via_connector.h"
#include "via_display_port.h"  /* Add this include for display port definitions */

#include <drm/drm_atomic_helper.h>
#include <drm/drm_edid.h>
#include <drm/drm_probe_helper.h>

/* Forward declarations for functions needed by structures */
int via_tmds_get_modes(struct drm_connector *connector);
enum drm_mode_status via_tmds_mode_valid(struct drm_connector *connector,
                                       struct drm_display_mode *mode);

/* TMDS (DVI) connector helper functions */
const struct drm_connector_helper_funcs via_tmds_connector_helper_funcs = {
    .get_modes = via_tmds_get_modes,
    .mode_valid = via_tmds_mode_valid
};

/* TMDS (DVI) detection implementation with proper I2C EDID reading */
enum drm_connector_status 
via_tmds_detect(struct drm_connector *connector, bool force)
{
    struct drm_device *dev = connector->dev;
    struct via_connector *con = to_via_connector(connector);
    struct i2c_adapter *i2c_bus = NULL;
    struct edid *edid = NULL;
    enum drm_connector_status status = connector_status_disconnected;
    
    drm_dbg_kms(dev, "Detecting TMDS (DVI) connector\n");
    
    /* Try to find an I2C bus for this connector */
    if (con->i2c_bus & VIA_I2C_BUS2) {
        i2c_bus = via_find_ddc_bus(VIA_I2C_PORT_1);
    } else if (con->i2c_bus & VIA_I2C_BUS1) {
        i2c_bus = via_find_ddc_bus(VIA_I2C_PORT_0);
    }
    
    if (!i2c_bus) {
        drm_dbg_kms(dev, "No I2C bus found for TMDS connector\n");
        return connector_status_unknown;
    }
    
    /* Try to read EDID from the display */
    edid = drm_get_edid(connector, i2c_bus);
    if (edid) {
        if (edid->input & DRM_EDID_INPUT_DIGITAL) {
            drm_dbg_kms(dev, "TMDS (DVI) connector detected via EDID\n");
            status = connector_status_connected;
        }
        kfree(edid);
    }
    
    return status;
}

/* TMDS encoder DPMS control */
static void via_tmds_dpms(struct drm_encoder *encoder, int mode)
{
    struct drm_device *dev = encoder->dev;
    
    drm_dbg_kms(dev, "TMDS encoder DPMS mode: %d\n", mode);
    
    /* Stub implementation */
    switch (mode) {
    case DRM_MODE_DPMS_ON:
        /* Power on the TMDS encoder */
        break;
    case DRM_MODE_DPMS_STANDBY:
    case DRM_MODE_DPMS_SUSPEND:
    case DRM_MODE_DPMS_OFF:
        /* Power off the TMDS encoder */
        break;
    }
}

/* TMDS encoder mode fixup */
static bool via_tmds_mode_fixup(struct drm_encoder *encoder,
                              const struct drm_display_mode *mode,
                              struct drm_display_mode *adjusted_mode)
{
    /* No adjustments needed for now */
    return true;
}

/* TMDS encoder prepare */
static void via_tmds_prepare(struct drm_encoder *encoder)
{
    struct drm_device *dev = encoder->dev;
    
    drm_dbg_kms(dev, "Preparing TMDS encoder\n");
    
    /* Turn off TMDS encoder before mode setting */
    via_tmds_dpms(encoder, DRM_MODE_DPMS_OFF);
}

/* TMDS encoder commit */
static void via_tmds_commit(struct drm_encoder *encoder)
{
    struct drm_device *dev = encoder->dev;
    
    drm_dbg_kms(dev, "Committing TMDS encoder\n");
    
    /* Turn on TMDS encoder after mode setting */
    via_tmds_dpms(encoder, DRM_MODE_DPMS_ON);
}

/* TMDS encoder mode set */
static void via_tmds_mode_set(struct drm_encoder *encoder,
                            struct drm_display_mode *mode,
                            struct drm_display_mode *adjusted_mode)
{
    struct drm_device *dev = encoder->dev;
    
    drm_dbg_kms(dev, "TMDS encoder mode set\n");
    
    /* Configure TMDS encoder for the requested mode */
}

/* TMDS encoder disable */
static void via_tmds_disable(struct drm_encoder *encoder)
{
    struct drm_device *dev = encoder->dev;
    
    drm_dbg_kms(dev, "Disabling TMDS encoder\n");
    
    /* Turn off TMDS encoder */
    via_tmds_dpms(encoder, DRM_MODE_DPMS_OFF);
}

/* TMDS encoder helper functions */
const struct drm_encoder_helper_funcs via_tmds_encoder_helper_funcs = {
    .dpms = via_tmds_dpms,
    .mode_fixup = via_tmds_mode_fixup,
    .prepare = via_tmds_prepare,
    .commit = via_tmds_commit,
    .mode_set = via_tmds_mode_set,
    .disable = via_tmds_disable
};

/* TMDS get modes function - moved up and made non-static */
int via_tmds_get_modes(struct drm_connector *connector)
{
    struct drm_device *dev = connector->dev;
    struct via_connector *con = to_via_connector(connector);
    struct i2c_adapter *i2c_bus = NULL;
    struct edid *edid = NULL;
    int count = 0;
    
    drm_dbg_kms(dev, "Getting modes for TMDS connector\n");
    
    /* Try to find I2C bus */
    if (con->i2c_bus & VIA_I2C_BUS2) {
        i2c_bus = via_find_ddc_bus(VIA_I2C_PORT_1);
    } else if (con->i2c_bus & VIA_I2C_BUS1) {
        i2c_bus = via_find_ddc_bus(VIA_I2C_PORT_0);
    }
    
    /* Try to read EDID */
    if (i2c_bus) {
        edid = drm_get_edid(connector, i2c_bus);
        if (edid) {
            drm_connector_update_edid_property(connector, edid);
            count = drm_add_edid_modes(connector, edid);
            kfree(edid);
        }
    }
    
    /* If no EDID, add some default modes */
    if (count == 0) {
        count = drm_add_modes_noedid(connector, 1920, 1200);
        drm_set_preferred_mode(connector, 1024, 768);
    }
    
    return count;
}

/* TMDS mode valid function - moved up and made non-static */
enum drm_mode_status via_tmds_mode_valid(struct drm_connector *connector,
                                        struct drm_display_mode *mode)
{
    /* Basic validation for DVI output */
    if (mode->clock > 165000)
        return MODE_CLOCK_HIGH;
        
    if (mode->hdisplay > 1920 || mode->vdisplay > 1200)
        return MODE_BAD_WIDTH;
        
    return MODE_OK;
}

/* Probe for external TMDS (DVI) encoders */
void via_tmds_probe(struct drm_device *dev)
{
    struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
    struct pci_dev *pdev = to_pci_dev(dev->dev);
    
    drm_dbg_kms(dev, "Probing for TMDS (DVI) encoders\n");
    
    /* CX700/VX700 has integrated TMDS */
    if (pdev->device == PCI_DEVICE_ID_VIA_UNICHROME_PRO_II) {
        dev_priv->ext_tmds_presence = true;
        dev_priv->ext_tmds_i2c_bus = VIA_I2C_BUS2;
        dev_priv->ext_tmds_di_port = VIA_DI_PORT_DFPLOW;
        drm_info(dev, "Detected integrated TMDS on CX700/VX700\n");
    }
    /* VT3364 (P4M900) has integrated TMDS */
    else if (pdev->device == PCI_DEVICE_ID_VIA_CHROME9_HC) {
        dev_priv->ext_tmds_presence = true;
        dev_priv->ext_tmds_i2c_bus = VIA_I2C_BUS5;
        dev_priv->ext_tmds_di_port = VIA_DI_PORT_DFPLOW;
        drm_info(dev, "Detected integrated TMDS on P4M900\n");
    }
    else {
        dev_priv->ext_tmds_presence = false;
    }
}

/* Initialize TMDS encoder - this is just a stub for now */
void via_tmds_init(struct drm_device *dev)
{
    drm_dbg_kms(dev, "Initializing TMDS encoder\n");
    /* Full implementation will check ext_tmds_presence and set up the encoder */
}

/* 
 * Probe for external DVI encoders - we will look for
 * specific DVI encoder chips in via_vt1632.c and via_sii164.c
 * so this is just a stub
 */
void via_ext_dvi_probe(struct drm_device *dev)
{
    drm_dbg_kms(dev, "Probing for external DVI encoders\n");
    /* This will be implemented in via_vt1632.c and via_sii164.c */
}

/* Initialize external DVI encoder - this is just a stub for now */
void via_ext_dvi_init(struct drm_device *dev)
{
    drm_dbg_kms(dev, "Initializing external DVI encoder\n");
    /* This will be implemented in via_vt1632.c and via_sii164.c */
}
