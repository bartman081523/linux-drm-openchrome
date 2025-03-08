/*
 * Copyright 2023 The OpenChrome Project
 * [drmP_compat.h]
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

/*
 * Compatibility header for building with kernel 6.12+
 * This provides compatibility definitions for symbols that
 * might have changed between kernel versions.
 */

#ifndef __DRMP_COMPAT_H__
#define __DRMP_COMPAT_H__

#include <linux/version.h>
#include <linux/moduleparam.h>
#include <linux/module.h>
#include <asm/io.h>

/*
 * Handle MTRR compatibility between kernels
 * The signatures changed between kernel versions:
 * - arch_phys_wc_add returns an int in newer kernels
 * - Methods for releasing WC memory differ between kernel versions
 */
#ifndef VIA_COMPAT_ARCH_IO_FREE_WC_DEFINED
#define VIA_COMPAT_ARCH_IO_FREE_WC_DEFINED

/* Helper for add/remove writecombining for MTRR regions */
static inline int via_add_wc_mtrr(resource_size_t base, resource_size_t size)
{
    return arch_phys_wc_add(base, size);
}

static inline void via_free_wc_mtrr(int mtrr)
{
    if (mtrr > 0)
        arch_phys_wc_del(mtrr);
}

#endif /* VIA_COMPAT_ARCH_IO_FREE_WC_DEFINED */

/* Handle symbol exports for out-of-tree module */
#define drm_compat_export_symbol_gpl(sym) \
    EXPORT_SYMBOL_GPL(sym)

#endif /* __DRMP_COMPAT_H__ */
