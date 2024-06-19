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

#ifndef _PS2_KEYBOARD_HPP
#define _PS2_KEYBOARD_HPP

#include "../Keyboard.hpp"

#include "PS2Controller.hpp"

class PS2Keyboard : public Keyboard {
public:
    PS2Keyboard(PS2Controller* controller, bool channel, char const* name);
    ~PS2Keyboard();

    const char* getVendorName() const override;
    const char* getDeviceName() const override;

    void HandleInterrupt();

    void Initialise() override;
    void Destroy() override;

private:

    struct ScanCodeMetadata {
        bool e0;
        bool e1;
        bool release;
        bool special; // for print screen's weird combination
        bool ignore_next;
    };

    void HandleKey(unsigned char* scancode, unsigned char size);

    void PushEvent(KeyboardEvent event);

private:
    PS2Controller* m_Controller;
    bool m_Channel;
    char const* m_Name;
    ScanCodeMetadata m_ScanCodeMetadata;
    uint8_t m_scancode[8];
    uint8_t m_scancode_offset;
    uint8_t m_light_states;
};

#endif /* _PS2_KEYBOARD_HPP */