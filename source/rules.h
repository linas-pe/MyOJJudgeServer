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
#pragma once

#include "server.h"

PEN_EXTERN_C_START

bool resource_limit(JudgeClient *jc);
bool c_cpp_seccomp_rules();
bool c_cpp_file_io_seccomp_rules();
bool check_result(int in_fd, int out_fd);

PEN_EXTERN_C_END

