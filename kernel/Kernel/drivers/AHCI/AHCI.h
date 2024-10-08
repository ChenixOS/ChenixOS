#pragma once

#include <stdint.h>
#include "memory/MemoryManager.h"

#include "drivers/DiskDeviceDriver/DiskDeviceDriver.h"
#include "drivers/PCI/PCI.h"

struct sata_cmd_fis {
    uint8 reg;
    uint8 pmp_type;
    uint8 command;
    uint8 feature;

    uint8 lba_low;
    uint8 lba_mid;
    uint8 lba_high;
    uint8 device;

    uint8 lba_low2;
    uint8 lba_mid2;
    uint8 lba_high2;
    uint8 feature2;

    uint8 sector_count;
    uint8 sector_count2;
    uint8 res_1;
    uint8 control;

    uint8 res_2[64 - 16];
};

struct ahci_ctrl_s {
    // struct pci_device *pci_tmp;
	const PCI::Device* pci_tmp;
    uint8  irq;
    void *iobase;
    uint32 caps;
    uint32 ports;
};

struct ahci_cmd_s {
    struct sata_cmd_fis fis;
    uint8 atapi[0x20];
    uint8 res[0x20];
    struct {
        uint32 base;
        uint32 baseu;
        uint32 res;
        uint32 flags;
    } prdt[];
};

/* command list */
struct ahci_list_s {
    uint32 flags;
    uint32 bytes;
    uint32 base;
    uint32 baseu;
    uint32 res[4];
};

struct ahci_fis_s {
    uint8 dsfis[0x1c];  /* dma setup */
    uint8 res_1[0x04];
    uint8 psfis[0x14];  /* pio setup */
    uint8 res_2[0x0c];
    uint8 rfis[0x14];   /* d2h register */
    uint8 res_3[0x04];
    uint8 sdbfis[0x08]; /* set device bits */
    uint8 ufis[0x40];   /* unknown */
    uint8 res_4[0x60];
};

struct ahci_port_s {
    struct drive_s     drive;
    struct ahci_ctrl_s *ctrl;
    struct ahci_list_s *list;
    struct ahci_fis_s  *fis;
    struct ahci_cmd_s  *cmd;
    uint32                pnr;
    uint32                atapi;
    char               *desc;
    int                prio;
};

#define AHCI_IRQ_ON_SG            (1 << 31)
#define AHCI_CMD_ATAPI            (1 << 5)
#define AHCI_CMD_WRITE            (1 << 6)
#define AHCI_CMD_PREFETCH         (1 << 7)
#define AHCI_CMD_RESET            (1 << 8)
#define AHCI_CMD_CLR_BUSY         (1 << 10)

#define RX_FIS_D2H_REG            0x40 /* offset of D2H Register FIS data */
#define RX_FIS_SDB                0x58 /* offset of SDB FIS data */
#define RX_FIS_UNK                0x60 /* offset of Unknown FIS data */

/* global controller registers */
#define HOST_CAP                  0x00 /* host capabilities */
#define HOST_CTL                  0x04 /* global host control */
#define HOST_IRQ_STAT             0x08 /* interrupt status */
#define HOST_PORTS_IMPL           0x0c /* bitmap of implemented ports */
#define HOST_VERSION              0x10 /* AHCI spec. version compliancy */
#define HOST_CCC_CONTROL 0x14
#define HOST_CCC_PORT 0x18
#define HOST_EM_LOCATION 0x1c
#define HOST_EM_CONTROL 0x20
#define HOST_EXT_CAP 0x24
#define HOST_BOHC 0x28

/* HOST_CTL bits */
#define HOST_CTL_RESET            (1 << 0)  /* reset controller; self-clear */
#define HOST_CTL_IRQ_EN           (1 << 1)  /* global IRQ enable */
#define HOST_CTL_AHCI_EN          (1 << 31) /* AHCI enabled */

/* HOST_CAP bits */
#define HOST_CAP_SSC              (1 << 14) /* Slumber capable */
#define HOST_CAP_AHCI             (1 << 18) /* AHCI only */
#define HOST_CAP_CLO              (1 << 24) /* Command List Override support */
#define HOST_CAP_SSS              (1 << 27) /* Staggered Spin-up */
#define HOST_CAP_NCQ              (1 << 30) /* Native Command Queueing */
#define HOST_CAP_64               (1 << 31) /* PCI DAC (64-bit DMA) support */

/* registers for each SATA port */
#define PORT_LST_ADDR             0x00 /* command list DMA addr */
#define PORT_LST_ADDR_HI          0x04 /* command list DMA addr hi */
#define PORT_FIS_ADDR             0x08 /* FIS rx buf addr */
#define PORT_FIS_ADDR_HI          0x0c /* FIS rx buf addr hi */
#define PORT_IRQ_STAT             0x10 /* interrupt status */
#define PORT_IRQ_MASK             0x14 /* interrupt enable/disable mask */
#define PORT_CMD                  0x18 /* port command */
#define PORT_TFDATA               0x20 /* taskfile data */
#define PORT_SIG                  0x24 /* device TF signature */
#define PORT_SCR_STAT             0x28 /* SATA phy register: SStatus */
#define PORT_SCR_CTL              0x2c /* SATA phy register: SControl */
#define PORT_SCR_ERR              0x30 /* SATA phy register: SError */
#define PORT_SCR_ACT              0x34 /* SATA phy register: SActive */
#define PORT_CMD_ISSUE            0x38 /* command issue */
#define PORT_RESERVED             0x3c /* reserved */

/* PORT_IRQ_{STAT,MASK} bits */
#define PORT_IRQ_COLD_PRES        (1 << 31) /* cold presence detect */
#define PORT_IRQ_TF_ERR           (1 << 30) /* task file error */
#define PORT_IRQ_HBUS_ERR         (1 << 29) /* host bus fatal error */
#define PORT_IRQ_HBUS_DATA_ERR    (1 << 28) /* host bus data error */
#define PORT_IRQ_IF_ERR           (1 << 27) /* interface fatal error */
#define PORT_IRQ_IF_NONFATAL      (1 << 26) /* interface non-fatal error */
#define PORT_IRQ_OVERFLOW         (1 << 24) /* xfer exhausted available S/G */
#define PORT_IRQ_BAD_PMP          (1 << 23) /* incorrect port multiplier */

#define PORT_IRQ_PHYRDY           (1 << 22) /* PhyRdy changed */
#define PORT_IRQ_DEV_ILCK         (1 << 7) /* device interlock */
#define PORT_IRQ_CONNECT          (1 << 6) /* port connect change status */
#define PORT_IRQ_SG_DONE          (1 << 5) /* descriptor processed */
#define PORT_IRQ_UNK_FIS          (1 << 4) /* unknown FIS rx'd */
#define PORT_IRQ_SDB_FIS          (1 << 3) /* Set Device Bits FIS rx'd */
#define PORT_IRQ_DMAS_FIS         (1 << 2) /* DMA Setup FIS rx'd */
#define PORT_IRQ_PIOS_FIS         (1 << 1) /* PIO Setup FIS rx'd */
#define PORT_IRQ_D2H_REG_FIS      (1 << 0) /* D2H Register FIS rx'd */

#define PORT_IRQ_FREEZE           (PORT_IRQ_HBUS_ERR | PORT_IRQ_IF_ERR |   \
                                   PORT_IRQ_CONNECT | PORT_IRQ_PHYRDY |    \
                                   PORT_IRQ_UNK_FIS)
#define PORT_IRQ_ERROR            (PORT_IRQ_FREEZE | PORT_IRQ_TF_ERR |     \
                                   PORT_IRQ_HBUS_DATA_ERR)
#define DEF_PORT_IRQ              (PORT_IRQ_ERROR | PORT_IRQ_SG_DONE |     \
                                   PORT_IRQ_SDB_FIS | PORT_IRQ_DMAS_FIS |  \
                                   PORT_IRQ_PIOS_FIS | PORT_IRQ_D2H_REG_FIS)

/* PORT_CMD bits */
#define PORT_CMD_ATAPI            (1 << 24) /* Device is ATAPI */
#define PORT_CMD_LIST_ON          (1 << 15) /* cmd list DMA engine running */
#define PORT_CMD_FIS_ON           (1 << 14) /* FIS DMA engine running */
#define PORT_CMD_FIS_RX           (1 << 4) /* Enable FIS receive DMA engine */
#define PORT_CMD_CLO              (1 << 3) /* Command list override */
#define PORT_CMD_POWER_ON         (1 << 2) /* Power up device */
#define PORT_CMD_SPIN_UP          (1 << 1) /* Spin up device */
#define PORT_CMD_START            (1 << 0) /* Enable port DMA engine */

#define PORT_CMD_ICC_MASK         (0xf << 28) /* i/f ICC state mask */
#define PORT_CMD_ICC_ACTIVE       (0x1 << 28) /* Put i/f in active state */
#define PORT_CMD_ICC_PARTIAL      (0x2 << 28) /* Put i/f in partial state */
#define PORT_CMD_ICC_SLUMBER      (0x6 << 28) /* Put i/f in slumber state */

#define PORT_IRQ_STAT_DHRS        (1 << 0) /* Device to Host Register FIS */
#define PORT_IRQ_STAT_PSS         (1 << 1) /* PIO Setup FIS */
#define PORT_IRQ_STAT_DSS         (1 << 2) /* DMA Setup FIS */
#define PORT_IRQ_STAT_SDBS        (1 << 3) /* Set Device Bits */
#define PORT_IRQ_STAT_UFS         (1 << 4) /* Unknown FIS */
#define PORT_IRQ_STAT_DPS         (1 << 5) /* Descriptor Processed */
#define PORT_IRQ_STAT_PCS         (1 << 6) /* Port Connect Change Status */
#define PORT_IRQ_STAT_DMPS        (1 << 7) /* Device Mechanical Presence
                                              Status */
#define PORT_IRQ_STAT_PRCS        (1 << 22) /* File Ready Status */
#define PORT_IRQ_STAT_IPMS        (1 << 23) /* Incorrect Port Multiplier
                                               Status */
#define PORT_IRQ_STAT_OFS         (1 << 24) /* Overflow Status */
#define PORT_IRQ_STAT_INFS        (1 << 26) /* Interface Non-Fatal Error
                                               Status */
#define PORT_IRQ_STAT_IFS         (1 << 27) /* Interface Fatal Error */
#define PORT_IRQ_STAT_HBDS        (1 << 28) /* Host Bus Data Error Status */
#define PORT_IRQ_STAT_HBFS        (1 << 29) /* Host Bus Fatal Error Status */
#define PORT_IRQ_STAT_TFES        (1 << 30) /* Task File Error Status */
#define PORT_IRQ_STAT_CPDS        (1 << 31) /* Code Port Detect Status */

#define SATA_SIG_ATA     0x00000101    // SATA drive
#define SATA_SIG_ATAPI   0xEB140101    // SATAPI drive
#define SATA_SIG_SEMB    0xC33C0101    // Enclosure management bridge
#define SATA_SIG_PM      0x96690101    // Port multiplier

class AhciDeviceDriver : public DiskDeviceDriver {
public:
	AhciDeviceDriver();
	
	void DriveReadData(struct drive_s* drive,uint64 startBlock,uint64 numBlocks,void* buffer, Atomic<uint64>* finishFlag, Atomic<uint64>* successFlag);
    void DriveWriteData(struct drive_s* drive,uint64 startBlock,uint64 numBlocks,void* buffer, Atomic<uint64>* finishFlag, Atomic<uint64>* successFlag);
    int64 DriveIoctl(struct drive_s* drive,int64 cmd,void* buffer);

	void ScanDevice(const PCI::Device& device);
};



