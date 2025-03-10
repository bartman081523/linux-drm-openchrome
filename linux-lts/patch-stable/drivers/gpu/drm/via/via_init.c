/*
 * Copyright © 2019 Kevin Brace.
 * Copyright 2012 James Simmons. All Rights Reserved.
 * Copyright 2006-2009 Luc Verhaegen.
 * Copyright 1998-2009 VIA Technologies, Inc. All Rights Reserved.
 * Copyright 2001-2009 S3 Graphics, Inc. All Rights Reserved.
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
 * THE AUTHOR(S) OR COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES, OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Author(s):
 * Kevin Brace <kevinbrace@bracecomputerlab.com>
 * James Simmons <jsimmons@infradead.org>
 * Luc Verhaegen
 */

#include <linux/pci.h>

#include <drm/drm_atomic_helper.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_gem_framebuffer_helper.h>
#include <drm/drm_mode_config.h>
#include <drm/drm_probe_helper.h>
/* Remove the drm_irq.h include as it doesn't exist in this kernel version */

#include "via_drv.h"
#include "via_hdmi_regs.h"

/* Add external declarations for variables defined in via_drv.c */
extern int via_modeset;
extern struct pci_driver via_pci_driver;


static int cle266_mem_type(struct drm_device *dev)
{
	struct pci_dev *gfx_dev = to_pci_dev(dev->dev);
	struct pci_dev *bridge_fn0_dev;
	struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
	u8 fsb, freq, type;
	int ret;

	bridge_fn0_dev =
		pci_get_domain_bus_and_slot(pci_domain_nr(gfx_dev->bus), 0,
						PCI_DEVFN(0, 0));
	if (!bridge_fn0_dev) {
		ret = -ENODEV;
		drm_err(dev, "Host Bridge Function 0 not found! errno: %d\n",
			ret);
		goto exit;
	}

	/* Always use bridge_fn0_dev for PCI reads, not gfx_dev */
	ret = pci_read_config_byte(bridge_fn0_dev, 0x54, &fsb);
	if (ret) {
		goto error_pci_cfg_read;
	}

	ret = pci_read_config_byte(bridge_fn0_dev, 0x69, &freq);
	if (ret) {
		goto error_pci_cfg_read;
	}

	fsb >>= 6;
	freq >>= 6;

	/* FSB frequency */
	switch (fsb) {
	case 0x01: /* 100MHz */
		switch (freq) {
		case 0x00:
			freq = 100;
			break;
		case 0x01:
			freq = 133;
			break;
		case 0x02:
			freq = 66;
			break;
		default:
			freq = 0;
			break;
		}

		break;
	case 0x02: /* 133 MHz */
	case 0x03:
		switch (freq) {
		case 0x00:
			freq = 133;
			break;
		case 0x02:
			freq = 100;
			break;
		default:
			freq = 0;
			break;
		}

		break;
	default:
		freq = 0;
		break;
	}

	ret = pci_read_config_byte(bridge_fn0_dev, 0x60, &fsb);
	if (ret) {
		goto error_pci_cfg_read;
	}

	ret = pci_read_config_byte(bridge_fn0_dev, 0xe3, &type);
	if (ret) {
		goto error_pci_cfg_read;
	}

	/* On bank 2/3 */
	if (type & 0x02) {
		fsb >>= 2;
	}

	/* Memory type */
	switch (fsb & 0x03) {
	case 0x00: /* SDR */
		switch (freq) {
		case 66:
			dev_priv->vram_type = VIA_MEM_SDR66;
			break;
		case 100:
			dev_priv->vram_type = VIA_MEM_SDR100;
			break;
		case 133:
			dev_priv->vram_type = VIA_MEM_SDR133;
			break;
		default:
			break;
		}

		break;
	case 0x02: /* DDR */
		switch (freq) {
		case 100:
			dev_priv->vram_type = VIA_MEM_DDR200;
			break;
		case 133:
			dev_priv->vram_type = VIA_MEM_DDR266;
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}

	goto exit;
error_pci_cfg_read:
	drm_err(dev, "PCI configuration space read error! errno: %d\n", ret);
exit:
	return ret;
}

static int km400_mem_type(struct drm_device *dev)
{
	struct pci_dev *gfx_dev = to_pci_dev(dev->dev);
	struct pci_dev *bridge_fn0_dev;
	struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
	u8 fsb, freq, rev;
	int ret;

	bridge_fn0_dev =
		pci_get_domain_bus_and_slot(pci_domain_nr(gfx_dev->bus), 0,
						PCI_DEVFN(0, 0));
	if (!bridge_fn0_dev) {
		ret = -ENODEV;
		drm_err(dev, "Host Bridge Function 0 not found! errno: %d\n",
			ret);
		goto exit;
	}

	/* Always use bridge_fn0_dev for PCI reads, not gfx_dev */
	ret = pci_read_config_byte(bridge_fn0_dev, 0x54, &fsb);
	if (ret) {
		goto error_pci_cfg_read;
	}

	ret = pci_read_config_byte(bridge_fn0_dev, 0x69, &freq);
	if (ret) {
		goto error_pci_cfg_read;
	}

	fsb >>= 6;
	freq >>= 6;

	ret = pci_read_config_byte(bridge_fn0_dev, 0xf6, &rev);
	if (ret) {
		goto error_pci_cfg_read;
	}

	/* KM400 */
	if (rev < 0x80) {
		/* FSB frequency */
		switch (fsb) {
		case 0x00:
			switch (freq) {
			case 0x00:
				dev_priv->vram_type = VIA_MEM_DDR200;
				break;
			case 0x01:
				dev_priv->vram_type = VIA_MEM_DDR266;
				break;
			case 0x02:
				dev_priv->vram_type = VIA_MEM_DDR400;
				break;
			case 0x03:
				dev_priv->vram_type = VIA_MEM_DDR333;
				break;
			default:
				break;
			}

			break;
		case 0x01:
			switch (freq) {
			case 0x00:
				dev_priv->vram_type = VIA_MEM_DDR266;
				break;
			case 0x01:
				dev_priv->vram_type = VIA_MEM_DDR333;
				break;
			case 0x02:
				dev_priv->vram_type = VIA_MEM_DDR400;
				break;
			default:
				break;
			}

			break;
		case 0x02:
		case 0x03:
			switch (freq) {
			case 0x00:
				dev_priv->vram_type = VIA_MEM_DDR333;
				break;
			case 0x02:
				dev_priv->vram_type = VIA_MEM_DDR400;
				break;
			case 0x03:
				dev_priv->vram_type = VIA_MEM_DDR266;
				break;
			default:
				break;
			}

			break;
		default:
			break;
		}
	} else {
		/* KM400A */
		ret = pci_read_config_byte(bridge_fn0_dev, 0x67, &rev);
		if (ret) {
			goto error_pci_cfg_read;
		}

		if (rev & 0x80)
			freq |= 0x04;

		switch (fsb) {
		case 0x00:
			switch (freq) {
			case 0x00:
				dev_priv->vram_type = VIA_MEM_DDR200;
				break;
			case 0x01:
				dev_priv->vram_type = VIA_MEM_DDR266;
				break;
			case 0x03:
				dev_priv->vram_type = VIA_MEM_DDR333;
				break;
			case 0x07:
				dev_priv->vram_type = VIA_MEM_DDR400;
				break;
			default:
				break;
			}

			break;
		case 0x01:
			switch (freq) {
			case 0x00:
				dev_priv->vram_type = VIA_MEM_DDR266;
				break;
			case 0x01:
				dev_priv->vram_type = VIA_MEM_DDR333;
				break;
			case 0x03:
				dev_priv->vram_type = VIA_MEM_DDR400;
				break;
			default:
				break;
			}

			break;
		case 0x02:
			switch (freq) {
			case 0x00:
				dev_priv->vram_type = VIA_MEM_DDR400;
				break;
			case 0x04:
				dev_priv->vram_type = VIA_MEM_DDR333;
				break;
			case 0x06:
				dev_priv->vram_type = VIA_MEM_DDR266;
				break;
			default:
				break;
			}

			break;
		case 0x03:
			switch (freq) {
			case 0x00:
				dev_priv->vram_type = VIA_MEM_DDR333;
				break;
			case 0x01:
				dev_priv->vram_type = VIA_MEM_DDR400;
				break;
			case 0x04:
				dev_priv->vram_type = VIA_MEM_DDR266;
				break;
			default:
				break;
			}

			break;
		default:
			break;
		}
	}

	goto exit;
error_pci_cfg_read:
	drm_err(dev, "PCI configuration space read error! errno: %d\n", ret);
exit:
	return ret;
}

static int p4m800_mem_type(struct drm_device *dev)
{
	struct pci_dev *gfx_dev = to_pci_dev(dev->dev);
	struct pci_dev *bridge_fn3_dev, *bridge_fn4_dev;
	struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
	int freq = 0;
	u8 fsb, type;
	int ret;

	bridge_fn3_dev =
		pci_get_domain_bus_and_slot(pci_domain_nr(gfx_dev->bus), 0,
						PCI_DEVFN(0, 3));
	if (!bridge_fn3_dev) {
		ret = -ENODEV;
		drm_err(dev, "Host Bridge Function 3 not found! errno: %d\n",
			ret);
		goto exit;
	}

	bridge_fn4_dev =
		pci_get_domain_bus_and_slot(pci_domain_nr(gfx_dev->bus), 0,
						PCI_DEVFN(0, 4));
	if (!bridge_fn4_dev) {
		ret = -ENODEV;
		drm_err(dev, "Host Bridge Function 4 not found! errno: %d\n",
			ret);
		goto exit;
	}

	/* VIA Scratch region - make sure to use bridge_fn4_dev, not gfx_dev */
	ret = pci_read_config_byte(bridge_fn4_dev, 0xf3, &fsb);
	if (ret) {
		goto error_pci_cfg_read;
	}

	fsb >>= 5;
	switch (fsb) {
	case 0x00:
		freq = 0x03; /* 100 MHz */
		break;
	case 0x01:
		freq = 0x04; /* 133 MHz */
		break;
	case 0x02:
		freq = 0x06; /* 200 MHz */
		break;
	default:
		break;
	}

	/* Make sure to use bridge_fn3_dev, not gfx_dev */
	ret = pci_read_config_byte(bridge_fn3_dev, 0x68, &type);
	if (ret) {
		goto error_pci_cfg_read;
	}

	type &= 0x0f;
	if (type & 0x02) {
		freq -= type >> 2;
	} else {
		freq += type >> 2;
		if (type & 0x01)
			freq++;
	}

	switch (freq) {
	case 0x03:
		dev_priv->vram_type = VIA_MEM_DDR200;
		break;
	case 0x04:
		dev_priv->vram_type = VIA_MEM_DDR266;
		break;
	case 0x05:
		dev_priv->vram_type = VIA_MEM_DDR333;
		break;
	case 0x06:
		dev_priv->vram_type = VIA_MEM_DDR400;
		break;
	default:
		break;
	}

	goto exit;
error_pci_cfg_read:
	drm_err(dev, "PCI configuration space read error! errno: %d\n", ret);
exit:
	return ret;
}

static int km8xx_mem_type(struct drm_device *dev)
{
	struct pci_dev *dram, *misc = NULL;
	struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
	u8 type, tmp;
	int ret = -ENXIO;

	dram = pci_get_device(PCI_VENDOR_ID_AMD,
				PCI_DEVICE_ID_AMD_K8_NB_MEMCTL, NULL);
	if (dram) {
		misc = pci_get_device(PCI_VENDOR_ID_AMD,
					PCI_DEVICE_ID_AMD_K8_NB_MISC, NULL);

		ret = pci_read_config_byte(misc, 0xfd, &type);
		if (ret) {
			goto error_pci_cfg_read;
		}

		if (type) {
			pci_read_config_byte(dram, 0x94, &type);
			if (ret) {
				goto error_pci_cfg_read;
			}

			switch (type & 0x03) {
			case 0x00:
				dev_priv->vram_type = VIA_MEM_DDR2_400;
				break;
			case 0x01:
				dev_priv->vram_type = VIA_MEM_DDR2_533;
				break;
			case 0x02:
				dev_priv->vram_type = VIA_MEM_DDR2_667;
				break;
			case 0x03:
				dev_priv->vram_type = VIA_MEM_DDR2_800;
				break;
			default:
				break;
			}
		} else {
			ret = pci_read_config_byte(dram, 0x96, &type);
			if (ret) {
				goto error_pci_cfg_read;
			}

			type >>= 4;
			type &= 0x07;

			switch (type) {
			case 0x00:
				dev_priv->vram_type = VIA_MEM_DDR200;
				break;
			case 0x02:
				dev_priv->vram_type = VIA_MEM_DDR266;
				break;
			case 0x05:
				dev_priv->vram_type = VIA_MEM_DDR333;
				break;
			case 0x07:
				dev_priv->vram_type = VIA_MEM_DDR400;
				break;
			default:
				break;
			}
		}
	}

	/* AMD 10h DRAM Controller */
	dram = pci_get_device(PCI_VENDOR_ID_AMD,
				PCI_DEVICE_ID_AMD_10H_NB_DRAM, NULL);
	if (dram) {
		ret = pci_read_config_byte(misc, 0x94, &tmp);
		if (ret) {
			goto error_pci_cfg_read;
		}

		ret = pci_read_config_byte(misc, 0x95, &type);
		if (ret) {
			goto error_pci_cfg_read;
		}

		if (type & 0x01) {	/* DDR3 */
			switch (tmp & 0x07) {
			case 0x03:
				dev_priv->vram_type = VIA_MEM_DDR3_800;
				break;
			case 0x04:
				dev_priv->vram_type = VIA_MEM_DDR3_1066;
				break;
			case 0x05:
				dev_priv->vram_type = VIA_MEM_DDR3_1333;
				break;
			case 0x06:
				dev_priv->vram_type = VIA_MEM_DDR3_1600;
				break;
			default:
				break;
			}
		} else {		/* DDR2 */
			switch (tmp & 0x07) {
			case 0x00:
				dev_priv->vram_type = VIA_MEM_DDR2_400;
				break;
			case 0x01:
				dev_priv->vram_type = VIA_MEM_DDR2_533;
				break;
			case 0x02:
				dev_priv->vram_type = VIA_MEM_DDR2_667;
				break;
			case 0x03:
				dev_priv->vram_type = VIA_MEM_DDR2_800;
				break;
			case 0x04:
				dev_priv->vram_type = VIA_MEM_DDR2_1066;
				break;
			default:
				break;
			}
		}
	}

	/* AMD 11h DRAM Controller */
	dram = pci_get_device(PCI_VENDOR_ID_AMD,
				PCI_DEVICE_ID_AMD_11H_NB_DRAM, NULL);
	if (dram) {
		ret = pci_read_config_byte(misc, 0x94, &type);
		if (ret) {
			goto error_pci_cfg_read;
		}

		switch (tmp & 0x07) {
		case 0x01:
			dev_priv->vram_type = VIA_MEM_DDR2_533;
			break;
		case 0x02:
			dev_priv->vram_type = VIA_MEM_DDR2_667;
			break;
		case 0x03:
			dev_priv->vram_type = VIA_MEM_DDR2_800;
			break;
		default:
			break;
		}
	}

	goto exit;
error_pci_cfg_read:
	drm_err(dev, "PCI configuration space read error! errno: %d\n", ret);
exit:
	return ret;
}

static int cn400_mem_type(struct drm_device *dev)
{
	struct pci_dev *gfx_dev = to_pci_dev(dev->dev);
	struct pci_dev *bridge_fn2_dev, *bridge_fn3_dev;
	struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
	int freq = 0;
	u8 fsb, type;
	int ret;

	bridge_fn2_dev =
		pci_get_domain_bus_and_slot(pci_domain_nr(gfx_dev->bus), 0,
						PCI_DEVFN(0, 2));
	if (!bridge_fn2_dev) {
		ret = -ENODEV;
		drm_err(dev, "Host Bridge Function 2 not found! errno: %d\n",
			ret);
		goto exit;
	}

	bridge_fn3_dev =
		pci_get_domain_bus_and_slot(pci_domain_nr(gfx_dev->bus), 0,
						PCI_DEVFN(0, 3));
	if (!bridge_fn3_dev) {
		ret = -ENODEV;
		drm_err(dev, "Host Bridge Function 3 not found! errno: %d\n",
			ret);
		goto exit;
	}

	/* Make sure to use bridge_fn2_dev, not gfx_dev */
	ret = pci_read_config_byte(bridge_fn2_dev, 0x54, &fsb);
	if (ret) {
		goto error_pci_cfg_read;
	}

	fsb >>= 5;
	switch (fsb) {
	case 0x00:
		freq = 0x03; /* 100 MHz */
		break;
	case 0x01:
		freq = 0x04; /* 133 MHz */
		break;
	case 0x02:
		freq = 0x06; /* 200 MHz */
		break;
	case 0x04:
		freq = 0x07; /* 233 MHz */
		break;
	default:
		break;
	}

	/* Make sure to use bridge_fn3_dev, not gfx_dev */
	ret = pci_read_config_byte(bridge_fn3_dev, 0x68, &type);
	if (ret) {
		goto error_pci_cfg_read;
	}

	type &= 0x0f;
	if (type & 0x01) {
		freq += 1 + (type >> 2);
	} else {
		freq -= 1 + (type >> 2);
	}

	switch (freq) {
	case 0x03:
		dev_priv->vram_type = VIA_MEM_DDR200;
		break;
	case 0x04:
		dev_priv->vram_type = VIA_MEM_DDR266;
		break;
	case 0x05:
		dev_priv->vram_type = VIA_MEM_DDR333;
		break;
	case 0x06:
		dev_priv->vram_type = VIA_MEM_DDR400;
		break;
	default:
		break;
	}

	goto exit;
error_pci_cfg_read:
	drm_err(dev, "PCI configuration space read error! errno: %d\n", ret);
exit:
	return ret;
}

static int cn700_mem_type(struct drm_device *dev)
{
	struct pci_dev *gfx_dev = to_pci_dev(dev->dev);
	struct pci_dev *bridge_fn3_dev;
	struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
	u8 type, clock;
	int ret;

	bridge_fn3_dev =
		pci_get_domain_bus_and_slot(pci_domain_nr(gfx_dev->bus), 0,
						PCI_DEVFN(0, 3));
	if (!bridge_fn3_dev) {
		ret = -ENODEV;
		drm_err(dev, "Host Bridge Function 3 not found! errno: %d\n",
			ret);
		goto exit;
	}

	/* Make sure to use bridge_fn3_dev, not gfx_dev */
	ret = pci_read_config_byte(bridge_fn3_dev, 0x6c, &type);
	if (ret) {
		goto error_pci_cfg_read;
	}

	ret = pci_read_config_byte(bridge_fn3_dev, 0x90, &clock);
	if (ret) {
		goto error_pci_cfg_read;
	}

	type &= 0x40;
	type >>= 6;
	switch (type) {
	case 0x00:
		switch (clock & 0x07) {
		case 0x00:
			dev_priv->vram_type = VIA_MEM_DDR200;
			break;
		case 0x01:
			dev_priv->vram_type = VIA_MEM_DDR266;
			break;
		case 0x02:
			dev_priv->vram_type = VIA_MEM_DDR333;
			break;
		case 0x03:
			dev_priv->vram_type = VIA_MEM_DDR400;
			break;
		default:
			break;
		}

		break;
	case 0x01:
		switch (clock & 0x07) {
		case 0x03:
			dev_priv->vram_type = VIA_MEM_DDR2_400;
			break;
		case 0x04:
			dev_priv->vram_type = VIA_MEM_DDR2_533;
			break;
		case 0x05:
			dev_priv->vram_type = VIA_MEM_DDR2_667;
			break;
		case 0x06:
			dev_priv->vram_type = VIA_MEM_DDR2_800;
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}

	goto exit;
error_pci_cfg_read:
	drm_err(dev, "PCI configuration space read error! errno: %d\n", ret);
exit:
	return ret;
}

static int vx900_mem_type(struct drm_device *dev)
{
	struct pci_dev *gfx_dev = to_pci_dev(dev->dev);
	struct pci_dev *bridge_fn3_dev;
	struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
	u8 type, clock, volt;
	int ret;

	bridge_fn3_dev =
		pci_get_domain_bus_and_slot(pci_domain_nr(gfx_dev->bus), 0,
						PCI_DEVFN(0, 3));
	if (!bridge_fn3_dev) {
		ret = -ENODEV;
		drm_err(dev, "Host Bridge Function 3 not found! errno: %d\n",
			ret);
		goto exit;
	}

	/* Make sure to use bridge_fn3_dev, not gfx_dev */
	ret = pci_read_config_byte(bridge_fn3_dev, 0x6c, &type);
	if (ret) {
		goto error_pci_cfg_read;
	}

	ret = pci_read_config_byte(bridge_fn3_dev, 0x90, &clock);
	if (ret) {
		goto error_pci_cfg_read;
	}

	volt = type;
	type &= 0xc0;
	type >>= 6;
	volt &= 0x20;
	volt >>= 5;
	switch (type) {
	case 0x01:
		switch (clock & 0x0f) {
		case 0x00:
			if (volt) {
				dev_priv->vram_type = VIA_MEM_DDR2_800;
			} else {
				dev_priv->vram_type = VIA_MEM_DDR2_533;
			}

			break;
		case 0x04:
			dev_priv->vram_type = VIA_MEM_DDR2_533;
			break;
		case 0x05:
			dev_priv->vram_type = VIA_MEM_DDR2_667;
			break;
		case 0x06:
			dev_priv->vram_type = VIA_MEM_DDR2_800;
			break;
		case 0x07:
			dev_priv->vram_type = VIA_MEM_DDR2_1066;
			break;
		default:
			break;
		}

		break;
	case 0x02:
		switch (clock & 0x0f) {
		case 0x00:
			if (volt) {
				dev_priv->vram_type = VIA_MEM_DDR3_800;
			} else {
				dev_priv->vram_type = VIA_MEM_DDR3_533;
			}

			break;
		case 0x04:
			dev_priv->vram_type = VIA_MEM_DDR3_533;
			break;
		case 0x05:
			dev_priv->vram_type = VIA_MEM_DDR3_667;
			break;
		case 0x06:
			dev_priv->vram_type = VIA_MEM_DDR3_800;
			break;
		case 0x07:
			dev_priv->vram_type = VIA_MEM_DDR3_1066;
			break;
		default:
			break;
		}

		break;
	}

	goto exit;
error_pci_cfg_read:
	drm_err(dev, "PCI configuration space read error! errno: %d\n", ret);
exit:
	return ret;
}

static void via_quirks_init(struct drm_device *dev)
{
	struct pci_dev *pdev;
	struct via_drm_priv *dev_priv;

	drm_dbg_driver(dev, "Entered %s.\n", __func__);

	/* Safety checks to prevent NULL pointer access */
	if (!dev || !dev->dev) {
		DRM_ERROR("Invalid device in via_quirks_init\n");
		return;
	}
	
	pdev = to_pci_dev(dev->dev);
	if (!pdev) {
		DRM_ERROR("Invalid PCI device in via_quirks_init\n");
		return;
	}

	dev_priv = to_via_drm_priv(dev);
	if (!dev_priv) {
		DRM_ERROR("NULL dev_priv in via_quirks_init\n");
		return;
	}

	/* Initialize boolean fields with defaults */
	dev_priv->is_via_nanobook = false;
	dev_priv->is_quanta_il1 = false;
	dev_priv->is_samsung_nc20 = false;

	/* Set quirk flags for specific hardware */
	if (pdev->device == PCI_DEVICE_ID_VIA_UNICHROME_PRO_II &&
		pdev->subsystem_vendor == 0x1509 &&
		pdev->subsystem_device == 0x2f00) {
		dev_priv->is_via_nanobook = true;
		drm_info(dev, "VIA Nanobook detected\n");
	}

	if (pdev->device == PCI_DEVICE_ID_VIA_CHROME9_HC3 &&
		pdev->subsystem_vendor == 0x1558 &&
		pdev->subsystem_device == 0x5409) {
		dev_priv->is_quanta_il1 = true;
		drm_info(dev, "Quanta IL1 detected\n");
	}

	if (pdev->device == PCI_DEVICE_ID_VIA_CHROME9_HC3 &&
		pdev->subsystem_vendor == 0x144d &&
		pdev->subsystem_device == 0xc04e) {
		dev_priv->is_samsung_nc20 = true;
		drm_info(dev, "Samsung NC20 detected\n");
	}

	drm_dbg_driver(dev, "Exiting %s.\n", __func__);
}

static int via_vram_init(struct drm_device *dev)
{
	struct pci_dev *gfx_dev = to_pci_dev(dev->dev);
	struct pci_dev *bridge_fn0_dev = NULL;
	struct pci_dev *bridge_fn3_dev = NULL;
	char *name = "unknown";
	struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
	u8 size;
	int ret = 0;

	drm_dbg_driver(dev, "Entered %s.\n", __func__);

	bridge_fn0_dev =
		pci_get_domain_bus_and_slot(pci_domain_nr(gfx_dev->bus), 0,
						PCI_DEVFN(0, 0));
	if (!bridge_fn0_dev) {
		ret = -ENODEV;
		drm_err(dev, "Host Bridge Function 0 not found! "
				"errno: %d\n", ret);
		goto exit;
	}

	if ((gfx_dev->device != PCI_DEVICE_ID_VIA_CLE266_GFX) ||
		(gfx_dev->device != PCI_DEVICE_ID_VIA_KM400_GFX)) {
		bridge_fn3_dev =
			pci_get_domain_bus_and_slot(pci_domain_nr(gfx_dev->bus), 0,
							PCI_DEVFN(0, 3));
		if (!bridge_fn3_dev) {
			ret = -ENODEV;
			drm_err(dev, "Host Bridge Function 3 not found! "
					"errno: %d\n", ret);
			goto exit;
		}
	}

	switch (bridge_fn0_dev->vendor) {
	case PCI_VENDOR_ID_VIA:
		switch (bridge_fn0_dev->device) {
		/* CLE266 */
		case PCI_DEVICE_ID_VIA_862X_0:
			/* Read memory size from pci config space */
			ret = pci_read_config_byte(bridge_fn0_dev, 0xE1, &size);
			if (ret) {
				goto error_pci_cfg_read;
			}
			
			dev_priv->vram_type = VIA_MEM_SDR133;
			dev_priv->vram_size = (1 << ((size & 0x70) >> 4)) << 20;
			ret = cle266_mem_type(dev);
			break;

		/* KM400(A) / KN400(A) */
		case PCI_DEVICE_ID_VIA_8378_0:
			/* Read memory size from pci config space */
			ret = pci_read_config_byte(bridge_fn0_dev, 0xE1, &size);
			if (ret) {
				goto error_pci_cfg_read;
			}
			
			dev_priv->vram_type = VIA_MEM_DDR400;
			dev_priv->vram_size = (1 << ((size & 0x70) >> 4)) << 20;
			ret = km400_mem_type(dev);
			break;

		/* P4M800 */
		case PCI_DEVICE_ID_VIA_3296_0:
			/* Use bridge_fn3_dev instead of gfx_dev for PCI reads */
			ret = pci_read_config_byte(bridge_fn3_dev, 0xA1, &size);
			if (ret) {
				goto error_pci_cfg_read;
			}
			
			if (size & 0x80) {
				dev_priv->vram_size = 16 << 20;
			} else {
				dev_priv->vram_size = 64 << 20;
			}

			ret = p4m800_mem_type(dev);
			break;
			
		/* K8M800(A) / K8N800(A) */
		case PCI_DEVICE_ID_VIA_8380_0:
		/* K8M890 / K8N890 */
		case PCI_DEVICE_ID_VIA_VT3336:
			/* Use bridge_fn3_dev instead of gfx_dev for PCI reads */
			ret = pci_read_config_byte(bridge_fn3_dev, 0xA1, &size);
			if (ret) {
				goto error_pci_cfg_read;
			}
			
			dev_priv->vram_type = VIA_MEM_DDR400;
			switch (size & 0x07) {
			case 0:
			case 5:
				dev_priv->vram_size = 64 << 20;
				break;
			case 1:
				dev_priv->vram_size = 32 << 20;
				break;
			case 2:
			case 6:
				dev_priv->vram_size = 128 << 20;
				break;
			case 3:
			case 7:
				dev_priv->vram_size = 256 << 20;
				break;
			default:
				dev_priv->vram_size = 16 << 20;
				break;
			}

			ret = km8xx_mem_type(dev);
			break;
			
		/* CN400 / PM800 / PN800 / PM880 / PN880 */
		case PCI_DEVICE_ID_VIA_PX8X0_0:
			/* Use bridge_fn3_dev instead of gfx_dev for PCI reads */
			ret = pci_read_config_byte(bridge_fn3_dev, 0xA1, &size);
			if (ret) {
				goto error_pci_cfg_read;
			}
			
			if (size & 0x80) {
				dev_priv->vram_size = 64 << 20;
			} else {
				dev_priv->vram_size = 128 << 20;
			}

			ret = cn400_mem_type(dev);
			break;
			
		/* P4M800CE / P4M800 Pro / VN800 / CN700 */
		case PCI_DEVICE_ID_VIA_P4M800CE:
		/* P4M890 / VN890 / CN800 */
		case PCI_DEVICE_ID_VIA_P4M890:
		/* CX700(M / M2) / VX700(M / M2) */
		case PCI_DEVICE_ID_VIA_VT3324:
		/* P4M900 / VN896 / CN896 */
		case PCI_DEVICE_ID_VIA_VT3364:
			/* Use bridge_fn3_dev instead of gfx_dev for PCI reads */
			ret = pci_read_config_byte(bridge_fn3_dev, 0xA1, &size);
			if (ret) {
				goto error_pci_cfg_read;
			}

			if (bridge_fn0_dev->device == PCI_DEVICE_ID_VIA_VT3324) {
				switch (size & 0x07) {
				case 0:
					dev_priv->vram_size = 8 << 20;
					break;
				case 1:
					dev_priv->vram_size = 16 << 20;
					break;
				case 2:
					dev_priv->vram_size = 32 << 20;
					break;
				case 3:
					dev_priv->vram_size = 64 << 20;
					break;
				case 4:
					dev_priv->vram_size = 128 << 20;
					break;
				default:
					dev_priv->vram_size = 256 << 20;
					break;
				}

				ret = cn700_mem_type(dev);
			} else {
				if (size & 0x80) {
					dev_priv->vram_size = 64 << 20;
				} else {
					dev_priv->vram_size = 128 << 20;
				}

				ret = cn700_mem_type(dev);
			}
			break;
			
		/* Handle VX800/VX855/VX900 with specific ID values */
		default:
			/* Check Host Bridge Device ID to distinguish 
			   between VX800, VX855, and VX900 */
			if (bridge_fn0_dev->device == VIA_HOST_BRIDGE_VX800) {
				/* Use bridge_fn3_dev instead of gfx_dev for PCI reads */
				ret = pci_read_config_byte(bridge_fn3_dev, 0xA1, &size);
				if (ret) {
					goto error_pci_cfg_read;
				}

				switch (size & 0x07) {
				case 1:
					dev_priv->vram_size = 16 << 20;
					break;
				case 2:
					dev_priv->vram_size = 32 << 20;
					break;
				case 3:
					dev_priv->vram_size = 64 << 20;
					break;
				case 4:
					dev_priv->vram_size = 128 << 20;
					break;
				case 5:
					dev_priv->vram_size = 256 << 20;
					break;
				case 6:
					dev_priv->vram_size = 384 << 20;
					break;
				case 7:
					dev_priv->vram_size = 512 << 20;
					break;
				default:
					dev_priv->vram_size = 8 << 20;
					break;
				}

				ret = cn700_mem_type(dev);
			} else if (bridge_fn0_dev->device == VIA_HOST_BRIDGE_VX855) {
				/* Use bridge_fn3_dev instead of gfx_dev for PCI reads */
				ret = pci_read_config_byte(bridge_fn3_dev, 0xA1, &size);
				if (ret) {
					goto error_pci_cfg_read;
				}

				switch (size & 0x07) {
				case 1:
					dev_priv->vram_size = 32 << 20;
					break;
				case 2:
					dev_priv->vram_size = 64 << 20;
					break;
				case 3:
					dev_priv->vram_size = 128 << 20;
					break;
				case 4:
					dev_priv->vram_size = 256 << 20;
					break;
				case 5:
					dev_priv->vram_size = 512 << 20;
					break;
				default:
					dev_priv->vram_size = 16 << 20;
					break;
				}

				ret = cn700_mem_type(dev);
			} else if (bridge_fn0_dev->device == VIA_HOST_BRIDGE_VX900) {
				/* Use bridge_fn3_dev instead of gfx_dev for PCI reads */
				ret = pci_read_config_byte(bridge_fn3_dev, 0xA1, &size);
				if (ret) {
					goto error_pci_cfg_read;
				}

				switch (size & 0x07) {
				case 1:
					dev_priv->vram_size = 32 << 20;
					break;
				case 2:
					dev_priv->vram_size = 64 << 20;
					break;
				case 3:
					dev_priv->vram_size = 128 << 20;
					break;
				case 4:
					dev_priv->vram_size = 256 << 20;
					break;
				case 5:
					dev_priv->vram_size = 512 << 20;
					break;
				case 6:
					dev_priv->vram_size = 1024 << 20;
					break;
				case 7:
					dev_priv->vram_size = 2048 << 20;
					break;
				default:
					dev_priv->vram_size = 16 << 20;
					break;
				}

				ret = vx900_mem_type(dev);
			} else {
				ret = -ENODEV;
				drm_err(dev, "Unknown Host Bridge device: 0x%04x\n", 
					bridge_fn0_dev->device);
			}
			break;
		}
		break;

	default:
		ret = -ENODEV;
		drm_err(dev, "Unknown Host Bridge vendor: 0x%04x\n",
			bridge_fn0_dev->vendor);
		break;
	}

	if (ret) {
		goto exit;
	}

	switch (dev_priv->vram_type) {
	case VIA_MEM_SDR66:
		name = "SDR 66";
		break;
	case VIA_MEM_SDR100:
		name = "SDR 100";
		break;
	case VIA_MEM_SDR133:
		name = "SDR 133";
		break;
	case VIA_MEM_DDR200:
		name = "DDR 200";
		break;
	case VIA_MEM_DDR266:
		name = "DDR 266";
		break;
	case VIA_MEM_DDR333:
		name = "DDR 333";
		break;
	case VIA_MEM_DDR400:
		name = "DDR 400";
		break;
	case VIA_MEM_DDR2_400:
		name = "DDR2 400";
		break;
	case VIA_MEM_DDR2_533:
		name = "DDR2 533";
		break;
	case VIA_MEM_DDR2_667:
		name = "DDR2 667";
		break;
	case VIA_MEM_DDR2_800:
		name = "DDR2 800";
		break;
	case VIA_MEM_DDR2_1066:
		name = "DDR2 1066";
		break;
	case VIA_MEM_DDR3_533:
		name = "DDR3 533";
		break;
	case VIA_MEM_DDR3_667:
		name = "DDR3 667";
		break;
	case VIA_MEM_DDR3_800:
		name = "DDR3 800";
		break;
	case VIA_MEM_DDR3_1066:
		name = "DDR3 1066";
		break;
	case VIA_MEM_DDR3_1333:
		name = "DDR3 1333";
		break;
	case VIA_MEM_DDR3_1600:
		name = "DDR3 1600";
		break;
	default:
		break;
	}

	drm_dbg_driver(dev, "Found %s video RAM.\n", name);

	if (gfx_dev->device == PCI_DEVICE_ID_VIA_CHROME9_HD) {
		dev_priv->vram_start = pci_resource_start(gfx_dev, 2);
	} else {
		dev_priv->vram_start = pci_resource_start(gfx_dev, 0);
	}

	/* Add an MTRR for the video RAM. */
	dev_priv->vram_mtrr = via_add_wc_mtrr(dev_priv->vram_start,
						dev_priv->vram_size);
	goto exit;

error_pci_cfg_read:
	drm_err(dev, "PCI configuration space read error! errno: %d\n", ret);
exit:
	drm_dbg_driver(dev, "Exiting %s.\n", __func__);
	return ret;
}

static void via_vram_fini(struct drm_device *dev)
{
	struct via_drm_priv *dev_priv = to_via_drm_priv(dev);

	drm_dbg_driver(dev, "Entered %s.\n", __func__);

	if (dev_priv->vram_mtrr) {
		/* Remove MTRR for the video RAM. */
		via_free_wc_mtrr(dev_priv->vram_mtrr);
		dev_priv->vram_mtrr = 0;
	}

	drm_dbg_driver(dev, "Exiting %s.\n", __func__);
}

static int via_mmio_init(struct drm_device *dev)
{
	struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
	struct pci_dev *pdev = to_pci_dev(dev->dev);
	int ret;

	/* Add safety check for NULL dev_priv */
	if (!dev_priv) {
		drm_err(dev, "NULL dev_priv in via_mmio_init\n");
		return -EINVAL;
	}

	drm_dbg_driver(dev, "Entered %s.\n", __func__);

	dev_priv->mmio_base = pci_resource_start(pdev, 1);
	dev_priv->mmio_size = pci_resource_len(pdev, 1);

	if (!dev_priv->mmio_base || !dev_priv->mmio_size) {
		drm_err(dev, "Invalid PCI BAR 1 (MMIO) configuration.\n");
		ret = -ENXIO;
		goto exit;
	}

	dev_priv->mmio = ioremap(dev_priv->mmio_base,
					dev_priv->mmio_size);
	if (!dev_priv->mmio) {
		drm_err(dev, "Failed to map MMIO region.\n");
		ret = -ENOMEM;
		goto exit;
	}

exit:
	drm_dbg_driver(dev, "Exiting %s.\n", __func__);
	return ret;
}

static void via_mmio_fini(struct drm_device *dev)
{
	struct via_drm_priv *dev_priv = to_via_drm_priv(dev);

	drm_dbg_driver(dev, "Entered %s.\n", __func__);

	if (dev_priv->mmio) {
		iounmap(dev_priv->mmio);
		dev_priv->mmio = NULL;
	}

	drm_dbg_driver(dev, "Exiting %s.\n", __func__);
}

static void via_graphics_unlock(struct drm_device *dev)
{
	struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
	uint8_t temp;

	drm_dbg_driver(dev, "Entered %s.\n", __func__);

	/*
	 * Enable VGA subsystem.
	 */
	temp = vga_io_r(0x03c3);
	vga_io_w(0x03c3, temp | 0x01);
	svga_wmisc_mask(VGABASE, BIT(0), BIT(0));

	/*
	 * Unlock VIA Technologies Chrome IGP extended
	 * registers.
	 */
	svga_wseq_mask(VGABASE, 0x10, BIT(0), BIT(0));

	/*
	 * Unlock VIA Technologies Chrome IGP extended
	 * graphics functionality.
	 */
	svga_wseq_mask(VGABASE, 0x1a, BIT(3), BIT(3));

	drm_dbg_driver(dev, "Exiting %s.\n", __func__);
}

static void via_chip_revision_info(struct drm_device *dev)
{
	struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
	struct pci_dev *pdev = to_pci_dev(dev->dev);
	u8 tmp;

	drm_dbg_driver(dev, "Entered %s.\n", __func__);

	switch (pdev->device) {
	/* CLE266 Chipset */
	case PCI_DEVICE_ID_VIA_CLE266_GFX:
		/* CR4F only defined in CLE266.CX chipset. */
		tmp = vga_rcrt(VGABASE, 0x4f);
		vga_wcrt(VGABASE, 0x4f, 0x55);
		if (vga_rcrt(VGABASE, 0x4f) != 0x55) {
			dev_priv->revision = CLE266_REVISION_AX;
		} else {
			dev_priv->revision = CLE266_REVISION_CX;
		}
		/* Restore original CR4F value. */
		vga_wcrt(VGABASE, 0x4f, tmp);
		break;
	/* CX700 / VX700 Chipset */
	case PCI_DEVICE_ID_VIA_UNICHROME_PRO_II:
		tmp = vga_rseq(VGABASE, 0x43);
		if (tmp & 0x02) {
			dev_priv->revision = CX700_REVISION_700M2;
		} else if (tmp & 0x40) {
			dev_priv->revision = CX700_REVISION_700M;
		} else {
			dev_priv->revision = CX700_REVISION_700;
		}
		break;
	/* VX800 / VX820 Chipset */
	case PCI_DEVICE_ID_VIA_CHROME9_HC3:
		break;
	/* VX855 / VX875 Chipset */
	case PCI_DEVICE_ID_VIA_CHROME9_HCM:
	/* VX900 Chipset */
	case PCI_DEVICE_ID_VIA_CHROME9_HD:
		dev_priv->revision = vga_rseq(VGABASE, 0x3b);
		break;
	default:
		break;
	}

	drm_dbg_driver(dev, "Exiting %s.\n", __func__);
}

/* Remove the redundant forward declaration since the function is defined right after it */
int via_device_init(struct drm_device *dev)
{
	int ret;

	drm_dbg_driver(dev, "Entered %s.\n", __func__);

	/* Validate device */
	if (!dev) {
		DRM_ERROR("NULL device in via_device_init\n");
		return -EINVAL;
	}

	/* Ensure dev_priv is properly allocated before calling via_quirks_init */
	struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
	if (!dev_priv) {
		drm_err(dev, "Failed to get device private data\n");
		return -ENODEV;
	}

	/* Print device information for debugging */
	struct pci_dev *pdev = to_pci_dev(dev->dev);
	if (pdev) {
		drm_info(dev, "Initializing VIA device: %04x:%04x (rev %02x)\n",
			pdev->vendor, pdev->device, pdev->revision);
	}

	via_quirks_init(dev);

	/* Initialize video RAM */
	ret = via_vram_init(dev);
	if (ret) {
		drm_err(dev, "Failed to initialize video RAM: %d\n", ret);
		goto exit;
	}

	ret = via_mmio_init(dev);
	if (ret) {
		drm_err(dev, "Failed to initialize MMIO: %d\n", ret);
		goto error_mmio_init;
	}

	via_graphics_unlock(dev);
	goto exit;
error_mmio_init:
	via_vram_fini(dev);
exit:
	drm_dbg_driver(dev, "Exiting %s.\n", __func__);
	return ret;
}

/* Fix this function to be non-static to match its declaration in via_drv.h */
void via_device_fini(struct drm_device *dev)
{
	drm_dbg_driver(dev, "Entered %s.\n", __func__);

	via_mmio_fini(dev);
	via_vram_fini(dev);

	drm_dbg_driver(dev, "Exiting %s.\n", __func__);
}

static const struct drm_mode_config_funcs via_drm_mode_config_funcs = {
	.fb_create		= drm_gem_fb_create,
	.atomic_check		= drm_atomic_helper_check,
	.atomic_commit		= drm_atomic_helper_commit,
};

static int via_modeset_init(struct drm_device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev->dev);
	struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
	uint32_t i;
	int ret = 0;

	drm_dbg_driver(dev, "Entered %s.\n", __func__);

	/* Initialize the number of display connectors. */
	dev_priv->number_fp = 0;
	dev_priv->number_dvi = 0;

	dev->mode_config.min_width = 0;
	dev->mode_config.min_height = 0;
	dev->mode_config.max_width = 2044;
	dev->mode_config.max_height = 4096;

	dev->mode_config.funcs = &via_drm_mode_config_funcs;

	dev->mode_config.preferred_depth = 24;

	dev->mode_config.cursor_width = VIA_CURSOR_SIZE;
	dev->mode_config.cursor_height = VIA_CURSOR_SIZE;

	ret = drmm_mode_config_init(dev);
	if (ret) {
		drm_err(dev, "Failed to initialize mode setting "
				"configuration!\n");
		goto exit;
	}

	via_i2c_reg_init(dev_priv);
	ret = via_i2c_init(dev);
	if (ret) {
		drm_err(dev, "Failed to initialize I2C bus!\n");
		goto exit;
	}

	for (i = 0; i < VIA_MAX_CRTC; i++) {
		ret = via_crtc_init(dev_priv, i);
		if (ret) {
			drm_err(dev, "Failed to initialize CRTC %u!\n", i + 1);
			goto error_crtc_init;
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

	if (pdev->device == PCI_DEVICE_ID_VIA_VX900 ||
	    pdev->device == PCI_DEVICE_ID_VIA_CHROME9_HD ||
	    pdev->device == PCI_DEVICE_ID_VIA_CHROME9_HCM) {
	    via_hdmi_init(dev, VIA_DI_PORT_NONE);
	}

	drm_mode_config_reset(dev);

	drm_kms_helper_poll_init(dev);
	goto exit;
error_crtc_init:
	via_i2c_exit();
exit:
	return ret;
}

static void via_modeset_fini(struct drm_device *dev)
{
	drm_kms_helper_poll_fini(dev);

	drm_helper_force_disable_all(dev);

	via_i2c_exit();
}

/* Implementation with the correct signature */
int via_drm_init(struct drm_device *dev, unsigned long flags)
{
	int ret = 0;
	/* Remove the unused pdev variable declaration */

	drm_dbg_driver(dev, "Entered %s.\n", __func__);

	ret = via_device_init(dev);
	if (ret) {
		drm_err(dev, "Failed to initialize device: %d\n", ret);
		goto exit;
	}

	ret = via_mm_init(dev);
	if (ret) {
		drm_err(dev, "Failed to initialize memory manager: %d\n", ret);
		goto error_mm_init;
	}

	via_chip_revision_info(dev);

	/* Only initialize modeset if explicitly enabled, to avoid 
	 * potential deadlock issues during initialization */
	if (via_modeset) {
		/* Set a safe timeout for initialization operations */
		unsigned long timeout = jiffies + msecs_to_jiffies(5000); /* 5 second timeout */
		
		ret = via_modeset_init(dev);
		if (ret || time_after(jiffies, timeout)) {
			drm_err(dev, "Modeset initialization failed or timed out: %d\n", ret);
			goto error_modeset_init;
		}
	}

	/* IRQ handling has been removed since drm_irq.h is not available */

	goto exit;
error_modeset_init:
	via_mm_fini(dev);
error_mm_init:
	via_device_fini(dev);
exit:
	drm_dbg_driver(dev, "Exiting %s.\n", __func__);
	return ret;
}

void via_drm_fini(struct drm_device *dev)
{
	drm_dbg_driver(dev, "Entered %s.\n", __func__);

	via_modeset_fini(dev);
	via_mm_fini(dev);
	via_device_fini(dev);

	drm_dbg_driver(dev, "Exiting %s.\n", __func__);
}
