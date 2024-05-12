/*
Copyright (©) 2022-2024  Frosty515

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

#ifndef _KERNEL_TTY_HPP
#define _KERNEL_TTY_HPP

#include <Graphics/Colour.hpp>
#include <Graphics/VGA.hpp>

#include <Memory/PageManager.hpp>

#include "KeyboardInput.hpp"

#include <spinlock.h>

class TTY {
public:
    TTY();
    TTY(BasicVGA* VGADevice, KeyboardInput* input, const Colour& fg_colour, const Colour& bg_colour); // Set the default foreground and background colours.
    ~TTY();

    int getc();
    void putc(char c);
    void puts(const char* str);

    void SetDefaultForeground(const Colour& colour);
    void SetDefaultBackground(const Colour& colour);

    BasicVGA* GetVGADevice();
    void SetVGADevice(BasicVGA* device);

    KeyboardInput* GetKeyboardInput();
    void SetKeyboardInput(KeyboardInput* input);

    void HandleKeyEvent(char c);

    void EnableInputMirroring();
    void DisableInputMirroring();

    bool isInputMirroringEnabled() const;

    void Lock() const;
    void Unlock() const;

private:
    BasicVGA* m_VGADevice;
    KeyboardInput* m_keyboardInput;
    Colour m_foreground;
    Colour m_background;
    bool m_inputMirroring;

    mutable spinlock_t m_lock;
    mutable bool m_locked;
};

extern TTY* g_CurrentTTY;

#endif /* _KERNEL_TTY_HPP */