/*
 * Copyright (C) 2025  <name of copyright holder>
 * Author: Linas <linas@justforfun.cn
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "rules.h"

bool
check_result(int in_fd, int out_fd)
{
    const size_t buffer_size = 4 * 1024;
    unsigned char in_buf[buffer_size];
    unsigned char out_buf[buffer_size];
    struct stat in_st, out_st;
    ssize_t len;

    if (fstat(in_fd, &in_st) || fstat(out_fd, &out_st)) return false;
    if (in_st.st_size != out_st.st_size) return false;
    if (in_st.st_size == 0) return true;
    lseek(in_fd, 0, SEEK_SET);
    lseek(out_fd, 0, SEEK_SET);

    do {
        len = read(in_fd, in_buf, buffer_size);
        if (len <= 0) return false;
        len = read(out_fd, out_buf, buffer_size);
        if (len <= 0) return false;
        if (memcmp(in_buf, out_buf, len) != 0) return false;
    } while(len == buffer_size);

    return true;
}

