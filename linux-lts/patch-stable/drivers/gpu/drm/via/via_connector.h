/*
 * Copyright 2023 The OpenChrome Project
 * [via_connector.h]
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

#ifndef __VIA_CONNECTOR_H__
#define __VIA_CONNECTOR_H__

#include <drm/drm_connector.h>

/* 
 * NOTE: We don't define I2C bus identifiers here anymore.
 * Use the definitions from via_drv.h instead:
 * VIA_I2C_NONE, VIA_I2C_BUS1, VIA_I2C_BUS2, VIA_I2C_BUS3, etc.
 */

/* VGA connector functions */
extern const struct drm_connector_funcs via_dac_connector_funcs;
extern const struct drm_connector_helper_funcs via_dac_connector_helper_funcs;
extern enum drm_connector_status via_dac_detect(struct drm_connector *connector, bool force);

/* DVI connector functions */
extern const struct drm_connector_funcs via_dvi_connector_funcs;
extern const struct drm_connector_helper_funcs via_tmds_connector_helper_funcs;
extern enum drm_connector_status via_tmds_detect(struct drm_connector *connector, bool force);

/* LVDS connector functions */
extern const struct drm_connector_funcs via_lvds_connector_funcs;
extern const struct drm_connector_helper_funcs via_lvds_connector_helper_funcs;
extern enum drm_connector_status via_lvds_detect(struct drm_connector *connector, bool force);

/* HDMI connector functions (for VX900) */
extern const struct drm_connector_funcs via_hdmi_connector_funcs;
extern const struct drm_connector_helper_funcs via_hdmi_connector_helper_funcs;
extern enum drm_connector_status via_hdmi_detect(struct drm_connector *connector, bool force);

/* Connector helper functions */
void via_connector_destroy(struct drm_connector *connector);
int via_connector_init(struct drm_connector *connector,
                      const struct drm_connector_funcs *funcs,
                      int connector_type);

/* Connector initialization functions */
struct drm_connector *via_connector_dac_init(struct drm_device *dev);
struct drm_connector *via_connector_digital_init(struct drm_device *dev);
struct drm_connector *via_connector_lvds_init(struct drm_device *dev);
int via_connector_create(struct drm_device *dev);

/* Find I2C bus from port */
struct i2c_adapter *via_find_ddc_bus(int port);

#endif /* __VIA_CONNECTOR_H__ */
