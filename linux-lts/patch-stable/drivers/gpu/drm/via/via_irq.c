/*
 * Copyright 2023 The OpenChrome Project
 * [via_irq.c]
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

/*
 * IRQ handling functionality for VIA/OpenChrome DRM driver
 * This is a stub implementation to satisfy module build requirements
 */

#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/module.h>
#include "via_drv.h"

/* Initialize IRQ functionality */
int via_irq_init(struct drm_device *dev)
{
    drm_dbg_driver(dev, "Entered %s.\n", __func__);
    
    /* No IRQ handling in this stub implementation */
    drm_dbg_driver(dev, "IRQ handling disabled in this build\n");
    
    drm_dbg_driver(dev, "Exiting %s.\n", __func__);
    return 0;
}

/* Clean up IRQ resources */
void via_irq_fini(struct drm_device *dev)
{
    drm_dbg_driver(dev, "Entered %s.\n", __func__);
    /* No resources to clean up in this stub */
    drm_dbg_driver(dev, "Exiting %s.\n", __func__);
}

/* These callbacks would handle the actual interrupts */
irqreturn_t via_irq_handler(int irq, void *arg)
{
    struct drm_device *dev = (struct drm_device *)arg;
    
    /* Just log that we received an interrupt but don't handle it */
    if (dev)
        drm_dbg_driver(dev, "IRQ received but not handled\n");
    
    return IRQ_NONE; /* No interrupts handled in stub implementation */
}
