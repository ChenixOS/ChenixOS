#include "VirtioRing.h"

#include "memory/MemoryManager.h"

vring::vring(unsigned int num, unsigned char *queue)  {
    this->num = num;
    this->desc = (vring_desc*) queue;
    this->avail = (struct vring_avail *)&this->desc[num];

    /* disable interrupts */
    this->avail->flags |= VRING_AVAIL_F_NO_INTERRUPT;

    /* physical address of used must be page aligned */
    this->used = (vring_used *)(void*)ALIGN((uint64)&this->avail->ring[num], 4096);

    int i;
    for (i = 0; i < num - 1; i++)
        this->desc[i].next = i + 1;
    this->desc[i].next = 0;
}

int vring_virtqueue::MoreUsed() {
    struct vring_used *used = this->vring.used;
    barrier();

    int more = this->last_used_idx != used->index;
    barrier();

    return more;
}

void vring_virtqueue::Detach(uint32 head) {
    struct vring *vr = &this->vring;
    struct vring_desc *desc = vr->desc;
    unsigned int i;

    /* find end of given descriptor */

    i = head;
    while (desc[i].flags & VRING_DESC_F_NEXT)
        i = desc[i].next;

    /* link it with free list and point to it */

    desc[i].next = this->free_head;
    this->free_head = head;
}

int vring_virtqueue::GetBuffer(uint32* len) {
    struct vring *vr = &this->vring;
    struct vring_used_item *elem;
    struct vring_used *used = this->vring.used;
    uint32 id;
    int ret;

    elem = &used->ring[this->last_used_idx % vr->num];
    id = elem->id;
    if (len != nullptr)
        *len = elem->length;

    ret = this->vdata[id];

    this->Detach(id);

    this->last_used_idx = this->last_used_idx + 1;

    return ret;
}

void vring_virtqueue::AddBuffer(struct vring_list list[],
                   unsigned int out,
                   unsigned int in,
                   int index,
                   int num_added) {
    struct vring* vr = &this->vring;
    int i, av, head, prev;
    struct vring_desc *desc = vr->desc;
    struct vring_avail *avail = vr->avail;

    prev = 0;
    head = this->free_head;
    for (i = head; out; i = desc[i].next, out--) {
        desc[i].flags = VRING_DESC_F_NEXT;
        desc[i].addr = (uint64)MemoryManager::KernelToPhysPtr(list->addr);
        desc[i].len = list->length;
        prev = i;
        list++;
    }
    for ( ; in; i = desc[i].next, in--) {
        desc[i].flags = VRING_DESC_F_NEXT|VRING_DESC_F_WRITE;
        desc[i].addr = (uint64)MemoryManager::KernelToPhysPtr(list->addr);
        desc[i].len = list->length;
        prev = i;
        list++;
    }
    desc[prev].flags = desc[prev].flags & ~VRING_DESC_F_NEXT;

    this->free_head = i;

    this->vdata[head] = index;

    av = (avail->index + num_added) % vr->num;
    avail->ring[av] = head;
}

void vring_virtqueue::Kick(struct vp_device *vp, int num_added) {
    struct vring *vr = &this->vring;
    struct vring_avail *avail = vr->avail;

    /* Make sure idx update is done after ring write. */
    barrier();
    avail->index = avail->index + num_added;
}