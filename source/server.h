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

#include <pen_socket/pen_event.h>
#include <pen_utils/pen_types.h>

PEN_EXTERN_C_START

typedef enum ResultCode {
    PEN_JUDGE_SUCCESS,
    PEN_JUDGE_DATA_ERROR,
    PEN_JUDGE_COMPILE_ERROR,
    PEN_JUDGE_PERMISSION_ERROR,
    PEN_JUDGE_CODE_END,
} ResultCode;

extern const char *g_unix_domain;
extern const char *g_local_host;
extern unsigned short g_local_port;
extern const char *judge_dir;
extern const char *testcase_dir;

PEN_WARN_UNUSED_RESULT
PEN_NONNULL(1)
bool pen_server_init(pen_event_t ev);
void pen_server_destroy(void);

PEN_WARN_UNUSED_RESULT
bool init_judger();
void destroy_judger();

PEN_NONNULL(1)
bool do_judge(pen_event_base_t *eb);

PEN_NONNULL(1)
PEN_WARN_UNUSED_RESULT
bool do_judge_result(pen_event_base_t *eb, ResultCode code);

PEN_EXTERN_C_END

