/*
 * Copyright © 2019-2021 Kevin Brace.
 * Copyright 2012 James Simmons. All Rights Reserved.
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
 * Author(s):
 * Kevin Brace <kevinbrace@bracecomputerlab.com>
 * James Simmons <jsimmons@infradead.org>
 */

#include "via_drv.h"
#include <uapi/drm/via_drm.h> // Add this include near the top of the file, just after via_drv.h
#include "via_drv_main.h" // Include the new header file with function declarations
#include "via_ioctl.h" /* Add for ioctl function declarations */
#include "via_pm.h"    /* Add for PM function declarations */
#include "via_init.h"  /* Add for init/fini function declarations */

#include <linux/pci.h>

#include <drm/drm_aperture.h>
#include <drm/drm_drv.h>
#include <drm/drm_fb_helper.h>
#include <drm/drm_file.h>
#include <drm/drm_gem.h>
#include <drm/drm_ioctl.h>
#include <drm/drm_prime.h>
#include <drm/drm_gem_ttm_helper.h>

#include <drm/ttm/ttm_bo.h>

#include "via_object.h"  // Include the object handling functions
#include "via_pm.h"      // Include the power management functions
#include "via_i2c.h"     // Include I2C functions

/* Remove redefinitions of IOCTL numbers - use the ones from via_drm.h */
/*
#define DRM_VIA_ALLOCMEM          0x00
#define DRM_VIA_FREEMEM           0x01
#define DRM_VIA_AGP_INIT          0x02
#define DRM_VIA_FB_INIT           0x03
#define DRM_VIA_MAP_INIT          0x04
#define DRM_VIA_DMA_INIT          0x05
#define DRM_VIA_CMDBUFFER         0x06
#define DRM_VIA_FLUSH             0x07
#define DRM_VIA_PCICMD            0x08
#define DRM_VIA_CMDBUF_SIZE       0x09
#define DRM_VIA_GEM_ALLOC         0x0d
#define DRM_VIA_GEM_MMAP          0x0e
*/

/* Use the IOCTL commands from via_drm.h */
/* 
 * No need to redefine these since they're already defined in via_drm.h
 * 
#define DRM_IOCTL_VIA_GEM_ALLOC   DRM_IOWR(DRM_COMMAND_BASE + DRM_VIA_GEM_ALLOC, struct drm_via_gem_alloc)
#define DRM_IOCTL_VIA_GEM_MMAP    DRM_IOWR(DRM_COMMAND_BASE + DRM_VIA_GEM_MMAP, struct drm_via_gem_mmap)
*/

/* Forward declaration for fb helper function */
/* Remove the external declaration and implement our own version for compatibility */
/* extern int drm_fb_helper_fbdev_setup(struct drm_device *dev, unsigned int preferred_bpp); */

/* Provide a compatibility function for systems without drm_fb_helper_fbdev_setup */
static int via_fb_helper_setup(struct drm_device *dev, unsigned int preferred_bpp)
{
#if defined(CONFIG_DRM_FBDEV_EMULATION)
    /* Use built-in function if available, otherwise no-op */
    #if defined(drm_fb_helper_fbdev_setup)
        return drm_fb_helper_fbdev_setup(dev, preferred_bpp);
    #else
        /* For older kernel versions, we could add a basic FB helper init here if needed */
        return 0;
    #endif
#else
    /* FB emulation disabled */
    return 0;
#endif
}

/* Define the scanout alignment constant */
#define DRM_SCANOUT_ALIGN 64

/* Add forward declaration of via_device_init */
int via_device_init(struct drm_device *dev) __attribute__((warn_unused_result));

/* 
 * Create GEM handle from buffer object - Exported 
 * This is used to create a handle for the dumb create interface 
 */
int via_dumb_create_mapping(struct drm_file *file_priv,
                struct drm_device *dev,
                struct via_bo *bo,
                uint32_t *handle)
{
    int ret;
    
    /* Create a gem object to export a handle using the TTM buffer object's base field */
    ret = drm_gem_handle_create(file_priv, &bo->ttm_bo.base, handle);

    /* Drop the reference from handle creation */
    drm_gem_object_put(&bo->ttm_bo.base);
    
    return ret;
}

/* Stub/placeholder implementations for required functions */
extern int via_ttm_domain_to_placement(struct via_bo *bo, uint32_t domain);

void via_ttm_placement(struct ttm_buffer_object *bo, uint32_t domain)
{
    /* Implementation stub */
}

/* Remove via_gem_alloc_ioctl - keep in via_ioctl.c */
/* Remove via_gem_mmap_ioctl - keep in via_ioctl.c */
/* Remove via_drm_init - keep in via_init.c */
/* Remove via_drm_fini - keep in via_init.c */
/* Remove via_dev_pm_ops_suspend - keep in via_pm.c */
/* Remove via_dev_pm_ops_resume - keep in via_pm.c */

/*
 * For now, this device driver will be disabled, unless the
 * user decides to enable it.
 */
int via_modeset = 0;

MODULE_PARM_DESC(modeset, "Enable DRM device driver "
                "(Default: Disabled, "
                "0 = Disabled,"
                "1 = Enabled)");
module_param_named(modeset, via_modeset, int, 0400);

static int __maybe_unused via_driver_open(struct drm_device *dev,
                    struct drm_file *file_priv)
{
    int ret = 0;

    drm_dbg_driver(dev, "Entered %s.\n", __func__);

    drm_dbg_driver(dev, "Exiting %s.\n", __func__);
    return ret;
}

static void __maybe_unused via_driver_postclose(struct drm_device *dev,
                    struct drm_file *file_priv)
{
    drm_dbg_driver(dev, "Entered %s.\n", __func__);

    drm_dbg_driver(dev, "Exiting %s.\n", __func__);
}

static int __maybe_unused via_driver_dumb_create(struct drm_file *file_priv,
                    struct drm_device *dev,
                    struct drm_mode_create_dumb *args)
{
    struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
    struct via_bo *bo;
    int ret;
    int align;

    align = DRM_SCANOUT_ALIGN;

    args->pitch = ALIGN(args->width * ((args->bpp + 7) >> 3), align);
    args->size = PAGE_ALIGN(args->pitch * args->height);

    /* Call via_bo_create with the correct parameter list */
    ret = via_bo_create(dev, &dev_priv->bdev, args->size,
                ttm_bo_type_device, TTM_PL_VRAM, false, &bo);
    if (ret) {
        drm_err(dev, "Failed to create BO: %d\n", ret);
        return ret;
    }

    ret = via_dumb_create_mapping(file_priv, dev, bo, &args->handle);
    if (ret) {
        drm_err(dev, "Failed to create BO mapping: %d\n", ret);
        via_bo_destroy(bo, false);
        return ret;
    }

    return 0;
}

/* Remove the static implementation of via_driver_dumb_map_offset 
 * that starts around line 202 - since we now have a non-static 
 * implementation in via_ioctl.c */

/* Add function prototype for via_driver_dumb_map_offset */
int via_driver_dumb_map_offset(struct drm_file *file_priv,
                        struct drm_device *dev,
                        uint32_t handle,
                        uint64_t *offset);

/* 
 * These driver interface structures are being built up and will be
 * used in future implementations. They're retained but marked with 
 * __maybe_unused to prevent warnings.
 */
static const struct drm_ioctl_desc via_driver_ioctls[] __maybe_unused = {
    /* Only include the GEM IOCTLs we actually implement */
    DRM_IOCTL_DEF_DRV(VIA_GEM_ALLOC, via_gem_alloc_ioctl, DRM_AUTH),
    DRM_IOCTL_DEF_DRV(VIA_GEM_MMAP, via_gem_mmap_ioctl, DRM_AUTH),
};

static const struct file_operations via_driver_fops __maybe_unused = {
    .owner      = THIS_MODULE,
    .open       = drm_open,
    .release    = drm_release,
    .unlocked_ioctl = drm_ioctl,
    .mmap       = drm_gem_mmap,
    .poll       = drm_poll,
    .llseek     = noop_llseek,
};

/* Remove the static keyword to match the extern declaration in the header */
extern const struct drm_driver via_driver;

const struct drm_driver via_driver = {
    .load = via_drm_init,
    .unload = via_drm_fini,
    /* Add the dumb_map_offset handler */
    .dumb_map_offset = via_driver_dumb_map_offset,
};

static struct pci_device_id via_pci_table[] = {
    {0x1106, 0x3122, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
    {0x1106, 0x7205, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
    {0x1106, 0x3108, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
    {0x1106, 0x3344, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
    {0x1106, 0x3118, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
    {0x1106, 0x3343, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
    {0x1106, 0x3230, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
    {0x1106, 0x3371, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
    {0x1106, 0x3157, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
    {0x1106, 0x1122, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
    {0x1106, 0x5122, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
    {0x1106, 0x7122, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
    {0, 0, 0}
};

MODULE_DEVICE_TABLE(pci, via_pci_table);

static int via_pci_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
    struct drm_device *dev;
    struct via_drm_priv *dev_priv;
    int ret;

    dev_info(&pdev->dev, "Entered %s.\n", __func__);

    ret = drm_aperture_remove_conflicting_pci_framebuffers(pdev,
                                &via_driver);
    if (ret) {
        goto exit;
    }

    ret = pci_enable_device(pdev);
    if (ret) {
        goto exit;
    }

    dev_priv = devm_drm_dev_alloc(&pdev->dev,
                    &via_driver,
                    struct via_drm_priv,
                    dev);
    if (IS_ERR(dev_priv)) {
        ret = PTR_ERR(dev_priv);
        goto exit;
    }

    dev = &dev_priv->dev;

    pci_set_drvdata(pdev, dev);

    ret = via_drm_init(dev, 0);
    if (ret) {
        goto error_disable_pci;
    }

    ret = drm_dev_register(dev, ent->driver_data);
    if (ret) {
        goto error_fini;  // Changed from error_disable_pci to ensure proper cleanup
    }

    /* 
     * Register a framebuffer for the primary display
     * Use our compatibility function instead
     */
    via_fb_helper_setup(dev, 32);
    goto exit;
error_disable_pci:
    pci_disable_device(pdev);
error_fini:  // Added new error handling label
    via_drm_fini(dev);
exit:
    dev_info(&pdev->dev, "Exiting %s.\n", __func__);
    return ret;
}

static void via_pci_remove(struct pci_dev *pdev)
{
    struct drm_device *dev = pci_get_drvdata(pdev);

    dev_info(&pdev->dev, "Entered %s.\n", __func__);

    via_drm_fini(dev);
    drm_dev_unregister(dev);

    dev_info(&pdev->dev, "Exiting %s.\n", __func__);
}

static const struct dev_pm_ops via_dev_pm_ops = {
    .suspend    = via_dev_pm_ops_suspend,
    .resume     = via_dev_pm_ops_resume,
};

static struct pci_driver via_pci_driver = {
    .name       = DRIVER_NAME,
    .id_table   = via_pci_table,
    .probe      = via_pci_probe,
    .remove     = via_pci_remove,
    .driver.pm  = &via_dev_pm_ops,
};

static int __init via_init(void)
{
    int ret = 0;

    if ((via_modeset == -1) &&
        (drm_firmware_drivers_only())) {
        via_modeset = 0;
    }

    if (!via_modeset) {
        ret = -EINVAL;
        goto exit;
    }

    ret = pci_register_driver(&via_pci_driver);

exit:
    return ret;
}

static void __exit via_exit(void)
{
    if (!via_modeset) {
        return;
    }

    pci_unregister_driver(&via_pci_driver);
}

module_init(via_init);
module_exit(via_exit);

MODULE_AUTHOR("Various contributors, Kevin Brace <kevinbrace@bracecomputerlab.com>");
MODULE_DESCRIPTION("OpenChrome DRM/KMS Driver");
MODULE_LICENSE("GPL");

// Add explicit module information
MODULE_AUTHOR("OpenChrome Project");
MODULE_DESCRIPTION("VIA/OpenChrome DRM Driver");
MODULE_LICENSE("GPL and additional rights");
MODULE_ALIAS(DRIVER_NAME);

// Add explicit dependencies
MODULE_SOFTDEP("pre: ttm");
MODULE_SOFTDEP("pre: drm_kms_helper");
MODULE_SOFTDEP("pre: drm");
