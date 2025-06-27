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
#include "server.h"

#include <pen_http/pen_http.h>

static char *err_name[PEN_JUDGE_CODE_END] = {
    "null",
    "\"JudgeClientError\"",
    "\"JudgeClientError\"",
    "\"JudgeClientError\"",
};

static char *err_msg[PEN_JUDGE_CODE_END] = {
    "success",
    "Request wrong data.",
    "Compile failed.",
    "Permission error.",
};


bool
do_judge_result(pen_event_base_t *eb, ResultCode code)
{
#define BUF_SIZE 128
#define DATA "{\"err\":%s,\"data\":\"%s\"}"
    static char buf[BUF_SIZE];
    int len;

    len = snprintf(buf, BUF_SIZE, DATA, err_name[code], err_msg[code]);
    return pen_http_resp_string(eb, buf, len);
}

