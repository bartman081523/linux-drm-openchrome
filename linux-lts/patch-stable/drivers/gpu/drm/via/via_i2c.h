/*
 * Copyright 2023 The OpenChrome Project
 * [via_i2c.h]
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

#ifndef __VIA_I2C_H__
#define __VIA_I2C_H__

#include <linux/i2c.h>

/* I2C bus port definitions (used by via_i2c_adapters) */
#define VIA_I2C_PORT_0     0x26
#define VIA_I2C_PORT_1     0x31
#define VIA_I2C_PORT_2     0x25
#define VIA_I2C_PORT_3     0x2C
#define VIA_I2C_PORT_4     0x3D

/* I2C functions */
int via_i2c_init(struct drm_device *dev);
void via_i2c_exit(void);
void via_i2c_reg_init(struct via_drm_priv *dev_priv);

/* I2C transfer functions for device communication */
void via_i2c_readbytes(struct i2c_adapter *adapter,
                      u8 slave_addr, char offset,
                      u8 *buffer, unsigned int size);
                      
void via_i2c_writebytes(struct i2c_adapter *adapter, 
                       u8 slave_addr, char offset,
                       u8 *data, unsigned int size);

/* Find I2C adapter by port */
struct i2c_adapter *via_find_ddc_bus(int port);

/* External variables */
extern struct via_i2c_par_t via_i2c_par;

#endif /* __VIA_I2C_H__ */
