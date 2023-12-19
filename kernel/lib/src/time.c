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

#include "time.h"

#include <HAL/time.h>

#include <stdbool.h>

time_t time(time_t* timer) {
    time_t current_time = getTime();
    if (timer != NULL)
        *timer = current_time;
    return current_time;
}

bool is_leap_year(int year) {
    if ((year % 4) != 0)
        return false;
    else if ((year % 100) != 0)
        return true;
    else if ((year % 400) != 0)
        return false;
    return true;
}

int day_seek_table[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

int day_of_year(int year, int month, int day) {
    if (month < 1)
        month = 1;
    else if (month > 12)
        month = 12;
    int year_day = day_seek_table[month - 1] + day - 1;

    if (is_leap_year(year) && month >= 3)
        year_day++;
    return year_day;
}

int64_t years_to_days_since_epoch(int year) {
    bool sign; // false for +, true for -
    int begin_year;
    int end_year;
    if (year < 1970) {
        begin_year = year;
        end_year = 1970;
        sign = true;
    }
    else {
        begin_year = 1970;
        end_year = year;
        sign = false;
    }
    int64_t days = 0;
    for (int64_t i = begin_year; i < end_year; i++) {
        days += 365;
        if (is_leap_year(i))
            days += 1;
    }
    if (sign)
        days *= -1;
    return days;
}

int64_t days_since_epoch(int year, int month, int day) {
    return years_to_days_since_epoch(year) + day_of_year(year, month, day);
}
