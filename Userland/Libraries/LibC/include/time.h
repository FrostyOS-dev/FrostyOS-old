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

#ifndef _TIME_H
#define _TIME_H

#ifdef __cplusplus
extern "C" {
#endif


#define CLOCKS_PER_SEC 1000

#ifndef NULL
#define NULL ((void*)0)
#endif

typedef unsigned long int clock_t;
typedef unsigned long int size_t;
typedef unsigned int time_t;

struct tm {
    int tm_sec;   /* seconds after the minute [0-60] */
    int tm_min;   /* minutes after the hour [0-59] */
    int tm_hour;  /* hours since midnight [0-23] */
    int tm_mday;  /* day of the month [1-31] */
    int tm_mon;   /* months since January [0-11] */
    int tm_year;  /* years since 1900 */
    int tm_wday;  /* days since Sunday [0-6] */
    int tm_yday;  /* days since January 1 [0-365] */
    int tm_isdst; /* Daylight Savings Time flag */
};

clock_t clock();
double difftime(time_t time1, time_t time0);
time_t mktime(struct tm* timeptr);
time_t time(time_t* timer);

char* asctime(const struct tm* timeptr);
char* ctime(const time_t* timer);
struct tm* gmtime(const time_t* timer);
struct tm* localtime(const time_t* timer);
size_t strftime(char* s, size_t maxsize, const char* format, const struct tm* timeptr);


#ifdef __cplusplus
}
#endif

#endif /* _TIME_H */