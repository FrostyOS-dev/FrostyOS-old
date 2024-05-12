/*
Copyright (©) 2024  Frosty515

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

#ifndef _SYNCHRONISATION_HPP
#define _SYNCHRONISATION_HPP

int sys_createSemaphore(int value);
int sys_acquireSemaphore(int ID);
int sys_releaseSemaphore(int ID);
int sys_destroySemaphore(int ID);

int sys_createMutex();
int sys_acquireMutex(int ID);
int sys_releaseMutex(int ID);
int sys_destroyMutex(int ID);

#endif /* _SYNCHRONISATION_HPP */