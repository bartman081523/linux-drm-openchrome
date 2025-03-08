#
# Makefile for the drm device driver.  This driver provides support for the
# Direct Rendering Infrastructure (DRI) in XFree86 4.1.0 and higher.

# Ensure proper dependencies for external module build

ifneq ($(KERNELRELEASE),)
# We're in kernel build system
include Kbuild

else
# We're being built as an external module
KDIR ?= /lib/modules/$(shell uname -r)/build
MDIR ?= /lib/modules/$(shell uname -r)/kernel/drivers/gpu/drm

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install:
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install
	depmod -a

# Copy required symbol files from kernel directory
symvers:
	cp $(KDIR)/Module.symvers $(PWD)/
	if [ -f $(MDIR)/drm.symvers ]; then \
		cp $(MDIR)/drm.symvers $(PWD)/; \
	fi
	if [ -f $(MDIR)/ttm/ttm.symvers ]; then \
		cp $(MDIR)/ttm/ttm.symvers $(PWD)/; \
	fi

.PHONY: all clean install symvers
endif

ccflags-y := -Iinclude/drm
ccflags-y += -I$(src)
ccflags-y += -include $(src)/drmP_compat.h

# Define module object dependencies correctly
# Only list files that actually exist in the source tree
via-y := \
	via_drv.o \
	via_pcie.o \
	via_mm.o \
	via_init.o \
	via_regset.o \
	via_irq.o \
	via_verifier.o \
	via_dmablit.o \
	via_pll.o \
	via_vgahw.o \
	via_pm.o \
	via_encoder.o \
	via_crtc.o \
	via_crtc_hw.o \
	via_cursor.o \
	via_tmds.o \
	via_hdmi.o \
	via_lvds.o \
	via_dac.o \
	via_vt1632.o \
	via_sii164.o \
	via_tx.o \
	via_ioctl.o \
	via_connector.o \
	via_connector_i2c.o \
	via_i2c.o \
	via_ttm.o \
	via_ttm_stubs.o \
	via_object.o \
	via_modeset.o

# Add TTM and DRM dependencies explicitly
KBUILD_EXTRA_SYMBOLS := $(shell find /lib/modules/$(shell uname -r)/kernel/drivers/gpu/drm -name Module.symvers -print)

obj-m += via.o

# Export dependencies to modpost
ifneq ($(CONFIG_DRM),)
  ccflags-y += -DHAVE_DRM_CONFIG_H
endif
