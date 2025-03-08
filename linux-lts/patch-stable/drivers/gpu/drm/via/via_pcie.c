/*
 * Copyright 2023 The OpenChrome Project
 * [via_pcie.c]
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
 * PCIe interface functionality for VIA/OpenChrome DRM driver
 * This is a stub implementation to satisfy module build requirements
 */

#include <linux/pci.h>
#include <linux/module.h>
#include "via_drv.h"

/* 
 * Initialize PCIe functionality
 * This is a stub implementation
 */
int via_pcie_init(struct drm_device *dev)
{
    struct pci_dev *pdev = to_pci_dev(dev->dev);
    
    drm_dbg_driver(dev, "Entered %s.\n", __func__);
    
    /* Check if the device supports PCIe */
    if (pci_is_pcie(pdev)) {
        drm_dbg_driver(dev, "PCIe device detected\n");
    } else {
        drm_dbg_driver(dev, "Non-PCIe device detected\n");
    }
    
    drm_dbg_driver(dev, "Exiting %s.\n", __func__);
    return 0;
}

/*
 * Cleanup PCIe resources
 * This is a stub implementation
 */
void via_pcie_fini(struct drm_device *dev)
{
    drm_dbg_driver(dev, "Entered %s.\n", __func__);
    /* No resources to clean up in this stub */
    drm_dbg_driver(dev, "Exiting %s.\n", __func__);
}
