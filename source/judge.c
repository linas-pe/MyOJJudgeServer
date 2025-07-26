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
#include <fcntl.h>
#include <pen_utils/pen_string.h>
#include <pen_json/pen_json.h>
#include <pen_http/pen_http.h>
#include <pen_utils/pen_memory_pool.h>


const char *judge_dir = NULL;
const char *testcase_dir = NULL;
int root_fd = -1;
int root_case_fd = -1;
int null_fd = -1;
static pen_memory_pool_t pool = NULL;

static bool
_on_json_data(const char *str, pen_json_pos_t key, pen_json_pos_t val, void *user)
{
    const char *tmp = str + key->start_;
    size_t len = key->end_ - key->start_;
    JudgeClient *jc = user;
    // TODO remove unused data
    // ignore unused data

    switch (len) {
    case 3:
        if (0 != strncmp(tmp, "src", 3))
            return true;
        jc->src.str_ = str + val->start_;
        jc->src.len_ = val->end_ - val->start_;
        break;
    case 10:
        if (0 == strncmp(tmp, "max_memory", 10))
            jc->max_memory = PEN_JSON_INT_VAL(val);
        break;
    case 12:
        if (0 == strncmp(tmp, "test_case_id", 12)) {
            tmp = str + val->start_;
            len = val->end_ - val->start_;
            if (len >32 || len == 0) {
                PEN_ERROR("case_id too long!!!");
                return false;
            }
            memcpy(jc->case_id, tmp, len);
            jc->case_id[len] = '\0';
        } else if (0 == strncmp(tmp, "max_cpu_time", 12)) {
            jc->max_cpu_time = PEN_JSON_INT_VAL(val);
        }
        break;
    }

    return true;
}

bool
do_judge(pen_event_base_t *eb)
{
    pen_json_parser_t parser;
    pen_string_t str;
    ResultCode code = PEN_JUDGE_SUCCESS;

    JudgeClient *jc = pen_memory_pool_get(pool);
    if (jc == NULL)
        return do_judge_result(eb, PEN_JUDGE_INTERNAL_ERROR);
    pen_get_uuid(jc->submission_id);

    if (PEN_UNLIKELY(!pen_http_body(eb, &str)))
        return do_judge_result(eb, PEN_JUDGE_DATA_ERROR);

    parser = pen_json_parser_init(_on_json_data, jc);
    if (pen_json_loads(parser, str.str_, str.len_) != (int)str.len_) {
        pen_json_parser_destroy(&parser);
        code = PEN_JUDGE_DATA_ERROR;
        goto end;
    }
    pen_json_parser_destroy(&parser);
    if (jc->src.len_ <= 0) {
        code = PEN_JUDGE_COMPILE_ERROR;
        goto end;
    }

    if (compile(jc)) return do_judge_result(eb, code);
    code = PEN_JUDGE_COMPILE_ERROR;
end:
    pen_memory_pool_put(pool, jc);
    return do_judge_result(eb, code);
}

bool
init_judger()
{
    if (testcase_dir == NULL || judge_dir == NULL || root_fd != -1)
        return false;
    if (!(access(judge_dir, R_OK) == 0 &&
        access(judge_dir, W_OK) == 0 &&
        access(judge_dir, X_OK) == 0))
        return false;
    if (!(access(testcase_dir, R_OK) == 0 &&
        access(testcase_dir, X_OK) == 0))
        return false;
    root_fd = open(judge_dir, O_RDONLY | O_DIRECTORY);
    if (root_fd == -1)
        return false;
    null_fd = open("/dev/null", O_WRONLY);
    if (null_fd == -1)
        goto error;
    root_case_fd = open(testcase_dir, O_RDONLY | O_DIRECTORY);
    if (root_case_fd == -1)
        goto error1;
    pool = PEN_MEMORY_POOL_INIT(1, JudgeClient);
    if (pool != NULL)
        return true;
error1:
    close(null_fd);
error:
    close(root_fd);
    return false;;
}

void
destroy_judger()
{
    close(root_fd);
    close(null_fd);
    root_fd = -1;
    null_fd = -1;
    pen_memory_pool_destroy(pool);
    pool = NULL;
}

