/*
Copyright (©) 2024  Frosty515

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _AHCI_DISK_HPP
#define _AHCI_DISK_HPP

#include <stdint.h>

#include "../Disk.hpp"

#include "AHCIPort.hpp"

namespace AHCI {

    struct AHCI_ATAIdentify {
        struct GeneralConfig {
            uint16_t Reserved0 : 1;
            uint16_t Retired1 : 1;
            uint16_t ResponseIncomplete : 1;
            uint16_t Retired2 : 3;
            uint16_t FixedDevice : 1;
            uint16_t RemovableMedia : 1;
            uint16_t Retired3 : 7;
            uint16_t DeviceType : 1; // should be 0
        } __attribute__((packed)) GeneralConfiguration;
        uint16_t Obsolete0;
        uint16_t SpecificConfig;
        uint16_t Obsolete1;
        uint16_t Retired0[2];
        uint16_t Obsolete2;
        uint16_t Reserved0[2];
        uint16_t Retired1;
        uint16_t SerialNumber[10];
        uint16_t Retired2[2];
        uint16_t Obsolete3;
        uint16_t FirmwareRevision[4];
        uint16_t ModelNumber[20];
        uint16_t Obsolete4;
        uint16_t TrustedComputingSupport;
        uint16_t Capabilities[2];
        uint16_t Obsolete5[2];
        uint16_t Other0;
        uint16_t Obsolete6[5];
        uint16_t Capabilities2;
        uint32_t TotalSectors;
        uint16_t Obsolete7;
        uint16_t MultiwordDMASupport;
        uint16_t AdvancedPIOModes;
        uint16_t MinimumMultiwordDMATime;
        uint16_t ManufacturerRecommendedDMATime;
        uint16_t MinimumPIOTimeWithoutFlowControl;
        uint16_t MinimumPIOTimeWithIORDY;
        uint16_t AdditionalSupported;
        uint16_t Reserved1[5];
        uint16_t QueueDepth;
        uint16_t SerialATACapabilities;
        uint16_t SerialATAAdditionalCapabilities;
        uint16_t SerialATAFeaturesSupported;
        uint16_t SerialATAFeaturesEnabled;
        uint16_t MajorVersionNumber;
        uint16_t MinorVersionNumber;
        uint16_t CommandSetSupported0;
        uint16_t CommandSetSupported1;
        uint16_t CommandSetSupported2;
        uint16_t CommandSetSupportedOrEnabled0;
        uint16_t CommandSetSupportedOrEnabled1;
        uint16_t CommandSetSupportedOrEnabled2;
        uint16_t UltraDMASupport;
        uint16_t NormalEraseTime;
        uint16_t EnhancedEraseTime;
        uint16_t CurrentAPMLevel;
        uint16_t MasterPasswordID;
        uint16_t HardwareResetResult;
        uint16_t Obsolete8;
        uint16_t StreamingMinimumRequestSize;
        uint16_t StreamingTransferTimeDMA;
        uint16_t StreamingAccessLatencyDMAPIO;
        uint32_t StreamingPerformanceGranularity;
        uint64_t NumberOfUserAddressableLogicalSectors;
        uint16_t StreamingTransferTimePIO;
        uint16_t MaximumNumber512ByteBlockDATASETMANAGEMENT;
        uint16_t PhysicalLogicalSectorSize;
        uint16_t InterSeekDelay;
        uint16_t WorldWideName[4];
        uint16_t Reserved2[4];
        uint16_t Obsolete9;
        uint32_t LogicalSectorSize;
        uint16_t CommandsAndFeatureSetsSupported;
        uint16_t CommandsAndFeatureSetsSupportedOrEnabled;
        uint16_t ReservedForExpandSupportAndEnabledSettings[6];
        uint16_t Obsolete10;
        uint16_t SecurityStatus;
        uint16_t VendorSpecific[31];
        uint16_t ReservedForCFAWord[8];
        uint16_t DeviceNorminalFormFactor;
        uint16_t DataSetManagementFeature;
        uint16_t AdditionalProductID[4];
        uint16_t Reserved3[2];
        uint16_t CurrentMediaSerialNumber[30];
        uint16_t SCTCommandTransport;
        uint16_t Reserved4[2];
        uint16_t AlignmentOfLogicalWithinPhysical;
        uint32_t WriteReadVerifySectorCountMode3Only;
        uint32_t WriteReadVerifySectorCountMode2Only;
        uint16_t Obsolete11[3];
        uint16_t NominalMediaRotationRate;
        uint16_t Reserved5;
        uint16_t Obsolete12;
        uint16_t WriteReadVerifyCurrentFeatureSet;
        uint16_t Reserved6;
        uint16_t TransportMajorVersionNumber;
        uint16_t TransportMinorVersionNumber;
        uint16_t Reserved7[6];
        uint64_t ExtendedNumberOfUserAddressableSectors;
        uint16_t MinBlocksPerDownloadMicrocode;
        uint16_t MaxBlocksPerDownloadMicrocode;
        uint16_t Reserved8[19];
        uint16_t Signature;
    } __attribute__((packed));

    class AHCIDisk : public Disk {
    public:
        AHCIDisk(AHCIPort* port);
        ~AHCIDisk();

        void Init() override;

        bool Read(uint8_t* buffer, uint64_t lba, uint64_t count = 1) override;
        bool Write(const uint8_t* buffer, uint64_t lba, uint64_t count = 1) override;

        size_t GetSectorSize() override;
        size_t GetSectorCount() override;

    private:
        AHCIPort* m_port;
        uint64_t m_sector_count;
        uint32_t m_sector_size;
    };

}

#endif /* _AHCI_DISK_HPP */