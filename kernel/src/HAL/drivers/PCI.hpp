#ifndef _HAL_PCI_HPP
#define _HAL_PCI_HPP

#include <stdint.h>

namespace PCI {
    struct CommonHeader {
        uint16_t VendorID;
        uint16_t DeviceID;
        uint16_t Command;
        uint16_t Status;
        uint8_t RevisionID;
        uint8_t ProgIF;
        uint8_t SubClass;
        uint8_t ClassCode;
        uint8_t CacheLineSize;
        uint8_t LatencyTimer;
        uint8_t HeaderType;
        uint8_t BIST;
    } __attribute__((packed));

    struct CommandRegister {
        uint8_t             IOSpace : 1; // RW
        uint8_t         MemorySpace : 1; // RW
        uint8_t           BusMaster : 1; // RW
        uint8_t       SpecialCycles : 1; // RO
        uint8_t        MWAndInvalid : 1; // RO
        uint8_t     VGAPaletteSnoop : 1; // RO
        uint8_t ParityErrorResponse : 1; // RW
        uint8_t           Reserved0 : 1; // RO
        uint8_t          SEEREnable : 1; // RW
        uint8_t        FastBBEnable : 1; // RO
        uint8_t    InterruptDisable : 1; // RW
        uint8_t           Reserved1 : 5; // RO
    } __attribute__((packed));

    struct StatusRegister {
        uint8_t        Reserved0 : 3; // RO
        uint8_t  InterruptStatus : 1; // RO
        uint8_t CapabilitiesList : 1; // RO
        uint8_t     Capable66MHz : 1; // RO
        uint8_t        Reserved1 : 1; // RO
        uint8_t    FastBBCapable : 1; // RO
        uint8_t        MDataPErr : 1; // RW1C
        uint8_t     DEVSELTiming : 1; // RO
        uint8_t   SignaledTAbort : 1; // RW1C
        uint8_t   ReceivedTAbort : 1; // RW1C
        uint8_t   SignaledSysErr : 1; // RW1C
        uint8_t     DetectedPErr : 1; // RW1C
    } __attribute__((packed));

    struct Header0 {
        CommonHeader ch;
        uint32_t BAR0;
        uint32_t BAR1;
        uint32_t BAR2;
        uint32_t BAR3;
        uint32_t BAR4;
        uint32_t BAR5;
        uint32_t CardbUsCISPTR;
        uint16_t SubsystemVendorID;
        uint16_t SubsystemID;
        uint32_t ExpandROMBA;
        uint8_t CapabilitiesPTR;
        uint8_t Reserved0;
        uint16_t Reserved1;
        uint32_t Reserved2;
        uint8_t INTLine;
        uint8_t INTPIN;
        uint8_t MinGrant;
        uint8_t MaxLatency;
    } __attribute__((packed));

    struct Header1 {
        CommonHeader ch;
        uint32_t BAR0;
        uint32_t BAR1;
        uint8_t PrimaryBusNum;
        uint8_t SecondaryBusNum;
        uint8_t SubordinateBusNum;
        uint8_t SecondaryLatencyTimer;
        uint8_t IOBase;
        uint8_t IOLimit;
        uint16_t SecondaryStatus;
        uint16_t MemoryBase;
        uint16_t MemoryLimit;
        uint16_t PrefetchMemBase;
        uint16_t PrefetchMemLimit;
        uint32_t PrefetchBaseUpper;
        uint32_t PrefetchLimitUpper;
        uint16_t IOBaseUpper;
        uint16_t IOLimitUpper;
        uint8_t CapabilityPTR;
        uint8_t Reserved0;
        uint16_t Reserved1;
        uint32_t ExpandROMBA;
        uint8_t INTLine;
        uint8_t INTPIN;
        uint16_t BridgeControl;
    } __attribute__((packed));

    struct HeaderTypeRegister {
        uint8_t HeaderType : 7;
        uint8_t MF : 1;
    } __attribute__((packed));

    struct BISTRegister {
        uint8_t CompletionCode : 4;
        uint8_t Reserved : 2;
        uint8_t StartBIST : 1;
        uint8_t BISTCapable : 1;
    } __attribute__((packed));

    struct Header2 {
        CommonHeader ch;
        uint32_t CardBusSocketBA;
        uint8_t CapabilitiesListOffset;
        uint8_t Reserved0;
        uint16_t SecondaryStatus;
        uint8_t PCIBusNum;
        uint8_t CardBusBNum;
        uint8_t SubordinateBusNum;
        uint8_t CardBusLatencyTimer;
        uint32_t MemBase0;
        uint32_t MemLimit0;
        uint32_t MemBase1;
        uint32_t MemLimit1;
        uint32_t IOBase0;
        uint32_t IOLimit0;
        uint32_t IOBase1;
        uint32_t IOLimit1;
        uint8_t INTLine;
        uint8_t INTPIN;
        uint16_t BridgeControl;
        uint16_t SubsystemDeviceID;
        uint16_t SubsystemVendorID;
        uint32_t LegacyModeBase;
    } __attribute__((packed));

    struct MemSpaceBaseAddressRegister {
        uint8_t always0 : 1;
        uint8_t type : 2;
        uint8_t Prefetchable : 1;
        uint32_t AlignedBaseAddress : 28; // 16-byte aligned
    } __attribute__((packed));

    struct IOSpaceBaseAddressRegister {
        uint8_t always1 : 1;
        uint8_t reserved : 1;
        uint32_t AlignedBaseAddress : 30; // 4-byte aligned
    } __attribute__((packed));

    void EnumerateFunctions(void* device_addr);
    void EnumerateDevices(void* bus_addr);
    void EnumerateBuses(void* segment_addr);

    namespace PCIDeviceList {
        Header0* GetPCIDevice(uint64_t index);
        void AddPCIDevice(Header0* device);
        void RemovePCIDevice(uint64_t index);
    }
}

#endif /* _HAL_PCI_HPP */