/**
 * @file AHCI.cpp
 * @author liankong xhsw.new@qq.com
 * @brief AHCI Controller's simple driver
 * @version 0.1
 * @date 2024-08-13
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "init/Init.h"
#include "drivers/PCI/PCI.h"
#include "AHCI.h"
#include "devices/DevFS.h"

#include "klib/stdio.h"
#include "klib/memory.h"

#include "memory/KernelHeap.h"

#include "time/Time.h"
#include "task/Scheduler.h"
#include "drivers/ATA/ATA.h"

#include "impl/lowbits.h"
#include "impl/pcireg.h"

static const PCI::DriverInfo devInfo = {
    .vendorID = PCIE_DEFAULT_VENDORID,
    .deviceID = PCIE_DEFAULT_DEVICEID,
    .classCode = 0x1,
    .subclassCode = 0x6,
    .progIf = 0x1 // AHCI rev 1
};

static AhciDeviceDriver* g_AhciDevice = nullptr;
static void* bounce_buf_fl;

void AHCI_Handler(const PCI::Device& device) {
	if(g_AhciDevice == nullptr) {
		g_AhciDevice = new AhciDeviceDriver();
	}
	g_AhciDevice->ScanDevice(device);

	bounce_buf_fl = KernelHeap::AllocPages(1);
}

void AHCI_Registers() {
    PCI::RegisterDriver(devInfo,AHCI_Handler);
}
REGISTER_INIT_FUNC(AHCI_Registers,INIT_STAGE_BUSDRIVERS);

// AHCI协议

#define AHCI_REQUEST_TIMEOUT 6000 // 6 seconds max for IDE ops
#define AHCI_RESET_TIMEOUT     500 // 500 miliseconds
#define AHCI_LINK_TIMEOUT       100 // 10 miliseconds

static inline void writel(void *addr, uint32 val) {
    barrier();
    *(volatile uint32 *)addr = val;
}
static inline void writew(void *addr, uint16 val) {
    barrier();
    *(volatile uint16 *)addr = val;
}
static inline void writeb(void *addr, uint8 val) {
    barrier();
    *(volatile uint8 *)addr = val;
}
static inline uint64 readq(const void *addr) {
    uint64 val = *(volatile const uint64 *)addr;
    barrier();
    return val;
}
static inline uint32 readl(const void *addr) {
    uint32 val = *(volatile const uint32 *)addr;
    barrier();
    return val;
}
static inline uint16 readw(const void *addr) {
    uint16 val = *(volatile const uint16 *)addr;
    barrier();
    return val;
}
static inline uint8 readb(const void *addr) {
    uint8 val = *(volatile const uint8 *)addr;
    barrier();
    return val;
}

// 这是是跟PCI有关的
static uint32 ahci_ctrl_readl(struct ahci_ctrl_s *ctrl, uint32 reg) {
	// return PCI::ReadConfigDWord(*ctrl->pci_tmp, reg);
    barrier();
	return readl(ctrl->iobase + reg);
}

static void ahci_ctrl_writel(struct ahci_ctrl_s *ctrl, uint32 reg, uint32 val) {
	// PCI::WriteConfigDWord(*ctrl->pci_tmp, reg, val);
	writel(ctrl->iobase + reg, val);
}

static uint32 ahci_port_to_ctrl(uint32 pnr, uint32 port_reg)
{
    uint32 ctrl_reg = 0x100;
    ctrl_reg += pnr * 0x80;
    ctrl_reg += port_reg;
    return ctrl_reg;
}

static uint32 ahci_port_readl(struct ahci_ctrl_s *ctrl, uint32 pnr, uint32 reg)
{
    uint32 ctrl_reg = ahci_port_to_ctrl(pnr, reg);
	// klog_info("AHCI Debug","ctrl=%X pnr=%d reg=%d",ctrl,pnr,ctrl_reg);
    return ahci_ctrl_readl(ctrl, ctrl_reg);
}

static void ahci_port_writel(struct ahci_ctrl_s *ctrl, uint32 pnr, uint32 reg, uint32 val)
{
    uint32 ctrl_reg = ahci_port_to_ctrl(pnr, reg);
    ahci_ctrl_writel(ctrl, ctrl_reg, val);
}

// prepare sata command fis
static void sata_prep_simple(struct sata_cmd_fis *fis, uint8 command)
{
    kmemset(fis, 0, sizeof(struct sata_cmd_fis));
    fis->command = command;
}

static void sata_prep_readwrite(struct sata_cmd_fis *fis,
                                struct disk_op_s *op, int iswrite)
{
    uint64 lba = op->lba;
    uint8 command;

    kmemset(fis, 0, sizeof(struct sata_cmd_fis));

    if (op->count >= (1<<8) || lba + op->count >= (1<<28)) {
        fis->sector_count2 = op->count >> 8;
        fis->lba_low2      = lba >> 24;
        fis->lba_mid2      = lba >> 32;
        fis->lba_high2     = lba >> 40;
        lba &= 0xffffff;
        command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                   : ATA_CMD_READ_DMA_EXT);
    } else {
        command = (iswrite ? ATA_CMD_WRITE_DMA
                   : ATA_CMD_READ_DMA);
    }
    fis->feature      = 1; /* dma */
    fis->command      = command;
    fis->sector_count = op->count;
    fis->lba_low      = lba;
    fis->lba_mid      = lba >> 8;
    fis->lba_high     = lba >> 16;
    fis->device       = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;
}

static void sata_prep_atapi(struct sata_cmd_fis *fis, uint16 blocksize)
{
    kmemset(fis, 0, sizeof(struct sata_cmd_fis));
    fis->command  = ATA_CMD_PACKET;
    fis->feature  = 1; /* dma */
    fis->lba_mid  = blocksize;
    fis->lba_high = blocksize >> 8;
}

static void
ahci_port_clear(struct ahci_ctrl_s *ctrl, uint32 pnr) {
    // non-queued error recovery (AHCI 1.3 section 6.2.2.1)
    // Clears PxCMD.ST to 0 to reset the PxCI register
    uint32 val = ahci_port_readl(ctrl, pnr, PORT_CMD);
    ahci_port_writel(ctrl, pnr, PORT_CMD, val & ~PORT_CMD_START);

    // waits for PxCMD.CR to clear to 0
    while (1) {
        val = ahci_port_readl(ctrl, pnr, PORT_CMD);
        if ((val & PORT_CMD_LIST_ON) == 0)
            break;
        Scheduler::Yield();
    }

    // Clears any error bits in PxSERR to enable capturing new errors
    val = ahci_port_readl(ctrl, pnr, PORT_SCR_ERR);
    ahci_port_writel(ctrl, pnr, PORT_SCR_ERR, val);

    // Clears status bits in PxIS as appropriate
    val = ahci_port_readl(ctrl, pnr, PORT_IRQ_STAT);
    ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, val);

    // If PxTFD.STS.BSY or PxTFD.STS.DRQ is set to 1, issue
    // a COMRESET to the device to put it in an idle state
    val = ahci_port_readl(ctrl, pnr, PORT_TFDATA);
    if (val & (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ)) {
        klog_warning("AHCI", "AHCI/%d: issue comreset", pnr);
        val = ahci_port_readl(ctrl, pnr, PORT_SCR_CTL);
        // set Device Detection Initialization (DET) to 1 for 1 ms for comreset
        ahci_port_writel(ctrl, pnr, PORT_SCR_CTL, val | 1);
        // mdelay (1);
        // Scheduler::Yield();
        uint64 start = Time::GetTSC();
        for(;(Time::GetTSC() - start) < (Time::GetTSCTicksPerMilli() * 5);) {
            barrier();
        }
        ahci_port_writel(ctrl, pnr, PORT_SCR_CTL, val);
    }

    // Sets PxCMD.ST to 1 to enable issuing new commands
    val = ahci_port_readl(ctrl, pnr, PORT_CMD);
    ahci_port_writel(ctrl, pnr, PORT_CMD, val | PORT_CMD_START);
}


// submit ahci command + wait for result
static int ahci_command(struct ahci_port_s *port_gf, int iswrite, int isatapi,
                        void *buffer, uint32 bsize)
{
    uint32 val, status, success, flags, intbits, error;
    struct ahci_ctrl_s *ctrl = port_gf->ctrl;
    struct ahci_cmd_s  *cmd  = port_gf->cmd;
    struct ahci_fis_s  *fis  = port_gf->fis;
    struct ahci_list_s *list = port_gf->list;
    uint32 pnr                  = port_gf->pnr;

    cmd->fis.reg       = 0x27;
    cmd->fis.pmp_type  = 1 << 7; /* cmd fis */
    // cmd->prdt[0].base  = (uint32)MemoryManager::KernelTo32PhysPtr(buffer);
    // cmd->prdt[0].baseu = 0;
    MemoryManager::SplitKernelToPhysPtr(buffer,&cmd->prdt[0].baseu,&cmd->prdt[0].base);
    cmd->prdt[0].flags = bsize-1;

    flags = ((1 << 16) | /* one prd entry */
             (iswrite ? (1 << 6) : 0) |
             (isatapi ? (1 << 5) : 0) |
             (5 << 0)); /* fis length (dwords) */
    list[0].flags  = flags;
    list[0].bytes  = 0;
    // list[0].base   = (uint32)MemoryManager::KernelTo32PhysPtr(cmd);
    // list[0].baseu  = 0;
    MemoryManager::SplitKernelToPhysPtr(cmd,&list[0].baseu,&list[0].base);

    intbits = ahci_port_readl(ctrl, pnr, PORT_IRQ_STAT);
    if (intbits)
        ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, intbits);
    ahci_port_writel(ctrl, pnr, PORT_CMD_ISSUE, 1);


	uint64 start = Time::GetTSC();
    do {
        for (;;) {
            intbits = ahci_port_readl(ctrl, pnr, PORT_IRQ_STAT);
            if (intbits) {
                ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, intbits);
                if (intbits & 0x40000000) {
                    uint32 tf = ahci_port_readl(ctrl, pnr, PORT_TFDATA);
                    status = tf & 0xff;
                    error = (tf & 0xff00) >> 8;
                    break;
                }
                if (intbits & 0x02) {
                    status = GET_LOWFLAT(fis->psfis[2]);
                    error  = GET_LOWFLAT(fis->psfis[3]);
                    break;
                }
                if (intbits & 0x01) {
                    status = GET_LOWFLAT(fis->rfis[2]);
                    error  = GET_LOWFLAT(fis->rfis[3]);
                    break;
                }
            }
            if(((Time::GetTSC() - start) / Time::GetTSCTicksPerMilli()) >= AHCI_REQUEST_TIMEOUT) {
				return -DISK_RET_ETIMEOUT;
			}
            Scheduler::Yield();
        }
    } while (status & ATA_CB_STAT_BSY);

	success = (0x00 == (status & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF |
                                  ATA_CB_STAT_ERR)) &&
               ATA_CB_STAT_RDY == (status & (ATA_CB_STAT_RDY)));
    
	if (!success) { // 失败则清理port的flags并重置
        ahci_port_clear(ctrl, pnr);
    }
    return success ? 0 : -DISK_RET_ECONTROLLER;
}


// 重置port，并验证是否可用
// 超时默认不可用
static void
ahci_port_reset(struct ahci_ctrl_s *ctrl, uint32 pnr)
{
    uint32 val;
    uint64 start;

    /* disable FIS + CMD */
	start = Time::GetTSC();
    for (;;) {
        val = ahci_port_readl(ctrl, pnr, PORT_CMD);
		
        if (!(val & (PORT_CMD_FIS_RX | PORT_CMD_START |
                     PORT_CMD_FIS_ON | PORT_CMD_LIST_ON)))
            break;
        val &= ~(PORT_CMD_FIS_RX | PORT_CMD_START);
        ahci_port_writel(ctrl, pnr, PORT_CMD, val);

		if(((Time::GetTSC() - start) / Time::GetTSCTicksPerMilli()) >= AHCI_RESET_TIMEOUT) {
			break;
		}

		Scheduler::Yield();
    }

    /* disable + clear IRQs */
    ahci_port_writel(ctrl, pnr, PORT_IRQ_MASK, 0);
    val = ahci_port_readl(ctrl, pnr, PORT_IRQ_STAT);
    if (val)
        ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, val);
}

// 这些是port相关

static struct ahci_port_s*
ahci_port_alloc(struct ahci_ctrl_s *ctrl, uint32 pnr)
{
    struct ahci_port_s *port = new ahci_port_s();
	kmemset(port, 0, sizeof(struct ahci_port_s));
	
	port->pnr = pnr;
	port->ctrl = ctrl;

	port->list = (ahci_list_s*)KernelHeap::AllocPages(1);
	port->fis = (ahci_fis_s*) KernelHeap::AllocPages(1);
	port->cmd = (ahci_cmd_s*) KernelHeap::AllocPages(1);

	kmemset(port->list, 0, 1024);
    kmemset(port->fis, 0, 256);
    kmemset(port->cmd, 0, 256);

	// ahci_port_writel(ctrl, pnr, PORT_LST_ADDR, MemoryManager::KernelTo32PhysPtr(port->list));
    // ahci_port_writel(ctrl, pnr, PORT_FIS_ADDR, MemoryManager::KernelTo32PhysPtr(port->fis));
    // if (ctrl->caps & HOST_CAP_64) {
    //    ahci_port_writel(ctrl, pnr, PORT_LST_ADDR_HI, 0);
    //    ahci_port_writel(ctrl, pnr, PORT_FIS_ADDR_HI, 0);
    // }
    uint32 addrHi,addrLow;
    MemoryManager::SplitKernelToPhysPtr(port->list,&addrHi,&addrLow);
    ahci_port_writel(ctrl, pnr, PORT_LST_ADDR, addrLow);
    ahci_port_writel(ctrl, pnr, PORT_LST_ADDR_HI, addrHi);
    MemoryManager::SplitKernelToPhysPtr(port->fis,&addrHi,&addrLow);
    ahci_port_writel(ctrl, pnr, PORT_FIS_ADDR, addrLow);
    ahci_port_writel(ctrl, pnr, PORT_FIS_ADDR_HI, addrHi);

    return port;
}

static void ahci_port_release(struct ahci_port_s *port)
{
	ahci_port_reset(port->ctrl, port->pnr);

	KernelHeap::FreePages(port->list);
	KernelHeap::FreePages(port->fis);
	KernelHeap::FreePages(port->cmd);

	delete port;
}


static struct ahci_port_s* ahci_port_realloc(struct ahci_port_s *port)
{
    struct ahci_port_s *tmp = new ahci_port_s();
    uint32 cmd;

    *tmp = *port;
    // free(port);
	delete port;
    port = tmp;

    ahci_port_reset(port->ctrl, port->pnr);

    KernelHeap::FreePages(port->list);
    KernelHeap::FreePages(port->fis);
    KernelHeap::FreePages(port->cmd);
    port->list = (ahci_list_s*)KernelHeap::AllocPages(1);
	port->fis = (ahci_fis_s*) KernelHeap::AllocPages(1);
	port->cmd = (ahci_cmd_s*) KernelHeap::AllocPages(1);

    // ahci_port_writel(port->ctrl, port->pnr, PORT_LST_ADDR, MemoryManager::KernelTo32PhysPtr(port->list));
    // ahci_port_writel(port->ctrl, port->pnr, PORT_FIS_ADDR, MemoryManager::KernelTo32PhysPtr(port->fis));
    uint32 addrHi,addrLow;
    struct ahci_ctrl_s *ctrl = port->ctrl;
    uint32 pnr = port->pnr;
    MemoryManager::SplitKernelToPhysPtr(port->list,&addrHi,&addrLow);
    ahci_port_writel(ctrl, pnr, PORT_LST_ADDR, addrLow);
    ahci_port_writel(ctrl, pnr, PORT_LST_ADDR_HI, addrHi);
    MemoryManager::SplitKernelToPhysPtr(port->fis,&addrHi,&addrLow);
    ahci_port_writel(ctrl, pnr, PORT_FIS_ADDR, addrLow);
    ahci_port_writel(ctrl, pnr, PORT_FIS_ADDR_HI, addrHi);

    cmd = ahci_port_readl(port->ctrl, port->pnr, PORT_CMD);
    cmd |= (PORT_CMD_FIS_RX|PORT_CMD_START);
    ahci_port_writel(port->ctrl, port->pnr, PORT_CMD, cmd);

    return port;
}

#define MAXMODEL 40

/* See ahci spec chapter 10.1 "Software Initialization of HBA" */
static int ahci_port_setup(struct ahci_port_s *port)
{
	struct ahci_ctrl_s *ctrl = port->ctrl;
    uint32 pnr = port->pnr;
    char model[MAXMODEL+1];
    uint16 buffer[256];
    uint32 cmd, stat, err, tf;
    int rc;
    // uint16* buffer = (uint16*)KernelHeap::AllocPages(1);

	/* enable FIS recv */
    cmd = ahci_port_readl(ctrl, pnr, PORT_CMD);
    cmd |= PORT_CMD_FIS_RX;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);

	/* spin up */
    cmd &= ~PORT_CMD_ICC_MASK;
    cmd |= PORT_CMD_SPIN_UP | PORT_CMD_POWER_ON | PORT_CMD_ICC_ACTIVE;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);
    
    /* wait for device link */
	uint64 start = Time::GetTSC();
    for (;;) {
        stat = ahci_port_readl(ctrl, pnr, PORT_SCR_STAT);
        if ((stat & 0x07) == 0x03) {
            klog_debug("AHCI", "AHCI/%d: link up", port->pnr);
            break;
        }
		if(((Time::GetTSC() - start) / Time::GetTSCTicksPerMilli()) >= AHCI_LINK_TIMEOUT) {
			klog_debug("AHCI", "AHCI/%d: link down 1", port->pnr);
			return -DISK_RET_ENOTREADY;
		}
		Scheduler::Yield();
    }

    /* clear device status */
    ahci_port_clear(port->ctrl, port->pnr);

	/* clear error status */
    err = ahci_port_readl(ctrl, pnr, PORT_SCR_ERR);
    if (err)
        ahci_port_writel(ctrl, pnr, PORT_SCR_ERR, err);

	/* wait for device becoming ready */
    start = Time::GetTSC();
    for (;;) {
        tf = ahci_port_readl(ctrl, pnr, PORT_TFDATA);
        if (!(tf & (ATA_CB_STAT_BSY |
                    ATA_CB_STAT_DRQ)))
            break;
        if(((Time::GetTSC() - start) / Time::GetTSCTicksPerMilli()) >= AHCI_REQUEST_TIMEOUT) {
			klog_info("AHCI","AHCI/%d: device not ready (tf 0x%x)", port->pnr, tf);
			return -DISK_RET_ETIMEOUT;
		}
        Scheduler::Yield();
    }

	/* start device */
    cmd |= PORT_CMD_START;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);

    // TODO: VMWare执行时出错
    /* send command to device */
    uint32 type = ahci_port_readl(ctrl, pnr, PORT_SIG);
    for(int i = 0; i < 3; i++) {
        if(type == SATA_SIG_ATA) {
            port->atapi = 0;
            sata_prep_simple(&port->cmd->fis, ATA_CMD_IDENTIFY_DEVICE);
            rc = ahci_command(port, 0, 0, buffer, 256);
            if(rc == 0)
                break;
        } else {
            sata_prep_simple(&port->cmd->fis, ATA_CMD_IDENTIFY_PACKET_DEVICE);
            rc = ahci_command(port, 0, 0, buffer, 256);
            if (rc == 0) {
                port->atapi = 1;
                break;
            } else {
                port->atapi = 0;
                sata_prep_simple(&port->cmd->fis, ATA_CMD_IDENTIFY_DEVICE);
                rc = ahci_command(port, 0, 0, buffer, 256);
                if(rc == 0)
                    break;
            }
        }
    }
    if(rc < 0)
        return rc;

    port->drive.cntl_id = pnr;
    port->drive.removable = (buffer[0] & 0x80) ? 1 : 0;

	if (!port->atapi) {
        // found disk (ata)
        port->drive.type = DTYPE_AHCI;
        port->drive.blksize = DISK_SECTOR_SIZE;
        port->drive.pchs.cylinder = buffer[1];
        port->drive.pchs.head = buffer[3];
        port->drive.pchs.sector = buffer[6];

        uint64 sectors;
        if (buffer[83] & (1 << 10)) // word 83 - lba48 support
            sectors = *(uint64*)&buffer[100]; // word 100-103
        else
            sectors = *(uint32*)&buffer[60]; // word 60 and word 61
        port->drive.sectors = sectors;
        uint64 adjsize = sectors >> 11;
        char adjprefix = 'M';
        if (adjsize >= (1 << 16)) {
            adjsize >>= 10;
            adjprefix = 'G';
        }

        int8 multi_dma = -1;
        int8 pio_mode = -1;
        int8 udma_mode = -1;
        // If bit 2 in word 53 is set, udma information is valid in word 88.
        if (buffer[53] & 0x04) {
            udma_mode = 6;
            while ((udma_mode >= 0) &&
                   !((buffer[88] & 0x7f) & ( 1 << udma_mode ))) {
                udma_mode--;
            }
        }
        // If bit 1 in word 53 is set, multiword-dma and advanced pio modes
        // are available in words 63 and 64.
        if (buffer[53] & 0x02) {
            pio_mode = 4;
            multi_dma = 3;
            while ((multi_dma >= 0) &&
                   !((buffer[63] & 0x7) & ( 1 << multi_dma ))) {
                multi_dma--;
            }
            while ((pio_mode >= 3) &&
                   !((buffer[64] & 0x3) & ( 1 << ( pio_mode - 3 ) ))) {
                pio_mode--;
            }
        }
        // dprintf(2, "AHCI/%d: supported modes: udma %d, multi-dma %d, pio %d",
        //         port->pnr, udma_mode, multi_dma, pio_mode);

        sata_prep_simple(&port->cmd->fis, ATA_CMD_SET_FEATURES);
        port->cmd->fis.feature = ATA_SET_FEATRUE_TRANSFER_MODE;
        // Select used mode. UDMA first, then Multi-DMA followed by
        // advanced PIO modes 3 or 4. If non, set default PIO.
        if (udma_mode >= 0) {
            /*dprintf(1, "AHCI/%d: Set transfer mode to UDMA-%d",
                    port->pnr, udma_mode);*/
            port->cmd->fis.sector_count = ATA_TRANSFER_MODE_ULTRA_DMA
                                          | udma_mode;
        } else if (multi_dma >= 0) {
            /*dprintf(1, "AHCI/%d: Set transfer mode to Multi-DMA-%d",
                    port->pnr, multi_dma);*/
            port->cmd->fis.sector_count = ATA_TRANSFER_MODE_MULTIWORD_DMA
                                          | multi_dma;
        } else if (pio_mode >= 3) {
            /*dprintf(1, "AHCI/%d: Set transfer mode to PIO-%d",
                    port->pnr, pio_mode);*/
            port->cmd->fis.sector_count = ATA_TRANSFER_MODE_PIO_FLOW_CTRL
                                          | pio_mode;
        } else {
            /*dprintf(1, "AHCI/%d: Set transfer mode to default PIO",
                    port->pnr);*/
            port->cmd->fis.sector_count = ATA_TRANSFER_MODE_DEFAULT_PIO;
        }
        rc = ahci_command(port, 1, 0, 0, 0);
        if (rc < 0) {
            // dprintf(1, "AHCI/%d: Set transfer mode failed.", port->pnr);
        }
    } else {
        // found cdrom (atapi)
        port->drive.type = DTYPE_AHCI_ATAPI;
        port->drive.blksize = CDROM_SECTOR_SIZE;
        port->drive.sectors = (uint64)-1;
        uint8 iscd = ((buffer[0] >> 8) & 0x1f) == 0x05;
        if (!iscd) {
            // dprintf(1, "AHCI/%d: atapi device isn't a cdrom", port->pnr);
            return -DISK_RET_EMEDIA;
        }
        /*port->desc = znprintf(MAXDESCSIZE
                              , "DVD/CD [AHCI/%d: %s ATAPI-%d DVD/CD]"
                              , port->pnr
                              , ata_extract_model(model, MAXMODEL, buffer)
                              , ata_extract_version(buffer));
        port->prio = bootprio_find_ata_device(ctrl->pci_tmp, pnr, 0);*/
    }
	return 0;
}

static struct ahci_ctrl_s* g_ahciBaseCtrl = nullptr;

void ahci_isr_handler(IDT::Registers *regs) {
    uint32 val = ahci_ctrl_readl(g_ahciBaseCtrl,HOST_IRQ_STAT);
    for(uint32 i = 0; i < 32; i++) {
        if(val & (1 << i)) {
            klog_error_isr("AHCI","interrupt on port %d occured!",i);
            ahci_port_writel(g_ahciBaseCtrl, i, PORT_IRQ_STAT, ~0);
            ahci_ctrl_writel(g_ahciBaseCtrl, HOST_IRQ_STAT, (1 << i));
        }
    }
}

/**
 * @brief 读写代码
 * 
 * @param drive 设备结构
 * @param op 操作缓冲区
 * @param iswrite 读/写
 * @return int 是否成功(0 成功 DISK_REY_EBADTRACK 失败)
 */
// read/write count blocks from a harddrive, op->buf_fl must be word aligned
static int
ahci_disk_readwrite_aligned(struct drive_s* drive,struct disk_op_s *op, int iswrite)
{
    struct ahci_port_s *port_gf = (struct ahci_port_s*) drive;
    struct ahci_cmd_s *cmd = port_gf->cmd;
    int rc;

    sata_prep_readwrite(&cmd->fis, op, iswrite);
    rc = ahci_command(port_gf, iswrite, 0, op->buf_fl,
                      op->count * DISK_SECTOR_SIZE);
    if (rc < 0)
        return DISK_RET_EBADTRACK;
    return DISK_RET_SUCCESS;
}


// ===========================================================================
// 驱动代码

static QueueLock g_AhciQueueLock;

AhciDeviceDriver::AhciDeviceDriver()
: DiskDeviceDriver("ahci") {
    // Void
}

void AhciDeviceDriver::DriveReadData(struct drive_s* drive,uint64 startBlock,uint64 numBlocks,void* buffer, Atomic<uint64>* finishFlag, Atomic<uint64>* successFlag) {
    bool success = false;
    struct disk_op_s op;
    uint64 size = numBlocks * drive->blksize;
    void* align_buffer = KernelHeap::AllocAlignMemory(size);
    int i;

    g_AhciQueueLock.Lock();

    make_disk_op(drive, &op, startBlock, numBlocks, align_buffer);
    i = ahci_disk_readwrite_aligned(drive, &op, 0);

    g_AhciQueueLock.Unlock();

    KernelHeap::FreeAlignMemory(align_buffer, size);

    success = (i == DISK_RET_SUCCESS);
    if(success)
        kmemcpy(buffer, align_buffer, size);
    finishFlag->Write(1);
    successFlag->Write(success);
}

void AhciDeviceDriver::DriveWriteData(struct drive_s* drive,uint64 startBlock,uint64 numBlocks,void* buffer, Atomic<uint64>* finishFlag, Atomic<uint64>* successFlag) {
    bool success = false;
    struct disk_op_s op;
    uint64 size = numBlocks * drive->blksize;
    void* align_buffer = KernelHeap::AllocAlignMemory(size);
    int i;

    kmemcpy(align_buffer, buffer, size);

    g_AhciQueueLock.Lock();

    make_disk_op(drive, &op, startBlock, numBlocks, align_buffer);
    i = ahci_disk_readwrite_aligned(drive, &op, 1);

    g_AhciQueueLock.Unlock();

    KernelHeap::FreeAlignMemory(align_buffer, size);

    success = (i == DISK_RET_SUCCESS);
    finishFlag->Write(1);
    successFlag->Write(success);
}

int64 AhciDeviceDriver::DriveIoctl(struct drive_s* drive,int64 cmd,void* buffer) {
    return -1;
}

// 扫描AHCI协议
void AhciDeviceDriver::ScanDevice(const PCI::Device& device) {
	struct ahci_port_s *port;
    uint32 val, pnr, max;
	
	// Io Base
	void* iobase = PCI::GetBARAddress(device, 5);
	PCI::EnableMemoryMaster(device);

	struct ahci_ctrl_s *ctrl = new ahci_ctrl_s();
	ctrl->pci_tmp = &device;
	ctrl->iobase = iobase;
	ctrl->irq = PCI::ReadConfigByte(device,PCI_INTERRUPT_LINE);
	PCI::EnableBusMaster(device);

    g_ahciBaseCtrl = ctrl;

    uint32 ext_cap = ahci_ctrl_readl(ctrl, HOST_EXT_CAP);
    if(ext_cap & 1) {
        /* request BIOS/OS ownership handoff */
        val = ahci_ctrl_readl(ctrl,HOST_BOHC);
        val |= (1 << 1);
        ahci_ctrl_writel(ctrl, HOST_BOHC, val);

        while(1) {
            val = ahci_ctrl_readl(ctrl,HOST_BOHC);
            if(!((val & 1) || !(val & (1<<1)))) {
                break;
            }
        }
    }

    // 重置设备
	val = ahci_ctrl_readl(ctrl, HOST_CTL);
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_AHCI_EN | HOST_CTL_RESET);

    // 等待重置完成
    while(ahci_ctrl_readl(ctrl, HOST_CTL) & HOST_CTL_RESET) {}

    val = ahci_ctrl_readl(ctrl, HOST_CTL);
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_AHCI_EN | HOST_CTL_IRQ_EN);

    // 设置IRQ处理函数
    PCI::SetInterruptHandler(device,&ahci_isr_handler);

    // 获取信息
    ctrl->caps = ahci_ctrl_readl(ctrl, HOST_CAP);
    ctrl->ports = ahci_ctrl_readl(ctrl, HOST_PORTS_IMPL);

	klog_debug("AHCI","ports=%X iobase=%X",(uint64)ctrl->ports,ctrl->iobase);

    // 遍历所有port
	max = 0x1f;
    for (pnr = 0; pnr <= max; pnr++) {
        if (!(ctrl->ports & (1 << pnr)))
            continue;

        port = ahci_port_alloc(ctrl, pnr);
        if (port == nullptr)
            continue;
		
    	int rc;

		ahci_port_reset(port->ctrl, port->pnr);
		rc = ahci_port_setup(port);
        
		klog_info("AHCI","Init port: ctrl=%X list=%X cmd=%X fis=%X pnr=%d, isr=%d, rc=%d",port->ctrl,MemoryManager::KernelToPhysPtr(port->list),MemoryManager::KernelToPhysPtr(port->cmd),MemoryManager::KernelToPhysPtr(port->fis),port->pnr,(uint32)ctrl->irq,(rc < 0) ? -rc : rc);
		
        if (rc < 0)
			ahci_port_release(port);
		else {
			port = ahci_port_realloc(port);
			if (port == nullptr)
				continue;
			
			klog_info("AHCI", "Register AHCI/%d", port->pnr);		
            this->RegisterDrive(&port->drive);
		}
    }
}
