/*
 * Copyright 2023 The OpenChrome Project
 * [via_mm.c]
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

#include <linux/module.h>
#include <drm/drm_managed.h>
#include <drm/drm_vma_manager.h>
/* Add TTM includes */
#include <drm/ttm/ttm_device.h>
#include <drm/ttm/ttm_range_manager.h>
#include <drm/ttm/ttm_placement.h>

#include "via_drv.h"
#include "via_mm.h"
#include "via_ttm_mod.h"

/* External TTM driver structure declaration */
extern struct ttm_device_funcs via_bo_driver;

/* Implementation of TTM memory manager initialization */
int via_mm_init(struct drm_device *dev)
{
    struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
    int ret;

    drm_dbg_driver(dev, "Entered %s.\n", __func__);

    /* Safety check for dev_priv */
    if (!dev_priv) {
        drm_err(dev, "NULL dev_priv in via_mm_init\n");
        return -EINVAL;
    }

    /* No TTM yet */
    dev_priv->ttm_initialized = false;

    /* Initialize VMA manager */
    drm_vma_offset_manager_init(&dev_priv->vma_manager, 0, 0);
    dev->vma_offset_manager = &dev_priv->vma_manager;

    /* Add resource manager for VRAM */
    ret = ttm_range_man_init(&dev_priv->bdev, TTM_PL_VRAM, false,
                 dev_priv->vram_size >> PAGE_SHIFT);
    if (ret) {
        drm_err(dev, "Failed to initialize VRAM memory manager: %d\n", ret);
        goto error_ttm_vram_mgr;
    }

    /* Add resource manager for System RAM */
    ret = ttm_range_man_init(&dev_priv->bdev, TTM_PL_SYSTEM, true, 0);
    if (ret) {
        drm_err(dev, "Failed to initialize System memory manager: %d\n", ret);
        goto error_ttm_system_mgr;
    }

    /* Initialize TTM device with proper parameters for kernel 6.12+ */
    ret = ttm_device_init(&dev_priv->bdev, &via_bo_driver, dev->dev,
                  dev->anon_inode->i_mapping,
                  dev->vma_offset_manager,
                  true, /* Enable swiotlb for 32-bit devices */
                  true);
    if (ret) {
        drm_err(dev, "Failed initializing buffer object driver: %d\n", ret);
        goto error_ttm_init;
    }

    dev_priv->ttm_initialized = true;
    drm_dbg_driver(dev, "TTM memory management initialized successfully\n");
    drm_dbg_driver(dev, "Exiting %s.\n", __func__);
    return 0;

error_ttm_init:
    ttm_range_man_fini(&dev_priv->bdev, TTM_PL_SYSTEM);
error_ttm_system_mgr:
    ttm_range_man_fini(&dev_priv->bdev, TTM_PL_VRAM);
error_ttm_vram_mgr:
    drm_vma_offset_manager_destroy(&dev_priv->vma_manager);
    drm_dbg_driver(dev, "Exiting %s with error: %d\n", __func__, ret);
    return ret;
}

void via_mm_fini(struct drm_device *dev)
{
    struct via_drm_priv *dev_priv = to_via_drm_priv(dev);

    drm_dbg_driver(dev, "Entered %s.\n", __func__);

    if (!dev_priv->ttm_initialized)
        goto exit;

    ttm_range_man_fini(&dev_priv->bdev, TTM_PL_VRAM);
    ttm_range_man_fini(&dev_priv->bdev, TTM_PL_SYSTEM);

    /* Clean up global TTM resources */
    if (dev_priv->ttm_initialized) {
        ttm_device_fini(&dev_priv->bdev);
        dev_priv->ttm_initialized = false;
    }

    /* Cleanup VMA manager */
    drm_vma_offset_manager_destroy(&dev_priv->vma_manager);

exit:
    drm_dbg_driver(dev, "Exiting %s.\n", __func__);
}
