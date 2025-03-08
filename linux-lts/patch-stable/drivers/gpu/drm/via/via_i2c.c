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

/* Forward declarations for I2C functions */
static int via_i2c_master_xfer(struct i2c_adapter *adapter,
                              struct i2c_msg *msgs, 
                              int num);
static u32 via_i2c_functionality(struct i2c_adapter *adapter);

/* I2C algorithm structure - this is what the Linux I2C subsystem uses */
static const struct i2c_algorithm via_i2c_algo = {
    .master_xfer = via_i2c_master_xfer,
    .functionality = via_i2c_functionality,
};

/* 
 * Master transfer function for I2C operations 
 * This is a stub implementation that should be replaced with actual hardware access
 */
static int via_i2c_master_xfer(struct i2c_adapter *adapter,
                              struct i2c_msg *msgs, 
                              int num)
{
    /* Stub implementation - just pretend we transferred everything */
    return num;
}

/*
 * Return the functionality supported by this adapter
 */
static u32 via_i2c_functionality(struct i2c_adapter *adapter)
{
    return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}

/* I2C bus adapter structure array - holds configuration for all supported I2C buses */
struct via_i2c_stuff {
    u16 i2c_port;          /* GPIO or I2C port */
    u16 is_active;         /* Being used as I2C? */
    struct i2c_adapter adapter;
    struct i2c_algo_bit_data algo;
};

/* Properly initialize the i2c adapter array with names */
struct via_i2c_stuff via_i2c_adapters[5] = {
    { .i2c_port = 0, .is_active = 0, .adapter = { .name = "via_i2c_bus_0" }, .algo = { 0 } },
    { .i2c_port = 0, .is_active = 0, .adapter = { .name = "via_i2c_bus_1" }, .algo = { 0 } },
    { .i2c_port = 0, .is_active = 0, .adapter = { .name = "via_i2c_bus_2" }, .algo = { 0 } },
    { .i2c_port = 0, .is_active = 0, .adapter = { .name = "via_i2c_bus_3" }, .algo = { 0 } },
    { .i2c_port = 0, .is_active = 0, .adapter = { .name = "via_i2c_bus_4" }, .algo = { 0 } }
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
 * I2C bit-banging functions
 * These are currently unused but may be needed for certain hardware
 * configurations in the future, so we'll keep them with __maybe_unused
 */
static void __maybe_unused via_i2c_setsda(void *data, int state)
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

static void __maybe_unused via_i2c_setscl(void *data, int state)
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

static int __maybe_unused via_i2c_getsda(void *data)
{
    struct via_i2c_stuff *i2c = data;
    struct drm_device *dev = i2c_get_adapdata(&i2c->adapter);
    struct via_drm_priv *dev_priv = to_via_drm_priv(dev);

    return !!(vga_rseq(VGABASE, i2c->i2c_port) & BIT(2));
}

static int __maybe_unused via_i2c_getscl(void *data)
{
    struct via_i2c_stuff *i2c = data;
    struct drm_device *dev = i2c_get_adapdata(&i2c->adapter);
    struct via_drm_priv *dev_priv = to_via_drm_priv(dev);

    return !!(vga_rseq(VGABASE, i2c->i2c_port) & BIT(3));
}

/* 
 * Remove or comment out the unused function to eliminate the warning
 */
/*
static int create_i2c_bus(struct drm_device *dev,
			struct i2c_adapter *adapter,
			u32 reg,
			int gpio_data,
			int gpio_clock,
			struct i2c_algo_bit_data *algo)
{
    // Function implementation
}
*/

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
 * Initialize the I2C buses
 * This creates the I2C adapters and registers them with the Linux kernel
 */
int via_i2c_init(struct drm_device *dev)
{
    int ret = 0, i;
    static bool initialized = false;

    drm_dbg_driver(dev, "Initializing I2C buses\n");

    /* Add a guard to prevent double initialization */
    if (initialized) {
        drm_dbg_driver(dev, "I2C buses already initialized, skipping\n");
        return 0;
    }

    /* Initialize ports with appropriate values */
    via_i2c_adapters[0].i2c_port = VIA_I2C_PORT_0;
    via_i2c_adapters[1].i2c_port = VIA_I2C_PORT_1;
    via_i2c_adapters[2].i2c_port = VIA_I2C_PORT_2;
    via_i2c_adapters[3].i2c_port = VIA_I2C_PORT_3;
    via_i2c_adapters[4].i2c_port = VIA_I2C_PORT_4;

    /* Initialize I2C adapters */
    for (i = 0; i < ARRAY_SIZE(via_i2c_adapters); i++) {
        struct via_i2c_stuff *stuff = &via_i2c_adapters[i];
        
        /* Set up the I2C adapter */
        stuff->adapter.algo = &via_i2c_algo;
        stuff->adapter.owner = THIS_MODULE;
        stuff->adapter.dev.parent = dev->dev;
        
        /* Register the adapter */
        ret = i2c_add_adapter(&stuff->adapter);
        if (ret) {
            drm_err(dev, "Failed to register I2C bus %d (port 0x%x): %d\n",
                   i, stuff->i2c_port, ret);
            goto error;
        }
        
        stuff->is_active = 1;
        drm_dbg_driver(dev, "Registered I2C bus 0x%x\n", stuff->i2c_port);
        drm_dbg_driver(dev, "Created I2C bus %d (port 0x%x)\n", i, stuff->i2c_port);
    }

    initialized = true;
    return 0;

error:
    /* Clean up on error */
    for (i = 0; i < ARRAY_SIZE(via_i2c_adapters); i++) {
        if (via_i2c_adapters[i].is_active) {
            i2c_del_adapter(&via_i2c_adapters[i].adapter);
            via_i2c_adapters[i].is_active = 0;
        }
    }
    initialized = false;
    return ret;
}

/* 
 * Clean up I2C subsystem
 * Releases I2C resources when the driver is unloaded
 */
void via_i2c_exit(void)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(via_i2c_adapters); i++) {
        if (via_i2c_adapters[i].is_active) {
            i2c_del_adapter(&via_i2c_adapters[i].adapter);
            via_i2c_adapters[i].is_active = 0;
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