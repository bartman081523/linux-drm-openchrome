// SPDX-License-Identifier: GPL-2.0-or-later
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) OR COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Author(s):
 *  Kevin Brace <kevinbrace@bracecomputerlab.com>
 *  James Simmons <jsimmons@infradead.org>
 */

#include <linux/pci.h>
#include <linux/module.h>
#include <linux/console.h>
#include <linux/delay.h>

#include <drm/drm_print.h>
#include <drm/drm_file.h>
#include <drm/drm_ioctl.h>
#include <drm/drm_drv.h>
#include <drm/drm_aperture.h>
#include <drm/drm_prime.h>
#include <drm/drm_mode_config.h>
#include <drm/drm_modeset_helper.h>
#include <drm/drm_gem.h>
#include <drm/drm_gem_ttm_helper.h>
#include <drm/drm_managed.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_fbdev_generic.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_atomic_state_helper.h>
#include <drm/drm_connector.h>
#include <drm/drm_modes.h>

#include "via_drv.h"

/*
 * By default, we now set via_modeset = 1 (enabled). This
 * eliminates the need to pass "via.modeset=1" in the kernel
 * command line or module parameters.
 */
static int via_modeset = 1;
MODULE_PARM_DESC(modeset,
				 "Enable DRM device driver (Default: Enabled, 0=Off, 1=On)");
module_param_named(modeset, via_modeset, int, 0400);

/*
 * The table of PCI device IDs recognized by this driver.
 * If a device ID is absent, the driver will not claim it.
 */
static struct pci_device_id via_pci_table[] = {
	/* CLE266 */
	{0x1106, 0x3122, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	/* KM400 / KN400 / P4M800 */
	{0x1106, 0x7205, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	{0x1106, 0x3108, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	{0x1106, 0x3344, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	{0x1106, 0x3118, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	/* K8M800 / PM800, etc. */
	{0x1106, 0x3343, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	{0x1106, 0x3230, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	{0x1106, 0x3371, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	{0x1106, 0x3157, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	/* P4M800CE / CN700 */
	{0x1106, 0x1122, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	/* P4M900 / VN896 / CN896 */
	{0x1106, 0x5122, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	/* CX700 / VX700 */
	{0x1106, 0x7122, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	{0, 0, 0}
};
MODULE_DEVICE_TABLE(pci, via_pci_table);

/* Forward declarations */
static int via_drm_init(struct drm_device *dev);
static void via_drm_fini(struct drm_device *dev);
static int via_dev_pm_ops_suspend(struct device *dev);
static int via_dev_pm_ops_resume(struct device *dev);

/*
 * The overall driver structure
 */
static const struct drm_driver via_driver = {
	.open = drm_open,
	.postclose = drm_release,  /* or custom if needed */
	.lastclose = drm_fb_helper_lastclose,

	.dumb_create      = drm_gem_ttm_dumb_create,
	.dumb_map_offset  = drm_gem_ttm_dumb_map_offset,
	.dumb_destroy     = drm_gem_dumb_destroy,

	/*
	 * For big-endian or unusual tiling setups, we might
	 * need a specialized .gem_prime_import_sg_table, etc.
	 * For now, the TTM default is used.
	 */

	.gem_create_object = drm_gem_ttm_create_object,

	/* DRM version details */
	.name      = DRIVER_NAME,
	.desc      = DRIVER_DESC,
	.date      = DRIVER_DATE,
	.major     = DRIVER_MAJOR,
	.minor     = DRIVER_MINOR,
	.patchlevel= DRIVER_PATCHLEVEL,

	.driver_features = DRIVER_GEM | DRIVER_MODESET | DRIVER_ATOMIC,
};

/*
 * PCI driver's probe / remove routines
 */
static int via_pci_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	struct drm_device *dev;
	int ret;

	dev_info(&pdev->dev, "%s: pdev %p, device ID = 0x%04X\n",
			 __func__, pdev, pdev->device);

	ret = drm_aperture_remove_conflicting_pci_framebuffers(pdev,
														   &via_driver);
	if (ret)
		return ret;

	ret = pci_enable_device(pdev);
	if (ret) {
		dev_err(&pdev->dev, "Failed to enable PCI device.\n");
		return ret;
	}

	dev = drm_dev_alloc(&via_driver, &pdev->dev);
	if (IS_ERR(dev)) {
		pci_disable_device(pdev);
		return PTR_ERR(dev);
	}
	pci_set_drvdata(pdev, dev);

	ret = via_drm_init(dev);
	if (ret) {
		dev_err(&pdev->dev, "Failed to initialize VIA DRM.\n");
		drm_dev_put(dev);
		pci_disable_device(pdev);
		return ret;
	}

	ret = drm_dev_register(dev, ent->driver_data);
	if (ret) {
		dev_err(&pdev->dev, "drm_dev_register() failed.\n");
		via_drm_fini(dev);
		drm_dev_put(dev);
		pci_disable_device(pdev);
		return ret;
	}

	drm_fbdev_generic_setup(dev, 32);
	return 0;
}

static void via_pci_remove(struct pci_dev *pdev)
{
	struct drm_device *dev = pci_get_drvdata(pdev);

	if (!dev)
		return;

	via_drm_fini(dev);
	drm_dev_unplug(dev);
	drm_dev_put(dev);

	pci_disable_device(pdev);
}

static const struct dev_pm_ops via_dev_pm_ops = {
	.suspend = via_dev_pm_ops_suspend,
	.resume  = via_dev_pm_ops_resume,
};

static struct pci_driver via_pci_driver = {
	.name     = DRIVER_NAME,
	.id_table = via_pci_table,
	.probe    = via_pci_probe,
	.remove   = via_pci_remove,
	.driver.pm= &via_dev_pm_ops,
};

/*
 * module init/exit
 */
static int __init via_init(void)
{
	int ret = 0;

	/* If user forcibly sets it to -1 and the system is restricted
	 * to firmware drivers, revert to enabling. */
	if ((via_modeset == -1) && drm_firmware_drivers_only())
		via_modeset = 1;

	/* Register the PCI driver for VIA Chrome. */
	ret = pci_register_driver(&via_pci_driver);

	return ret;
}

static void __exit via_exit(void)
{
	pci_unregister_driver(&via_pci_driver);
}

module_init(via_init);
module_exit(via_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

/*
 * Implementation details for the KMS device init/fini and PM ops
 */

static int via_device_init(struct drm_device *dev);
static void via_device_fini(struct drm_device *dev);

static int via_modeset_init(struct drm_device *dev);
static void via_modeset_fini(struct drm_device *dev);

static void via_graphics_unlock(struct drm_device *dev);
static void via_quirks_init(struct drm_device *dev);

/*
 * via_drm_init - sets up TTM, VRAM, MMIO, registers, and configures
 * the KMS mode setting capabilities.
 */
static int via_drm_init(struct drm_device *dev)
{
	int ret;

	drm_dbg_driver(dev, "[via_drm_init] Enter.\n");

	ret = via_device_init(dev);
	if (ret) {
		drm_err(dev, "Failed to initialize Chrome IGP device.\n");
		return ret;
	}

	ret = via_mm_init(dev);
	if (ret) {
		drm_err(dev, "Failed to initialize TTM.\n");
		via_device_fini(dev);
		return ret;
	}

	/*
	 * Extra step to get chipset revision, if needed.
	 */
	via_chip_revision_info(dev);

	ret = via_modeset_init(dev);
	if (ret) {
		drm_err(dev, "Failed to initialize KMS mode setting.\n");
		via_mm_fini(dev);
		via_device_fini(dev);
		return ret;
	}

	drm_dbg_driver(dev, "[via_drm_init] Exit.\n");
	return 0;
}

/*
 * via_drm_fini - tears down what was set up in via_drm_init
 */
static void via_drm_fini(struct drm_device *dev)
{
	drm_dbg_driver(dev, "[via_drm_fini] Enter.\n");

	via_modeset_fini(dev);
	via_mm_fini(dev);
	via_device_fini(dev);

	drm_dbg_driver(dev, "[via_drm_fini] Exit.\n");
}

/*
 * PM ops: for standby / resume
 */
int via_dev_pm_ops_suspend(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct drm_device *drm_dev = pci_get_drvdata(pdev);
	struct via_drm_priv *dev_priv = to_via_drm_priv(drm_dev);
	int ret;

	drm_dbg_driver(drm_dev, "[via_dev_pm_ops_suspend] Enter.\n");

	console_lock();

	if ((pdev->device == PCI_DEVICE_ID_VIA_CHROME9_HC3) ||
		(pdev->device == PCI_DEVICE_ID_VIA_CHROME9_HCM) ||
		(pdev->device == PCI_DEVICE_ID_VIA_CHROME9_HD)) {
		/* Save critical registers for those chipsets. */
		dev_priv->saved_sr14 = vga_rseq(VGABASE, 0x14);
	dev_priv->saved_sr66 = vga_rseq(VGABASE, 0x66);
	dev_priv->saved_sr67 = vga_rseq(VGABASE, 0x67);
	dev_priv->saved_sr68 = vga_rseq(VGABASE, 0x68);
	dev_priv->saved_sr69 = vga_rseq(VGABASE, 0x69);
	dev_priv->saved_sr6a = vga_rseq(VGABASE, 0x6a);
	dev_priv->saved_sr6b = vga_rseq(VGABASE, 0x6b);
	dev_priv->saved_sr6c = vga_rseq(VGABASE, 0x6c);
	dev_priv->saved_sr6d = vga_rseq(VGABASE, 0x6d);
	dev_priv->saved_sr6e = vga_rseq(VGABASE, 0x6e);
	dev_priv->saved_sr6f = vga_rseq(VGABASE, 0x6f);
		}

		dev_priv->saved_cr3b = vga_rcrt(VGABASE, 0x3b);
		dev_priv->saved_cr3c = vga_rcrt(VGABASE, 0x3c);
		dev_priv->saved_cr3d = vga_rcrt(VGABASE, 0x3d);
		dev_priv->saved_cr3e = vga_rcrt(VGABASE, 0x3e);
		dev_priv->saved_cr3f = vga_rcrt(VGABASE, 0x3f);

		console_unlock();

		ret = drm_mode_config_helper_suspend(drm_dev);
		if (ret) {
			drm_err(drm_dev, "Failed to prepare for suspend!\n");
			goto out_disable;
		}

		pci_save_state(pdev);
		pci_disable_device(pdev);

		out_disable:
		drm_dbg_driver(drm_dev, "[via_dev_pm_ops_suspend] Exit.\n");
		return ret;
}

int via_dev_pm_ops_resume(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct drm_device *drm_dev = pci_get_drvdata(pdev);
	struct via_drm_priv *dev_priv = to_via_drm_priv(drm_dev);
	void __iomem *regs = ioport_map(0x3c0, 100);
	u8 val;
	int ret;

	drm_dbg_driver(drm_dev, "[via_dev_pm_ops_resume] Enter.\n");

	ret = pci_enable_device(pdev);
	if (ret) {
		drm_err(drm_dev, "pci_enable_device() failed after resume!\n");
		return -EIO;
	}

	console_lock();

	val = ioread8(regs + 0x03);
	iowrite8(val | 0x01, regs + 0x03);
	val = ioread8(regs + 0x0C);
	iowrite8(val | 0x01, regs + 0x02);

	/* Unlock extended IO space. */
	iowrite8(0x10, regs + 0x04);
	iowrite8(0x01, regs + 0x05);

	/* Unlock CRTC register protect. */
	iowrite8(0x47, regs + 0x14);

	/* Enable MMIO. */
	iowrite8(0x1a, regs + 0x04);
	val = ioread8(regs + 0x05);
	iowrite8(val | 0x38, regs + 0x05);

	if ((pdev->device == PCI_DEVICE_ID_VIA_CHROME9_HC3) ||
		(pdev->device == PCI_DEVICE_ID_VIA_CHROME9_HCM) ||
		(pdev->device == PCI_DEVICE_ID_VIA_CHROME9_HD)) {
		vga_wseq(VGABASE, 0x14, dev_priv->saved_sr14);
	vga_wseq(VGABASE, 0x66, dev_priv->saved_sr66);
	vga_wseq(VGABASE, 0x67, dev_priv->saved_sr67);
	vga_wseq(VGABASE, 0x68, dev_priv->saved_sr68);
	vga_wseq(VGABASE, 0x69, dev_priv->saved_sr69);
	vga_wseq(VGABASE, 0x6a, dev_priv->saved_sr6a);
	vga_wseq(VGABASE, 0x6b, dev_priv->saved_sr6b);
	vga_wseq(VGABASE, 0x6c, dev_priv->saved_sr6c);
	vga_wseq(VGABASE, 0x6d, dev_priv->saved_sr6d);
	vga_wseq(VGABASE, 0x6e, dev_priv->saved_sr6e);
	vga_wseq(VGABASE, 0x6f, dev_priv->saved_sr6f);
		}

		vga_wcrt(VGABASE, 0x3b, dev_priv->saved_cr3b);
		vga_wcrt(VGABASE, 0x3c, dev_priv->saved_cr3c);
		vga_wcrt(VGABASE, 0x3d, dev_priv->saved_cr3d);
		vga_wcrt(VGABASE, 0x3e, dev_priv->saved_cr3e);
		vga_wcrt(VGABASE, 0x3f, dev_priv->saved_cr3f);

		console_unlock();

		ret = drm_mode_config_helper_resume(drm_dev);
		if (ret)
			drm_err(drm_dev, "Failed to do modeset after resume!\n");

	drm_dbg_driver(drm_dev, "[via_dev_pm_ops_resume] Exit.\n");
	return ret;
}

/*
 * The following are local "device init/fini" functions that set up VRAM,
 * map MMIO, etc.
 */

static int via_device_init(struct drm_device *dev)
{
	int ret;
	drm_dbg_driver(dev, "[via_device_init] Enter.\n");

	via_quirks_init(dev);

	/*
	 * Setup VRAM, MTRR, etc.
	 */
	ret = via_vram_init(dev);
	if (ret) {
		drm_err(dev, "Failed to initialize VRAM.\n");
		return ret;
	}

	ret = via_mmio_init(dev);
	if (ret) {
		via_vram_fini(dev);
		drm_err(dev, "Failed to initialize MMIO.\n");
		return ret;
	}

	via_graphics_unlock(dev);

	drm_dbg_driver(dev, "[via_device_init] Exit.\n");
	return 0;
}

static void via_device_fini(struct drm_device *dev)
{
	drm_dbg_driver(dev, "[via_device_fini] Enter.\n");

	via_mmio_fini(dev);
	via_vram_fini(dev);

	drm_dbg_driver(dev, "[via_device_fini] Exit.\n");
}

/* The mode-setting init/fini routines */
static int via_modeset_init(struct drm_device *dev)
{
	int ret;

	drm_dbg_driver(dev, "[via_modeset_init] Enter.\n");

	dev->mode_config.min_width    = 0;
	dev->mode_config.min_height   = 0;
	dev->mode_config.max_width    = 2048;
	dev->mode_config.max_height   = 4096;
	dev->mode_config.preferred_depth = 24;
	dev->mode_config.funcs = &via_drm_mode_config_funcs;

	ret = drmm_mode_config_init(dev);
	if (ret) {
		drm_err(dev, "drmm_mode_config_init() failed.\n");
		return ret;
	}

	via_i2c_reg_init(to_via_drm_priv(dev));
	ret = via_i2c_init(dev);
	if (ret)
		drm_err(dev, "Failed to initialize I2C.\n");

	/* CRTC init */
	{
		int i;
		for (i = 0; i < VIA_MAX_CRTC; i++) {
			ret = via_crtc_init(to_via_drm_priv(dev), i);
			if (ret) {
				drm_err(dev, "CRTC init %d failed.\n", i);
				goto out_err;
			}
		}
	}

	via_ext_dvi_probe(dev);
	via_tmds_probe(dev);
	via_lvds_probe(dev);
	via_dac_probe(dev);

	via_ext_dvi_init(dev);
	via_tmds_init(dev);
	via_dac_init(dev);
	via_lvds_init(dev);

	drm_mode_config_reset(dev);
	drm_kms_helper_poll_init(dev);

	drm_dbg_driver(dev, "[via_modeset_init] Exit.\n");
	return 0;
	out_err:
	via_i2c_exit();
	return ret;
}

static void via_modeset_fini(struct drm_device *dev)
{
	drm_dbg_driver(dev, "[via_modeset_fini] Enter.\n");
	drm_kms_helper_poll_fini(dev);
	drm_helper_force_disable_all(dev);
	via_i2c_exit();
	drm_dbg_driver(dev, "[via_modeset_fini] Exit.\n");
}

/*
 * Additional local "helpers" to handle device quirks, unlocking, etc.
 */
static void via_graphics_unlock(struct drm_device *dev)
{
	u8 temp;

	drm_dbg_driver(dev, "[via_graphics_unlock] Enter.\n");

	/* Enable VGA. */
	temp = vga_io_r(0x3c3);
	vga_io_w(0x3c3, temp | 0x01);

	/* Unlock seq ext. */
	svga_wseq_mask(VGABASE, 0x10, BIT(0), BIT(0));
	svga_wseq_mask(VGABASE, 0x1a, BIT(3), BIT(3));
	drm_dbg_driver(dev, "[via_graphics_unlock] Exit.\n");
}

static void via_quirks_init(struct drm_device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev->dev);
	struct via_drm_priv *dev_priv = to_via_drm_priv(dev);

	drm_dbg_driver(dev, "[via_quirks_init] Enter.\n");

	/* Identify special subvendor boards if needed */
	dev_priv->is_via_nanobook = false;
	dev_priv->is_quanta_il1 = false;
	dev_priv->is_samsung_nc20 = false;

	if ((pdev->device == PCI_DEVICE_ID_VIA_UNICHROME_PRO_II) &&
		(pdev->subsystem_vendor == 0x1509) &&
		(pdev->subsystem_device == 0x2d30)) {
		dev_priv->is_via_nanobook = true;
		} else if ((pdev->device == PCI_DEVICE_ID_VIA_CHROME9_HC3) &&
			(pdev->subsystem_vendor == 0x152d) &&
			(pdev->subsystem_device == 0x0771)) {
			dev_priv->is_quanta_il1 = true;
			} else if ((pdev->device == PCI_DEVICE_ID_VIA_CHROME9_HC3) &&
				(pdev->subsystem_vendor == 0x144d) &&
				(pdev->subsystem_device == 0xc04e)) {
				dev_priv->is_samsung_nc20 = true;
				}
				drm_dbg_driver(dev, "[via_quirks_init] Exit.\n");
}
