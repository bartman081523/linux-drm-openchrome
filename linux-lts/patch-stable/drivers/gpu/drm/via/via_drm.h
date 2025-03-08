// ...existing code...

/*
 * Below are the existing definitions - keep these and remove duplicates
 */
#define DRM_VIA_GEM_ALLOC 0x20
#define DRM_VIA_GEM_MMAP 0x21

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

/*
 * IOCTL definitions - using the existing values defined above
 */
#define DRM_IOCTL_VIA_GEM_ALLOC DRM_IOWR(DRM_COMMAND_BASE + DRM_VIA_GEM_ALLOC, struct drm_via_gem_alloc)
#define DRM_IOCTL_VIA_GEM_MMAP  DRM_IOWR(DRM_COMMAND_BASE + DRM_VIA_GEM_MMAP, struct drm_via_gem_mmap)

// ...existing code...

/* REMOVE THE FOLLOWING DUPLICATE DEFINITIONS:
#define DRM_VIA_GEM_ALLOC 0x00
#define DRM_VIA_GEM_MMAP  0x01

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
*/

// ...existing code...
