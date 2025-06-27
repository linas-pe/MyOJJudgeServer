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
#include <dirent.h>
#include <pen_utils/pen_string.h>
#include <pen_json/pen_json.h>
#include <pen_http/pen_http.h>

typedef struct JudgeData {
    pen_string_t src;
    pen_string_t case_id;
} JudgeData;

const char *judge_dir = NULL;
const char *testcase_dir = NULL;
int root_fd = -1;

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

static inline bool
_write_src_file(JudgeData *data, const char *submission_id)
{
    // TODO src_data size
    static char src_data[8192];
    int fd, src_fd, len;
    bool ret = false;

    fd = openat(root_fd, submission_id, O_RDONLY | O_DIRECTORY);
    if (fd == -1)
        return false;
    // TODO set read file with code type
    src_fd = openat(fd, "main.c", O_RDWR | O_CREAT, 0600);
    if (src_fd == -1)
        goto end;
    len = pen_unescape_string(&data->src, src_data);
    if (write(src_fd, src_data, len) == (ssize_t)len)
        ret = true;
    close(src_fd);
end:
    close(fd);
    return ret;
}

static inline bool
_init_submission_env(JudgeData *data, const char *submission_id)
{
    if (mkdirat(root_fd, submission_id, 0700) != 0)
        return false;

    return _write_src_file(data, submission_id);
}

static inline void
_destroy_submission_env(const char *submission_id)
{
    DIR *dir;
    struct dirent *entry;

    int fd = openat(root_fd, submission_id, O_RDONLY | O_DIRECTORY);
    if (fd == -1)
        goto end;
    dir = fdopendir(fd);
    if (dir == NULL) {
        close(fd);
        goto end;
    }
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
            continue;
        unlinkat(fd, entry->d_name, 0);
    }
    closedir(dir);
end:
    if (unlinkat(root_fd, submission_id, AT_REMOVEDIR) != 0) {
        PEN_ERROR("clean submission env failed.");
    }
}

bool
do_judge(pen_event_base_t *eb)
{
    char submission_id[37] = {0x00};
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

    pen_get_uuid(submission_id);
    if (!_init_submission_env(&data, submission_id)) {
        code = PEN_JUDGE_PERMISSION_ERROR;
        goto end;
    }

    printf("submission_id: %s\nsrc: %.*s\ntest_case_id: %.*s\n", submission_id,
        (int)data.src.len_, data.src.str_, (int)data.case_id.len_, data.case_id.str_);

end:
    _destroy_submission_env(submission_id);
    pen_json_parser_destroy(&parser);
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
    return root_fd != -1;
}

void
destroy_judger()
{
    close(root_fd);
    root_fd = -1;
}

