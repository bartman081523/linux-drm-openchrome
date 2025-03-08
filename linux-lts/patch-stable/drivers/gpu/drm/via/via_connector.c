/*
 * Copyright 2016 The OpenChrome Project
 * [via_connector.c]
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
 * Authors: Kevin Brace <kevinbrace@gmx.com>
 */

#include "via_drv.h"      // MUST be first
#include "via_connector.h"

#include <drm/drm_atomic_state_helper.h>
#include <drm/drm_connector.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_edid.h>
#include <drm/drm_modeset_helper_vtables.h>
#include <drm/drm_probe_helper.h>

struct via_i2c_stuff {
    u16 i2c_port;
    u16 is_active;
    struct i2c_adapter adapter;
    struct i2c_algo_bit_data algo;
};

/* Forward declaration of via_i2c_adapters */
extern struct via_i2c_stuff via_i2c_adapters[5];

/* Import the global via_i2c_adapters variable */
extern struct via_i2c_stuff via_i2c_adapters[];

/* 
 * REMOVED: via_find_ddc_bus
 * This function is now defined only in via_i2c.c
 */

void via_connector_destroy(struct drm_connector *connector)
{
    struct via_connector *con;

    if (!connector)
        return;

    con = to_via_connector(connector);
    
    /* No need to check con since to_via_connector is just a container_of macro */

    drm_connector_unregister(connector);
     
    kfree(con);
}

int via_connector_init(struct drm_connector *connector,
                     const struct drm_connector_funcs *funcs,
                     int connector_type)
{
    /* Only check connector and funcs - don't check connector->dev 
     * since it hasn't been set yet! */
    if (!connector || !funcs)
        return -EINVAL;

    /* Initialize the connector with DRM subsystem */
    drm_connector_init(connector->dev, connector, funcs, connector_type);
    drm_connector_register(connector);
    return 0;
}

/* 
 * REMOVED: via_tmds_detect
 * This function is now defined only in via_tmds.c
 */

/* Connector function structures for DVI connections */
const struct drm_connector_funcs via_dvi_connector_funcs = {
    .dpms = drm_helper_connector_dpms,
    .detect = via_tmds_detect,
    .fill_modes = drm_helper_probe_single_connector_modes,
    .destroy = via_connector_destroy,
    .reset = drm_atomic_helper_connector_reset,
    .atomic_duplicate_state = drm_atomic_helper_connector_duplicate_state,
    .atomic_destroy_state = drm_atomic_helper_connector_destroy_state,
};

/* 
 * Common connector helper functions for all connector types
 * This avoids having separate unused helper function structures
 */
static const struct drm_connector_helper_funcs via_connector_helper_funcs = {
    .get_modes = drm_connector_helper_get_modes,
    .mode_valid = NULL, /* Default to allowing all modes */
};

/* DAC/VGA connector initialization function */
struct drm_connector *
via_connector_dac_init(struct drm_device *dev) {   
    struct via_connector *con;

    con = kzalloc(sizeof(*con), GFP_KERNEL);
    if (!con)
        return NULL;

    struct drm_connector *connector = &con->base;
    INIT_LIST_HEAD(&con->props); 
    connector->connector_type = DRM_MODE_CONNECTOR_VGA; 
    con->i2c_bus = 0; 
  
    int ret = via_connector_init(connector, &via_dac_connector_funcs, 
                DRM_MODE_CONNECTOR_VGA);
    if (ret) {
        kfree(con);
        return NULL;
    }
    
    /* Register helper functions */
    drm_connector_helper_add(connector, &via_connector_helper_funcs);
     
    return connector;
}

struct drm_connector *
via_connector_digital_init(struct drm_device *dev)
{
    struct via_connector *con;
 
    con = kzalloc(sizeof(*con), GFP_KERNEL);
    if (!con)
        return NULL;

    struct drm_connector *connector = &con->base;
    INIT_LIST_HEAD(&con->props);
    connector->connector_type = DRM_MODE_CONNECTOR_DVII;
    con->i2c_bus = 0;
  
    int ret = via_connector_init(connector, &via_dvi_connector_funcs,
                      DRM_MODE_CONNECTOR_DVII);
    if (ret) {
        kfree(con);
        return NULL;
    }
    
    /* Register helper functions */
    drm_connector_helper_add(connector, &via_connector_helper_funcs);
     
    return connector;
}

struct drm_connector *
via_connector_lvds_init(struct drm_device *dev)
{
    struct via_connector *con; 
 
    con = kzalloc(sizeof(*con), GFP_KERNEL);
    if (!con)
        return NULL;

    struct drm_connector *connector = &con->base; 
    INIT_LIST_HEAD(&con->props);
    connector->connector_type = DRM_MODE_CONNECTOR_LVDS; 
    con->i2c_bus = 0;
  
    int ret = via_connector_init(connector, &via_lvds_connector_funcs,
                      DRM_MODE_CONNECTOR_LVDS);
    if (ret) {
        kfree(con);
        return NULL;
    }
    
    /* Register helper functions */
    drm_connector_helper_add(connector, &via_connector_helper_funcs);
     
    return connector;
}

int via_connector_create(struct drm_device *dev)
{
    struct drm_connector *vga_connector;
    struct drm_connector *dvi_connector;
    struct drm_connector *lvds_connector;

    /* Initialize DAC connector */
    vga_connector = via_connector_dac_init(dev);
    if (vga_connector == NULL) {
        DRM_ERROR("Failed to create DAC connector\n");
        return -ENOMEM;
    }

    /* Initialize digital (DVI) connector */
    dvi_connector = via_connector_digital_init(dev);
    if (!dvi_connector) {
        DRM_ERROR("Failed to create digital connector\n");
        return -ENOMEM;
    }

    /* Initialize LVDS connector */
    lvds_connector = via_connector_lvds_init(dev);
    if (!lvds_connector) {
        DRM_ERROR("Failed to create LVDS connector\n");
        return -ENOMEM;
    }

    return 0;
}

