# OpenChrome Driver Notes

## Kernel 6.12+ Porting Notes

### Key Architecture Changes

The OpenChrome driver has undergone significant changes to support kernel 6.12+:

1. **TTM Memory Management**:
   - Updated to use modern TTM APIs with `ttm_device` structure
   - Removed legacy TTM initialization methods
   - Properly implemented TTM resource management with range managers

2. **DRM Subsystem Integration**:
   - Migrated to atomic modesetting helpers
   - Updated connector/encoder interfaces
   - Implemented modern DRM GEM/TTM helpers

3. **Hardware Initialization**:
   - Improved initialization sequence with proper timeouts
   - Added safety checks to prevent NULL pointer dereferences
   - Better error handling during device initialization

### Known Issues

- Some hardware configurations may still experience instability
- Power management functionality requires further testing
- Some video outputs may not be properly detected depending on hardware

### Development Tips

- Use `drm.debug=0x1f` and `dyndbg=+pflmt` kernel parameters for debugging
- Monitor dmesg for TTM initialization errors as these are common failure points
- When adding new hardware support, always implement proper connector detection

## Performance Considerations

The VIA/OpenChrome hardware has limitations that affect performance:

- Maximum resolution and refresh rates are limited by the hardware's capabilities
- Older chipsets (CLE266, KM400) have very limited 3D acceleration
- Memory bandwidth is significantly constrained compared to competitors

## Future Directions

- Better suspend/resume support
- Complete DRI3 integration
- Improved multi-head support
- Better handling of hotplugging

# OpenChrome VIA DRM Driver Notes

## Overview
This document describes the architecture of the OpenChrome DRM driver and the changes needed for compatibility with Linux 6.12.

## Current Status
The driver compiles but needs several fixes to function correctly. We've addressed compilation issues including missing helper functions, type mismatches, and header inclusion order.

## Fixed Issues

### Symbol Visibility Issues
Multiple function definitions and declarations were fixed to ensure proper symbol visibility:

1. Functions used in multiple files now have correct `extern` declarations in headers
2. Functions used only in one file are properly marked `static`
3. Removed duplicate function definitions that caused "multiple definition" errors

### Function Signature Mismatches
Fixed several function signatures to ensure they match across declaration and implementation:

1. Fixed `via_drm_init` to include the correct `flags` parameter
2. Updated various BO-related functions to match their usage
3. Fixed encoder and connector function signatures

### Structure and Define Issues
Added missing structure definitions and correct field sizes:

1. Added proper defines for register set sizes
2. Fixed register handling functions

## Remaining Issues

- The driver needs more testing with actual hardware
- Some display types may not be fully supported yet
- Need to verify full compatibility with 6.12 kernel interfaces

## To Do
See the 6.12_PORTING_TODO.md file for the current task list.

## Contributing
When making changes, please follow the code style of the existing codebase and ensure all functions are properly documented.

## References
- UniChrome Pro II Graphics Processor Programming Manual (OGPM_UniChrome_ProII_R108a_Part1_Core_2D.txt)
- Linux DRM subsystem documentation

## Header Organization

- `via_drv.h`: Main header file with core data structures, function prototypes, and macros.
  MUST be included first in all OpenChrome source files.
- `via_crtc_hw.h`: Definitions and declarations related to CRTC hardware registers and operations.
- `via_disp_reg.h`:  Defines for register offsets.
- Other headers include their specific functionality (connectors, encoders, PLL, etc.)

## Key Decisions and Implementation Notes

### Header Inclusion Order

The correct include order is *critical* to avoid compilation errors due to missing or conflicting definitions. The required order is:

1. **`via_drv.h`:**  This MUST be the very first include in every `.c` file.
2. **`via_crtc_hw.h`:**  Includes register definitions and low-level hardware manipulation functions.
3. **`via_disp_reg.h`:** Defines for register offsets.
4. **Other headers:**  Standard Linux headers, DRM headers, etc.

### Structure Definitions

- All core data structures (`via_crtc`, `via_connector`, `via_encoder`, `via_bo`, etc.) are defined in `via_drv.h`.
- `vga_regset` and `vga_registers`, are defined in `via_crtc_hw.h`

### Register Access

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

### TTM Buffer Management

- The driver uses the TTM (Translation Table Manager) framework for managing graphics memory.
- Buffer objects are represented by `struct via_bo`, which wraps a `ttm_buffer_object`.
- The `via_ttm_domain_to_placement` function sets up the memory placement policies for buffer objects.
- The `via_bo_pin` and `via_bo_unpin` functions handle pinning and unpinning buffers in VRAM.
- To convert from ttm_buffer_object to via_bo, use the to_via_bo helper function.

### Connector Detection

- The `via_dac_detect`, `via_tmds_detect`, and `via_lvds_detect` functions are stubs.  They currently always return `connector_status_connected`.
- These functions *must* be implemented to correctly detect connected displays, likely using I2C communication.

### I2C Communication

- The `via_i2c.c` file provides basic I2C bit-banging functions. These are currently stubs.
- The `via_find_ddc_bus` function is a stub and needs to be implemented to map I2C bus numbers to actual `i2c_adapter` structures.
- The I2C functions will be used for:
    - Detecting connected displays (via EDID).
    - Controlling external transmitter chips (e.g., SiI 164, VT1632).

### Mode Setting

- The `via_crtc.c` file handles CRTC (display controller) configuration.
- `via_load_crtc_timing` loads timing parameters into CRTC registers.
- `via_iga1_display_fifo_regs` and `via_iga2_display_fifo_regs` configure the display FIFO.
- `via_set_iga_scale_function` and `via_load_iga_scale_factor_regs` handle scaling (when the source and destination resolutions don't match).
- `via_mode_set_nofb` is the main function that sets up the CRTC for a given display mode.

### Encoder/Transmitter Control

- Separate files (`via_dac.c`, `via_tmds.c`, `via_lvds.c`, `via_hdmi.c`, `via_sii164.c`, `via_vt1632.c`) handle the specifics of each display output type.
- These files provide functions for:
    - DPMS control (powering on/off the output).
    - Mode setting (configuring the transmitter for the specific display mode).
    - Detecting connected displays.

### Cursor Handling

- `via_cursor.c` handles the hardware cursor.

### Remaining Tasks (High Priority)

1. **Implement I2C Functions:** Implement `via_i2c_readbytes`, `via_i2c_writebytes`, and `via_find_ddc_bus` in `via_i2c.c`. This is *essential* for detecting displays and configuring external transmitters.

2. **Implement Connector Detection:** Implement the `via_dac_detect`, `via_tmds_detect`, and `via_lvds_detect` functions in `via_connector.c`. These should use the I2C functions to read EDID data and/or check GPIO pins.

3. **Populate Register Tables:** The `iga1_...` and `iga2_...` register arrays in `via_crtc.c` and `via_regset.c` are currently empty.  These *must* be populated with the correct register settings for each supported chipset and display mode. This is the most complex and hardware-specific part of the port.  You *must* refer to the VIA hardware documentation.

4. **Implement Mode Setting:** Complete the `via_mode_set_nofb` function in `via_crtc.c`. This involves:
   * Setting up the CRTC timings (horizontal/vertical sync, blanking, etc.).  You'll use the `vga_wcrt` and `vga_wseq` functions extensively here, along with the data from the `vga_regset` structures.
   * Configuring the PLL (Phase-Locked Loop) to generate the correct pixel clock.  You have a `via_get_clk_value` function, but it needs to be integrated into the mode setting process.
   * Setting up the display FIFO.
   * Enabling/disabling scaling as needed.
   * Setting the color depth and pixel format.

5. **Complete Encoder Functions:** Implement the remaining functions in `via_tmds.c`, `via_lvds.c`, and `via_hdmi.c` to control the various transmitters.

### Debugging Tips

* **Use `drm_dbg_kms`:** This macro is your friend.  Use it extensively to print debugging information at various points in your code.  You can enable/disable different debug categories using the `drm.debug` kernel parameter.
* **Kernel Debugger:** If possible, use a kernel debugger (like kgdb) to step through the code and inspect register values. This is invaluable for understanding the hardware's behavior.
* **Register Dumps:** Add code to dump the values of relevant VGA registers before and after your changes.  This will help you verify that your register writes are having the intended effect.
* **Start Simple:**  Focus on getting a basic display mode working first (e.g., 640x480).  Once you have that working, you can add support for more complex modes and features.
* **Incremental Changes:**  Make small, incremental changes and test frequently.  This makes it much easier to isolate the cause of any problems.
* **Reference Information:** Make extensive use of the UniChrome Pro II Graphics Processor Programming Manual to understand register values and bit meanings.

This `NOTES.md` file should be updated as you make progress, to keep track of your decisions and any remaining issues. Good luck!