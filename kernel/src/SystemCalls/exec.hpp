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

#ifndef _EXEC_HPP
#define _EXEC_HPP

#include <Scheduling/Process.hpp>

// Userland wrapper around exec
int sys$exec(Scheduling::Process* parent, const char *path, char *const argv[], char *const envv[]);

// Execute a program. No memory checks are performed on any arguments, as they are assumed to be valid.
int Execute(Scheduling::Process* parent, const char *path, int argc, char *const argv[], int envc, char *const envv[]);

#endif /* _EXEC_HPP */