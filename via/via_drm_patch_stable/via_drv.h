/*
 * Copyright © 2019 Kevin Brace.
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

#ifndef _VIA_DRV_H
#define _VIA_DRV_H

#include <linux/module.h> /* Often needed for module_init/module_exit macros */
#include <drm/drm_connector.h>
#include <drm/drm_crtc.h>
#include <drm/drm_encoder.h>
#include <drm/drm_plane.h>

#include <drm/ttm/ttm_bo.h>
#include <drm/ttm/ttm_placement.h>

#include "via_crtc_hw.h"
#include "via_regs.h"

/* Version details */
#define DRIVER_MAJOR            3
#define DRIVER_MINOR            6
#define DRIVER_PATCHLEVEL       14
#define DRIVER_NAME             "via"
#define DRIVER_DESC             "OpenChrome DRM for VIA Technologies Chrome"
#define DRIVER_DATE             "20240215"
#define DRIVER_AUTHOR           "OpenChrome Project"

#define VIA_TTM_PL_NUM          2
#define VIA_MAX_CRTC            2
#define VIA_CURSOR_SIZE         64
#define VIA_MM_ALIGN_SIZE       16

#define CLE266_REVISION_AX      0x0A
#define CLE266_REVISION_CX      0x0C

#define CX700_REVISION_700      0x0
#define CX700_REVISION_700M     0x1
#define CX700_REVISION_700M2    0x2

/* Memory types (SDR, DDR, DDR2, DDR3, etc.) */
#define VIA_MEM_NONE            0x00
#define VIA_MEM_SDR66           0x01
#define VIA_MEM_SDR100          0x02
#define VIA_MEM_SDR133          0x03
#define VIA_MEM_DDR_200         0x04
#define VIA_MEM_DDR_266         0x05
#define VIA_MEM_DDR_333         0x06
#define VIA_MEM_DDR_400         0x07
#define VIA_MEM_DDR2_400        0x08
#define VIA_MEM_DDR2_533        0x09
#define VIA_MEM_DDR2_667        0x0A
#define VIA_MEM_DDR2_800        0x0B
#define VIA_MEM_DDR2_1066       0x0C
#define VIA_MEM_DDR3_533        0x0D
#define VIA_MEM_DDR3_667        0x0E
#define VIA_MEM_DDR3_800        0x0F
#define VIA_MEM_DDR3_1066       0x10
#define VIA_MEM_DDR3_1333       0x11
#define VIA_MEM_DDR3_1600       0x12

/* IGA (Integrated Graphics Accelerator) scaling modes */
#define VIA_NO_SCALING          0

/* IGA scaling down */
#define VIA_HOR_SHRINK          BIT(0)
#define VIA_VER_SHRINK          BIT(1)
#define VIA_SHRINK              (BIT(0) | BIT(1))

/* IGA scaling up */
#define VIA_HOR_EXPAND          BIT(2)
#define VIA_VER_EXPAND          BIT(3)
#define VIA_EXPAND              (BIT(2) | BIT(3))

/* Helper flags for horizontal or vertical scaling */
#define HOR_SCALE               BIT(0)
#define VER_SCALE               BIT(1)
#define HOR_VER_SCALE           (BIT(0) | BIT(1))

/* I2C bus flags */
#define VIA_I2C_NONE            0x0
#define VIA_I2C_BUS1            BIT(0)
#define VIA_I2C_BUS2            BIT(1)
#define VIA_I2C_BUS3            BIT(2)
#define VIA_I2C_BUS4            BIT(3)
#define VIA_I2C_BUS5            BIT(4)

/* Digital Interface ports (DI_PORT) */
#define VIA_DI_PORT_NONE        0x0
#define VIA_DI_PORT_DIP0        BIT(0)
#define VIA_DI_PORT_DIP1        BIT(1)
#define VIA_DI_PORT_DVP0        BIT(2)
#define VIA_DI_PORT_DVP1        BIT(3)
#define VIA_DI_PORT_DFPL        BIT(4)
#define VIA_DI_PORT_FPDPLOW     BIT(4)
#define VIA_DI_PORT_DFPH        BIT(5)
#define VIA_DI_PORT_FPDPHIGH    BIT(5)
#define VIA_DI_PORT_DFP         BIT(6)
#define VIA_DI_PORT_LVDS1       BIT(7)
#define VIA_DI_PORT_TMDS        BIT(7)
#define VIA_DI_PORT_LVDS2       BIT(8)

/* External TMDS (DVI) Transmitter Types */
#define VIA_TMDS_NONE           0x0
#define VIA_TMDS_VT1632         BIT(0)
#define VIA_TMDS_SII164         BIT(1)

/*
 * LVDS panel info (for representing x/y resolution).
 */
struct via_lvds_info {
	u32 x;
	u32 y;
};

/*
 * This structure tracks per-CRTC (IGA) resources:
 *  - Base DRM CRTC object
 *  - Timings data for both standard registers (timings) and pixel-level regs (pixel_timings)
 *  - FIFO / queue config parameters
 *  - Scaling mode, etc.
 */
struct via_crtc {
	struct drm_crtc        base;

	/* Pixel (register) timings used for advanced modes / shadow registers */
	struct crtc_timings    pixel_timings;

	/* Standard CRTC timing registers (horizontal, vertical, sync, blank, etc.) */
	struct crtc_timings    timings;

	/* FIFO / queue / threshold registers for controlling memory fetch */
	struct vga_registers   display_queue;
	struct vga_registers   high_threshold;
	struct vga_registers   threshold;
	struct vga_registers   fifo_depth;
	struct vga_registers   offset;
	struct vga_registers   fetch;

	/* Scaling mode flags (VIA_NO_SCALING, VIA_SHRINK, VIA_EXPAND, etc.) */
	int                    scaling_mode;

	/* CRTC index (0 = IGA1, 1 = IGA2) */
	uint32_t               index;
};

/*
 * VIA connector structure
 */
struct via_connector {
	struct drm_connector base;
	u32 i2c_bus;
	struct list_head props;
	uint32_t flags;
};

/*
 * VIA encoder structure
 */
struct via_encoder {
	struct drm_encoder base;
	u32 i2c_bus;
	u32 di_port;
	struct via_connector cons[]; /* some encoders embed connectors */
};

/*
 * TTM buffer object wrapper
 */
struct via_bo {
	struct ttm_buffer_object ttm_bo;
	struct ttm_bo_kmap_obj   kmap;
	struct ttm_placement     placement;
	struct ttm_place         placements[VIA_TTM_PL_NUM];
};

/*
 * Driver private structure
 */
struct via_drm_priv {
	struct drm_device dev;       /* DRM device struct */

	struct ttm_device bdev;      /* TTM device object */
	bool need_dma32;             /* If TTM has to stay < 4 GB */

	int revision;                /* e.g., CLE266 A0, C0, etc. */

	u8  vram_type;               /* Memory type (DDR2, DDR3, etc.) */
	resource_size_t vram_start;  /* Start of VRAM resource */
	resource_size_t vram_size;   /* Size of VRAM resource in bytes */
	int vram_mtrr;               /* MTRR handle if used */

	resource_size_t mmio_base;   /* Base of MMIO registers */
	resource_size_t mmio_size;   /* Size of MMIO region */
	void __iomem *mmio;          /* Virtual (ioremap) pointer */

	bool spread_spectrum;        /* If spread spectrum is in use */

	/*
	 * On certain chipsets (VX800, VX855, VX900),
	 * we need to save/restore SR14 and SR66~SR6F on resume,
	 * or it can lead to corruption.
	 */
	uint8_t saved_sr14;
	uint8_t saved_sr66;
	uint8_t saved_sr67;
	uint8_t saved_sr68;
	uint8_t saved_sr69;
	uint8_t saved_sr6a;
	uint8_t saved_sr6b;
	uint8_t saved_sr6c;
	uint8_t saved_sr6d;
	uint8_t saved_sr6e;
	uint8_t saved_sr6f;

	/* CR3B ~ CR3F scratch pad registers */
	uint8_t saved_cr3b;
	uint8_t saved_cr3c;
	uint8_t saved_cr3d;
	uint8_t saved_cr3e;
	uint8_t saved_cr3f;

	/* Board-specific quirks and detection flags */
	bool is_via_nanobook;
	bool is_quanta_il1;
	bool is_samsung_nc20;

	/* DAC presence and I2C bus for integrated analog (VGA) */
	bool dac_presence;
	u32  dac_i2c_bus;

	/* Internal TMDS (DVI) presence, DI port, and I2C bus */
	bool int_tmds_presence;
	u32  int_tmds_di_port;
	u32  int_tmds_i2c_bus;

	/* External TMDS presence, DI port, I2C bus, and transmitter type */
	bool ext_tmds_presence;
	u32  ext_tmds_di_port;
	u32  ext_tmds_i2c_bus;
	u32  ext_tmds_transmitter;

	/* Internal FP presence (possibly LVDS or DVP lines) */
	bool int_fp1_presence;
	u32  int_fp1_di_port;
	u32  int_fp1_i2c_bus;

	bool int_fp2_presence;
	u32  int_fp2_di_port;
	u32  int_fp2_i2c_bus;

	/* Bookkeeping for number of DVI or FP connectors discovered */
	u32 number_dvi;
	u32 number_fp;

	/* Tracks last used I2C bus for some ops */
	u32 mapped_i2c_bus;
};

/*
 * Shortcuts for container_of uses
 */
#define to_via_drm_priv(x)  container_of(x, struct via_drm_priv, dev)
#define to_ttm_bo(x)        container_of(x, struct via_bo, ttm_bo)

/*
 * Macros for reading/writing MMIO registers
 */
#define VIA_BASE            (dev_priv->mmio)

#define VIA_READ(reg)       ioread32(VIA_BASE + (reg))
#define VIA_WRITE(reg,val)  iowrite32((val), VIA_BASE + (reg))
#define VIA_READ8(reg)      ioread8(VIA_BASE + (reg))
#define VIA_WRITE8(reg,val) iowrite8((val), VIA_BASE + (reg))

#define VIA_WRITE_MASK(reg, data, mask) \
do { \
	u32 tmp = VIA_READ(reg); \
	tmp &= ~(mask); \
	tmp |= ((data) & (mask)); \
	VIA_WRITE(reg, tmp); \
} while (0)

#define VGABASE (VIA_BASE + VIA_MMIO_VGABASE)

/*
 * Functions exported from various via_* files
 */

/* via_connector.c */
void via_connector_destroy(struct drm_connector *connector);

/* via_crtc.c */
int via_crtc_init(struct via_drm_priv *dev_priv, uint32_t index);
void via_load_crtc_pixel_timing(struct drm_crtc *crtc,
								struct drm_display_mode *mode);

/* via_cursor.c */
extern const struct drm_plane_helper_funcs via_cursor_drm_plane_helper_funcs;
extern const struct drm_plane_funcs via_cursor_drm_plane_funcs;
extern const uint32_t via_cursor_formats[];
extern const unsigned int via_cursor_formats_size;

/* via_encoder.c */
void via_encoder_destroy(struct drm_encoder *encoder);

/* via_i2c.c */
struct i2c_adapter *via_find_ddc_bus(int port);
void via_i2c_readbytes(struct i2c_adapter *adapter,
					   u8 slave_addr, char offset,
					   u8 *buffer, unsigned int size);
void via_i2c_writebytes(struct i2c_adapter *adapter,
						u8 slave_addr, char offset,
						u8 *data, unsigned int size);
void via_i2c_reg_init(struct via_drm_priv *dev_priv);
int via_i2c_init(struct drm_device *dev);
void via_i2c_exit(void);

/* via_init.c */
int via_drm_init(struct drm_device *dev);
void via_drm_fini(struct drm_device *dev);

/* via_ioctl.c */
int via_gem_alloc_ioctl(struct drm_device *dev, void *data,
						struct drm_file *file_priv);
int via_gem_mmap_ioctl(struct drm_device *dev, void *data,
					   struct drm_file *file_priv);

/* via_object.c */
void via_ttm_domain_to_placement(struct via_bo *bo, uint32_t ttm_domain);
void via_ttm_bo_destroy(struct ttm_buffer_object *tbo);
int via_bo_pin(struct via_bo *bo, uint32_t ttm_domain);
void via_bo_unpin(struct via_bo *bo);
int via_bo_create(struct drm_device *dev, struct ttm_device *bdev,
				  uint64_t size, enum ttm_bo_type type,
				  uint32_t ttm_domain, bool kmap,
				  struct via_bo **bo_ptr);
void via_bo_destroy(struct via_bo *bo, bool kmap);
int via_mm_init(struct drm_device *dev);
void via_mm_fini(struct drm_device *dev);

/* via_pll.c */
u32 via_get_clk_value(struct drm_device *dev, u32 clk);
void via_set_vclock(struct drm_crtc *crtc, u32 clk);

/* via_pm.c */
int via_dev_pm_ops_suspend(struct device *dev);
int via_dev_pm_ops_resume(struct device *dev);

/* via_ttm.c */
extern struct ttm_device_funcs via_bo_driver;
void via_ttm_debugfs_init(struct drm_device *dev);

/* via_tx.c */
void via_transmitter_io_pad_state(struct drm_device *dev,
								  uint32_t di_port,
								  bool io_pad_on);
void via_output_enable(struct drm_device *dev,
					   uint32_t di_port, bool output_enable);
void via_clock_source(struct drm_device *dev,
					  uint32_t di_port, bool clock_source);
void via_transmitter_clock_drive_strength(struct drm_device *dev,
										  u32 di_port,
										  u8 drive_strength);
void via_transmitter_data_drive_strength(struct drm_device *dev,
										 u32 di_port,
										 u8 drive_strength);
void via_transmitter_display_source(struct drm_device *dev,
									u32 di_port, int index);

/* Additional display probes / inits */
void via_dac_init(struct drm_device *dev);
void via_dac_probe(struct drm_device *dev);
bool via_vt1632_probe(struct drm_device *dev, struct i2c_adapter *i2c_bus);
void via_vt1632_init(struct drm_device *dev);
bool via_sii164_probe(struct drm_device *dev, struct i2c_adapter *i2c_bus);
void via_sii164_init(struct drm_device *dev);
void via_ext_dvi_probe(struct drm_device *dev);
void via_ext_dvi_init(struct drm_device *dev);
void via_tmds_probe(struct drm_device *dev);
void via_tmds_init(struct drm_device *dev);
void via_lvds_probe(struct drm_device *dev);
void via_lvds_init(struct drm_device *dev);
void via_hdmi_init(struct drm_device *dev, u32 di_port);

#endif /* _VIA_DRV_H_ */
