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
#include <pen_utils/pen_log.h>
#include <pen_http/pen_http_server.h>
#include "sys_info.h"

#define JUDGE_SERVER_VERSION "2.1.1"

const char *g_unix_domain = NULL;
const char *g_local_host = NULL;
unsigned short g_local_port = 0;

static struct {
    pen_http_server_t server_;
} g_self;

static bool
_check_token() {
    return true;
}

PEN_HTTP_POST(ping) {

#define JUDGE_SERVER_INFO_SIZE 128
#define JUDGE_SERVER_INFO_TEMPL \
    "{\"hostname\":\"%s\",\"cpu\":%f,\"cpu_core\":%d,\"memory\":%f,\"judger_version\":\"%s\",\"data\":\"pong\"}"

    static char server_info[JUDGE_SERVER_INFO_SIZE];
    if (!_check_token())
        goto error;

    snprintf(server_info, JUDGE_SERVER_INFO_SIZE, JUDGE_SERVER_INFO_TEMPL,
        get_hostname(), get_cpu_percent(), get_ncpu(), get_mem_percent(), JUDGE_SERVER_VERSION);

    return true;
error:
    if (pen_http_resp_code(eb, 404) != 1)
        PEN_WARN("[http] send resp stoped");
    return false;
}

PEN_HTTP_POST(judge) {
    if (!_check_token())
        goto error;

    return true;
error:
    if (pen_http_resp_code(eb, 404) != 1)
        PEN_WARN("[http] send resp stoped");
    return false;
}

bool
pen_server_init(pen_event_t ev)
{
    if (PEN_UNLIKELY(!init_sys_info()))
        return false;
    g_self.server_ = pen_http_server_init(ev, g_local_host, g_local_port, g_unix_domain, 5);
    if (PEN_UNLIKELY(g_self.server_ == NULL))
        return false;

    PEN_HTTP_POST_REGISTER(g_self.server_, /ping, ping);
    PEN_HTTP_POST_REGISTER(g_self.server_, /judge, judge);
    return true;
}

void
pen_server_destroy(void)
{
    destroy_sys_info();
    pen_http_server_destroy(g_self.server_);
}

