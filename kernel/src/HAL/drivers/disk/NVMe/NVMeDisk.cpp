#include "NVMeDisk.hpp"

#include <Memory/PagingUtil.hpp>
#include <Memory/PageManager.hpp>

#include <assert.h>
#include <stdio.hpp>
#include <util.h>

namespace NVMe {

    NVMeDisk::NVMeDisk(uint32_t ID, NVMeController* controller, NVMeIOQueue* IOQueue) : m_ID(ID), m_controller(controller), m_IOQueue(IOQueue) {

    }

    NVMeDisk::~NVMeDisk() {

    }

    void NVMeDisk::Read(uint8_t* buffer, uint64_t lba, uint64_t count) {
        
    }

    void NVMeDisk::Write(const uint8_t* buffer, uint64_t lba, uint64_t count) {

    }

    size_t NVMeDisk::GetSectorSize() {

    }

}
