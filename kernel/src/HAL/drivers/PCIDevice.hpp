/*
Copyright (©) 2022-2023  Frosty515

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

#ifndef _HAL_PCI_DEVICE_HPP
#define _HAL_PCI_DEVICE_HPP

#include "Device.hpp"
#include "PCI.hpp"

class PCIDevice : public Device {
public:
    virtual ~PCIDevice() override {}

    virtual void InitPCIDevice(PCI::Header0* device) = 0;

    virtual const char* getDeviceClass() const = 0;
    virtual const char* getDeviceSubClass() const = 0;
    virtual const char* getDeviceProgramInterface() const = 0;

protected:
    PCI::Header0* p_device = nullptr;
};

#endif /* _HAL_PCI_DEVICE_HPP */