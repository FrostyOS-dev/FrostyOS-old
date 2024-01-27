#include "AHCIDisk.hpp"

#include <string.h>
#include <util.h>

#include <Memory/PageManager.hpp>

namespace AHCI {

AHCIDisk::AHCIDisk(AHCIPort* port) : m_port(port), m_sector_count(0), m_sector_size(0) {

}

AHCIDisk::~AHCIDisk() {

}

void AHCIDisk::Init() {
    // Issue ATA IDENTIFY command
    FIS_RegH2D h2d;
    memset(&h2d, 0, sizeof(FIS_RegH2D));
    h2d.FISType = FISTypeRegH2D;
    h2d.PMPort = 0;
    h2d.C = 1;
    h2d.Command = ATA_IDENTIFY_DEVICE;
    h2d.FeatureLow = 0;
    h2d.LBA0 = 0;
    h2d.LBA1 = 0;
    h2d.LBA2 = 0;
    h2d.Device = 0;
    h2d.LBA3 = 0;
    h2d.LBA4 = 0;
    h2d.LBA5 = 0;
    h2d.FeatureHigh = 0;
    h2d.CountLow = 0;
    h2d.CountHigh = 0;
    h2d.Control = 0;
    
    AHCICommandHeader* header = m_port->GetCommandSlot();
    //memset(header, 0, sizeof(AHCICommandHeader));
    header->DescriptorInformation.CFL = sizeof(FIS_RegH2D) / 4;
    header->DescriptorInformation.A = 0;
    header->DescriptorInformation.W = 0;
    header->DescriptorInformation.P = 0;
    header->DescriptorInformation.R = 0;
    header->DescriptorInformation.B = 0;
    header->DescriptorInformation.C = 1;
    header->DescriptorInformation.PMP = 0;
    header->DescriptorInformation.PRDTL = 1;
    header->CommandStatus = 512;

    AHCICommandTable* cmd_header = m_port->getCommandTable(header);
    memset(cmd_header, 0, sizeof(AHCICommandTable) - sizeof(AHCIPRDTEntry) * 65535); // table and 1 PRDT entry
    memcpy(&(cmd_header->CFIS), &h2d, sizeof(FIS_RegH2D));

    AHCIPRDTEntry* prdt = cmd_header->PRDTEntries;
    AHCI_ATAIdentify* data = (AHCI_ATAIdentify*)g_KPM->AllocatePage(PagePermissions::READ_WRITE);
    memset(data, 0, 4096);
    void* data_phys = g_KPM->GetPageTable().GetPhysicalAddress(data);
    prdt->DataBaseAddress = ((uint64_t)data_phys & 0xFFFFFFFF) >> 1;
    prdt->DataBaseAddressUpper = ((uint64_t)data_phys >> 32) & 0xFFFFFFFF;
    prdt->ByteCount = 4095;
    prdt->InterruptOnCompletion = 1;

    m_port->IssueCommand(header, 255, true); // fixme

    //delete header;

    //dbgprintf("Device identify complete\n");

    __asm__ volatile ("" ::: "memory");
    while (data->GeneralConfiguration.ResponseIncomplete)
        __asm__ volatile ("" ::: "memory");

    // Dump the raw contents of the data.
    for (int i = 0; i < 512; i++) {
        if ((i % 16) == 0)
            dbgprintf("\n");
        else if ((i % 8) == 0)
            dbgprintf(" ");
        dbgprintf("%02hhx ", ((uint8_t*)data)[i]);
    }
    dbgprintf("\n");

    dbgprintf("ATA Command Set %hu.%hu\n", data->MajorVersionNumber, data->MinorVersionNumber);

    // Print all the sector information and largest accessible LBA
    dbgprintf("Sector size: %x\n", data->LogicalSectorSize);
    dbgprintf("Sector count: %x\n", data->TotalSectors);

    while (true) {}
}

bool AHCIDisk::Read(uint8_t* buffer, uint64_t lba, uint64_t count) {
    if ((lba + count) > m_sector_count)
        return false;
}

bool AHCIDisk::Write(const uint8_t* buffer, uint64_t lba, uint64_t count) {
    if ((lba + count) > m_sector_count)
        return false;
}

size_t AHCIDisk::GetSectorSize() {
    return m_sector_size;
}

size_t AHCIDisk::GetSectorCount() {
    return m_sector_count;
}

}