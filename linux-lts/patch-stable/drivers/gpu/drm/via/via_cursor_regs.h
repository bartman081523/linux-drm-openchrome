/*
 * Copyright 2023 The OpenChrome Project
 * [via_cursor_regs.h]
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

#ifndef __VIA_CURSOR_REGS_H__
#define __VIA_CURSOR_REGS_H__

/* Hardware cursor register definitions */
#define HI_CONTROL                  0x0310
#define HI_TRANSPARENT_COLOR        0x0314
#define HI_INVTCOLOR                0x0318
#define HI_FBOFFSET                 0x031C
#define HI_POSSTART                 0x0320
#define HI_CENTEROFFSET             0x0324

/* Primary hardware cursor registers */
#define PRIM_HI_CTRL                0x0310
#define PRIM_HI_TRANSCOLOR          0x0314
#define PRIM_HI_INVTCOLOR           0x0318
#define PRIM_HI_FBOFFSET            0x031C
#define PRIM_HI_POSSTART            0x0320
#define PRIM_HI_CENTEROFFSET        0x0324

/* V327 hardware cursor registers */
#define V327_HI_INVTCOLOR           0x0518

/* Alpha blending registers for hardware cursor */
#define ALPHA_V3_PREFIFO_CONTROL    0x0508
#define ALPHA_V3_FIFO_CONTROL       0x050C
#define PRIM_HI_FIFO                0x0510

#endif /* __VIA_CURSOR_REGS_H__ */
