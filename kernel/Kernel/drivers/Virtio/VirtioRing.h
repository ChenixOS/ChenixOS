#pragma once

#include <types.h>

#include "drivers/PCI/PCI.h"


/* Status byte for guest to report progress, and synchronize features. */
/* We have seen device and processed generic fields (VIRTIO_CONFIG_F_VIRTIO) */
#define VIRTIO_CONFIG_S_ACKNOWLEDGE     1
/* We have found a driver for the device. */
#define VIRTIO_CONFIG_S_DRIVER          2
/* Driver has used its parts of the config, and is happy */
#define VIRTIO_CONFIG_S_DRIVER_OK       4
/* Driver has finished configuring features */
#define VIRTIO_CONFIG_S_FEATURES_OK     8
/* We've given up on this device. */
#define VIRTIO_CONFIG_S_FAILED          0x80

/* v1.0 compliant. */
#define VIRTIO_F_VERSION_1              32
#define VIRTIO_F_IOMMU_PLATFORM         33

#define VRING_DESC_F_NEXT  1
#define VRING_DESC_F_WRITE 2

#define VRING_AVAIL_F_NO_INTERRUPT 1
#define VRING_USED_F_NO_NOTIFY     1

struct vring_desc
{
    uint64 addr;
    uint32 len;
    uint16 flags;
    uint16 next;
};

struct vring_avail
{
    uint16 flags;
    uint16 index;
    uint16 ring[];
}; // vring_avail

struct vring_used_item
{
    uint32 id;
    uint32 length;
};

struct vring_used
{
    uint16 flags;
    uint16 index;
    struct vring_used_item ring[];
};
// ======================================================================
struct vring;

#define vring_size(num) \
    (ALIGN(sizeof(struct vring_desc) * num + sizeof(struct vring_avail) \
           + sizeof(uint16) * num, 4096)                              \
     + sizeof(struct vring_used) + sizeof(struct vring_used_item) * num)


#define MAX_QUEUE_NUM      (256)
typedef unsigned char virtio_queue_t[vring_size(MAX_QUEUE_NUM)];

struct vp_device;

// ======================================================================

struct vring {
    uint32 num;
    struct vring_desc *desc;
    struct vring_avail *avail;
    struct vring_used *used;

    // Function
    vring(unsigned int num, unsigned char *queue);
};

struct vring_virtqueue {
   virtio_queue_t queue;
   struct vring vring;
   uint16 free_head;
   uint16 last_used_idx;
   uint16 vdata[MAX_QUEUE_NUM];
   /* PCI */
   int queue_index;
   int queue_notify_off;

    // Function
    int MoreUsed();
    void Detach(uint32 head);
    int GetBuffer(uint32* len);
    void AddBuffer(struct vring_list list[],
                   unsigned int out,
                   unsigned int in,
                   int index,
                   int num_added);
    
    void Kick(struct vp_device *vp, int num_added);
};

struct vring_list {
  char *addr;
  unsigned int length;
};


