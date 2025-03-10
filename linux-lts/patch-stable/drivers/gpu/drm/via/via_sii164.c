/*
 * Copyright © 2016-2018 Kevin Brace.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Author(s):
 * Kevin Brace <kevinbrace@bracecomputerlab.com>
 */

#include <linux/pci.h>

#include <drm/drm_atomic_state_helper.h>
#include <drm/drm_connector.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_edid.h>
#include <drm/drm_modeset_helper_vtables.h>
#include <drm/drm_probe_helper.h>

#include "via_drv.h"
#include "via_tmds_regs.h" // Include the new header file


#define SII164_VEN		BIT(5)
#define SII164_HEN		BIT(4)
#define SII164_DSEL		BIT(3)
#define SII164_BSEL		BIT(2)
#define SII164_EDGE		BIT(1)
#define SII164_PDB		BIT(0)


static void via_sii164_power(struct drm_device *dev,
				struct i2c_adapter *i2c_bus,
				bool power_state)
{
	u8 buf;
	u8 power_bit;

	drm_dbg_kms(dev, "Entered %s.\n", __func__);

	via_i2c_readbytes(i2c_bus, 0x38, 0x08, &buf, 1);
	power_bit = power_state ? SII164_PDB : 0x00;
	buf &= ~power_bit;
	buf |= power_bit;
	via_i2c_writebytes(i2c_bus, 0x38, 0x08, &buf, 1);
	drm_dbg_kms(dev, "SiI 164 (DVI) Power: %s\n",
			power_state ? "On" : "Off");

	drm_dbg_kms(dev, "Exiting %s.\n", __func__);
}


static bool via_sii164_sense(struct drm_device *dev,
				struct i2c_adapter *i2c_bus)
{
	u8 buf;
	bool rx_detected = false;

	drm_dbg_kms(dev, "Entered %s.\n", __func__);

	via_i2c_readbytes(i2c_bus, 0x38, 0x09, &buf, 1);
	if (buf & BIT(2)) {
		rx_detected = true;
	}

	drm_dbg_kms(dev, "SiI 164 (DVI) Connector Sense: %s\n",
			rx_detected ? "Connected" : "Not Connected");

	drm_dbg_kms(dev, "Exiting %s.\n", __func__);
	return rx_detected;
}

static void via_sii164_display_registers(struct drm_device *dev,
					struct i2c_adapter *i2c_bus)
{
	uint8_t i;
	u8 buf;

	drm_dbg_kms(dev, "Entered %s.\n", __func__);

	drm_dbg_kms(dev, "SiI 164 Registers:\n");
	for (i = 0; i < 0x10; i++) {
		via_i2c_readbytes(i2c_bus, 0x38, i, &buf, 1);
		drm_dbg_kms(dev, "0x%02x: 0x%02x\n", i, buf);
	}

	drm_dbg_kms(dev, "Exiting %s.\n", __func__);
}

static void via_sii164_init_registers(struct drm_device *dev,
					struct i2c_adapter *i2c_bus)
{
	u8 buf;

	drm_dbg_kms(dev, "Entered %s.\n", __func__);

	buf = SII164_VEN | SII164_HEN |
		SII164_DSEL |
		SII164_EDGE | SII164_PDB;
	via_i2c_writebytes(i2c_bus, 0x38, 0x08, &buf, 1);

	/*
	 * Route receiver detect bit (Offset 0x09[2]) as the output
	 * of MSEN pin.
	 */
	buf = BIT(5);
	via_i2c_writebytes(i2c_bus, 0x38, 0x09, &buf, 1);

	buf = 0x90;
	via_i2c_writebytes(i2c_bus, 0x38, 0x0a, &buf, 1);

	buf = 0x89;
	via_i2c_writebytes(i2c_bus, 0x38, 0x0c, &buf, 1);

	drm_dbg_kms(dev, "Exiting %s.\n", __func__);
}


static const struct drm_encoder_funcs via_sii164_drm_encoder_funcs = {
	.destroy = via_encoder_destroy,
};

static void via_sii164_dpms(struct drm_encoder *encoder, int mode)
{
	struct via_encoder *enc = container_of(encoder,
					struct via_encoder, base);
	struct drm_device *dev = encoder->dev;
	struct i2c_adapter *i2c_bus;

	drm_dbg_kms(dev, "Entered %s.\n", __func__);

	if (enc->i2c_bus & VIA_I2C_BUS1) {
		i2c_bus = via_find_ddc_bus(0x26);
	} else if (enc->i2c_bus & VIA_I2C_BUS2) {
		i2c_bus = via_find_ddc_bus(0x31);
	} else if (enc->i2c_bus & VIA_I2C_BUS3) {
		i2c_bus = via_find_ddc_bus(0x25);
	} else if (enc->i2c_bus & VIA_I2C_BUS4) {
		i2c_bus = via_find_ddc_bus(0x2c);
	} else if (enc->i2c_bus & VIA_I2C_BUS5) {
		i2c_bus = via_find_ddc_bus(0x3d);
	} else {
		i2c_bus = NULL;
		goto exit;
	}

	via_sii164_display_registers(dev, i2c_bus);
	switch (mode) {
	case DRM_MODE_DPMS_ON:
		via_sii164_power(dev, i2c_bus, true);
		via_transmitter_io_pad_state(dev, enc->di_port, true);
		break;
	case DRM_MODE_DPMS_STANDBY:
	case DRM_MODE_DPMS_SUSPEND:
	case DRM_MODE_DPMS_OFF:
		via_sii164_power(dev, i2c_bus, false);
		via_transmitter_io_pad_state(dev, enc->di_port, false);
		break;
	default:
		drm_err(dev, "Bad DPMS mode.");
		break;
	}

exit:
	drm_dbg_kms(dev, "Exiting %s.\n", __func__);
}

static bool via_sii164_mode_fixup(struct drm_encoder *encoder,
				const struct drm_display_mode *mode,
				struct drm_display_mode *adjusted_mode)
{
	struct drm_device *dev = encoder->dev;

	drm_dbg_kms(dev, "Entered %s.\n", __func__);

	drm_mode_set_crtcinfo(adjusted_mode, 0);

	drm_dbg_kms(dev, "Exiting %s.\n", __func__);
	return true;
}

static void via_sii164_mode_set(struct drm_encoder *encoder,
				struct drm_display_mode *mode,
				struct drm_display_mode *adjusted_mode)
{
	struct via_crtc *iga = container_of(encoder->crtc, struct via_crtc, base);
	struct via_encoder *enc = container_of(encoder,
					struct via_encoder, base);
	struct drm_device *dev = encoder->dev;
	struct pci_dev *pdev = to_pci_dev(dev->dev);
	struct i2c_adapter *i2c_bus;

	drm_dbg_kms(dev, "Entered %s.\n", __func__);

	if (enc->i2c_bus & VIA_I2C_BUS1) {
		i2c_bus = via_find_ddc_bus(0x26);
	} else if (enc->i2c_bus & VIA_I2C_BUS2) {
		i2c_bus = via_find_ddc_bus(0x31);
	} else if (enc->i2c_bus & VIA_I2C_BUS3) {
		i2c_bus = via_find_ddc_bus(0x25);
	} else if (enc->i2c_bus & VIA_I2C_BUS4) {
		i2c_bus = via_find_ddc_bus(0x2c);
	} else if (enc->i2c_bus & VIA_I2C_BUS5) {
		i2c_bus = via_find_ddc_bus(0x3d);
	} else {
		i2c_bus = NULL;
		goto exit;
	}

	via_transmitter_clock_drive_strength(dev, enc->di_port, 0x03);
	via_transmitter_data_drive_strength(dev, enc->di_port, 0x03);
	via_transmitter_io_pad_state(dev, enc->di_port, true);
	if (pdev->device == PCI_DEVICE_ID_VIA_CLE266_GFX) {
		via_clock_source(dev, enc->di_port, true);
	}

	via_sii164_display_registers(dev, i2c_bus);
	via_sii164_init_registers(dev, i2c_bus);
	via_sii164_display_registers(dev, i2c_bus);

	via_transmitter_display_source(dev, enc->di_port, iga->index);
exit:

	drm_dbg_kms(dev, "Exiting %s.\n", __func__);
}

static void via_sii164_prepare(struct drm_encoder *encoder)
{
	struct via_encoder *enc = container_of(encoder,
					struct via_encoder, base);
	struct drm_device *dev = encoder->dev;
	struct pci_dev *pdev = to_pci_dev(dev->dev);
	struct i2c_adapter *i2c_bus;

	drm_dbg_kms(dev, "Entered %s.\n", __func__);

	if (enc->i2c_bus & VIA_I2C_BUS1) {
		i2c_bus = via_find_ddc_bus(0x26);
	} else if (enc->i2c_bus & VIA_I2C_BUS2) {
		i2c_bus = via_find_ddc_bus(0x31);
	} else if (enc->i2c_bus & VIA_I2C_BUS3) {
		i2c_bus = via_find_ddc_bus(0x25);
	} else if (enc->i2c_bus & VIA_I2C_BUS4) {
		i2c_bus = via_find_ddc_bus(0x2c);
	} else if (enc->i2c_bus & VIA_I2C_BUS5) {
		i2c_bus = via_find_ddc_bus(0x3d);
	} else {
		i2c_bus = NULL;
		goto exit;
	}

	via_sii164_power(dev, i2c_bus, false);
	via_transmitter_io_pad_state(dev, enc->di_port, false);
	if (pdev->device == PCI_DEVICE_ID_VIA_CLE266_GFX) {
		via_output_enable(dev, enc->di_port, false);
	}

exit:
	drm_dbg_kms(dev, "Exiting %s.\n", __func__);
}

static void via_sii164_commit(struct drm_encoder *encoder)
{
	struct via_encoder *enc = container_of(encoder,
					struct via_encoder, base);
	struct drm_device *dev = encoder->dev;
	struct pci_dev *pdev = to_pci_dev(dev->dev);
	struct i2c_adapter *i2c_bus;

	drm_dbg_kms(dev, "Entered %s.\n", __func__);

	if (enc->i2c_bus & VIA_I2C_BUS1) {
		i2c_bus = via_find_ddc_bus(0x26);
	} else if (enc->i2c_bus & VIA_I2C_BUS2) {
		i2c_bus = via_find_ddc_bus(0x31);
	} else if (enc->i2c_bus & VIA_I2C_BUS3) {
		i2c_bus = via_find_ddc_bus(0x25);
	} else if (enc->i2c_bus & VIA_I2C_BUS4) {
		i2c_bus = via_find_ddc_bus(0x2c);
	} else if (enc->i2c_bus & VIA_I2C_BUS5) {
		i2c_bus = via_find_ddc_bus(0x3d);
	} else {
		i2c_bus = NULL;
		goto exit;
	}

	via_sii164_power(dev, i2c_bus, true);
	via_transmitter_io_pad_state(dev, enc->di_port, true);
	if (pdev->device == PCI_DEVICE_ID_VIA_CLE266_GFX) {
		via_output_enable(dev, enc->di_port, true);
	}

exit:
	drm_dbg_kms(dev, "Exiting %s.\n", __func__);
}

static void via_sii164_disable(struct drm_encoder *encoder)
{
	struct via_encoder *enc = container_of(encoder,
					struct via_encoder, base);
	struct drm_device *dev = encoder->dev;
	struct i2c_adapter *i2c_bus;

	drm_dbg_kms(dev, "Entered %s.\n", __func__);

	if (enc->i2c_bus & VIA_I2C_BUS1) {
		i2c_bus = via_find_ddc_bus(0x26);
	} else if (enc->i2c_bus & VIA_I2C_BUS2) {
		i2c_bus = via_find_ddc_bus(0x31);
	} else if (enc->i2c_bus & VIA_I2C_BUS3) {
		i2c_bus = via_find_ddc_bus(0x25);
	} else if (enc->i2c_bus & VIA_I2C_BUS4) {
		i2c_bus = via_find_ddc_bus(0x2c);
	} else if (enc->i2c_bus & VIA_I2C_BUS5) {
		i2c_bus = via_find_ddc_bus(0x3d);
	} else {
		i2c_bus = NULL;
		goto exit;
	}

	via_sii164_power(dev, i2c_bus, false);
	via_transmitter_io_pad_state(dev, enc->di_port, false);
exit:
	drm_dbg_kms(dev, "Exiting %s.\n", __func__);
}


static const struct drm_encoder_helper_funcs
via_sii164_drm_encoder_helper_funcs = {
	.dpms = via_sii164_dpms,
	.mode_fixup = via_sii164_mode_fixup,
	.mode_set = via_sii164_mode_set,
	.prepare = via_sii164_prepare,
	.commit = via_sii164_commit,
	.disable = via_sii164_disable,
};


static enum drm_connector_status via_sii164_detect(
					struct drm_connector *connector,
					bool force)
{
	struct drm_device *dev = connector->dev;
	struct via_connector *con = container_of(connector,
					struct via_connector, base);
	struct i2c_adapter *i2c_bus;
	enum drm_connector_status ret = connector_status_disconnected;

	drm_dbg_kms(dev, "Entered %s.\n", __func__);

	if (con->i2c_bus & VIA_I2C_BUS1) {
		i2c_bus = via_find_ddc_bus(0x26);
	} else if (con->i2c_bus & VIA_I2C_BUS2) {
		i2c_bus = via_find_ddc_bus(0x31);
	} else if (con->i2c_bus & VIA_I2C_BUS3) {
		i2c_bus = via_find_ddc_bus(0x25);
	} else if (con->i2c_bus & VIA_I2C_BUS4) {
		i2c_bus = via_find_ddc_bus(0x2c);
	} else if (con->i2c_bus & VIA_I2C_BUS5) {
		i2c_bus = via_find_ddc_bus(0x3d);
	} else {
		i2c_bus = NULL;
		goto exit;
	}

	if (via_sii164_sense(dev, i2c_bus)) {
		ret = connector_status_connected;
		drm_dbg_kms(dev, "DVI detected.\n");
	}

exit:
	drm_dbg_kms(dev, "Exiting %s.\n", __func__);
	return ret;
}

static const struct drm_connector_funcs via_sii164_drm_connector_funcs = {
	.dpms = drm_helper_connector_dpms,
	.detect = via_sii164_detect,
	.fill_modes = drm_helper_probe_single_connector_modes,
	.destroy = via_connector_destroy,
	.reset = drm_atomic_helper_connector_reset,
	.atomic_duplicate_state =
			drm_atomic_helper_connector_duplicate_state,
	.atomic_destroy_state =
			drm_atomic_helper_connector_destroy_state,
};


static int via_sii164_mode_valid(struct drm_connector *connector,
					struct drm_display_mode *mode)
{
	struct drm_device *dev = connector->dev;
	struct via_connector *con = container_of(connector,
					struct via_connector, base);
	struct i2c_adapter *i2c_bus;
	u8 buf;
	uint32_t low_freq_limit, high_freq_limit;
	int ret;

	drm_dbg_kms(dev, "Entered %s.\n", __func__);

	if (con->i2c_bus & VIA_I2C_BUS1) {
		i2c_bus = via_find_ddc_bus(0x26);
	} else if (con->i2c_bus & VIA_I2C_BUS2) {
		i2c_bus = via_find_ddc_bus(0x31);
	} else if (con->i2c_bus & VIA_I2C_BUS3) {
		i2c_bus = via_find_ddc_bus(0x25);
	} else if (con->i2c_bus & VIA_I2C_BUS4) {
		i2c_bus = via_find_ddc_bus(0x2c);
	} else if (con->i2c_bus & VIA_I2C_BUS5) {
		i2c_bus = via_find_ddc_bus(0x3d);
	} else {
		i2c_bus = NULL;
		ret = MODE_ERROR;
		goto exit;
	}

	via_i2c_readbytes(i2c_bus, 0x38, 0x06, &buf, 1);
	low_freq_limit = buf * 1000;
	via_i2c_readbytes(i2c_bus, 0x38, 0x07, &buf, 1);
	high_freq_limit = (buf + 65) * 1000;
	drm_dbg_kms(dev, "Low Frequency Limit: %u KHz\n", low_freq_limit);
	drm_dbg_kms(dev, "High Frequency Limit: %u KHz\n", high_freq_limit);

	if (mode->clock < low_freq_limit) {
		ret = MODE_CLOCK_LOW;
		goto exit;
	}

	if (mode->clock > high_freq_limit) {
		ret = MODE_CLOCK_HIGH;
		goto exit;
	}

	ret = MODE_OK;
exit:
	drm_dbg_kms(dev, "Exiting %s.\n", __func__);
	return ret;
}

static int via_sii164_get_modes(struct drm_connector *connector)
{
	struct drm_device *dev = connector->dev;
	struct via_connector *con = container_of(connector,
					struct via_connector, base);
	int count = 0;
	struct i2c_adapter *i2c_bus;
	struct edid *edid = NULL;

	drm_dbg_kms(dev, "Entered %s.\n", __func__);

	if (con->i2c_bus & VIA_I2C_BUS1) {
		i2c_bus = via_find_ddc_bus(0x26);
	} else if (con->i2c_bus & VIA_I2C_BUS2) {
		i2c_bus = via_find_ddc_bus(0x31);
	} else if (con->i2c_bus & VIA_I2C_BUS3) {
		i2c_bus = via_find_ddc_bus(0x25);
	} else if (con->i2c_bus & VIA_I2C_BUS4) {
		i2c_bus = via_find_ddc_bus(0x2c);
	} else if (con->i2c_bus & VIA_I2C_BUS5) {
		i2c_bus = via_find_ddc_bus(0x3d);
	} else {
		i2c_bus = NULL;
		goto exit;
	}

	edid = drm_get_edid(&con->base, i2c_bus);
	if (edid) {
		if (edid->input & DRM_EDID_INPUT_DIGITAL) {
			drm_connector_update_edid_property(connector, edid);
			count = drm_add_edid_modes(connector, edid);
			drm_dbg_kms(dev, "DVI EDID information was obtained.\n");
		}

		kfree(edid);
	}

exit:
	drm_dbg_kms(dev, "Exiting %s.\n", __func__);
	return count;
}

static const struct drm_connector_helper_funcs
via_sii164_drm_connector_helper_funcs = {
	.mode_valid = via_sii164_mode_valid,
	.get_modes = via_sii164_get_modes,
};

// Fix the function signature to match the header declaration
bool via_sii164_probe(struct drm_device *dev, u32 i2c_bus_id, u32 di_port)
{
    struct i2c_adapter *i2c_bus = NULL;
    u8 buf;
    u16 vendor_id, device_id, revision;
    bool device_detected = false;

    drm_dbg_kms(dev, "Entered %s.\n", __func__);

    // Convert i2c_bus_id to an actual i2c_adapter
    if (i2c_bus_id == VIA_I2C_BUS1) {
        i2c_bus = via_find_ddc_bus(0x26);
    } else if (i2c_bus_id == VIA_I2C_BUS2) {
        i2c_bus = via_find_ddc_bus(0x31);
    } else if (i2c_bus_id == VIA_I2C_BUS3) {
        i2c_bus = via_find_ddc_bus(0x25);
    } else if (i2c_bus_id == VIA_I2C_BUS4) {
        i2c_bus = via_find_ddc_bus(0x2c);
    } else if (i2c_bus_id == VIA_I2C_BUS5) {
        i2c_bus = via_find_ddc_bus(0x3d);
    }

    if (!i2c_bus) {
        goto exit;
    }

    via_i2c_readbytes(i2c_bus, 0x38, 0x00, &buf, 1);
    vendor_id = buf;
    via_i2c_readbytes(i2c_bus, 0x38, 0x01, &buf, 1);
    vendor_id |= (buf << 8);
    drm_dbg_kms(dev, "Vendor ID: %x\n", vendor_id);
    via_i2c_readbytes(i2c_bus, 0x38, 0x02, &buf, 1);
    device_id = buf;
    via_i2c_readbytes(i2c_bus, 0x38, 0x03, &buf, 1);
    device_id |= (buf << 8);
    drm_dbg_kms(dev, "Device ID: %x\n", device_id);
    via_i2c_readbytes(i2c_bus, 0x38, 0x04, &buf, 1);
    revision = buf;
    drm_dbg_kms(dev, "Revision: %x\n", revision);

    if ((vendor_id != 0x0001) || (device_id != 0x0006)) {
        goto exit;
    }

    device_detected = true;
exit:
    drm_dbg_kms(dev, "Exiting %s.\n", __func__);
    return device_detected;
}

void via_sii164_init(struct drm_device *dev)
{
    struct via_connector *con;
    struct via_encoder *enc;
    struct via_drm_priv *dev_priv = to_via_drm_priv(dev);

    if (!dev_priv->ext_tmds_presence) {
        drm_dbg_kms(dev, "No External DVI (SiI164) detected.\n");
        return;
    }

    /* Allocate encoder and connector */
    enc = kzalloc(sizeof(*enc) + sizeof(*con), GFP_KERNEL);
    if (!enc) {
        drm_err(dev, "Failed to allocate encoder for SiI164\n");
        return;
    }

    /* Setup the encoder - use correct variable names */
    drm_encoder_init(dev, &enc->base, &via_sii164_drm_encoder_funcs,
                    DRM_MODE_ENCODER_TMDS, NULL);
    drm_encoder_helper_add(&enc->base, &via_sii164_drm_encoder_helper_funcs);

    enc->i2c_bus = dev_priv->ext_tmds_i2c_bus;
    enc->di_port = dev_priv->ext_tmds_di_port;
    enc->base.possible_crtcs = BIT(0) | BIT(1);
    enc->base.possible_clones = 0;

    /* Fix: Use con instead of cons[0] */
    con = &enc->con;
    INIT_LIST_HEAD(&con->props);

    via_connector_init(&con->base, &via_sii164_drm_connector_funcs,
                        DRM_MODE_CONNECTOR_DVID);

    drm_connector_helper_add(&con->base, &via_sii164_drm_connector_helper_funcs);
    drm_connector_register(&con->base);

    con->i2c_bus = dev_priv->ext_tmds_i2c_bus;
    con->base.doublescan_allowed = false;
    con->base.interlace_allowed = true;

    drm_connector_attach_encoder(&con->base, &enc->base);

    dev_priv->ext_tmds_encoder = enc;
}
