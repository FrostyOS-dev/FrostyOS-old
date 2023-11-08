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

#ifndef _TTY_KEYBOARD_INPUT_HPP
#define _TTY_KEYBOARD_INPUT_HPP

#include <HAL/drivers/Keyboard.hpp>

#include <Data-structures/Buffer.hpp>

bool KeyboardEventHandler(void* data, KeyboardEvent event);

class KeyboardInput {
public:
    KeyboardInput();
    ~KeyboardInput();

    void Initialise(Keyboard* keyboard);
    void Destroy();

    // Get first character in buffer. returns int to allow for better ISO C compatibility
    int GetChar();
    bool HandleEvent(KeyboardEvent event);

    // Gets called when a character is appended to the buffer
    void OnKey(void (*func)(void*, char), void* data);

private:
    void AppendChar(char c);

private:
    uint64_t m_bufferOffset;
    Keyboard* m_keyboard;
    struct KeyboardState {
        bool control;
        bool shift;
        bool alt;
        bool caps_lock;
        bool num_lock;
    } m_keyboardState;
    size_t m_bufferSize;
    Buffer m_buffer;
    struct {
        void (*func)(void*, char);
        void* data;
    } m_keyCallback;
};

#endif /* _TTY_KEYBOARD_INPUT_HPP */