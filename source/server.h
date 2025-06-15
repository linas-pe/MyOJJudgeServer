/*
 * Copyright (C) 2023  linas <linas@justforfin.cn>
 * Author: linas <linas@justforfin.cn>
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

#include <pen_utils/pen_types.h>

PEN_EXTERN_C_START

extern const char *g_unix_domain;
extern const char *g_local_host;
extern unsigned short g_local_port;

PEN_WARN_UNUSED_RESULT
PEN_NONNULL(1)
bool pen_server_init(pen_event_t ev);
void pen_server_destroy(void);

PEN_EXTERN_C_END

