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
#include <pen_utils/pen_string.h>
#include <pen_json/pen_json.h>
#include <pen_http/pen_http.h>

typedef struct JudgeData {
    pen_string_t src;
    pen_string_t case_id;
} JudgeData;

const char *workspace_judge = NULL;

static bool
_on_json_data(const char *str, pen_json_pos_t key, pen_json_pos_t val, void *user)
{
    const char *tmp = str + key->start_;
    const size_t len = key->end_ - key->start_;
    JudgeData *d = user;

    switch (len) {
    case 3:
        // TODO remove unused data
        // ignore unused data
        if (0 != strncmp(tmp, "src", 3))
            return true;
        d->src.str_ = str + val->start_;
        d->src.len_ = val->end_ - val->start_;
        break;
    case 12:
        // TODO remove unused data
        // ignore unused data
        if (0 != strncmp(tmp, "test_case_id", 12))
            return true;
        d->case_id.str_ = str + val->start_;;
        d->case_id.len_ = val->end_ - val->start_;
        break;
    }

    return true;
}

bool
do_judge(pen_event_base_t *eb)
{
    pen_json_parser_t parser;
    pen_string_t str;
    JudgeData data;
    ResultCode code = PEN_JUDGE_SUCCESS;

    if (PEN_UNLIKELY(!pen_http_body(eb, &str)))
        return do_judge_result(eb, PEN_JUDGE_DATA_ERROR);

    parser = pen_json_parser_init(_on_json_data, &data);
    if (pen_json_loads(parser, str.str_, str.len_) != (int)str.len_) {
        code = PEN_JUDGE_DATA_ERROR;
        goto end;
    }
    if (data.src.len_ <= 0 || data.case_id.len_ <= 0) {
        code = PEN_JUDGE_COMPILE_ERROR;
        goto end;
    }

    printf("src: %.*s\ntest_case_id: %.*s\n",
        (int)data.src.len_, data.src.str_, (int)data.case_id.len_, data.case_id.str_);

end:
    pen_json_parser_destroy(&parser);
    return do_judge_result(eb, code);
}

bool
init_judger()
{
    if (workspace_judge == NULL)
        return false;
    return (access(workspace_judge, R_OK) == 0 &&
        access(workspace_judge, W_OK) == 0 &&
        access(workspace_judge, X_OK) == 0);
}

void
destroy_judger()
{
}

