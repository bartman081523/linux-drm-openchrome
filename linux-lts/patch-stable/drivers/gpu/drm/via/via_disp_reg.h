/*
 * Copyright 2023 The OpenChrome Project
 * [via_disp_reg.h]
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
 */

#ifndef __VIA_DISP_REG_H__
#define __VIA_DISP_REG_H__

/* VIA display register definitions */
#define VIA_REG_DISPLAY_CTRL      0x0080
#define VIA_REG_DISPLAY_STATUS    0x0084
#define VIA_REG_DISPLAY_CONFIG    0x0088

/* Display engine control bits */
#define VIA_DISP_ENABLE           0x00000001
#define VIA_DISP_RESET            0x00000002
#define VIA_DISP_SUSPEND          0x00000004

/* Pixel timing register definitions - comment out the conflicting definition */
/* #define IGA1_PIX_HALF_LINE_REG    0x8DD4 */
#define IGA1_PIX_HALF_LINE_MASK   0x1FFF

/* IGA1 timing formulas */
#define IGA1_HOR_TOTAL_FORMULA(x) ((x) / 8 - 5)
#define IGA1_HOR_ADDR_FORMULA(x) ((x) / 8 - 1)
#define IGA1_HOR_BLANK_START_FORMULA(x) ((x) / 8 - 1)
#define IGA1_HOR_BLANK_END_FORMULA(x) ((x) / 8 - 1)
#define IGA1_HOR_SYNC_START_FORMULA(x) ((x) / 8)
#define IGA1_HOR_SYNC_END_FORMULA(x) ((x) / 8)
#define IGA1_VER_TOTAL_FORMULA(x) ((x) - 2)
#define IGA1_VER_ADDR_FORMULA(x) ((x) - 1)
#define IGA1_VER_BLANK_START_FORMULA(x) ((x) - 1)
#define IGA1_VER_BLANK_END_FORMULA(x) ((x) - 1)
#define IGA1_VER_SYNC_START_FORMULA(x) ((x) - 1)
#define IGA1_VER_SYNC_END_FORMULA(x) ((x) - 1)

/* IGA2 timing formulas */
#define IGA2_HOR_TOTAL_FORMULA(x) ((x) / 8 - 5)
#define IGA2_HOR_ADDR_FORMULA(x) ((x) / 8 - 1)
#define IGA2_HOR_BLANK_START_FORMULA(x) ((x) / 8 - 1)
#define IGA2_HOR_BLANK_END_FORMULA(x) ((x) / 8 - 1)
#define IGA2_HOR_SYNC_START_FORMULA(x) ((x) / 8)
#define IGA2_HOR_SYNC_END_FORMULA(x) ((x) / 8)
#define IGA2_VER_TOTAL_FORMULA(x) ((x) - 2)
#define IGA2_VER_ADDR_FORMULA(x) ((x) - 1)
#define IGA2_VER_BLANK_START_FORMULA(x) ((x) - 1)
#define IGA2_VER_BLANK_END_FORMULA(x) ((x) - 1)
#define IGA2_VER_SYNC_START_FORMULA(x) ((x) - 1)
#define IGA2_VER_SYNC_END_FORMULA(x) ((x) - 1)

/* Pixel timing formulas */
#define IGA1_PIXELTIMING_HOR_TOTAL_FORMULA(x) ((x) - 1)
#define IGA1_PIXELTIMING_HOR_ADDR_FORMULA(x) ((x) - 1)
#define IGA1_PIXELTIMING_HOR_BLANK_START_FORMULA(x) ((x) - 1)
#define IGA1_PIXELTIMING_HOR_BLANK_END_FORMULA(x) ((x) - 1)
#define IGA1_PIXELTIMING_HOR_SYNC_START_FORMULA(x) ((x) - 1)
#define IGA1_PIXELTIMING_HOR_SYNC_END_FORMULA(x) ((x) - 1)
#define IGA1_PIXELTIMING_VER_TOTAL_FORMULA(x) ((x) - 1)
#define IGA1_PIXELTIMING_VER_ADDR_FORMULA(x) ((x) - 1)
#define IGA1_PIXELTIMING_VER_BLANK_START_FORMULA(x) ((x) - 1)
#define IGA1_PIXELTIMING_VER_BLANK_END_FORMULA(x) ((x) - 1)
#define IGA1_PIXELTIMING_VER_SYNC_START_FORMULA(x) ((x) - 1)
#define IGA1_PIXELTIMING_VER_SYNC_END_FORMULA(x) ((x) - 1)
#define IGA1_PIXELTIMING_HVSYNC_OFFSET_END_FORMULA(h, s) (((h) - (s)) / 2)

/* FIFO depth and threshold formulas */
#define IGA1_FIFO_DEPTH_SELECT_FORMULA(x) ((x) >> 1)
#define IGA1_FIFO_THRESHOLD_FORMULA(x) (((x) >> 1) - 1)
#define IGA1_FIFO_HIGH_THRESHOLD_FORMULA(x) (((x) * 3 / 4) - 1)
#define IGA1_FIFO_LOW_THRESHOLD_SELECT_FORMULA(x) (((x) >> 2) - 1)
#define IGA2_FIFO_DEPTH_SELECT_FORMULA(x) ((x) >> 2)

/* VX800 FIFO constants */
#define CN700_IGA1_FIFO_MAX_DEPTH            192
#define CN700_IGA1_FIFO_THRESHOLD            128
#define CN700_IGA1_FIFO_HIGH_THRESHOLD       64
#define CN700_IGA1_DISPLAY_QUEUE_EXPIRE_NUM  124

#define CX700_IGA1_FIFO_MAX_DEPTH            192
#define CX700_IGA1_FIFO_THRESHOLD            128
#define CX700_IGA1_FIFO_HIGH_THRESHOLD       64
#define CX700_IGA1_DISPLAY_QUEUE_EXPIRE_NUM  124

#define K8M890_IGA1_FIFO_MAX_DEPTH           384
#define K8M890_IGA1_FIFO_THRESHOLD           328
#define K8M890_IGA1_FIFO_HIGH_THRESHOLD      296
#define K8M890_IGA1_DISPLAY_QUEUE_EXPIRE_NUM 124

#define P4M890_IGA1_FIFO_MAX_DEPTH           384
#define P4M890_IGA1_FIFO_THRESHOLD           328
#define P4M890_IGA1_FIFO_HIGH_THRESHOLD      296
#define P4M890_IGA1_DISPLAY_QUEUE_EXPIRE_NUM 124

#define P4M900_IGA1_FIFO_MAX_DEPTH           384
#define P4M900_IGA1_FIFO_THRESHOLD           328
#define P4M900_IGA1_FIFO_HIGH_THRESHOLD      296
#define P4M900_IGA1_DISPLAY_QUEUE_EXPIRE_NUM 124

#define VX855_IGA1_FIFO_MAX_DEPTH            384
#define VX855_IGA1_FIFO_THRESHOLD            328
#define VX855_IGA1_FIFO_HIGH_THRESHOLD       296
#define VX855_IGA1_DISPLAY_QUEUE_EXPIRE_NUM  124

#define VX900_IGA1_FIFO_MAX_DEPTH            384
#define VX900_IGA1_FIFO_THRESHOLD            328
#define VX900_IGA1_FIFO_HIGH_THRESHOLD       296
#define VX900_IGA1_DISPLAY_QUEUE_EXPIRE_NUM  124

/* IGA2 FIFO constants */
#define CN700_IGA2_FIFO_MAX_DEPTH            96
#define CN700_IGA2_FIFO_THRESHOLD            64
#define CN700_IGA2_FIFO_HIGH_THRESHOLD       32
#define CN700_IGA2_DISPLAY_QUEUE_EXPIRE_NUM  124

#define CX700_IGA2_FIFO_MAX_DEPTH            96
#define CX700_IGA2_FIFO_THRESHOLD            64
#define CX700_IGA2_FIFO_HIGH_THRESHOLD       32
#define CX700_IGA2_DISPLAY_QUEUE_EXPIRE_NUM  124

#define K8M890_IGA2_FIFO_MAX_DEPTH           384
#define K8M890_IGA2_FIFO_THRESHOLD           328
#define K8M890_IGA2_FIFO_HIGH_THRESHOLD      296
#define K8M890_IGA2_DISPLAY_QUEUE_EXPIRE_NUM 124

#define P4M890_IGA2_FIFO_MAX_DEPTH           384
#define P4M890_IGA2_FIFO_THRESHOLD           328
#define P4M890_IGA2_FIFO_HIGH_THRESHOLD      296
#define P4M890_IGA2_DISPLAY_QUEUE_EXPIRE_NUM 124

#define P4M900_IGA2_FIFO_MAX_DEPTH           384
#define P4M900_IGA2_FIFO_THRESHOLD           328
#define P4M900_IGA2_FIFO_HIGH_THRESHOLD      296
#define P4M900_IGA2_DISPLAY_QUEUE_EXPIRE_NUM 124

#define VX855_IGA2_FIFO_MAX_DEPTH            384
#define VX855_IGA2_FIFO_THRESHOLD            328
#define VX855_IGA2_FIFO_HIGH_THRESHOLD       296
#define VX855_IGA2_DISPLAY_QUEUE_EXPIRE_NUM  124

#define VX900_IGA2_FIFO_MAX_DEPTH            384
#define VX900_IGA2_FIFO_THRESHOLD            328
#define VX900_IGA2_FIFO_HIGH_THRESHOLD       296
#define VX900_IGA2_DISPLAY_QUEUE_EXPIRE_NUM  124

#endif /* __VIA_DISP_REG_H__ */
