#include "Virtio.h"
#include "arch/Port.h"

uint64 vp_cap::Read(uint32 offset, uint8 size)
{
    uint64 var = 0;
    switch (this->mode)
    {
    case VP_ACCESS_IO:
        {
            uint16 addr = this->ioaddr + offset;
            switch(size) {
                case 8:
                    var = Port::InDWord(addr);
                    var |= (uint64)Port::InDWord(addr + 4) << 32;
                    break;
                case 4:
                    var = Port::InDWord(addr);
                    break;
                case 2:
                    var = Port::InWord(addr);
                    break;
                case 1:
                    var = Port::InByte(addr);
                    break;
            }
            break;
        }
    case VP_ACCESS_MMIO:
    {
        switch (size) {
        case 8:
            var = IoBase::ReadQWord(this->memaddr, offset);
            break;
        case 4:
            var = IoBase::ReadDWord(this->memaddr, offset);
            break;
        case 2:
            var = IoBase::ReadWord(this->memaddr, offset);
            break;
        case 1:
            var = IoBase::ReadByte(this->memaddr, offset);
            break;
        }
        break;
    }
    case VP_ACCESS_PCICFG:
    {
        uint32 addr = this->baroff + offset;

        PCI::WriteConfigByte(*this->device, this->cfg + 4,this->bar);
        PCI::WriteConfigWord(*this->device, this->cfg + 8, addr);
        PCI::WriteConfigWord(*this->device, this->cfg + 12, (size > 4) ? 4 : size);

        switch(size) {
            case 8:
                var = PCI::ReadConfigDWord(*this->device,this->cfg + sizeof(struct virtio_pci_cap));
                PCI::WriteConfigWord(*this->device, this->cfg + 8, addr + 4);
                var |= (uint64)PCI::ReadConfigDWord(*this->device,this->cfg + sizeof(struct virtio_pci_cap)) << 32;
                break;
            case 4:
                var = PCI::ReadConfigDWord(*this->device,this->cfg + sizeof(struct virtio_pci_cap));
                break;
            case 2:
                var = PCI::ReadConfigWord(*this->device,this->cfg + sizeof(struct virtio_pci_cap));
                break;
            case 1:
                var = PCI::ReadConfigByte(*this->device,this->cfg + sizeof(struct virtio_pci_cap));
                break;
        }
    }
    default:
        break;
    }

    return var;
}

void vp_cap::Write(uint32 offset, uint8 size, uint64 val)
{
    switch (this->mode)
    {
    case VP_ACCESS_IO:
        {
            uint16 addr = this->ioaddr + offset;
            switch(size) {
                case 4:
                    Port::OutDWord(addr, val);
                    break;
                case 2:
                    Port::OutWord(addr, val);
                    break;
                case 1:
                    Port::OutByte(addr, val);
                    break;
            }
            break;
        }
    case VP_ACCESS_MMIO:
    {
        switch (size) {
        case 8:
            IoBase::WriteQWord(this->memaddr, offset, val);
            break;
        case 4:
            IoBase::WriteDWord(this->memaddr, offset, val);
            break;
        case 2:
            IoBase::WriteWord(this->memaddr, offset, val);
            break;
        case 1:
            IoBase::WriteByte(this->memaddr, offset, val);
            break;
        }
        break;
    }
    case VP_ACCESS_PCICFG:
    {
        uint32 addr = this->baroff + offset;

        PCI::WriteConfigByte(*this->device, this->cfg + 4,this->bar);
        PCI::WriteConfigWord(*this->device, this->cfg + 8, addr);
        PCI::WriteConfigWord(*this->device, this->cfg + 12, (size > 4) ? 4 : size);

        switch(size) {
            case 4:
                PCI::WriteConfigDWord(*this->device,this->cfg + sizeof(struct virtio_pci_cap), val);
                break;
            case 2:
                PCI::WriteConfigWord(*this->device,this->cfg + sizeof(struct virtio_pci_cap), val);
                break;
            case 1:
                PCI::WriteConfigByte(*this->device,this->cfg + sizeof(struct virtio_pci_cap), val);
                break;
        }
    }
    default:
        break;
    }
}
