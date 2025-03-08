/*
 * Copyright 2023 The OpenChrome Project
 * [via_ioctl.h]
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

#ifndef __VIA_IOCTL_H__
#define __VIA_IOCTL_H__

#include <drm/drm_gem.h>
#include <drm/drm_ioctl.h>
#include <drm/drm_file.h>

/* Forward declarations for IOCTL functions */
int via_gem_alloc_ioctl(struct drm_device *dev, void *data, struct drm_file *file_priv);
int via_gem_mmap_ioctl(struct drm_device *dev, void *data, struct drm_file *file_priv);
int via_driver_dumb_map_offset(struct drm_file *file_priv,
                              struct drm_device *dev,
                              uint32_t handle,
                              uint64_t *offset);

#endif /* __VIA_IOCTL_H__ */
