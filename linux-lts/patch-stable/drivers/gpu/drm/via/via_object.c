/*
 * Copyright © 2018-2019 Kevin Brace.
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
 *
 * Author(s):
 * Kevin Brace <kevinbrace@bracecomputerlab.com>
 */
/*
 * via_object.c
 *
 * Manages Buffer Objects (BO) via TTM.
 * Part of the TTM memory allocator.
 *
 */

#include <drm/drm_file.h>
#include <drm/drm_gem.h>
#include <drm/drm_gem_ttm_helper.h>

#include <drm/ttm/ttm_bo.h>
#include <drm/ttm/ttm_range_manager.h>

#include "via_drv.h"


static void via_gem_free(struct drm_gem_object *obj)
{
	struct ttm_buffer_object *ttm_bo = container_of(obj,
				struct ttm_buffer_object, base);
	struct drm_device *dev = ttm_bo->base.dev;

	drm_dbg_driver(dev, "Entered %s.\n", __func__);

	ttm_bo_put(ttm_bo);

	drm_dbg_driver(dev, "Exiting %s.\n", __func__);
}

static const struct vm_operations_struct via_ttm_bo_vm_ops = {
	.fault = ttm_bo_vm_fault,
	.open = ttm_bo_vm_open,
	.close = ttm_bo_vm_close,
	.access = ttm_bo_vm_access
};

static const struct drm_gem_object_funcs via_gem_object_funcs = {
	.free = via_gem_free,
	.vmap = drm_gem_ttm_vmap,
	.vunmap = drm_gem_ttm_vunmap,
	.mmap = drm_gem_ttm_mmap,
	.vm_ops = &via_ttm_bo_vm_ops,
};

/* Keep these as the canonical TTM domain placement functions */
int via_ttm_domain_to_placement(struct via_bo *bo, uint32_t domain)
{
	struct ttm_buffer_object *ttm_bo = &bo->ttm_bo;
	struct drm_device *dev = ttm_bo->base.dev;
	unsigned i = 0;

	drm_dbg_driver(dev, "Entered %s.\n", __func__);

	bo->placement.placement = bo->placements;

	if (domain == TTM_PL_SYSTEM) {
		bo->placements[i].fpfn = 0;
		bo->placements[i].lpfn = 0;
		bo->placements[i].mem_type = TTM_PL_SYSTEM;
		bo->placements[i].flags = 0;
		i++;
	}

	if (domain == TTM_PL_TT) {
		bo->placements[i].fpfn = 0;
		bo->placements[i].lpfn = 0;
		bo->placements[i].mem_type = TTM_PL_TT;
		bo->placements[i].flags = 0;
		i++;
	}

	if (domain == TTM_PL_VRAM) {
		bo->placements[i].fpfn = 0;
		bo->placements[i].lpfn = 0;
		bo->placements[i].mem_type = TTM_PL_VRAM;
		bo->placements[i].flags = 0;
		i++;
	}

	bo->placement.num_placement = i;

	drm_dbg_driver(dev, "Exiting %s.\n", __func__);
	return 0;  /* Return int value to match declaration */
}

/* Add proper prototype to avoid warning */
void via_ttm_bo_destroy(struct ttm_buffer_object *tbo);

void via_ttm_bo_destroy(struct ttm_buffer_object *tbo)
{
	struct via_bo *bo;
	struct drm_device *dev = tbo->base.dev;

	drm_dbg_driver(dev, "Entered %s.\n", __func__);

	/* Fix: use to_via_bo instead of to_ttm_bo */
	bo = to_via_bo(tbo);

	drm_gem_object_release(&tbo->base);
	kfree(bo);

	drm_dbg_driver(dev, "Exiting %s.\n", __func__);
}

/* Fix parameter type to match the declaration (uint32_t -> int) */
int via_bo_pin(struct via_bo *bo, int ttm_domain)
{
	struct ttm_buffer_object *ttm_bo = &bo->ttm_bo;
	struct drm_device *dev = ttm_bo->base.dev;
	struct ttm_operation_ctx ctx = {false, false};
	int ret = 0;

	drm_dbg_driver(dev, "Entered %s.\n", __func__);

	if (ttm_bo->pin_count) {
		goto pin;
	}

	via_ttm_domain_to_placement(bo, ttm_domain);
	ret = ttm_bo_validate(ttm_bo, &bo->placement, &ctx);
	if (ret) {
		goto exit;
	}

pin:
	ttm_bo_pin(ttm_bo);
exit:

	drm_dbg_driver(dev, "Exiting %s.\n", __func__);
	return ret;
}

void via_bo_unpin(struct via_bo *bo)
{
	struct ttm_buffer_object *ttm_bo = &bo->ttm_bo;
	struct drm_device *dev = ttm_bo->base.dev;

	drm_dbg_driver(dev, "Entered %s.\n", __func__);

	ttm_bo_unpin(ttm_bo);

	drm_dbg_driver(dev, "Exiting %s.\n", __func__);
}

/* Fix parameter type to match the declaration (uint64_t -> size_t) */
int via_bo_create(struct drm_device *dev,
			struct ttm_device *bdev,
			size_t size,
			enum ttm_bo_type type,
			uint32_t ttm_domain,
			bool kmap,
			struct via_bo **bo_ptr)
{
	struct ttm_buffer_object *ttm_bo;
	struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
	struct via_bo *bo;
	int ret;

	drm_dbg_driver(dev, "Entered %s.\n", __func__);

	bo = kzalloc(sizeof(*bo), GFP_KERNEL);
	if (!bo) {
		drm_err(dev, "Cannot allocate a TTM buffer object!\n");
		ret = -ENOMEM;
		goto exit;
	}

	ttm_bo = &bo->ttm_bo;

	/*
	 * It is an imperative to page align the requested buffer size
	 * prior to a memory allocation request, or various memory
	 * allocation related system instabilities may occur.
	 */
	size = ALIGN(size, PAGE_SIZE);

	ret = drm_gem_object_init(dev, &ttm_bo->base, size);
	if (ret) {
		drm_err(dev, "Cannot initialize a GEM object!\n");
		goto error;
	}

	ttm_bo->base.funcs = &via_gem_object_funcs;

	via_ttm_domain_to_placement(bo, ttm_domain);
	ret = ttm_bo_init_validate(&dev_priv->bdev, ttm_bo,
				type, &bo->placement,
				PAGE_SIZE >> PAGE_SHIFT, false,
				NULL, NULL, via_ttm_bo_destroy);
	if (ret) {
		drm_err(dev, "Cannot initialize a TTM object!\n");
		goto error;  /* Changed from exit to error for proper cleanup */
	}

	if (kmap) {
		ret = ttm_bo_reserve(ttm_bo, true, false, NULL);
		if (ret) {
			ttm_bo_put(ttm_bo);
			goto exit;
		}

		ret = via_bo_pin(bo, ttm_domain);
		ttm_bo_unreserve(ttm_bo);
		if (ret) {
			ttm_bo_put(ttm_bo);
			goto exit;
		}

		ret = ttm_bo_kmap(ttm_bo, 0, PFN_UP(ttm_bo->resource->size),
					&bo->kmap);
		if (ret) {
			ttm_bo_put(ttm_bo);
			goto exit;
		}
	}

	*bo_ptr = bo;
	goto exit;
error:
	kfree(bo);
exit:
	drm_dbg_driver(dev, "Exiting %s.\n", __func__);
	return ret;
}

void via_bo_destroy(struct via_bo *bo, bool kmap)
{
	struct ttm_buffer_object *ttm_bo = &bo->ttm_bo;
	struct drm_device *dev = ttm_bo->base.dev;
	int ret;

	drm_dbg_driver(dev, "Entered %s.\n", __func__);

	if (kmap) {
		ttm_bo_kunmap(&bo->kmap);

		ret = ttm_bo_reserve(ttm_bo, true, false, NULL);
		if (ret) {
			goto exit;
		}

		via_bo_unpin(bo);
		ttm_bo_unreserve(ttm_bo);
		if (ret) {
			goto exit;
		}
	}

	ttm_bo_put(ttm_bo);
exit:
	drm_dbg_driver(dev, "Exiting %s.\n", __func__);
}

/* Add declaration for via_bo_driver - use extern to reference the actual declaration in via_ttm.c */
extern struct ttm_device_funcs via_bo_driver;

/* Add declaration for via_ttm_debugfs_init */
void via_ttm_debugfs_init(struct drm_device *dev);

/* 
 * Remove the implementations of via_mm_init and via_mm_fini
 * They are now defined in via_mm.c
 * Keep only the external declarations here
 */
extern int via_mm_init(struct drm_device *dev);
extern void via_mm_fini(struct drm_device *dev);
