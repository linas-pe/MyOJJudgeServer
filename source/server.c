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
#include <pen_http/pen_http_server.h>

const char *g_unix_domain = NULL;
const char *g_local_host = NULL;
unsigned short g_local_port = 0;

static struct {
    pen_http_server_t server_;
} g_self;

bool
pen_server_init(pen_event_t ev)
{
    g_self.server_ = pen_http_server_init(ev, g_local_host, g_local_port, g_unix_domain, 5);
    if (PEN_UNLIKELY(g_self.server_ == NULL))
        return false;

    // PEN_HTTP_PUT_REGISTER(g_self.server_, /api/file, upload);
    return true;
}

void
pen_server_destroy(void)
{
    pen_http_server_destroy(g_self.server_);
}

