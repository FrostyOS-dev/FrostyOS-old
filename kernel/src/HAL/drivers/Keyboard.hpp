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

#ifndef _KEYBOARD_HPP
#define _KEYBOARD_HPP

#include "Device.hpp"

#include <Data-structures/Stack.hpp>

#define ENUMERATE_KEYCODES(F) \
    F(KC_0) \
    F(KC_1) \
    F(KC_2) \
    F(KC_3) \
    F(KC_4) \
    F(KC_5) \
    F(KC_6) \
    F(KC_7) \
    F(KC_8) \
    F(KC_9) \
    F(KC_A) \
    F(KC_B) \
    F(KC_C) \
    F(KC_D) \
    F(KC_E) \
    F(KC_F) \
    F(KC_G) \
    F(KC_H) \
    F(KC_I) \
    F(KC_J) \
    F(KC_K) \
    F(KC_L) \
    F(KC_M) \
    F(KC_N) \
    F(KC_O) \
    F(KC_P) \
    F(KC_Q) \
    F(KC_R) \
    F(KC_S) \
    F(KC_T) \
    F(KC_U) \
    F(KC_V) \
    F(KC_W) \
    F(KC_X) \
    F(KC_Y) \
    F(KC_Z) \
    F(KC_F1) \
    F(KC_F2) \
    F(KC_F3) \
    F(KC_F4) \
    F(KC_F5) \
    F(KC_F6) \
    F(KC_F7) \
    F(KC_F8) \
    F(KC_F9) \
    F(KC_F10) \
    F(KC_F11) \
    F(KC_F12) \
    F(KC_F13) \
    F(KC_F14) \
    F(KC_F15) \
    F(KC_F16) \
    F(KC_F17) \
    F(KC_F18) \
    F(KC_F19) \
    F(KC_F20) \
    F(KC_F21) \
    F(KC_F22) \
    F(KC_F23) \
    F(KC_F24) \
    F(KC_GRAVE) \
    F(KC_MINUS) \
    F(KC_EQUALS) \
    F(KC_BACKSPACE) \
    F(KC_TAB) \
    F(KC_LBRACKET) \
    F(KC_RBRACKET) \
    F(KC_BACKSLASH) \
    F(KC_CAPSLOCK) \
    F(KC_SEMICOLON) \
    F(KC_APOSTROPHE) \
    F(KC_ENTER) \
    F(KC_LSHIFT) \
    F(KC_RSHIFT) \
    F(KC_COMMA) \
    F(KC_PERIOD) \
    F(KC_SLASH) \
    F(KC_LCTRL) \
    F(KC_RCTRL) \
    F(KC_LALT) \
    F(KC_RALT) \
    F(KC_SPACE) \
    F(KC_LMETA) \
    F(KC_RMETA) \
    F(KC_MENU) \
    F(KC_SCROLLLOCK) \
    F(KC_NUMLOCK) \
    F(KC_PRINTSCREEN) \
    F(KC_PAUSE) \
    F(KC_INSERT) \
    F(KC_HOME) \
    F(KC_END) \
    F(KC_PAGEUP) \
    F(KC_PAGEDOWN) \
    F(KC_UP) \
    F(KC_DOWN) \
    F(KC_LEFT) \
    F(KC_RIGHT) \
    F(KC_NUMPAD_0) \
    F(KC_NUMPAD_1) \
    F(KC_NUMPAD_2) \
    F(KC_NUMPAD_3) \
    F(KC_NUMPAD_4) \
    F(KC_NUMPAD_5) \
    F(KC_NUMPAD_6) \
    F(KC_NUMPAD_7) \
    F(KC_NUMPAD_8) \
    F(KC_NUMPAD_9) \
    F(KC_NUMPAD_ENTER) \
    F(KC_NUMPAD_PLUS) \
    F(KC_NUMPAD_MINUS) \
    F(KC_NUMPAD_ASTERISK) \
    F(KC_NUMPAD_SLASH) \
    F(KC_NUMPAD_PERIOD) \
    F(KC_ESC) \
    F(KC_DEL) \
    F(KC_UNKNOWN)

enum class KeyCodes {
#define __ENUMERATE_KEYCODES(x) x,
    ENUMERATE_KEYCODES(__ENUMERATE_KEYCODES)
#undef __ENUMERATE_KEYCODES
};

const char* GetKeyName(KeyCodes keyCode);

struct KeyboardEvent {
    KeyCodes keyCode;
    bool pressed;
};

typedef bool (*KeyboardEventHandler_t)(void*, KeyboardEvent);

class Keyboard : public Device {
public:
    virtual ~Keyboard() {};

    virtual const char* getVendorName() const override = 0;
    virtual const char* getDeviceName() const override = 0;

    virtual void Initialise() = 0;
    virtual void Destroy() = 0;

    inline virtual void RegisterEventHandler(KeyboardEventHandler_t handler, void* data) {
        p_eventHandler = {handler, data};
    }

    inline virtual KeyboardEventHandler_t GetEventHandler() const {
        return p_eventHandler.handler;
    }

protected:
    struct {
        KeyboardEventHandler_t handler;
        void* data;
    } p_eventHandler;
    Stack<KeyboardEvent> p_eventStack;
};

#endif /* _KEYBOARD_HPP */