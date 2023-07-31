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

#ifndef _STDDEF_H
#define _STDDEF_H

typedef unsigned long size_t;

#ifdef __cplusplus
    typedef decltype(nullptr) nullptr_t;
    #define NULL nullptr
#else
    #define NULL ((void*)0)
#endif /* __cplusplus */

typedef long ptrdiff_t;

typedef struct {
  long long __max_align_nonce1
      __attribute__((__aligned__(__alignof__(long long))));
  long double __max_align_nonce2
      __attribute__((__aligned__(__alignof__(long double))));
} max_align_t;

#define offsetof(t, d) __builtin_offsetof(t, d)

#endif /* _STDDEF_H */