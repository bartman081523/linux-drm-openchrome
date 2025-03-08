/*
 * Copyright © 2019-2023 Kevin Brace.
 * Copyright 2016 James Simmons. All Rights Reserved.
 * Copyright 2013 Intel Corporation
 * Copyright 1998-2009 VIA Technologies, Inc. All Rights Reserved.
 * Copyright 2001-2008 S3 Graphics, Inc. All Rights Reserved.
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

#ifndef __VIA_DRV_H__
#define __VIA_DRV_H__

#include <asm/io.h>

// Standard C includes *before* any DRM headers
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/pci_ids.h>
#include <linux/ioport.h>
#include <linux/uaccess.h>
#include <linux/i2c.h>
#include <linux/i2c-algo-bit.h>
#include <video/vga.h>

// DRM headers *after* standard headers and PCI headers
#include <drm/drm_drv.h>  // Explicitly include drm_drv.h for struct drm_driver definition
#include <drm/drm_atomic.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_atomic_state_helper.h>
#include <drm/drm_crtc.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_connector.h>
#include <drm/drm_encoder.h>
#include <drm/drm_framebuffer.h>
#include <drm/drm_gem.h>
#include <drm/drm_managed.h>
#include <drm/drm_modeset_helper_vtables.h>
#include <drm/drm_probe_helper.h>
#include <drm/drm_print.h>

// TTM Headers
#include <drm/ttm/ttm_bo.h>
#include <drm/ttm/ttm_device.h>
#include <drm/ttm/ttm_placement.h>
#include <drm/ttm/ttm_tt.h>
#include <drm/drm_gem_ttm_helper.h>

// Forward declarations to break circular dependencies
struct via_crtc;
struct via_connector;
struct via_encoder;
struct via_block;
struct ttm_buffer_object;
struct via_bo;
struct via_i2c_stuff;

/* Register definitions */

/* VGA register access macros */
#define VGA_SEQ_I          0x3C4
#define VGA_SEQ_D          0x3C5
#define VGA_CRT_IC         0x3D4
#define VGA_CRT_DC         0x3D5

/* Register definitions */
#define IGA1_PIX_HALF_LINE_REG     0x3C
#define IGA1_FIFO_DEPTH_SELECT_REG 0x3C5

/* LVDS/DVI flags */
#define VIA_LVDS1          BIT(0)
#define VIA_LVDS2          BIT(1)
#define VIA_DVI            BIT(2)

/* VIA chipset types */
enum via_chip {
    VIA_CLE266 = 0,
    VIA_KM400,
    VIA_K8M800,
    VIA_PM800,
    VIA_P4M800_PRO,
    VIA_CX700,
    VIA_P4M890,
    VIA_K8M890,
    VIA_P4M900,
    VIA_VX800,
    VIA_VX855,
    VIA_VX900
};

/* VIA CLE266 revisions */
enum via_revision {
    CLE266_REVISION_AX = 0,
    CLE266_REVISION_CX
};

/* CX700 revision definitions */
#define CX700_REVISION_700    0
#define CX700_REVISION_700M   1
#define CX700_REVISION_700M2  2

/* Register data structures for describing control registers */
struct vga_regset {
    unsigned char type;      /* Register type (VGA_SEQ_I, etc.) */
    unsigned char index;     /* Register index */
    unsigned char low;       /* Start bit position */
    unsigned char high;      /* End bit position */
};

struct vga_reg {
    int type;    /* Register type */
    int index;   /* Register index */
    int value;   /* Register value */
    int low;     /* For bit range operations, low bit */
    int high;    /* For bit range operations, high bit */
    int start_bit; /* For helping with bit range calculations */
    int mask;    /* For masking operations */
};

struct vga_registers {
    int count;
    const struct vga_regset *regs;
};

/* Define VGA base address */
#define VGABASE (dev_priv->mmio)

/* Function prototypes */
void load_value_to_registers(void __iomem *regbase, const struct vga_registers *regs,
                           unsigned int value);
                           
/* These are declared in via_vgahw.h */
void svga_wcrt_mask(void __iomem *vgabase, u8 index, u8 value, u8 mask);
void svga_wseq_mask(void __iomem *vgabase, u8 index, u8 value, u8 mask);

/* Structure definitions */
struct crtc_timings {
    struct vga_registers htotal;
    struct vga_registers hdisplay;
    struct vga_registers hblank_start;
    struct vga_registers hblank_end;
    struct vga_registers hsync_start;
    struct vga_registers hsync_end;
    struct vga_registers vtotal;
    struct vga_registers vdisplay;
    struct vga_registers vblank_start;
    struct vga_registers vblank_end;
    struct vga_registers vsync_start;
    struct vga_registers vsync_end;
};

/* VIA connector */
struct via_connector {
    struct drm_connector base;
    int port;
    int i2c_bus;
    struct list_head props;
    uint32_t flags;  
};

static inline struct via_connector *
to_via_connector(struct drm_connector *connector)
{
    return container_of(connector, struct via_connector, base);
}

struct via_encoder {
    struct drm_encoder base;
    struct via_connector con;
    u32 encoder_type; 
    int index;
    u32 i2c_bus;
    u32 di_port;  /* Add di_port field for HDMI/DP support */
};

static inline struct via_encoder *to_via_encoder(struct drm_encoder *encoder)
{
    return container_of(encoder, struct via_encoder, base);
}

struct via_crtc {
    struct drm_crtc base;
    struct drm_plane *plane;
    enum via_chip chip;
    int index;
    u32 address;
    u32 id;
    struct vga_registers fifo_depth;
    struct vga_registers threshold;
    struct vga_registers high_threshold;
    struct vga_registers display_queue;
    struct vga_registers fetch;
    struct vga_registers offset;
    u8 gamma_lut_adjust[256 * 3];
    struct crtc_timings timings;
    u32 scaling_mode;
    struct drm_display_mode native_mode;
};

static inline struct via_crtc *to_via_crtc(struct drm_crtc *crtc)
{
    return container_of(crtc, struct via_crtc, base);
}

struct via_bo {
    struct ttm_buffer_object ttm_bo;
    struct ttm_placement placement;
    struct ttm_place placements[3];
    struct ttm_bo_kmap_obj kmap;
    int pin_count;   /* Add pin_count field */
};

static inline struct via_bo *to_via_bo(struct ttm_buffer_object *bo)
{
    return container_of(bo, struct via_bo, ttm_bo);
}

struct via_i2c {
    struct i2c_adapter adapter;
    struct i2c_algo_bit_data algo;
    void __iomem *mmio;
    u8 port;
};

/* Via driver private structure */
struct via_properties {
    /* properties placeholder */
    int dummy;
};

/**
 * Subclass for struct drm_device
 */
struct via_drm_priv {
    struct drm_device dev;
    /* Memory management */
    phys_addr_t vram_start;
    u64 vram_size;
    void __iomem *mmio;
    phys_addr_t mmio_base;
    u32 mmio_size;
    u8 revision;
    u8 vram_type;
    int vram_mtrr;
    
    /* TTM memory management */
    bool ttm_initialized;
    struct ttm_device bdev;
    struct drm_vma_offset_manager vma_manager;  /* Add VMA manager */
    
    /* Power Management saved registers */
    /* Sequencer registers */
    u8 saved_sr14;
    u8 saved_sr66;
    u8 saved_sr67;
    u8 saved_sr68;
    u8 saved_sr69;
    u8 saved_sr6a;
    u8 saved_sr6b;
    u8 saved_sr6c;
    u8 saved_sr6d;
    u8 saved_sr6e;
    u8 saved_sr6f;
    
    /* CRT Controller registers */
    u8 saved_cr3b;
    u8 saved_cr3c;
    u8 saved_cr3d;
    u8 saved_cr3e;
    u8 saved_cr3f;
    
    /* Display output configuration */
    /* Counter for different display types */
    u32 number_fp;
    u32 number_dvi;
    
    /* I2C bus mapping */
    u32 mapped_i2c_bus;
    
    /* External TMDS (DVI) */
    bool ext_tmds_presence;
    u32 ext_tmds_i2c_bus;
    u32 ext_tmds_di_port;
    struct via_encoder *ext_tmds_encoder;
    
    /* VGA DAC */
    bool dac_presence;
    u32 dac_i2c_bus;
    struct via_encoder *dac;
    
    /* LVDS */
    struct via_encoder *lvds;
    
    /* HDMI */
    struct via_encoder *hdmi;    /* HDMI output */
    
    /* Presence flags */
    bool int_lvds_presence;
    bool hdmi_presence;
    
    /* Hardware-specific quirk flags */
    bool is_via_nanobook;  /* VIA Nanobook specific quirks */
    bool is_quanta_il1;    /* Quanta IL1 specific quirks */
    bool is_samsung_nc20;  /* Samsung NC20 specific quirks */

    /* Display outputs */
    struct via_encoder *tmds;    /* TMDS (DVI) output */

    /* I2C buses for each encoder */
    u32 int_lvds_i2c_bus;
    u32 hdmi_i2c_bus;

    /* Display interface ports */
    u32 int_lvds_di_port;
    u32 hdmi_di_port;
};

// ... rest of existing code ...

static inline struct via_drm_priv *to_via_drm_priv(struct drm_device *dev)
{
    if (!dev) {
        DRM_ERROR("NULL drm_device pointer!\n");
        return NULL;
    }
    return container_of(dev, struct via_drm_priv, dev);
}

/* Register access functions - use vga.h functions */
#define VIA_BASE dev_priv->mmio
#define VIA_READ(reg) ioread32(dev_priv->mmio + (reg))
#define VIA_WRITE(reg, val) iowrite32((val), dev_priv->mmio + (reg))
#define VIA_WRITE_MASK(reg, val, mask) \
    iowrite32(((ioread32(dev_priv->mmio + (reg)) & ~(mask)) | ((val) & (mask))), \
              dev_priv->mmio + (reg))

/* VIA PCI Device IDs */
#define PCI_DEVICE_ID_VIA_CLE266_GFX      0x3122
#define PCI_DEVICE_ID_VIA_KM400_GFX       0x3205
#define PCI_DEVICE_ID_VIA_K8M800_GFX      0x3108
#define PCI_DEVICE_ID_VIA_PM800_GFX       0x3118
#define PCI_DEVICE_ID_VIA_P4M800_PRO_GFX  0x3344
#define PCI_DEVICE_ID_VIA_UNICHROME_PRO_II 0x3258
#define PCI_DEVICE_ID_VIA_CHROME9         0x3157
#define PCI_DEVICE_ID_VIA_P4M890_GFX      0x3343
#define PCI_DEVICE_ID_VIA_CHROME9_HC      0x3225
#define PCI_DEVICE_ID_VIA_CHROME9_HC3     0x3371
#define PCI_DEVICE_ID_VIA_CHROME9_HCM     0x3327
#define PCI_DEVICE_ID_VIA_CHROME9_HD      0x3317

/* PCI Device IDs for host bridges - rename to avoid conflicts */
#define VIA_HOST_BRIDGE_VX800  0x0314
#define VIA_HOST_BRIDGE_VX855  0x0324 
#define VIA_HOST_BRIDGE_VX900  0x0410

/* Memory types - fix naming inconsistencies and add missing types */
#define VIA_MEM_SDR66      0
#define VIA_MEM_SDR100     1
#define VIA_MEM_SDR133     2 
#define VIA_MEM_DDR200     3
#define VIA_MEM_DDR266     4
#define VIA_MEM_DDR333     5
#define VIA_MEM_DDR400     6

/* Add DDR2 memory types */
#define VIA_MEM_DDR2_400   7
#define VIA_MEM_DDR2_533   8
#define VIA_MEM_DDR2_667   9
#define VIA_MEM_DDR2_800   10
#define VIA_MEM_DDR2_1066  11

/* Add DDR3 memory types */
#define VIA_MEM_DDR3_533   12
#define VIA_MEM_DDR3_667   13
#define VIA_MEM_DDR3_800   14
#define VIA_MEM_DDR3_1066  15
#define VIA_MEM_DDR3_1333  16
#define VIA_MEM_DDR3_1600  17

/* Define missing constants */
#define VIA_CURSOR_SIZE    64
#define VIA_MAX_CRTC       2

/* Function prototypes for connector functions */
void via_connector_destroy(struct drm_connector *connector);
int via_connector_init(struct drm_connector *connector,
                     const struct drm_connector_funcs *funcs,
                     int connector_type);
enum drm_connector_status via_dac_detect(struct drm_connector *connector,
                                       bool force);
enum drm_connector_status via_tmds_detect(struct drm_connector *connector,
                                        bool force);
enum drm_connector_status via_lvds_detect(struct drm_connector *connector,
                                        bool force);
                                        
/* Connector function prototypes */
struct drm_connector *via_connector_dac_init(struct drm_device *dev);
struct drm_connector *via_connector_digital_init(struct drm_device *dev);
struct drm_connector *via_connector_lvds_init(struct drm_device *dev);
int via_connector_create(struct drm_device *dev);

/* Function prototypes for encoder/crtc functions */
extern const struct drm_connector_funcs via_dac_connector_funcs;
extern const struct drm_connector_funcs via_dvi_connector_funcs;
extern const struct drm_connector_funcs via_lvds_connector_funcs;
extern const struct drm_encoder_funcs via_crtc_encoder_funcs;
extern const struct drm_encoder_helper_funcs via_crtc_helper_funcs;
extern const struct drm_plane_helper_funcs via_cursor_drm_plane_helper_funcs;
extern const struct drm_plane_funcs via_cursor_drm_plane_funcs;
extern const uint32_t via_cursor_formats[];
extern const size_t via_cursor_formats_size; // Ensure consistent declaration as size_t

int via_crtc_init(struct via_drm_priv *dev_priv, uint32_t index);

/* Forward declarations for I2C functions */
int via_analog_i2c_init(struct via_drm_priv *dev_priv, int bus_index);
int via_digital_i2c_init(struct via_drm_priv *dev_priv, int bus_index);
int via_lvds_i2c_init(struct via_drm_priv *dev_priv, int bus_index);
void via_i2c_readbytes(struct i2c_adapter *adapter,
                     u8 slave_addr, char offset,
                     u8 *buffer, unsigned int size);
void via_i2c_writebytes(struct i2c_adapter *adapter,
                       u8 slave_addr, char offset,
                       u8 *data, unsigned int size);

/* I2C helper functions */
void via_i2c_writebytes(struct i2c_adapter *adapter,
                      u8 slave_addr, char offset,
                      u8 *data, unsigned int size);
void via_i2c_readbytes(struct i2c_adapter *adapter,
                      u8 slave_addr, char offset, 
                      u8 *buffer, unsigned int size);
struct i2c_adapter *via_find_ddc_bus(int ddc_id);

/* Update the externally-declared variable name here back to original */
extern struct via_i2c_par_t via_i2c_par;

/* I2C initialization functions */
int via_i2c_init(struct drm_device *dev);
void via_i2c_exit(void);
void via_i2c_reg_init(struct via_drm_priv *dev_priv);
int via_analog_i2c_init(struct via_drm_priv *dev_priv, int bus_index);
int via_digital_i2c_init(struct via_drm_priv *dev_priv, int bus_index);
int via_lvds_i2c_init(struct via_drm_priv *dev_priv, int bus_index);

/* Scaling mode definitions */
#define VIA_NO_SCALING      0x00000000
#define VIA_HOR_EXPAND      0x00000001
#define VIA_VER_EXPAND      0x00000002
#define VIA_EXPAND          (VIA_HOR_EXPAND | VIA_VER_EXPAND)
#define VIA_SHRINK          0x00000004

/* Scaling constants */
#define HOR_SCALE           0x00000001
#define VER_SCALE           0x00000002
#define HOR_VER_SCALE       (HOR_SCALE | VER_SCALE)

/* Hardware constants */
#define VX800_IGA1_FIFO_MAX_DEPTH            384
#define VX800_IGA1_FIFO_THRESHOLD            328
#define VX800_IGA1_FIFO_HIGH_THRESHOLD       296
#define VX800_IGA1_DISPLAY_QUEUE_EXPIRE_NUM  124

#define VX800_IGA2_FIFO_MAX_DEPTH            384
#define VX800_IGA2_FIFO_THRESHOLD            328
#define VX800_IGA2_FIFO_HIGH_THRESHOLD       296
#define VX800_IGA2_DISPLAY_QUEUE_EXPIRE_NUM  124

/* Function prototypes */
int via_bo_pin(struct via_bo *bo, int mem_type);
void via_bo_unpin(struct via_bo *bo);
extern void via_ttm_bo_destroy(struct ttm_buffer_object *bo);
int via_mm_init(struct drm_device *dev);

/* Add forward declarations for device initialization/finalization functions */
int via_device_init(struct drm_device *dev);
void via_device_fini(struct drm_device *dev);

void via_mm_fini(struct drm_device *dev);
void via_ttm_debugfs_init(struct drm_device *dev);

/* Add missing function prototypes */
int via_bo_create(struct drm_device *dev, struct ttm_device *bdev, 
                 size_t size, enum ttm_bo_type type, uint32_t placement,
                 bool interruptible, struct via_bo **p_bo);
void via_bo_destroy(struct via_bo *bo, bool force);
int via_ttm_domain_to_placement(struct via_bo *bo, uint32_t domain);
void via_ttm_placement(struct ttm_buffer_object *bo, uint32_t domain);
void via_ttm_bo_destroy(struct ttm_buffer_object *bo);
int via_mm_init(struct drm_device *dev);
void via_mm_fini(struct drm_device *dev);
void via_ttm_debugfs_init(struct drm_device *dev);

/* Driver init/cleanup */
int via_drm_init(struct drm_device *dev, unsigned long flags);
void via_drm_fini(struct drm_device *dev);

/* Power management */
int via_dev_pm_ops_suspend(struct device *dev);
int via_dev_pm_ops_resume(struct device *dev);

/* GEM/TTM functions */
int via_gem_alloc_ioctl(struct drm_device *dev, void *data, struct drm_file *file_priv);
int via_gem_mmap_ioctl(struct drm_device *dev, void *data, struct drm_file *file_priv);

/* Function prototypes for encoder/dac functions */
void via_encoder_destroy(struct drm_encoder *encoder);
void via_dac_set_dpms_control(void __iomem *regbase, int dpms_state);
void via_dac_set_power(void __iomem *regbase, bool power_on);
void via_dac_set_sync_polarity(void __iomem *regbase, u8 polarity);
void via_dac_set_display_source(void __iomem *regbase, int source);

/* Function prototypes for crtc functions */
void via_load_crtc_pixel_timing(struct drm_crtc *crtc, struct drm_display_mode *mode);
int via_crtc_init(struct via_drm_priv *dev_priv, uint32_t index);

/* Function prototypes for missing functions */
void svga_wmisc_mask(void __iomem *vgabase, u8 value, u8 mask);
void via_dac_probe(struct drm_device *dev);
void via_dac_init(struct drm_device *dev);
void via_tmds_probe(struct drm_device *dev);
void via_tmds_init(struct drm_device *dev);
void via_lvds_probe(struct drm_device *dev);
void via_lvds_init(struct drm_device *dev);
void via_ext_dvi_probe(struct drm_device *dev);
void via_ext_dvi_init(struct drm_device *dev);
int via_mm_init(struct drm_device *dev);
void via_mm_fini(struct drm_device *dev);

/* Function prototypes for lvds.c exported functions */
void via_lvds_probe(struct drm_device *dev);
void via_lvds_init(struct drm_device *dev);
int via_lvds_power_on(struct drm_device *dev);
int via_lvds_power_off(struct drm_device *dev);
int via_lvds_set_mode(struct drm_device *dev, struct drm_display_mode *mode);

/* Define VGA register port constants as unsigned int to avoid overflow warnings */
#ifndef VGA_SEQ_I
#define VGA_SEQ_I          0x14  /* Use a value that fits in unsigned char */
#endif

#ifndef VGA_CRT_IC
#define VGA_CRT_IC         0x15  /* Use a value that fits in unsigned char */
#endif

#ifndef VGA_GFX_I
#define VGA_GFX_I          0x16  /* Use a value that fits in unsigned char */
#endif

/* Define VGA register port types as enum values instead of constants */
enum via_vga_port_type {
    VIA_PORT_NONE = 0,
    VIA_PORT_SEQ = 1,    /* Sequencer port */
    VIA_PORT_CRT = 2,    /* CRT Controller port */
    VIA_PORT_GFX = 3     /* Graphics Controller port */
};

/* Map standard VGA port defines to our enum values */
#ifndef VGA_SEQ_I
#define VGA_SEQ_I          VIA_PORT_SEQ
#endif

#ifndef VGA_CRT_IC
#define VGA_CRT_IC         VIA_PORT_CRT
#endif

#ifndef VGA_GFX_I
#define VGA_GFX_I          VIA_PORT_GFX
#endif

/* 
 * Forward declarations for via_driver and initialization functions 
 * to ensure proper linking and visibility
 * Update function signature to match implementation in via_drv.c
 */
extern const struct drm_driver via_driver;
extern int via_drm_init(struct drm_device *dev, unsigned long flags);
extern void via_drm_fini(struct drm_device *dev);
extern int via_device_init(struct drm_device *dev);
extern void via_device_fini(struct drm_device *dev);

/* Driver name for identification purposes */
#define DRIVER_NAME "via"

/* Safe modeset initialization functions with timeout protection */
int via_modeset_init_safe(struct drm_device *dev);
void via_modeset_fini_safe(struct drm_device *dev);

/* CRTC initialization functions */
int via_crtc1_init(struct drm_device *dev);
int via_crtc2_init(struct drm_device *dev);

/* Add function prototypes for PCIe functions */
int via_pcie_init(struct drm_device *dev);
void via_pcie_fini(struct drm_device *dev);

/* Add function prototypes for memory manager */
int via_mm_init(struct drm_device *dev);
void via_mm_fini(struct drm_device *dev);

/* Add function prototypes for IRQ handler */
int via_irq_init(struct drm_device *dev);
void via_irq_fini(struct drm_device *dev);
irqreturn_t via_irq_handler(int irq, void *arg);

/* Add function prototypes for command verifier */
int via_verifier_init(struct drm_device *dev);
void via_verifier_fini(struct drm_device *dev);

/* Add function prototypes for DMA BLIT */
int via_dmablit_init(struct drm_device *dev);
void via_dmablit_fini(struct drm_device *dev);

/* CRTC init functions */
int via_crtc1_init(struct drm_device *dev);
int via_crtc2_init(struct drm_device *dev);

/* Add missing prototype for via_driver_dumb_map_offset */
int via_driver_dumb_map_offset(struct drm_file *file_priv,
                              struct drm_device *dev,
                              uint32_t handle,
                              uint64_t *offset);

/* I2C bus identifiers - keep only one definition, not both
 * Remove the duplicate definitions and keep only these:
 */
#define VIA_I2C_NONE  0x00
#define VIA_I2C_BUS1  0x01
#define VIA_I2C_BUS2  0x02
#define VIA_I2C_BUS3  0x04
#define VIA_I2C_BUS4  0x08
#define VIA_I2C_BUS5  0x10

/* Encoder type definitions */
enum via_encoder_type {
    VIA_ENCODER_NONE = 0,
    VIA_ENCODER_DAC,     /* VGA analog output */
    VIA_ENCODER_TMDS,    /* Integrated TMDS/DVI */
    VIA_ENCODER_LVDS,    /* LVDS output for laptop panels */
    VIA_ENCODER_TV,      /* TV encoders */
    VIA_ENCODER_HDMI,    /* HDMI output */
    VIA_ENCODER_DVO      /* Digital Video Output */
};

#endif /* __VIA_DRV_H__ */
