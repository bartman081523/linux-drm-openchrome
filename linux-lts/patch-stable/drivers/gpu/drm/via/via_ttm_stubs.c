/*
 * Copyright 2023 The OpenChrome Project
 * [via_ttm_stubs.c]
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

#include <drm/drm_gem.h>
#include <drm/drm_gem_ttm_helper.h>
#include <linux/module.h>
#include <linux/io.h>

/* 
 * Implementation stubs for GEM TTM functions for compatibility with
 * newer kernel interfaces that use iosys_map
 */

/**
 * drm_gem_ttm_mmap - TTM mmap support for GEM objects
 * @gem_obj: The GEM object to map
 * @vma: the VMA to contain the mapping
 * 
 * Provides compatibility with newer kernel interfaces
 */
int drm_gem_ttm_mmap(struct drm_gem_object *gem_obj, struct vm_area_struct *vma)
{
    struct drm_device *dev = gem_obj->dev;
    int ret;
    
    /* Use drm_gem_mmap_obj which takes a GEM object directly */
    ret = drm_gem_mmap_obj(gem_obj, gem_obj->size, vma);
    if (ret < 0) {
        drm_dbg(dev, "failed to mmap GEM object: %d\n", ret);
    }
    
    return ret;
}
EXPORT_SYMBOL_GPL(drm_gem_ttm_mmap);

/**
 * drm_gem_ttm_vmap - Map a GEM object using TTM helpers
 * @gem_obj: The GEM object to map
 * @map: iosys_map structure to store the mapping information
 * 
 * Compatibility layer for modern kernel interfaces.
 */
int drm_gem_ttm_vmap(struct drm_gem_object *gem_obj, struct iosys_map *map)
{
    struct ttm_buffer_object *bo = container_of(gem_obj, struct ttm_buffer_object, base);
    
    /* Call the ttm_bo_vmap function with the proper parameters */
    return ttm_bo_vmap(bo, map);
}
EXPORT_SYMBOL_GPL(drm_gem_ttm_vmap);

/**
 * drm_gem_ttm_vunmap - Unmap a previously mapped GEM object
 * @gem_obj: The GEM object to unmap
 * @map: iosys_map structure containing mapping information
 */
void drm_gem_ttm_vunmap(struct drm_gem_object *gem_obj, struct iosys_map *map)
{
    struct ttm_buffer_object *bo = container_of(gem_obj, struct ttm_buffer_object, base);
    
    /* Call the ttm_bo_vunmap function with the proper parameters */
    ttm_bo_vunmap(bo, map);
}
EXPORT_SYMBOL_GPL(drm_gem_ttm_vunmap);
