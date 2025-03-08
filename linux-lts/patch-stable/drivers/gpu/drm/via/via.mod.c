#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

MODULE_INFO(intree, "Y");

KSYMTAB_FUNC(load_value_to_registers, "_gpl", "");
KSYMTAB_FUNC(load_register_tables, "_gpl", "");
KSYMTAB_DATA(vpit_table, "_gpl", "");
KSYMTAB_DATA(lcd_hor_scaling, "_gpl", "");
KSYMTAB_DATA(lcd_ver_scaling, "_gpl", "");
KSYMTAB_DATA(iga1_hor_total, "_gpl", "");
KSYMTAB_DATA(iga1_hor_addr, "_gpl", "");
KSYMTAB_DATA(iga1_hor_blank_start, "_gpl", "");
KSYMTAB_DATA(iga1_hor_blank_end, "_gpl", "");
KSYMTAB_DATA(iga1_hor_sync_start, "_gpl", "");
KSYMTAB_DATA(iga1_hor_sync_end, "_gpl", "");
KSYMTAB_DATA(iga1_ver_total, "_gpl", "");
KSYMTAB_DATA(iga1_ver_addr, "_gpl", "");
KSYMTAB_DATA(iga1_ver_blank_start, "_gpl", "");
KSYMTAB_DATA(iga1_ver_blank_end, "_gpl", "");
KSYMTAB_DATA(iga1_ver_sync_start, "_gpl", "");
KSYMTAB_DATA(iga1_ver_sync_end, "_gpl", "");
KSYMTAB_DATA(iga1_fetch_count, "_gpl", "");
KSYMTAB_DATA(iga1_offset, "_gpl", "");
KSYMTAB_DATA(iga2_hor_total, "_gpl", "");
KSYMTAB_DATA(iga2_hor_addr, "_gpl", "");
KSYMTAB_DATA(iga2_hor_blank_start, "_gpl", "");
KSYMTAB_DATA(iga2_hor_blank_end, "_gpl", "");
KSYMTAB_DATA(iga2_hor_sync_start, "_gpl", "");
KSYMTAB_DATA(iga2_hor_sync_end, "_gpl", "");
KSYMTAB_DATA(iga2_ver_total, "_gpl", "");
KSYMTAB_DATA(iga2_ver_addr, "_gpl", "");
KSYMTAB_DATA(iga2_ver_blank_start, "_gpl", "");
KSYMTAB_DATA(iga2_ver_blank_end, "_gpl", "");
KSYMTAB_DATA(iga2_ver_sync_start, "_gpl", "");
KSYMTAB_DATA(iga2_ver_sync_end, "_gpl", "");
KSYMTAB_DATA(iga2_fetch_count, "_gpl", "");
KSYMTAB_DATA(iga2_offset, "_gpl", "");
KSYMTAB_FUNC(via_load_regset, "_gpl", "");
KSYMTAB_DATA(iga1_cle266_fifo_depth_select, "_gpl", "");
KSYMTAB_DATA(iga1_k8m800_fifo_depth_select, "_gpl", "");
KSYMTAB_DATA(iga2_cle266_fifo_depth_select, "_gpl", "");
KSYMTAB_DATA(iga2_k8m800_fifo_depth_select, "_gpl", "");
KSYMTAB_DATA(iga1_cle266_fifo_threshold_select, "_gpl", "");
KSYMTAB_DATA(iga1_k8m800_fifo_threshold_select, "_gpl", "");
KSYMTAB_DATA(iga2_cle266_fifo_threshold_select, "_gpl", "");
KSYMTAB_DATA(iga2_k8m800_fifo_threshold_select, "_gpl", "");
KSYMTAB_DATA(iga1_cle266_fifo_high_threshold_select, "_gpl", "");
KSYMTAB_DATA(iga1_k8m800_fifo_high_threshold_select, "_gpl", "");
KSYMTAB_DATA(iga2_fifo_high_threshold_select, "_gpl", "");
KSYMTAB_DATA(iga1_cle266_display_queue_expire_num, "_gpl", "");
KSYMTAB_DATA(iga1_k8m800_display_queue_expire_num, "_gpl", "");
KSYMTAB_DATA(iga2_display_queue_expire_num, "_gpl", "");
KSYMTAB_FUNC(via_iga2_set_interlace_mode, "_gpl", "");
KSYMTAB_FUNC(via_iga2_set_palette_lut_resolution, "_gpl", "");
KSYMTAB_FUNC(via_iga1_set_interlace_mode, "_gpl", "");
KSYMTAB_FUNC(via_iga1_set_hsync_shift, "_gpl", "");
KSYMTAB_FUNC(via_iga1_set_palette_lut_resolution, "_gpl", "");
KSYMTAB_DATA(via_crtc_encoder_funcs, "_gpl", "");
KSYMTAB_DATA(via_crtc_helper_funcs, "_gpl", "");
KSYMTAB_FUNC(via_driver_dumb_map_offset, "_gpl", "");
KSYMTAB_FUNC(via_crtc1_init, "_gpl", "");
KSYMTAB_FUNC(via_crtc2_init, "_gpl", "");
KSYMTAB_DATA(via_i2c_adapters, "_gpl", "");
KSYMTAB_DATA(via_i2c_par, "_gpl", "");
KSYMTAB_FUNC(drm_gem_ttm_mmap, "_gpl", "");
KSYMTAB_FUNC(drm_gem_ttm_vmap, "_gpl", "");
KSYMTAB_FUNC(drm_gem_ttm_vunmap, "_gpl", "");

MODULE_INFO(depends, "ttm");

MODULE_ALIAS("pci:v00001106d00003122sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00001106d00007205sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00001106d00003108sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00001106d00003344sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00001106d00003118sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00001106d00003343sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00001106d00003230sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00001106d00003371sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00001106d00003157sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00001106d00001122sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00001106d00005122sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00001106d00007122sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "29A4C679C02C99E4AF9AE87");
