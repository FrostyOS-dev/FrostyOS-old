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

#ifndef _SEMAPHORE_H
#define _SEMAPHORE_H

#ifdef __cplusplus
extern "C" {
#endif

int createSemaphore(int value);
int acquireSemaphore(int ID);
int releaseSemaphore(int ID);
int destroySemaphore(int ID);

#ifdef __cplusplus
}
#endif

#endif /* _SEMAPHORE_H */