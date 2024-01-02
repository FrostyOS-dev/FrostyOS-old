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

#include <signal.h>
#include <stdint.h>
#include <errno.h>

#include <kernel/process.h>

#include "util.h"

sighandler_t signal(int signum, sighandler_t handler) {
    struct signal_action old_action;
    struct signal_action new_action;
    int int_handler = (int)(uint64_t)handler;
    if (int_handler == SIG_DFL || int_handler == SIG_IGN)
        new_action.flags = int_handler;
    else
        new_action.handler = handler;
    int rc = onsignal(signum, &new_action, &old_action);
    if (rc < 0) {
        __SET_ERRNO(rc);
        return (sighandler_t)(uint64_t)SIG_ERR;
    }
    if (old_action.flags == SIG_DFL || old_action.flags == SIG_IGN)
        return (sighandler_t)(uint64_t)old_action.flags;
    return (sighandler_t)old_action.handler;
}

int raise(int sig) {
    return sendsig(getpid(), sig);
}