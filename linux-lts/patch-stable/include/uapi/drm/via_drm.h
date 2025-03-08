/*
 * VIA DRM UAPI definitions
 */

#ifndef _VIA_DRM_H_
#define _VIA_DRM_H_

#include "drm.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* VIA specific ioctls */
#define DRM_VIA_GEM_ALLOC   0x20
#define DRM_VIA_GEM_MMAP    0x21

#define DRM_IOCTL_VIA_GEM_ALLOC   DRM_IOWR(DRM_COMMAND_BASE + DRM_VIA_GEM_ALLOC, struct drm_via_gem_alloc)
#define DRM_IOCTL_VIA_GEM_MMAP    DRM_IOWR(DRM_COMMAND_BASE + DRM_VIA_GEM_MMAP, struct drm_via_gem_mmap)

/* Structures for VIA-specific ioctls */
struct drm_via_gem_alloc {
    uint64_t size;
    uint32_t handle;
    uint32_t alignment;
    uint32_t initial_domain;
    uint32_t pad;
};

struct drm_via_gem_mmap {
    uint32_t handle;
    uint32_t pad;
    uint64_t offset;
    uint64_t size;
    uint64_t addr_ptr;
};

#if defined(__cplusplus)
}
#endif

#endif /* _VIA_DRM_H_ */
