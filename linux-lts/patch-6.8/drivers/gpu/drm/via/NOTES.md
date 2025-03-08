# OpenChrome DRM Driver (Stable 6.8 Kernel) - Developer Notes

## Overview

This document outlines the key considerations and implementation details for the OpenChrome DRM driver, specifically adapted for a stable 6.8 kernel. This version is derived from a newer, potentially unstable, codebase, and this file highlights the differences, backporting choices, and any limitations.

## Status (Stable Branch)

This driver targets the stable 6.8 Linux kernel. The goal is to provide basic modesetting support with a focus on stability and compatibility. Advanced features (like 3D acceleration, advanced power management, or complex display configurations) may be limited or absent.

**Key Differences from Unstable/Development:**

- **Focus on Stability:** The primary goal is rock-solid stability. New features and optimizations that could introduce regressions are generally avoided.
- **Reduced Feature Set:** This version is likely to have a smaller feature set than any development branches. Features known to cause issues on older VIA hardware, or features that require significant kernel API changes, may be omitted. The focus is *modesetting*.
- **Conservative Register Access:** The driver prioritizes safe and well-understood register access. Experimental or poorly documented hardware features are avoided.
- **Limited Hardware Support:** The driver may support a narrower range of VIA chipsets compared to upstream. The priority is to support the most common chipsets and configurations reliably.

## Implementation Details and Developer Notes

### 1. General Structure

The overall structure of the driver mirrors a standard DRM driver:

- `via_drv.c`: Core driver initialization, device probing, file operations, and top-level ioctls.
- `via_crtc.c`: CRTC (display controller) management, mode setting, and framebuffer setup.
- `via_connector.c`: Connector detection, EDID parsing, and mode enumeration.
- `via_encoder.c`:  Encoder initialization (abstract base).
- `via_dac.c`, `via_lvds.c`, `via_sii164.c`, `via_vt1632.c`, `via_hdmi.c`, `via_tmds.c`:  Specific encoder implementations (DAC, LVDS, HDMI, TMDS).
- `via_object.c`:  Buffer object management (GEM/TTM integration).
- `via_ttm.c`:  TTM (Translation Table Manager) integration for memory management.
- `via_i2c.c`:  I2C bit-banging routines for communication with external devices (e.g., monitors, encoders).
- `via_pm.c`: Power management functions, including suspend/resume support.
- `via_vgahw.c`, `via_vgahw.h`: Low-level VGA register access functions.
- `via_3d_reg.h`, `via_disp_reg.h`, `via_regs.h`: Register definitions.
- `via_crtc_hw.h`: CRTC related hardware definitions.
- `via_regset.c`: Contains pre-defined register value sets for different chipsets and modes.

### 2. Header Inclusion Order
The correct include order is *critical* to avoid compilation errors due to missing or conflicting definitions. The required order is:

1.   **`via_drv.h`:** This MUST be the very first include in every `.c` file.
2.   **`via_crtc_hw.h`:** Includes register definitions and low-level hardware manipulation functions. (If applicable to that file)
3.   **`via_disp_reg.h`:** Defines for register offsets. (If applicable to that file).
4.   **Other headers:** Standard Linux headers, DRM headers, etc.

### 3. Structure Definitions

- All core data structures (`via_crtc`, `via_connector`, `via_encoder`, `via_bo`, etc.) are defined in `via_drv.h`.
- `vga_regset` and `vga_registers`, are defined in `via_crtc_hw.h`.

### 4. Register Access

- The driver uses a combination of direct MMIO access (for non-VGA registers) and VGA I/O port access (for VGA registers).
- **MMIO Access:**
  - `VIA_READ(reg)`: Reads a 32-bit value from the specified MMIO register offset.
  - `VIA_WRITE(reg, val)`: Writes a 32-bit value to the specified MMIO register offset.
  - `VIA_WRITE_MASK(reg, val, mask)`: Performs a read-modify-write to update specific bits within an MMIO register.
- **VGA I/O Port Access:**
  - Uses standard functions, defined in `via_vgahw.c` which use inline functions from `<video/vga.h>`:
    - `vga_rcrt(base, index)`: Reads from a CRTC register.
    - `vga_wcrt(base, index, value)`: Writes to a CRTC register.
    - `vga_rseq(base, index)`: Reads from a Sequencer register.
    - `vga_wseq(base, index, value)`: Writes to a Sequencer register.
    - `svga_wcrt_mask(base, index, value, mask)`:  Masked write to a CRTC register.
    - `svga_wseq_mask(base, index, value, mask)`: Masked write to a Sequencer register.
-  `base` for memory mapped registers is `dev_priv->mmio`.

### 5. TTM Buffer Management

- The driver uses the TTM (Translation Table Manager) framework for managing graphics memory.
- Buffer objects are represented by `struct via_bo`, which wraps a `ttm_buffer_object`.
- The `via_ttm_domain_to_placement` function sets up the memory placement policies for buffer objects.
- The `via_bo_pin` and `via_bo_unpin` functions handle pinning and unpinning buffers in VRAM.

### 6. Connector Detection

- The `via_dac_detect`, `via_tmds_detect`, and `via_lvds_detect` functions perform connector detection.
- These use a combination of I2C communication (to read EDID) and direct register reads to sense connection status.

### 7. I2C Communication

- The `via_i2c.c` file provides basic I2C bit-banging functions.
- The `via_find_ddc_bus` function maps I2C bus numbers to actual `i2c_adapter` structures.
- The I2C functions are used for:
    - Detecting connected displays (via EDID).
    - Controlling external transmitter chips (e.g., SiI 164, VT1632).

### 8. Mode Setting

- The `via_crtc.c` file handles CRTC (display controller) configuration.
- `via_load_crtc_timing` loads timing parameters into CRTC registers.
- `via_iga1_display_fifo_regs` and `via_iga2_display_fifo_regs` configure the display FIFO.
- `via_set_iga_scale_function` and `via_load_iga_scale_factor_regs` handle scaling (when the source and destination resolutions don't match).
- `via_mode_set_nofb` is the main function that sets up the CRTC for a given display mode.

### 9. Encoder/Transmitter Control

- Separate files (`via_dac.c`, `via_tmds.c`, `via_lvds.c`, `via_hdmi.c`, `via_sii164.c`, `via_vt1632.c`) handle the specifics of each display output type.
- These files provide functions for:
    - DPMS control (powering on/off the output).
    - Mode setting (configuring the transmitter for the specific display mode).
    - Detecting connected displays.

### 10. Cursor Handling

- `via_cursor.c` handles the hardware cursor.

### 11. Hardware Specific Quirks

-   `via_quirks_init` in `via_init.c` handles hardware-specific workarounds. These are crucial for stability and correct operation on different VIA chipsets and board designs. The existing quirks are:
    -   **VIA NanoBook Detection:**  Identifies VIA NanoBook reference designs (and clones like the Everex CloudBook and Sylvania g netbook) using PCI subsystem IDs (vendor 0x1509, device 0x2d30). This is important because these devices have incorrect strapping resistors, affecting DVI functionality.  The `is_via_nanobook` flag is set accordingly.

    -   **Quanta IL1 Detection:**  Identifies the Quanta IL1 netbook (vendor 0x152d, device 0x0771).  This is important because the IL1 connects its flat panel to DVP1 (Digital Video Port 1) instead of the usual LVDS channel. The `is_quanta_il1` flag is set.

    -   **Samsung NC20 Detection:** Identifies the Samsung NC20 netbook (vendor 0x144d, device 0xc04e). This netbook uses LVDS2 for its internal flat panel, rather than LVDS1.  The `is_samsung_nc20` flag is set.

-   **Implication:** These quirks directly influence display detection, power management, and encoder configuration.  Any changes to these detection routines *must* be thoroughly tested on the affected hardware.

### 12. Display FIFO Configuration

-   The `via_iga1_display_fifo_regs` and `via_iga2_display_fifo_regs` functions configure the display FIFO (First-In, First-Out) buffer.  The FIFO is crucial for smooth display updates, preventing underruns.
-   The FIFO depth, threshold, and high threshold values are chipset-specific and sometimes depend on the display resolution and color depth.  These values are hardcoded in large switch statements, based on PCI device ID and revision.
-   The code uses macros like `IGA1_FIFO_DEPTH_SELECT_FORMULA`, `IGA1_FIFO_LOW_THRESHOLD_SELECT_FORMULA`, etc., to calculate register values from pixel counts.
-   **CLE266/KM400 Specifics:** These older chipsets have "extended display FIFO" capabilities that are enabled/disabled based on the display resolution.  The code explicitly handles this.
-   **Developer Note:**  The FIFO settings are *critical* for preventing display glitches and artifacts. Incorrect FIFO settings can lead to flickering, tearing, or complete display failure.  The current hardcoded values are likely based on empirical testing and should be treated as "known good" configurations.  Any changes *must* be thoroughly tested on a wide range of resolutions and refresh rates.

### 13. CRTC Timing Loading (`via_load_crtc_timing`)

-   This function takes a `drm_display_mode` structure (which contains the detailed timing parameters for a display mode) and programs the VGA CRTC registers.
-   It uses helper macros (e.g., `IGA1_PIXELTIMING_HOR_TOTAL_FORMULA`, `IGA1_PIXELTIMING_VER_BLANK_START_FORMULA`) to convert the mode parameters (in pixels and lines) into the values expected by the hardware registers.
-   Separate register tables (e.g., `iga1_hor_total`, `iga2_ver_sync_end`) are used for each CRTC register, allowing for chipset-specific variations and handling of overflow bits.
-   The code handles interlaced modes by adjusting the vertical timings and setting the appropriate bit in CRTC register 0x32.
-   **Developer Note:** The timing formulas and register tables are *highly* hardware-specific.  Do not modify these without a deep understanding of the VGA timing specifications and the VIA hardware documentation.  Incorrect timing parameters can damage the display.

### 14. Scaling (`via_set_scale_path`, `via_disable_iga_scaling`, `via_set_iga_scale_function`, `via_load_iga_scale_factor_regs`)

- The driver supports both upscaling (expanding a smaller source image to a larger display) and downscaling (shrinking a larger source image to a smaller display).
- `via_set_scale_path` selects which IGA (Integrated Graphics Accelerator, i.e., CRTC) is used for scaling.
- `via_disable_iga_scaling` disables all scaling.
- `via_set_iga_scale_function` enables scaling and selects the scaling type (horizontal, vertical, or both). It also selects the interpolation method (e.g., bilinear).
- `via_load_iga_scale_factor_regs` calculates and loads the scaling factors into the hardware registers. The scaling factors are based on the ratio between the source and destination resolutions.
-   **Developer Note:** Scaling is a complex operation, and the register settings are intertwined with the CRTC timing parameters.  The existing code provides a basic framework, but more advanced scaling features (e.g., different scaling filters) might be difficult to implement.  Testing on various display resolutions is crucial.

### 15. PLL (Phase-Locked Loop) Configuration (`via_get_clk_value`, `via_set_vclock`)

-   The `via_get_clk_value` function calculates the PLL register values (M, N, R dividers) required to generate the desired pixel clock frequency. It uses different formulas for older (CLE266/KM400) and newer chipsets. The calculations are based on a reference clock frequency (`VIA_CLK_REFERENCE`).
-   `via_set_vclock` programs the calculated PLL values into the hardware registers. It also handles resetting the PLL.
-   **Developer Note:** Incorrect PLL settings can lead to unstable display output or no output at all.  The formulas used here are specific to the VIA hardware and should be carefully reviewed.

### 16. Power Management

- The `via_dac_dpms`, `via_tmds_dpms`, and similar functions handle the low-level power management of the display outputs (DAC, LVDS, DVI, HDMI). This typically involves enabling/disabling the output and controlling the power to the transmitter.
-   The global power state (`via_dev_pm_ops_suspend`, `via_dev_pm_ops_resume`) uses hooks for low-level power management via a call to the `via_vram_fini` function.  These save and restore specific VGA registers (SR14, SR66-SR6F, and CR3B-CR3F) that are known to lose their values during power state transitions on certain chipsets.  This is a hardware-specific workaround.

### 17. Framebuffer / Cursor Initialization

   - The modesetting code creates a framebuffer (`drm_fb_helper`) in the init function to provide a default framebuffer if one isn't already configured.
   - The cursor is setup using the planes helper functions.
   - The driver has no support for overlay planes, so the code only uses the primary and cursor plane functions.

### 18. Chipset / Revision Info.

The via_chip_revision_info function stores important
chipset revision data for use in the VIA_READ and VIA_WRITE macros
for handling differences between hardware register locations.

### 19. Debugging

- The code uses the `drm_dbg_kms` macro for debug messages.
- Kernel command-line parameters:
	- `drm.debug=0x0e`: This will enable KMS debugging messages (among others). The bitmask values are defined in `drm_print.h`.

This enhanced `NOTES.md` provides a comprehensive overview of the OpenChrome DRM driver's code for the stable 6.8 kernel, highlighting the key implementation details, hardware-specific considerations, and areas where caution is needed.
