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

#include "PS2Controller.hpp"
#include "PS2Keyboard.hpp"

#ifdef __x86_64__

#include <arch/x86_64/8042PS2Controller.hpp>

#endif

#include <HAL/hal.hpp>

/* Define all the commands */

#define PS2_CMD_READ_CONFIG_BYTE 0x20
#define PS2_CMD_WRITE_CONFIG_BYTE 0x60
#define PS2_CMD_DISABLE_PORT_1 0xAD
#define PS2_CMD_DISABLE_PORT_2 0xA7
#define PS2_CMD_ENABLE_PORT_1 0xAE
#define PS2_CMD_ENABLE_PORT_2 0xA8
#define PS2_CMD_TEST_PORT_1 0xAB
#define PS2_CMD_TEST_PORT_2 0xA9
#define PS2_CMD_SELF_TEST 0xAA
#define PS2_CMD_TEST_CONTROLLER 0xA9
#define PS2_CMD_WRITE_PORT_1 0xD1
#define PS2_CMD_WRITE_PORT_2 0xD3
#define PS2_CMD_READ_PORT_1 0xD0
#define PS2_CMD_READ_PORT_2 0xD2
#define PS2_CMD_WRITE_PORT_2_INPUT 0xD4

/* Define the generic device commands */

#define PS2_DEVICE_CMD_ECHO 0xEE
#define PS2_DEVICE_CMD_IDENTIFY 0xF2
#define PS2_DEVICE_CMD_ENABLE_SCANNING 0xF4
#define PS2_DEVICE_CMD_DISABLE_SCANNING 0xF5
#define PS2_DEVICE_CMD_RESET_ENABLE 0xF6
#define PS2_DEVICE_CMD_ALL_MAKE_RELEASE 0xF8
#define PS2_DEVICE_CMD_ALL_MAKE 0xF9
#define PS2_DEVICE_CMD_SINGLE_MAKE_RELEASE 0xFC
#define PS2_DEVICE_CMD_SINGLE_BREAK 0xFD
#define PS2_DEVICE_CMD_RESEND 0xFE
#define PS2_DEVICE_CMD_RESET 0xFF

/* Define the keyboard responses */

#define PS2_DEVICE_RESPONSE_ACK 0xFA
#define PS2_DEVICE_RESPONSE_ECHO 0xEE
#define PS2_DEVICE_RESPONSE_RESEND 0xFE
#define PS2_DEVICE_RESPONSE_RESET 0xAA

PS2Controller::PS2Controller() {

}

PS2Controller::~PS2Controller() {

}

void PS2Controller::Init() {
    // Disable the controller
    SendCommand(PS2_CMD_DISABLE_PORT_1);
    SendCommand(PS2_CMD_DISABLE_PORT_2);

    // Flush the output buffer
    ReadData();

    // Set the controller configuration byte
    SendCommand(PS2_CMD_READ_CONFIG_BYTE);
    unsigned char configByte = ReadData();
    configByte &= ~(0b01000011);
    SendCommand(PS2_CMD_WRITE_CONFIG_BYTE);
    WriteData(configByte);

    // Perform the controller self test
    SendCommand(PS2_CMD_SELF_TEST);
    unsigned char selfTestResult = ReadData();
    if (selfTestResult != 0x55) {
        PANIC("PS2 controller self test failed!");
    }

    // Check if it is a dual channel controller
    SendCommand(PS2_CMD_ENABLE_PORT_2);
    SendCommand(PS2_CMD_READ_CONFIG_BYTE);
    configByte = ReadData();
    m_DualChannel = (configByte & 0b00000010) != 0;
    if (m_DualChannel)
        SendCommand(PS2_CMD_DISABLE_PORT_2);
    
    // Perform interface tests
    SendCommand(PS2_CMD_TEST_PORT_1);
    unsigned char port1TestResult = ReadData();
    if (port1TestResult != 0x00) {
        PANIC("PS2 port 1 test failed!");
    }
    if (m_DualChannel) {
        SendCommand(PS2_CMD_TEST_PORT_2);
        unsigned char port2TestResult = ReadData();
        if (port2TestResult != 0x00) {
            PANIC("PS2 port 2 test failed!");
        }
    }

    // Enable the ports
    SendCommand(PS2_CMD_ENABLE_PORT_1);
    if (m_DualChannel)
        SendCommand(PS2_CMD_ENABLE_PORT_2);

    // Reset ports
    //SendCommand(PS2_CMD_WRITE_PORT_1);
    WriteData(PS2_DEVICE_CMD_RESET);
    // read the result
    uint8_t attempts = UINT8_MAX;
    bool stage = false; // false for no response, true for pending result
    bool done = false;
    do {
        unsigned char ResetResult = ReadData();
        switch (ResetResult) {
            case PS2_DEVICE_RESPONSE_ACK:
                stage = true;
                break;
            case PS2_DEVICE_RESPONSE_RESEND:
                stage = false;
                break;
            case PS2_DEVICE_RESPONSE_RESET:
                if (!stage)
                    dbgprintf("PS/2 Controller Warning: PS/2 port 1 reset response before acknowledgement!\n");
                done = true;
                break;
            default:
                PANIC("PS/2 port 1 reset failed! (UNKNOWN)");
                break;
        }
    } while (--attempts > 0 && !done);
    if (!done) {
            PANIC("PS/2 port 1 reset timed out!");
        }
    if (m_DualChannel) {
        SendCommand(PS2_CMD_WRITE_PORT_2);
        WriteData(PS2_DEVICE_CMD_RESET);
        // read the result
        uint8_t attempts = UINT8_MAX;
        bool stage = false; // false for no response, true for pending result
        bool done = false;
        do {
            unsigned char ResetResult = ReadData();
            switch (ResetResult) {
                case PS2_DEVICE_RESPONSE_ACK:
                    stage = true;
                    break;
                case PS2_DEVICE_RESPONSE_RESEND:
                    stage = false;
                    break;
                case PS2_DEVICE_RESPONSE_RESET:
                    if (!stage)
                        dbgprintf("PS/2 Controller Warning: PS/2 port 2 reset response before acknowledgement!\n");
                    done = true;
                    break;
                default:
                    PANIC("PS/2 port 2 reset failed! (UNKNOWN)");
                    break;
            }
        } while (--attempts > 0 && !done);
        if (!done) {
            PANIC("PS/2 port 2 reset timed out!");
        }
    }

    // Temporary disable scanning
    WriteData(PS2_DEVICE_CMD_DISABLE_SCANNING);
    unsigned char result = ReadData();
    if (result != PS2_DEVICE_RESPONSE_ACK) {
        PANIC("PS2 device 1 identification failed!");
    }

    // Identify the devices
    WriteData(PS2_DEVICE_CMD_IDENTIFY);
    // read the result
    result = ReadData();
    if (result != PS2_DEVICE_RESPONSE_ACK) {
        PANIC("PS2 device 1 identification failed!");
    }
    uint8_t bytes_received = 0;
    uint8_t bytes[2];
    {
        uint16_t attempts = 0;
        do {
            if (GetStatus() & 1) {
                bytes[bytes_received++] = ReadData();
                attempts = 0;
            }
        } while (++attempts < UINT16_MAX && bytes_received < 2);
    }
    char const* device_type_name = GetDeviceTypeName(bytes, bytes_received);
    bool device_type = GetDeviceType(bytes, bytes_received);
    printf("Detected PS/2 %s: %s\n", device_type ? "Mouse" : "Keyboard", device_type_name);

    WriteData(PS2_DEVICE_CMD_ENABLE_SCANNING);
    result = ReadData();
    if (result != PS2_DEVICE_RESPONSE_ACK) {
        PANIC("PS2 device 1 setup failed!");
    }
    {
        Device* device = nullptr;
        if (device_type)
            printf("PS/2 Mouse not supported yet!\n");
        else {
            PS2Keyboard* keyboard = new PS2Keyboard(this, false, device_type_name);
            keyboard->Initialise();
            device = (Device*)keyboard;
        }
        m_Devices[0].type = device_type;
        m_Devices[0].name = device_type_name;
        m_Devices[0].device = device;
    }

    
    if (m_DualChannel) {
        // Temporary disable scanning
        SendCommand(PS2_CMD_WRITE_PORT_2);
        WriteData(PS2_DEVICE_CMD_DISABLE_SCANNING);
        unsigned char result = ReadData();
        if (result != PS2_DEVICE_RESPONSE_ACK) {
            PANIC("PS2 device 2 identification failed!");
        }

        // Identify the devices
        SendCommand(PS2_CMD_WRITE_PORT_2);
        WriteData(PS2_DEVICE_CMD_IDENTIFY);
        // read the result
        result = ReadData();
        if (result != PS2_DEVICE_RESPONSE_ACK) {
            PANIC("PS2 device 2 identification failed!");
        }
        uint8_t bytes_received = 0;
        uint8_t bytes[2];
        {
            uint16_t attempts = 0;
            do {
                if (GetStatus() & 1) {
                    bytes[bytes_received++] = ReadData();
                    attempts = 0;
                }
            } while (++attempts < UINT16_MAX && bytes_received < 2);
        }
        char const* device_type_name = GetDeviceTypeName(bytes, bytes_received);
        bool device_type = GetDeviceType(bytes, bytes_received);
        printf("Detected PS/2 %s: %s\n", device_type ? "Mouse" : "Keyboard", device_type_name);
        SendCommand(PS2_CMD_WRITE_PORT_2);
        WriteData(PS2_DEVICE_CMD_ENABLE_SCANNING);
        result = ReadData();
        if (result != PS2_DEVICE_RESPONSE_ACK) {
            PANIC("PS2 device 2 setup failed!");
        }
        Device* device = nullptr;
        if (device_type)
            printf("PS/2 Mouse not supported yet!\n");
        else {
            PS2Keyboard* keyboard = new PS2Keyboard(this, true, device_type_name);
            keyboard->Initialise();
            device = (Device*)keyboard;
        }
        m_Devices[1].type = device_type;
        m_Devices[1].name = device_type_name;
        m_Devices[1].device = device;
    }
    

}

#ifdef __x86_64__

const char* PS2Controller::getVendorName() {
    return "Intel Corporation";
}

const char* PS2Controller::getDeviceName() {
    if (m_DualChannel)
        return "8042 Dual Channel PS/2 Controller";
    else
        return "8042 Single Channel PS/2 Controller";
}

uint8_t PS2Controller::SendCommandToDevice(uint8_t command, bool channel, uint8_t* data, uint64_t data_size) {
    if (channel)
        SendCommand(PS2_CMD_WRITE_PORT_2);
    WriteData(command);
    for (uint64_t i = 0; i < data_size; i++)
        WriteData(data[i]);
    return ReadData();
}

void PS2Controller::EnableInterrupts(bool channel) {
    SendCommand(PS2_CMD_READ_CONFIG_BYTE);
    unsigned char configByte = ReadData();
    if (channel)
        configByte |= 0b00000010;
    else
        configByte |= 0b00000001;
    SendCommand(PS2_CMD_WRITE_CONFIG_BYTE);
    WriteData(configByte);
}

PS2Keyboard* PS2Controller::GetKeyboard() {
    if (!m_Devices[0].type)
        return (PS2Keyboard*)m_Devices[0].device;
    else if (!m_Devices[1].type)
        return (PS2Keyboard*)m_Devices[1].device;
    return nullptr;
}

uint8_t PS2Controller::GetStatus() {
    return x86_64_8042_ReadStatusRegister_Raw();
}

void PS2Controller::SendCommand(uint8_t command) {
    x86_64_8042_WriteCommand(command);
}

uint8_t PS2Controller::ReadData() {
    return x86_64_8042_ReadData();
}

void PS2Controller::WriteData(uint8_t data) {
    x86_64_8042_WriteData(data);
}

char const* PS2Controller::GetDeviceTypeName(uint8_t* type, uint8_t type_bytes) {
    char const* device_type_name = "Unknown device";
    if (type_bytes == 0)
        device_type_name = "Ancient AT Keyboard";
    else if (type_bytes == 1) {
        switch (type[0]) {
        case 0:
            device_type_name = "Standard PS/2 mouse";
            break;
        case 3:
            device_type_name = "Mouse with scroll wheel";
            break;
        case 4:
            device_type_name = "5-button mouse";
            break;
        }
    }
    else {
        switch (type[0]) {
        case 0xAB:
            switch (type[1]) {
                case 0x83:
                    device_type_name = "MF2 Keyboard";
                    break;
                case 0xC1:
                    device_type_name = "MF2 Keyboard";
                    break;
                case 0x84:
                    device_type_name = "IBM Thinkpads, Spacesaver keyboards, many other \"short\" keyboards";
                    break;
                case 0x85:
                    device_type_name = "NCD N-97 keyboard or 122-Key Host Connect(ed) keyboard";
                    break;
                case 0x86:
                    device_type_name = "122-key keyboards";
                    break;
                case 0x90:
                    device_type_name = "Japanese \"G\" keyboards";
                    break;
                case 0x91:
                    device_type_name = "Japanese \"P\" keyboards";
                    break;
                case 0x92:
                    device_type_name = "Japanese \"A\" keyboards";
                    break;
            }
            break;
        case 0xAC:
            switch (type[1]) {
            case 0xA1:
                device_type_name = "NCD Sun layout keyboard";
                break;
            }
            break;
        }
    }
    return device_type_name;
}

bool PS2Controller::GetDeviceType(uint8_t* type, uint8_t type_bytes) {
    bool device_type = false; // false for keyboard, true for mouse
    if (type_bytes == 0)
        device_type = false;
    else if (type_bytes == 1) {
        switch (type[0]) {
        case 0:
            device_type = true;
            break;
        case 3:
            device_type = true;
            break;
        case 4:
            device_type = true;
            break;
        }
    }
    else {
        switch (type[0]) {
        case 0xAB:
            switch (type[1]) {
                case 0x83:
                    device_type = false;
                    break;
                case 0xC1:
                    device_type = false;
                    break;
                case 0x84:
                    device_type = false;
                    break;
                case 0x85:
                    device_type = false;
                    break;
                case 0x86:
                    device_type = false;
                    break;
                case 0x90:
                    device_type = false;
                    break;
                case 0x91:
                    device_type = false;
                    break;
                case 0x92:
                    device_type = false;
                    break;
            }
            break;
        case 0xAC:
            switch (type[1]) {
            case 0xA1:
                device_type = false;
                break;
            }
            break;
        }
    }
    return device_type;
}

#endif /* __x86_64__ */
