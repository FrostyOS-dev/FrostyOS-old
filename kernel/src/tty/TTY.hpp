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

#ifndef _KERNEL_TTY_HPP
#define _KERNEL_TTY_HPP

#include <Graphics/Colour.hpp>
#include <Graphics/VGA.hpp>

#include <Memory/PageManager.hpp>

class TTY {
public:
    TTY();
    TTY(BasicVGA* VGADevice, const Colour& fg_colour, const Colour& bg_colour); // Set the default foreground and background colours.
    ~TTY();

    void putc(char c);
    void puts(const char* str);

    void SetDefaultForeground(const Colour& colour);
    void SetDefaultBackground(const Colour& colour);

    BasicVGA* GetVGADevice();
    void SetVGADevice(BasicVGA* device);

private:
    BasicVGA* m_VGADevice;
    Colour m_foreground;
    Colour m_background;
};

extern TTY* g_CurrentTTY;

#endif /* _KERNEL_TTY_HPP */