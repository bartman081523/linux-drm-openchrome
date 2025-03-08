// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2012 James Simmons. All Rights Reserved.
 * Copyright 1998-2009 VIA Technologies, Inc. All Rights Reserved.
 * Copyright 2001-2008 S3 Graphics, Inc. All Rights Reserved.
 *
 * Author(s):
 * James Simmons <jsimmons@infradead.org>
 * Kevin Brace <kevinbrace@bracecomputerlab.com>  2017-2020, 2024
 */

/*
 * Copyright 2023 The OpenChrome Project
 * [via_i2c.c]
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

#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/i2c-algo-bit.h>
#include <linux/module.h>

#include <uapi/linux/i2c.h>

#include "via_drv.h"
#include "via_i2c.h"
#include "via_connector.h"

#define SERIAL	0
#define	GPIO	1

/* I2C bus adapter structure array - holds configuration for all supported I2C buses */
struct via_i2c_stuff {
    u16 i2c_port;          /* GPIO or I2C port */
    u16 is_active;         /* Being used as I2C? */
    struct i2c_adapter adapter;
    struct i2c_algo_bit_data algo;
};

/* Make the adapter array global so other modules can find it */
struct via_i2c_stuff via_i2c_adapters[5] = {
    { .i2c_port = 0, .is_active = 0, .adapter = { .name = "" }, .algo = { 0 } },
    { .i2c_port = 0, .is_active = 0, .adapter = { .name = "" }, .algo = { 0 } },
    { .i2c_port = 0, .is_active = 0, .adapter = { .name = "" }, .algo = { 0 } },
    { .i2c_port = 0, .is_active = 0, .adapter = { .name = "" }, .algo = { 0 } },
    { .i2c_port = 0, .is_active = 0, .adapter = { .name = "" }, .algo = { 0 } }
};
EXPORT_SYMBOL_GPL(via_i2c_adapters);

/* Provide a global I2C parameters structure for compatibility with older code */
struct via_i2c_par_t {
    struct drm_device *dev;
    u32 bus_index;
    u32 ddc_base;
};

struct via_i2c_par_t via_i2c_par;
EXPORT_SYMBOL_GPL(via_i2c_par);

/* 
 * Set I2C SDA line
 * Sets the I2C SDA line to high (1) or low (0)
 */
static void via_i2c_setsda(void *data, int state)
{
    struct via_i2c_stuff *i2c = data;
    struct drm_device *dev = i2c_get_adapdata(&i2c->adapter);
    struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
    u8 value, mask;

    if (i2c->is_active == GPIO) {
        mask = state ? BIT(6) : BIT(6) | BIT(4);
        value = state ? 0x00 : BIT(6);
    } else {
        value = state ? BIT(4) | BIT(0) : BIT(0);
        mask = BIT(4) | BIT(0);
    }

    svga_wseq_mask(VGABASE, i2c->i2c_port, value, mask);
}

/* 
 * Set I2C SCL line
 * Sets the I2C SCL (clock) line to high (1) or low (0)
 */
static void via_i2c_setscl(void *data, int state)
{
    struct via_i2c_stuff *i2c = data;
    struct drm_device *dev = i2c_get_adapdata(&i2c->adapter);
    struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
    u8 value, mask;

    if (i2c->is_active == GPIO) {
        mask = state ? BIT(7) : BIT(7) | BIT(5);
        value = state ? 0x00 : BIT(7);
    } else {
        value = state ? BIT(5) | BIT(0) : BIT(0);
        mask = BIT(5) | BIT(0);
    }

    svga_wseq_mask(VGABASE, i2c->i2c_port, value, mask);
}

/* 
 * Get I2C SDA line state
 * Returns the state of the I2C SDA line (1 for high, 0 for low)
 */
static int via_i2c_getsda(void *data)
{
    struct via_i2c_stuff *i2c = data;
    struct drm_device *dev = i2c_get_adapdata(&i2c->adapter);
    struct via_drm_priv *dev_priv = to_via_drm_priv(dev);

    return !!(vga_rseq(VGABASE, i2c->i2c_port) & BIT(2));
}

/* 
 * Get I2C SCL line state
 * Returns the state of the I2C SCL line (1 for high, 0 for low)
 */
static int via_i2c_getscl(void *data)
{
    struct via_i2c_stuff *i2c = data;
    struct drm_device *dev = i2c_get_adapdata(&i2c->adapter);
    struct via_drm_priv *dev_priv = to_via_drm_priv(dev);

    return !!(vga_rseq(VGABASE, i2c->i2c_port) & BIT(3));
}

/* 
 * Create I2C bus
 * Sets up an I2C bus with the specified parameters
 */
static int create_i2c_bus(struct drm_device *dev,
                         struct via_i2c_stuff *i2c_par)
{
    struct i2c_adapter *adapter = &i2c_par->adapter;
    struct i2c_algo_bit_data *algo = &i2c_par->algo;
    int ret;

    /* Set up the algorithm structure with our bit-banging functions */
    algo->setsda = via_i2c_setsda;
    algo->setscl = via_i2c_setscl;
    algo->getsda = via_i2c_getsda;
    algo->getscl = via_i2c_getscl;
    algo->udelay = 15;  /* 15 microseconds delay between clock edges */
    algo->timeout = usecs_to_jiffies(2200); /* 2.2ms timeout from VESA spec */
    algo->data = i2c_par;

    /* Set up the adapter structure */
    snprintf(adapter->name, sizeof(adapter->name),
             "via i2c bit bus 0x%02x", i2c_par->i2c_port);
    adapter->owner = THIS_MODULE;
    adapter->algo_data = algo;
    i2c_set_adapdata(adapter, dev);

    /* Initialize the bus lines to idle state (high) */
    via_i2c_setsda(i2c_par, 1);
    via_i2c_setscl(i2c_par, 1);
    udelay(20);

    /* Register the adapter with the I2C subsystem */
    ret = i2c_bit_add_bus(adapter);
    if (ret < 0) {
        drm_err(dev, "Failed to register I2C bus 0x%02x: %d\n", 
                i2c_par->i2c_port, ret);
    } else {
        drm_dbg_kms(dev, "Registered I2C bus 0x%02x\n", i2c_par->i2c_port);
    }

    return ret;
}

/* 
 * Read bytes from I2C device
 * Reads data from the specified I2C device at the specified offset
 */
void via_i2c_readbytes(struct i2c_adapter *adapter,
                      u8 slave_addr, char offset,
                      u8 *buffer, unsigned int size)
{
    u8 out_buf[2];
    u8 in_buf[256]; /* Ensure buffer is large enough for any request */
    int ret;
    struct via_i2c_stuff *i2c = i2c_get_adapdata(adapter);
    struct drm_device *dev = i2c_get_adapdata(&i2c->adapter);
    struct i2c_msg msgs[] = {
        {
            .addr = slave_addr,
            .flags = 0,
            .len = 1,
            .buf = out_buf,
        },
        {
            .addr = slave_addr,
            .flags = I2C_M_RD,
            .len = size,
            .buf = in_buf,
        }
    };

    /* Safety check for buffer size */
    if (size > sizeof(in_buf)) {
        drm_err(dev, "I2C read request too large (%u bytes)\n", size);
        return;
    }

    /* Prepare the write message with the offset */
    out_buf[0] = offset;

    /* Execute the transaction */
    ret = i2c_transfer(adapter, msgs, 2);
    if (ret != 2) {
        drm_err(dev, "I2C read failed: %d\n", ret);
        return;
    }

    /* Copy the data to the output buffer */
    memcpy(buffer, in_buf, size);
}

/* 
 * Write bytes to I2C device
 * Writes data to the specified I2C device at the specified offset
 */
void via_i2c_writebytes(struct i2c_adapter *adapter,
                       u8 slave_addr, char offset,
                       u8 *data, unsigned int size)
{
    u8 *out_buf;
    int ret;
    struct via_i2c_stuff *i2c = i2c_get_adapdata(adapter);
    struct drm_device *dev = i2c_get_adapdata(&i2c->adapter);
    struct i2c_msg msg = { 0 };

    /* Allocate a buffer for the offset + data */
    out_buf = kzalloc(size + 1, GFP_KERNEL);
    if (!out_buf) {
        drm_err(dev, "Failed to allocate I2C write buffer\n");
        return;
    }

    /* Prepare the message with offset and data */
    out_buf[0] = offset;
    memcpy(&out_buf[1], data, size);
    
    msg.addr = slave_addr;
    msg.flags = 0;
    msg.len = size + 1;
    msg.buf = out_buf;

    /* Execute the transaction */
    ret = i2c_transfer(adapter, &msg, 1);
    if (ret != 1) {
        drm_err(dev, "I2C write failed: %d\n", ret);
    }

    /* Free the allocated buffer */
    kfree(out_buf);
}

/* 
 * Initialize I2C registers
 * Sets up the I2C registers for the OpenChrome hardware
 */
void via_i2c_reg_init(struct via_drm_priv *dev_priv)
{
    struct drm_device *dev = &dev_priv->dev;

    drm_dbg_kms(dev, "Initializing I2C registers\n");

    /* Initialize I2C registers based on VIA hardware documentation */
    svga_wseq_mask(VGABASE, 0x31, 0x30, 0x30);
    svga_wseq_mask(VGABASE, 0x26, 0x30, 0x30);
    vga_wseq(VGABASE, 0x2C, 0xc2);
    vga_wseq(VGABASE, 0x3D, 0xc0);
    svga_wseq_mask(VGABASE, 0x2C, 0x30, 0x30);
    svga_wseq_mask(VGABASE, 0x3D, 0x30, 0x30);
}

/* 
 * Initialize I2C subsystem
 * Sets up all I2C buses for the OpenChrome driver
 */
int via_i2c_init(struct drm_device *dev)
{
    /* Define types and ports for all 5 supported I2C buses */
    int types[] = { SERIAL, SERIAL, GPIO, GPIO, GPIO };
    int ports[] = { VIA_I2C_PORT_0, VIA_I2C_PORT_1, VIA_I2C_PORT_2, 
                   VIA_I2C_PORT_3, VIA_I2C_PORT_4 };
    int count = ARRAY_SIZE(via_i2c_adapters);
    int ret, i;
    struct via_i2c_stuff *i2c = via_i2c_adapters;

    drm_dbg_kms(dev, "Initializing I2C buses\n");

    /* Initialize each I2C bus */
    for (i = 0; i < count; i++) {
        i2c->is_active = types[i];
        i2c->i2c_port = ports[i];

        ret = create_i2c_bus(dev, i2c);
        if (ret < 0) {
            drm_err(dev, "Failed to create I2C bus %d (port 0x%x): %d\n",
                   i, ports[i], ret);
        } else {
            drm_dbg_kms(dev, "Created I2C bus %d (port 0x%x)\n", 
                        i, ports[i]);
        }
        i2c++;
    }

    return 0;
}

/* 
 * Clean up I2C subsystem
 * Releases I2C resources when the driver is unloaded
 */
void via_i2c_exit(void)
{
    int i;

    /* Unregister each I2C adapter */
    for (i = 0; i < ARRAY_SIZE(via_i2c_adapters); i++) {
        if (via_i2c_adapters[i].adapter.algo_data) {
            i2c_del_adapter(&via_i2c_adapters[i].adapter);
        }
    }
}

/* 
 * Find I2C bus by port number
 * Returns the I2C adapter for the specified port
 */
struct i2c_adapter *via_find_ddc_bus(int port)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(via_i2c_adapters); i++) {
        if (via_i2c_adapters[i].i2c_port == port) {
            return &via_i2c_adapters[i].adapter;
        }
    }
    
    return NULL;
}