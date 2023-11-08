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

#include "KeyboardInput.hpp"

bool KeyboardEventHandler(void* data, KeyboardEvent event) {
    KeyboardInput* input = (KeyboardInput*)data;
    if (input == nullptr)
        return false;
    return input->HandleEvent(event);
}

KeyboardInput::KeyboardInput() : m_bufferOffset(0), m_keyboard(nullptr), m_keyboardState({false, false, false, false, false}), m_bufferSize(0), m_buffer(DEFAULT_BUFFER_BLOCK_SIZE, DEFAULT_BUFFER_BLOCK_SIZE), m_keyCallback({nullptr, nullptr}) {

}

KeyboardInput::~KeyboardInput() {

}

void KeyboardInput::Initialise(Keyboard* keyboard) {
    m_keyboard = keyboard;
    if (m_keyboard != nullptr)
        m_keyboard->RegisterEventHandler(KeyboardEventHandler, this);
}

void KeyboardInput::Destroy() {
    if (m_keyboard != nullptr)
        m_keyboard->RegisterEventHandler(nullptr, nullptr);
}

int KeyboardInput::GetChar() {
    if (m_bufferSize == 0)
        return -1;
    size_t initial_size = m_buffer.GetSize();
    char c = 0;
    m_buffer.Read(m_bufferOffset, (uint8_t*)&c, sizeof(char));
    m_buffer.ClearUntil(m_bufferOffset + sizeof(char));
    if (m_buffer.GetSize() < initial_size)
        m_bufferOffset = 0;
    m_bufferOffset += sizeof(char);
    m_bufferSize--;
    return (int)c;
}

bool KeyboardInput::HandleEvent(KeyboardEvent event) {
    // FIXME: The CTRL, SHIFT and ALT keys have a left and right equivalent, this should take that into account instead of just assuming they are the same key.
    if (event.keyCode == KeyCodes::KC_LCTRL || event.keyCode == KeyCodes::KC_RCTRL) {
        m_keyboardState.control = event.pressed;
        return true;
    }
    else if (event.keyCode == KeyCodes::KC_LSHIFT || event.keyCode == KeyCodes::KC_RSHIFT) {
        m_keyboardState.shift = event.pressed;
        return true;
    }
    else if (event.keyCode == KeyCodes::KC_LALT || event.keyCode == KeyCodes::KC_RALT) {
        m_keyboardState.alt = event.pressed;
        return true;
    }
    else if (event.keyCode == KeyCodes::KC_CAPSLOCK && !event.pressed) { // only update on release
        m_keyboardState.caps_lock = !m_keyboardState.caps_lock;
        return true;
    }
    else if (event.keyCode == KeyCodes::KC_NUMLOCK && !event.pressed) { // only update on release
        m_keyboardState.num_lock = !m_keyboardState.num_lock;
        return true;
    }
    if (!event.pressed)
        return true; // for all the other keys we deal with, we only care about the press event, not the release
    if (event.keyCode >= KeyCodes::KC_A && event.keyCode <= KeyCodes::KC_Z) {
        if (m_keyboardState.control) {
            char c = (char)event.keyCode - (char)KeyCodes::KC_A + 1;
            AppendChar(c);
        }
        else if (m_keyboardState.alt)
            return false; // Unknown alt key
        else if (m_keyboardState.shift || m_keyboardState.caps_lock) {
            char c = (char)event.keyCode - (char)KeyCodes::KC_A + 'A';
            AppendChar(c);
        }
        else {
            char c = (char)event.keyCode - (char)KeyCodes::KC_A + 'a';
            AppendChar(c);
        }
    }
    else if (event.keyCode >= KeyCodes::KC_0 && event.keyCode <= KeyCodes::KC_9) {
        if (m_keyboardState.shift) {
            if (event.keyCode == KeyCodes::KC_0)
                AppendChar(')');
            else if (event.keyCode == KeyCodes::KC_1)
                AppendChar('!');
            else if (event.keyCode == KeyCodes::KC_2) {
                if (m_keyboardState.control)
                    AppendChar('\x00');
                else
                    AppendChar('@');
            }
            else if (event.keyCode == KeyCodes::KC_3)
                AppendChar('#');
            else if (event.keyCode == KeyCodes::KC_4)
                AppendChar('$');
            else if (event.keyCode == KeyCodes::KC_5)
                AppendChar('%');
            else if (event.keyCode == KeyCodes::KC_6) {
                if (m_keyboardState.control)
                    AppendChar('\x1e');
                else
                    AppendChar('^');
            }
            else if (event.keyCode == KeyCodes::KC_7)
                AppendChar('&');
            else if (event.keyCode == KeyCodes::KC_8)
                AppendChar('*');
            else if (event.keyCode == KeyCodes::KC_9)
                AppendChar('(');
            else
                return false; // Unknown shift key
        }
        else {
            char c = (char)event.keyCode - (char)KeyCodes::KC_0 + '0';
            AppendChar(c);
        }
    }
    else if (event.keyCode >= KeyCodes::KC_NUMPAD_0 && event.keyCode <= KeyCodes::KC_NUMPAD_9) {
        if (m_keyboardState.num_lock) {
            char c = (char)event.keyCode - (char)KeyCodes::KC_NUMPAD_0 + '0';
            AppendChar(c);
        }
    }
    else if (event.keyCode == KeyCodes::KC_SPACE)
        AppendChar(' ');
    else if (event.keyCode == KeyCodes::KC_MINUS) {
        if (m_keyboardState.shift) {
            if (m_keyboardState.control)
                AppendChar('\x1f');
            else
                AppendChar('_');
        }
        else
            AppendChar('-');
    }
    else if (event.keyCode == KeyCodes::KC_EQUALS) {
        if (m_keyboardState.shift)
            AppendChar('+');
        else
            AppendChar('=');
    }
    else if (event.keyCode == KeyCodes::KC_LBRACKET) {
        if (m_keyboardState.shift)
            AppendChar('{');
        else if (m_keyboardState.control)
            AppendChar('\x1b');
        else
            AppendChar('[');
    }
    else if (event.keyCode == KeyCodes::KC_RBRACKET) {
        if (m_keyboardState.shift)
            AppendChar('}');
        else if (m_keyboardState.control)
            AppendChar('\x1d');
        else
            AppendChar(']');
    }
    else if (event.keyCode == KeyCodes::KC_BACKSLASH) {
        if (m_keyboardState.shift)
            AppendChar('|');
        else if (m_keyboardState.control)
            AppendChar('\x1c');
        else
            AppendChar('\\');
    }
    else if (event.keyCode == KeyCodes::KC_SEMICOLON) {
        if (m_keyboardState.shift)
            AppendChar(':');
        else
            AppendChar(';');
    }
    else if (event.keyCode == KeyCodes::KC_APOSTROPHE) {
        if (m_keyboardState.shift)
            AppendChar('"');
        else
            AppendChar('\'');
    }
    else if (event.keyCode == KeyCodes::KC_COMMA) {
        if (m_keyboardState.shift)
            AppendChar('<');
        else
            AppendChar(',');
    }
    else if (event.keyCode == KeyCodes::KC_PERIOD) {
        if (m_keyboardState.shift)
            AppendChar('>');
        else
            AppendChar('.');
    }
    else if (event.keyCode == KeyCodes::KC_SLASH) {
        if (m_keyboardState.shift)
            AppendChar('?');
        else
            AppendChar('/');
    }
    else if (event.keyCode == KeyCodes::KC_GRAVE) {
        if (m_keyboardState.shift)
            AppendChar('~');
        else
            AppendChar('`');
    }
    else if (event.keyCode == KeyCodes::KC_TAB)
        AppendChar('\t');
    else if (event.keyCode == KeyCodes::KC_ENTER)
        AppendChar('\n');
    else if (event.keyCode == KeyCodes::KC_BACKSPACE)
        AppendChar('\b');
    else if (event.keyCode == KeyCodes::KC_ESC)
        AppendChar('\x1b');
    else if (event.keyCode == KeyCodes::KC_DEL)
        AppendChar('\x7f');
    else if (event.keyCode == KeyCodes::KC_NUMPAD_ENTER)
        AppendChar('\n');
    else if (event.keyCode == KeyCodes::KC_NUMPAD_PLUS)
        AppendChar('+');
    else if (event.keyCode == KeyCodes::KC_NUMPAD_MINUS)
        AppendChar('-');
    else if (event.keyCode == KeyCodes::KC_NUMPAD_ASTERISK)
        AppendChar('*');
    else if (event.keyCode == KeyCodes::KC_NUMPAD_SLASH)
        AppendChar('/');
    else if (event.keyCode == KeyCodes::KC_NUMPAD_PERIOD) {
        if (m_keyboardState.num_lock)
            AppendChar('.');
        else
            AppendChar('\x7f');
    }
    else
        return false; // Unknown key
    return true;
}

void KeyboardInput::OnKey(void (*func)(void*, char), void* data) {
    m_keyCallback.func = func;
    m_keyCallback.data = data;
}

void KeyboardInput::AppendChar(char c) {
    m_buffer.Write(m_bufferOffset, (const uint8_t*)&c, sizeof(char));
    m_bufferSize++;
    if (m_keyCallback.func != nullptr)
        m_keyCallback.func(m_keyCallback.data, c);
}
