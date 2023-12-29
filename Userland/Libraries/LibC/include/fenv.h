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

#ifndef _FENV_H
#define _FENV_H

#ifdef __cplusplus
extern "C" {
#endif


typedef unsigned long fenv_t;
typedef unsigned long fexcept_t;

#define FE_INVALID   0x01
#define FE_DIVBYZERO 0x04
#define FE_OVERFLOW  0x08
#define FE_UNDERFLOW 0x10
#define FE_INEXACT   0x20
#define FE_ALL_EXCEPT (FE_DIVBYZERO | FE_INEXACT | FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW)

#define FE_TONEAREST 0x0000
#define FE_DOWNWARD  0x0400
#define FE_UPWARD    0x0800

#define FE_DFL_ENV ((const fenv_t*) -1)

int feclearexcept(int excepts);
int feraiseexcept(int excepts);
int fegetexceptflag(fexcept_t* flagp, int excepts);
int fegsetexceptflag(const fexcept_t* flagp, int excepts);

int fesetround(int round);
int fegetround(void);

int fegetenv(fenv_t* envp);
int fesetenv(const fenv_t* envp);
int feholdexcept(fenv_t* envp);
int feupdateenv(const fenv_t* envp);

int fetestexcept(int excepts);


#ifdef __cplusplus
}
#endif

#endif /* _FENV_H */