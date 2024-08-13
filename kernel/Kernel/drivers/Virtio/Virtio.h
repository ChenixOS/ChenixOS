#pragma once

#include "VirtioRing.h"
#include "drivers/PCI/PCI.h"

/* The bit of the ISR which indicates a device configuration change. */
#define VIRTIO_PCI_ISR_CONFIG           0x2

/* Virtio ABI version, this must match exactly */
#define VIRTIO_PCI_ABI_VERSION          0

/* --- virtio 0.9.5 (legacy) struct --------------------------------- */

typedef struct virtio_pci_legacy {
    uint32 host_features;
    uint32 guest_features;
    uint32 queue_pfn;
    uint16 queue_num;
    uint16 queue_sel;
    uint16 queue_notify;
    uint8  status;
    uint8  isr;
    uint8  device[];
} virtio_pci_legacy;

/* --- virtio 1.0 (modern) structs ---------------------------------- */

/* Common configuration */
#define VIRTIO_PCI_CAP_COMMON_CFG       1
/* Notifications */
#define VIRTIO_PCI_CAP_NOTIFY_CFG       2
/* ISR access */
#define VIRTIO_PCI_CAP_ISR_CFG          3
/* Device specific configuration */
#define VIRTIO_PCI_CAP_DEVICE_CFG       4
/* PCI configuration access */
#define VIRTIO_PCI_CAP_PCI_CFG          5

/* This is the PCI capability header: */
struct virtio_pci_cap {
    uint8 cap_vndr;          /* Generic PCI field: PCI_CAP_ID_VNDR - 0 */ 
    uint8 cap_next;          /* Generic PCI field: next ptr. - 1 */
    uint8 cap_len;           /* Generic PCI field: capability length - 2 */
    uint8 cfg_type;          /* Identifies the structure. - 3 */
    uint8 bar;               /* Where to find it. - 4 */
    uint8 padding[3];        /* Pad to full dword. - 5-7 */
    uint32 offset;           /* Offset within bar. - 8-11 */
    uint32 length;           /* Length of the structure, in bytes. 12-15 */
};

struct virtio_pci_notify_cap {
    struct virtio_pci_cap cap;
    uint32 notify_off_multiplier;   /* Multiplier for queue_notify_off. */
};

struct virtio_pci_cfg_cap {
    struct virtio_pci_cap cap;
    uint8 pci_cfg_data[4]; /* Data for BAR access. */
};

typedef struct virtio_pci_common_cfg {
    /* About the whole device. */
    uint32 device_feature_select;   /* read-write */
    uint32 device_feature;          /* read-only */
    uint32 guest_feature_select;    /* read-write */
    uint32 guest_feature;           /* read-write */
    uint16 msix_config;             /* read-write */
    uint16 num_queues;              /* read-only */
    uint8 device_status;            /* read-write */
    uint8 config_generation;        /* read-only */

    /* About a specific virtqueue. */
    uint16 queue_select;            /* read-write */
    uint16 queue_size;              /* read-write, power of 2. */
    uint16 queue_msix_vector;       /* read-write */
    uint16 queue_enable;            /* read-write */
    uint16 queue_notify_off;        /* read-only */
    uint32 queue_desc_lo;           /* read-write */
    uint32 queue_desc_hi;           /* read-write */
    uint32 queue_avail_lo;          /* read-write */
    uint32 queue_avail_hi;          /* read-write */
    uint32 queue_used_lo;           /* read-write */
    uint32 queue_used_hi;           /* read-write */
} virtio_pci_common_cfg;

typedef struct virtio_pci_isr {
    uint8 isr;
} virtio_pci_isr;

/* --- driver structs ----------------------------------------------- */

#define VP_ACCESS_IO       1
#define VP_ACCESS_MMIO     2
#define VP_ACCESS_PCICFG   3

struct vp_cap {
    union {
        void *memaddr;
        uint32 ioaddr;
        uint32 baroff;
    };
    const PCI::Device* device;
    uint8 cap;
    uint8 cfg;
    uint8 bar;
    uint8 mode;

    // Function
    uint64 Read(uint32 offset, uint8 size);
    void Write(uint32 offset, uint8 size, uint64 var);
};

struct vp_device {
    struct vp_cap common, notify, isr, device, legacy;
    uint32 notify_off_multiplier;
    uint8 use_modern;
    uint8 use_mmio;
};

#define vp_read(_cap, _struct, _field)        \
    (_cap)->Read( offsetof(_struct, _field), \
             sizeof(((_struct *)0)->_field))

#define vp_write(_cap, _struct, _field, _var)           \
    (_cap)->Write( offsetof(_struct, _field),          \
             sizeof(((_struct *)0)->_field), _var)
