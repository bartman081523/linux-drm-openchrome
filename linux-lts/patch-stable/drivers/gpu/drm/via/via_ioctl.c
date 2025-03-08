/*
 * Copyright © 2019 Kevin Brace.
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

#include <linux/stat.h>

#include <drm/drm_file.h>

#include <uapi/drm/via_drm.h>

#include "via_drv.h"
#include "via_drv_main.h"
#include "via_object.h"
#include "via_ioctl.h"

int via_gem_alloc_ioctl(struct drm_device *dev, void *data,
			      struct drm_file *file_priv)
{
	struct drm_via_gem_alloc *args = data;
	struct via_bo *bo;
	struct via_drm_priv *dev_priv = to_via_drm_priv(dev);
	struct ttm_buffer_object *ttm_bo;
	int ret;

	/* Allocate buffer object - using bdev instead of ttm.bdev */
	ret = via_bo_create(dev, &dev_priv->bdev, args->size,
				ttm_bo_type_device, args->initial_domain, false, &bo);
	if (ret)
		return ret;

	/* bo doesn't have base field, ttm_bo is returned directly from via_bo_create */
	ttm_bo = &bo->ttm_bo;  /* Access the ttm_bo member of the via_bo structure */

	/* Create mapping */
	ret = via_dumb_create_mapping(file_priv, dev, bo, &args->handle);
	if (ret)
		goto err_unref;

	ret = ttm_bo_reserve(ttm_bo, true, false, NULL);
	if (unlikely(ret))
		goto err;

	ttm_bo_unreserve(ttm_bo);

	args->handle    = args->handle;
	args->size      = ttm_bo->resource->size;
	args->initial_domain = ttm_bo->resource->placement;
	args->alignment = PAGE_SIZE;
	return 0;

err:
	drm_gem_handle_delete(file_priv, args->handle);
err_unref:
	/* Fix the kref_put call to use the proper type */
	ttm_bo_put(ttm_bo);
	return ret;
}

int via_gem_mmap_ioctl(struct drm_device *dev, void *data,
			     struct drm_file *file_priv)
{
	struct drm_gem_object *gem;
	struct ttm_buffer_object *ttm_bo;
	struct drm_via_gem_mmap *args = data;
	int ret = 0;

	gem = drm_gem_object_lookup(file_priv, args->handle);
	if (!gem) {
		ret = -EINVAL;
		drm_err(dev, "%s: Failed to find GEM object.\n", __func__);
		goto exit;
	}

	ttm_bo = container_of(gem, struct ttm_buffer_object, base);
	args->offset = drm_vma_node_offset_addr(&ttm_bo->base.vma_node);
	drm_gem_object_put(gem);

exit:
	return ret;
}

int via_driver_dumb_map_offset(struct drm_file *file_priv,
                        struct drm_device *dev,
                        uint32_t handle,
                        uint64_t *offset)
{
    struct drm_gem_object *gem;
    struct ttm_buffer_object *ttm_bo;
    int ret = 0;

    drm_dbg_driver(dev, "Entered %s.\n", __func__);

    gem = drm_gem_object_lookup(file_priv, handle);
    if (!gem) {
        drm_err(dev, "%s: Failed to find GEM object.\n", __func__);
        return -ENOENT; // Fix: Added proper error return for gem lookup failure
    }

    ttm_bo = container_of(gem, struct ttm_buffer_object, base);
    *offset = drm_vma_node_offset_addr(&ttm_bo->base.vma_node);

    drm_gem_object_put(gem);
    drm_dbg_driver(dev, "Exiting %s.\n", __func__);
    return ret;
}
EXPORT_SYMBOL_GPL(via_driver_dumb_map_offset);
