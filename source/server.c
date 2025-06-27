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
#include "server.h"
#include <pen_http/pen_http_server.h>
#include <pen_utils/pen_log.h>
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
#define INFO_SIZE 128
#define BASE_INFO "{\"data\":\"pong\",\"judger_version\":\"" JUDGE_SERVER_VERSION "\",\"hostname\":\""
#define INFO_TEMPL "%s\",\"cpu\":%f,\"cpu_core\":%d,\"memory\":%f}"

    static char server_info[INFO_SIZE] = BASE_INFO;
    static const int base_len = sizeof(BASE_INFO) - 1;
    static const int rest_len = INFO_SIZE - base_len;
    static char * const str = server_info + base_len;
    int len;
    if (!_check_token())
        goto error;

    len = base_len + snprintf(str, rest_len, INFO_TEMPL,
        get_hostname(), get_cpu_percent(), get_ncpu(), get_mem_percent());

    return pen_http_resp_string(eb, server_info, len);
error:
    if (pen_http_resp_code(eb, 404) != 1)
        PEN_WARN("[http] send resp stoped");
    return false;
#undef INFO_SIZE
#undef BASE_INFO
#undef INFO_TEMPL
}

PEN_HTTP_POST(judge) {
    if (!_check_token())
        goto error;
    return do_judge(eb);
error:
    if (pen_http_resp_code(eb, 404) != 1)
        PEN_WARN("[http] send resp stoped");
    return false;
}

PEN_HTTP_POST(compile_spj) {
    if (!_check_token())
        goto error;
error:
    if (pen_http_resp_code(eb, 404) != 1)
        PEN_WARN("[http] send resp stoped");
    return false;
}

bool
pen_server_init(pen_event_t ev)
{
    if (PEN_UNLIKELY(!init_judger()))
        return false;
    if (PEN_UNLIKELY(!init_sys_info()))
        return false;
    g_self.server_ = pen_http_server_init(ev, g_local_host, g_local_port, g_unix_domain, 5);
    if (PEN_UNLIKELY(g_self.server_ == NULL))
        return false;

    PEN_HTTP_POST_REGISTER(g_self.server_, /ping, ping);
    PEN_HTTP_POST_REGISTER(g_self.server_, /judge, judge);
    PEN_HTTP_POST_REGISTER(g_self.server_, /compile_spj, compile_spj);
    return true;
}

void
pen_server_destroy(void)
{
    destroy_judger();
    destroy_sys_info();
    pen_http_server_destroy(g_self.server_);
}

