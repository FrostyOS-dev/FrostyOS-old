/*
Copyright (©) 2023-2024  Frosty515

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

#ifndef _SIGNAL_H
#define _SIGNAL_H

#ifdef __cplusplus
extern "C" {
#endif


#ifndef SIGABRT
#define SIGABRT 1
#endif

#ifndef SIGFPE
#define SIGFPE 2
#endif

#ifndef SIGILL
#define SIGILL 3
#endif

#ifndef SIGINT
#define SIGINT 4
#endif

#ifndef SIGSEGV
#define SIGSEGV 5
#endif

#ifndef SIGTERM
#define SIGTERM 6
#endif


#ifndef SIG_DFL
#define SIG_DFL 1
#endif

#ifndef SIG_IGN
#define SIG_IGN 2
#endif

#ifndef SIG_ERR
#define SIG_ERR -1
#endif

typedef int sig_atomic_t;

typedef void (*sighandler_t)(int);

sighandler_t signal(int signum, sighandler_t handler);
int raise(int sig);


#ifdef __cplusplus
}
#endif

#endif /* _SIGNAL_H */