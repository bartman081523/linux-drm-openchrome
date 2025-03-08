/*
 * Copyright 2023 The OpenChrome Project
 * [via_object.h]
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

#ifndef __VIA_OBJECT_H__
#define __VIA_OBJECT_H__

#include <drm/drm_gem.h>
#include "via_drv.h"

/* Buffer object functions - updated signatures to match implementations */
extern int via_ttm_domain_to_placement(struct via_bo *bo, uint32_t domain);
extern int via_bo_create(struct drm_device *dev, struct ttm_device *bdev, 
                        size_t size, enum ttm_bo_type bo_type, 
                        uint32_t flags, bool interruptible, 
                        struct via_bo **pvbo);
extern void via_bo_destroy(struct via_bo *bo, bool force);
extern int via_bo_pin(struct via_bo *bo, int domain);
extern void via_bo_unpin(struct via_bo *bo);
extern void via_ttm_bo_destroy(struct ttm_buffer_object *bo);

#endif /* __VIA_OBJECT_H__ */
