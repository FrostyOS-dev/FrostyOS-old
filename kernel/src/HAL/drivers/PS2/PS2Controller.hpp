/*
Copyright (©) 2023  Frosty515

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

#ifndef _PS2_CONTROLLER_HPP
#define _PS2_CONTROLLER_HPP

#include "../Device.hpp"

#include <stdint.h>
#include <stdio.h>

struct PS2DeviceInfo {
    bool type;
    char const* name;
    Device* device;
};

class PS2Keyboard;

class PS2Controller : public Device {
public:
    PS2Controller();
    ~PS2Controller();

    void Init();

    const char* getVendorName() const override;
    const char* getDeviceName() const override;

    uint8_t SendCommandToDevice(uint8_t command, bool channel, uint8_t* data = nullptr, uint64_t data_size = 0);
    void EnableInterrupts(bool channel);

    bool ReadData(uint8_t* out, uint64_t attempts = 0);
    uint8_t ReadData();

    PS2Keyboard* GetKeyboard();

    void PrintDeviceInfo(fd_t fd) const;
    
protected:
    uint8_t GetStatus();
    void SendCommand(uint8_t command);
    void WriteData(uint8_t data);

    char const* GetDeviceTypeName(uint8_t* type, uint8_t type_bytes);
    bool GetDeviceType(uint8_t* type, uint8_t type_bytes);

private:
    bool m_DualChannel;
    PS2DeviceInfo m_Devices[2];
};

#endif /* _PS2_CONTROLLER_HPP */