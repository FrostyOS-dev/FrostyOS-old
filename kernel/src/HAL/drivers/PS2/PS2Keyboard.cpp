/*
Copyright (©) 2023-2024  Frosty515

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

#include "PS2Keyboard.hpp"

#include <HAL/hal.hpp>

#ifdef __x86_64__

#include <arch/x86_64/8042PS2Controller.hpp>

#endif

/* Define the keyboard commands */

#define PS2_KEYBOARD_CMD_SET_LEDS 0xED
#define PS2_KEYBOARD_CMD_ECHO 0xEE
#define PS2_KEYBOARD_CMD_SCAN_CODE_SET 0xF0
#define PS2_KEYBOARD_CMD_IDENTIFY 0xF2
#define PS2_KEYBOARD_CMD_TYPEMATIC 0xF3
#define PS2_KEYBOARD_CMD_ENABLE_SCANNING 0xF4
#define PS2_KEYBOARD_CMD_DISABLE_SCANNING 0xF5
#define PS2_KEYBOARD_CMD_RESET_ENABLE 0xF6
#define PS2_KEYBOARD_CMD_ALL_TYPEMATIC 0xF7
#define PS2_KEYBOARD_CMD_ALL_MAKE_RELEASE 0xF8
#define PS2_KEYBOARD_CMD_ALL_MAKE 0xF9
#define PS2_KEYBOARD_CMD_ALL_TYPEMATIC_MAKE_RELEASE 0xFA
#define PS2_KEYBOARD_CMD_SINGLE_TYPEMATIC 0xFB
#define PS2_KEYBOARD_CMD_SINGLE_MAKE_RELEASE 0xFC
#define PS2_KEYBOARD_CMD_SINGLE_BREAK 0xFD
#define PS2_KEYBOARD_CMD_RESEND 0xFE
#define PS2_KEYBOARD_CMD_RESET 0xFF

/* Define the keyboard responses */

#define PS2_KEYBOARD_RESPONSE_ACK 0xFA
#define PS2_KEYBOARD_RESPONSE_ECHO 0xEE
#define PS2_KEYBOARD_RESPONSE_RESEND 0xFE
#define PS2_KEYBOARD_RESPONSE_RESET 0xAA


PS2Keyboard::PS2Keyboard(PS2Controller* controller, bool channel, char const* name) : m_Controller(controller), m_Channel(channel), m_Name(name), m_scancode{0}, m_scancode_offset(0), m_light_states(0) {
    p_eventHandler = {nullptr, nullptr};
    p_eventStack = Stack<KeyboardEvent>(4096);
}

PS2Keyboard::~PS2Keyboard() {

}

const char* PS2Keyboard::getVendorName() const {
    return "";
}

const char* PS2Keyboard::getDeviceName() const {
    return "";
}

void PS2Keyboard::HandleInterrupt() {
    uint8_t data = m_Controller->ReadData();
    if (m_ScanCodeMetadata.ignore_next) {
        m_ScanCodeMetadata.ignore_next = false;
        return;
    }
    m_scancode[m_scancode_offset++] = data;
    if (!m_ScanCodeMetadata.e1 && !m_ScanCodeMetadata.e0 && !m_ScanCodeMetadata.special && data == 0xE0)
        m_ScanCodeMetadata.e0 = true;
    else if (!m_ScanCodeMetadata.e1 && !m_ScanCodeMetadata.e0 && !m_ScanCodeMetadata.special && data == 0xE1)
        m_ScanCodeMetadata.e1 = true;
    else if (!m_ScanCodeMetadata.release && data == 0xF0)
        m_ScanCodeMetadata.release = true;
    else if (!m_ScanCodeMetadata.special && data == 0x12 && m_ScanCodeMetadata.e0 && !m_ScanCodeMetadata.release)
        m_ScanCodeMetadata.special = true;
    else if (!m_ScanCodeMetadata.special && data == 0x7C && m_ScanCodeMetadata.e0 && m_ScanCodeMetadata.release)
        m_ScanCodeMetadata.special = true;
    else if (m_ScanCodeMetadata.e1 && m_scancode_offset < 8)
        return; // not big enough yet
    else if (m_ScanCodeMetadata.special && ((m_ScanCodeMetadata.release && m_scancode_offset < 6) || (!m_ScanCodeMetadata.release && m_scancode_offset < 4)))
        return; // also not big enough yet
    else {
        HandleKey(m_scancode, m_scancode_offset);
        m_scancode_offset = 0;
        m_ScanCodeMetadata = {false, false, false, false, false};
    }
}

void HandleKeyboardInterrupt(void* data) {
    PS2Keyboard* keyboard = (PS2Keyboard*)data;
    keyboard->HandleInterrupt();
}

void PS2Keyboard::Initialise() {
#ifdef __x86_64__
    x86_64_8042_RegisterIRQHandler(HandleKeyboardInterrupt, this, m_Channel);
#endif
    m_Controller->EnableInterrupts(m_Channel);
    puts("PS/2 Keyboard init success\n");
}

void PS2Keyboard::Destroy() {
    // FIXME: actually implement this
}

void PS2Keyboard::HandleKey(unsigned char* scancode, unsigned char size) {
    KeyboardEvent event;
    if (size >= 2 && scancode[0] == 0xE0) { // extended scan codes
        uint8_t byte = 0;
        if (size == 4 || size == 6) {
            event.pressed = size == 4;
            event.keyCode = KeyCodes::KC_PRINTSCREEN;
            PushEvent(event);
            return;
        }
        if (size == 3 && scancode[1] == 0xF0) {
            event.pressed = false;
            byte = scancode[2];
        }
        else if (size == 2) {
            event.pressed = true;
            byte = scancode[1];
        }
        switch (byte) {
        case 0x11:
            event.keyCode = KeyCodes::KC_RALT;
            break;
        case 0x14:
            event.keyCode = KeyCodes::KC_RCTRL;
            break;
        case 0x1F:
            event.keyCode = KeyCodes::KC_LMETA;
            break;
        case 0x2F:
            event.keyCode = KeyCodes::KC_RMETA;
            break;
        case 0x4A:
            event.keyCode = KeyCodes::KC_NUMPAD_SLASH;
            break;
        case 0x5A:
            event.keyCode = KeyCodes::KC_NUMPAD_ENTER;
            break;
        case 0x69:
            event.keyCode = KeyCodes::KC_END;
            break;
        case 0x6B:
            event.keyCode = KeyCodes::KC_LEFT;
            break;
        case 0x6C:
            event.keyCode = KeyCodes::KC_HOME;
            break;
        case 0x70:
            event.keyCode = KeyCodes::KC_INSERT;
            break;
        case 0x71:
            event.keyCode = KeyCodes::KC_DEL;
            break;
        case 0x72:
            event.keyCode = KeyCodes::KC_DOWN;
            break;
        case 0x74:
            event.keyCode = KeyCodes::KC_RIGHT;
            break;
        case 0x75:
            event.keyCode = KeyCodes::KC_UP;
            break;
        case 0x7A:
            event.keyCode = KeyCodes::KC_PAGEDOWN;
            break;
        case 0x7D:
            event.keyCode = KeyCodes::KC_PAGEUP;
            break;
        default:
            event.keyCode = KeyCodes::KC_UNKNOWN;
            break;
        }
        PushEvent(event);
        return;
    }
    else if (size == 8 && scancode[0] == 0xE1) {
        event.keyCode = KeyCodes::KC_PAUSE;
        event.pressed = true;
        PushEvent(event);
        event.pressed = false;
        PushEvent(event);
        return;
    }
    else if (size == 1 || (size == 2 && scancode[0] == 0xF0)) {
        uint8_t byte;
        if (size == 1) {
            event.pressed = true;
            byte = scancode[0];
        }
        else {
            event.pressed = false;
            byte = scancode[1];
        }
        switch (byte) {
        case 0x01:
            event.keyCode = KeyCodes::KC_F9;
            break;
        case 0x03:
            event.keyCode = KeyCodes::KC_F5;
            break;
        case 0x04:
            event.keyCode = KeyCodes::KC_F3;
            break;
        case 0x05:
            event.keyCode = KeyCodes::KC_F1;
            break;
        case 0x06:
            event.keyCode = KeyCodes::KC_F2;
            break;
        case 0x07:
            event.keyCode = KeyCodes::KC_F12;
            break;
        case 0x09:
            event.keyCode = KeyCodes::KC_F10;
            break;
        case 0x0A:
            event.keyCode = KeyCodes::KC_F8;
            break;
        case 0x0B:
            event.keyCode = KeyCodes::KC_F6;
            break;
        case 0x0C:
            event.keyCode = KeyCodes::KC_F4;
            break;
        case 0x0D:
            event.keyCode = KeyCodes::KC_TAB;
            break;
        case 0x0E:
            event.keyCode = KeyCodes::KC_GRAVE;
            break;
        case 0x11:
            event.keyCode = KeyCodes::KC_LALT;
            break;
        case 0x12:
            event.keyCode = KeyCodes::KC_LSHIFT;
            break;
        case 0x14:
            event.keyCode = KeyCodes::KC_LCTRL;
            break;
        case 0x15:
            event.keyCode = KeyCodes::KC_Q;
            break;
        case 0x16:
            event.keyCode = KeyCodes::KC_1;
            break;
        case 0x1A:
            event.keyCode = KeyCodes::KC_Z;
            break;
        case 0x1B:
            event.keyCode = KeyCodes::KC_S;
            break;
        case 0x1C:
            event.keyCode = KeyCodes::KC_A;
            break;
        case 0x1D:
            event.keyCode = KeyCodes::KC_W;
            break;
        case 0x1E:
            event.keyCode = KeyCodes::KC_2;
            break;
        case 0x21:
            event.keyCode = KeyCodes::KC_C;
            break;
        case 0x22:
            event.keyCode = KeyCodes::KC_X;
            break;
        case 0x23:
            event.keyCode = KeyCodes::KC_D;
            break;
        case 0x24:
            event.keyCode = KeyCodes::KC_E;
            break;
        case 0x25:
            event.keyCode = KeyCodes::KC_4;
            break;
        case 0x26:
            event.keyCode = KeyCodes::KC_3;
            break;
        case 0x29:
            event.keyCode = KeyCodes::KC_SPACE;
            break;
        case 0x2A:
            event.keyCode = KeyCodes::KC_V;
            break;
        case 0x2B:
            event.keyCode = KeyCodes::KC_F;
            break;
        case 0x2C:
            event.keyCode = KeyCodes::KC_T;
            break;
        case 0x2D:
            event.keyCode = KeyCodes::KC_R;
            break;
        case 0x2E:
            event.keyCode = KeyCodes::KC_5;
            break;
        case 0x31:
            event.keyCode = KeyCodes::KC_N;
            break;
        case 0x32:
            event.keyCode = KeyCodes::KC_B;
            break;
        case 0x33:
            event.keyCode = KeyCodes::KC_H;
            break;
        case 0x34:
            event.keyCode = KeyCodes::KC_G;
            break;
        case 0x35:
            event.keyCode = KeyCodes::KC_Y;
            break;
        case 0x36:
            event.keyCode = KeyCodes::KC_6;
            break;
        case 0x3A:
            event.keyCode = KeyCodes::KC_M;
            break;
        case 0x3B:
            event.keyCode = KeyCodes::KC_J;
            break;
        case 0x3C:
            event.keyCode = KeyCodes::KC_U;
            break;
        case 0x3D:
            event.keyCode = KeyCodes::KC_7;
            break;
        case 0x3E:
            event.keyCode = KeyCodes::KC_8;
            break;
        case 0x41:
            event.keyCode = KeyCodes::KC_COMMA;
            break;
        case 0x42:
            event.keyCode = KeyCodes::KC_K;
            break;
        case 0x43:
            event.keyCode = KeyCodes::KC_I;
            break;
        case 0x44:
            event.keyCode = KeyCodes::KC_O;
            break;
        case 0x45:
            event.keyCode = KeyCodes::KC_0;
            break;
        case 0x46:
            event.keyCode = KeyCodes::KC_9;
            break;
        case 0x49:
            event.keyCode = KeyCodes::KC_PERIOD;
            break;
        case 0x4A:
            event.keyCode = KeyCodes::KC_SLASH;
            break;
        case 0x4B:
            event.keyCode = KeyCodes::KC_L;
            break;
        case 0x4C:
            event.keyCode = KeyCodes::KC_SEMICOLON;
            break;
        case 0x4D:
            event.keyCode = KeyCodes::KC_P;
            break;
        case 0x4E:
            event.keyCode = KeyCodes::KC_MINUS;
            break;
        case 0x52:
            event.keyCode = KeyCodes::KC_APOSTROPHE;
            break;
        case 0x54:
            event.keyCode = KeyCodes::KC_LBRACKET;
            break;
        case 0x55:
            event.keyCode = KeyCodes::KC_EQUALS;
            break;
        case 0x58:
            event.keyCode = KeyCodes::KC_CAPSLOCK;
            break;
        case 0x59:
            event.keyCode = KeyCodes::KC_RSHIFT;
            break;
        case 0x5A:
            event.keyCode = KeyCodes::KC_ENTER;
            break;
        case 0x5B:
            event.keyCode = KeyCodes::KC_RBRACKET;
            break;
        case 0x5D:
            event.keyCode = KeyCodes::KC_BACKSLASH;
            break;
        case 0x66:
            event.keyCode = KeyCodes::KC_BACKSPACE;
            break;
        case 0x69:
            event.keyCode = KeyCodes::KC_NUMPAD_1;
            break;
        case 0x6B:
            event.keyCode = KeyCodes::KC_NUMPAD_4;
            break;
        case 0x6C:
            event.keyCode = KeyCodes::KC_NUMPAD_7;
            break;
        case 0x70:
            event.keyCode = KeyCodes::KC_NUMPAD_0;
            break;
        case 0x71:
            event.keyCode = KeyCodes::KC_NUMPAD_PERIOD;
            break;
        case 0x72:
            event.keyCode = KeyCodes::KC_NUMPAD_2;
            break;
        case 0x73:
            event.keyCode = KeyCodes::KC_NUMPAD_5;
            break;
        case 0x74:
            event.keyCode = KeyCodes::KC_NUMPAD_6;
            break;
        case 0x75:
            event.keyCode = KeyCodes::KC_NUMPAD_8;
            break;
        case 0x76:
            event.keyCode = KeyCodes::KC_ESC;
            break;
        case 0x77:
            event.keyCode = KeyCodes::KC_NUMLOCK;
            break;
        case 0x78:
            event.keyCode = KeyCodes::KC_F11;
            break;
        case 0x79:
            event.keyCode = KeyCodes::KC_NUMPAD_PLUS;
            break;
        case 0x7A:
            event.keyCode = KeyCodes::KC_NUMPAD_3;
            break;
        case 0x7B:
            event.keyCode = KeyCodes::KC_NUMPAD_MINUS;
            break;
        case 0x7C:
            event.keyCode = KeyCodes::KC_NUMPAD_ASTERISK;
            break;
        case 0x7D:
            event.keyCode = KeyCodes::KC_NUMPAD_9;
            break;
        case 0x7E:
            event.keyCode = KeyCodes::KC_SCROLLLOCK;
            break;
        case 0x83:
            event.keyCode = KeyCodes::KC_F7;
            break;
        case 0xFA:
            return; // ignore
        default:
            event.keyCode = KeyCodes::KC_UNKNOWN;
            break;
        }
        PushEvent(event);
        return;
    }
    else {
        event.keyCode = KeyCodes::KC_UNKNOWN;
        event.pressed = false;
        PushEvent(event);
        return;
    }
}

void PS2Keyboard::PushEvent(KeyboardEvent event) {
    if ((event.keyCode == KeyCodes::KC_CAPSLOCK || event.keyCode == KeyCodes::KC_NUMLOCK || event.keyCode == KeyCodes::KC_SCROLLLOCK) && !event.pressed) {
        uint8_t mask;
        switch (event.keyCode) {
        case KeyCodes::KC_CAPSLOCK:
            mask = 4;
            break;
        case KeyCodes::KC_NUMLOCK:
            mask = 2;
            break;
        case KeyCodes::KC_SCROLLLOCK:
            mask = 1;
            break;
        }
        if (m_light_states & mask)
            m_light_states &= ~mask;
        else
            m_light_states |= mask;
        uint8_t result = m_Controller->SendCommandToDevice(PS2_KEYBOARD_CMD_SET_LEDS, m_Channel, &m_light_states, 1);
        if (result != PS2_KEYBOARD_RESPONSE_ACK)
            dbgprintf("PS2Keyboard: Failed to set LEDs\n");
    }
    if (p_eventHandler.handler == nullptr || !(p_eventHandler.handler(p_eventHandler.data, event))) {
        KeyboardEvent* real_event = new KeyboardEvent(event);
        p_eventStack.Push(real_event);
    }
}