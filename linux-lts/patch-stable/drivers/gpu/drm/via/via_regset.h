/*
 * Copyright 2023 The OpenChrome Project
 * [via_regset.h]
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
 * THE AUTHOR(S) OR COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef __VIA_REGSET_H__
#define __VIA_REGSET_H__

#include <linux/io.h>
#include <video/vga.h>
#include "via_drv.h" 

/* Array sizes for all register sets */
#define VIA_LCD_HOR_SCALING_SIZE 1
#define VIA_LCD_VER_SCALING_SIZE 1
#define VIA_IGA1_HOR_TOTAL_SIZE 1
#define VIA_IGA1_HOR_ADDR_SIZE 1
#define VIA_IGA1_HOR_BLANK_START_SIZE 1
#define VIA_IGA1_HOR_BLANK_END_SIZE 1
#define VIA_IGA1_HOR_SYNC_START_SIZE 1
#define VIA_IGA1_HOR_SYNC_END_SIZE 1
#define VIA_IGA1_VER_TOTAL_SIZE 1
#define VIA_IGA1_VER_ADDR_SIZE 1
#define VIA_IGA1_VER_BLANK_START_SIZE 1
#define VIA_IGA1_VER_BLANK_END_SIZE 1
#define VIA_IGA1_VER_SYNC_START_SIZE 1
#define VIA_IGA1_VER_SYNC_END_SIZE 1
#define VIA_IGA1_FETCH_COUNT_SIZE 1
#define VIA_IGA1_OFFSET_SIZE 1

#define VIA_IGA2_HOR_TOTAL_SIZE 1
#define VIA_IGA2_HOR_ADDR_SIZE 1
#define VIA_IGA2_HOR_BLANK_START_SIZE 1
#define VIA_IGA2_HOR_BLANK_END_SIZE 1
#define VIA_IGA2_HOR_SYNC_START_SIZE 1
#define VIA_IGA2_HOR_SYNC_END_SIZE 1
#define VIA_IGA2_VER_TOTAL_SIZE 1
#define VIA_IGA2_VER_ADDR_SIZE 1
#define VIA_IGA2_VER_BLANK_START_SIZE 1
#define VIA_IGA2_VER_BLANK_END_SIZE 1
#define VIA_IGA2_VER_SYNC_START_SIZE 1
#define VIA_IGA2_VER_SYNC_END_SIZE 1
#define VIA_IGA2_FETCH_COUNT_SIZE 1
#define VIA_IGA2_OFFSET_SIZE 2

#define VIA_IGA1_CLE266_FIFO_DEPTH_SIZE 1
#define VIA_IGA1_K8M800_FIFO_DEPTH_SIZE 1
#define VIA_IGA2_CLE266_FIFO_DEPTH_SIZE 1
#define VIA_IGA2_K8M800_FIFO_DEPTH_SIZE 1
#define VIA_IGA1_CLE266_FIFO_THRESHOLD_SIZE 1
#define VIA_IGA1_K8M800_FIFO_THRESHOLD_SIZE 1
#define VIA_IGA2_CLE266_FIFO_THRESHOLD_SIZE 1
#define VIA_IGA2_K8M800_FIFO_THRESHOLD_SIZE 1
#define VIA_IGA1_CLE266_FIFO_HIGH_THRESHOLD_SIZE 1
#define VIA_IGA1_K8M800_FIFO_HIGH_THRESHOLD_SIZE 1
#define VIA_IGA2_FIFO_HIGH_THRESHOLD_SIZE 1
#define VIA_IGA1_CLE266_DISPLAY_QUEUE_SIZE 1
#define VIA_IGA1_K8M800_DISPLAY_QUEUE_SIZE 1
#define VIA_IGA2_DISPLAY_QUEUE_SIZE 1

/* 
 * IMPORTANT: Do NOT redefine struct vga_regset here!
 * It's already defined in via_drv.h
 */

/* External register definitions */
extern const struct vga_regset iga1_cle266_fifo_depth_select[];
extern const struct vga_regset iga1_k8m800_fifo_depth_select[];
extern const struct vga_regset iga2_cle266_fifo_depth_select[];
extern const struct vga_regset iga2_k8m800_fifo_depth_select[];
extern const struct vga_regset iga1_cle266_fifo_threshold_select[];
extern const struct vga_regset iga1_k8m800_fifo_threshold_select[];
extern const struct vga_regset iga2_cle266_fifo_threshold_select[];
extern const struct vga_regset iga2_k8m800_fifo_threshold_select[];
extern const struct vga_regset iga1_cle266_fifo_high_threshold_select[];
extern const struct vga_regset iga1_k8m800_fifo_high_threshold_select[];
extern const struct vga_regset iga2_fifo_high_threshold_select[];
extern const struct vga_regset iga1_cle266_display_queue_expire_num[];
extern const struct vga_regset iga1_k8m800_display_queue_expire_num[];
extern const struct vga_regset iga2_display_queue_expire_num[];
extern const struct vga_regset lcd_hor_scaling[];
extern const struct vga_regset lcd_ver_scaling[];
extern const struct vga_regset iga1_hor_total[];
extern const struct vga_regset iga1_hor_addr[];
extern const struct vga_regset iga1_hor_blank_start[];
extern const struct vga_regset iga1_hor_blank_end[];
extern const struct vga_regset iga1_hor_sync_start[];
extern const struct vga_regset iga1_hor_sync_end[];
extern const struct vga_regset iga1_ver_total[];
extern const struct vga_regset iga1_ver_addr[];
extern const struct vga_regset iga1_ver_blank_start[];
extern const struct vga_regset iga1_ver_blank_end[];
extern const struct vga_regset iga1_ver_sync_start[];
extern const struct vga_regset iga1_ver_sync_end[];
extern const struct vga_regset iga1_fetch_count[];
extern const struct vga_regset iga1_offset[];
extern const struct vga_regset iga2_hor_total[];
extern const struct vga_regset iga2_hor_addr[];
extern const struct vga_regset iga2_hor_blank_start[];
extern const struct vga_regset iga2_hor_blank_end[];
extern const struct vga_regset iga2_hor_sync_start[];
extern const struct vga_regset iga2_hor_sync_end[];
extern const struct vga_regset iga2_ver_total[];
extern const struct vga_regset iga2_ver_addr[];
extern const struct vga_regset iga2_ver_blank_start[];
extern const struct vga_regset iga2_ver_blank_end[];
extern const struct vga_regset iga2_ver_sync_start[];
extern const struct vga_regset iga2_ver_sync_end[];
extern const struct vga_regset iga2_fetch_count[];
extern const struct vga_regset iga2_offset[];

/* VGA Init Table */
extern struct vga_registers vpit_table[];

/* Function prototypes for register operations */
void load_value_to_registers(void __iomem *regbase, const struct vga_registers *regs, 
                         unsigned int value);
void load_register_tables(void __iomem *regbase, struct vga_registers *table);

/* Adapter function for compatibility */
void via_load_regset(void __iomem *regbase, struct vga_registers *regs, unsigned int value);

#endif /* __VIA_REGSET_H__ */
