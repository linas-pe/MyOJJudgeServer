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
#define _GNU_SOURCE
#include "server.h"
#include <fcntl.h>
#include <dirent.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <signal.h>
#include <linux/resource.h>

#include "rules.h"

static inline bool
_write_src_file(JudgeClient *jc)
{
    extern int root_fd;
    // TODO src_data size
    static char src_data[8192];
    int fd, src_fd, len;
    bool ret = false;

    fd = openat(root_fd, jc->submission_id, O_RDONLY | O_DIRECTORY);
    if (fd == -1)
        return false;
    // TODO set read file with code type
    src_fd = openat(fd, "main.c", O_RDWR | O_CREAT, 0600);
    if (src_fd == -1)
        goto end;
    len = pen_unescape_string(&jc->src, src_data);
    if (write(src_fd, src_data, len) == (ssize_t)len)
        ret = true;
    close(src_fd);
end:
    close(fd);
    return ret;
}

static inline bool
_init_submission_env(JudgeClient *jc)
{
    extern int root_fd;
    if (mkdirat(root_fd, jc->submission_id, 0700) != 0) {
        PEN_ERROR("mkdirat failed.");
        return false;
    }

    return _write_src_file(jc);
}

static inline void
_destroy_submission_env(const char *submission_id)
{
    extern int root_fd;
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

static inline int
pidfd_open(pid_t pid)
{
    return syscall(SYS_pidfd_open, pid, 0);
}

static void
_on_compile_child_finish(pen_event_base_t *eb, uint16_t pe PEN_UNUSED)
{
    char buf[128];
    int status;
    int len;
    JudgeClient *jc = PEN_ENTRY(eb, JudgeClient, pid_eb);

    waitpid(jc->pid, &status, 0);
    if (WEXITSTATUS(status) != 0) {
        len = read(jc->out, buf, 128);
        buf[127] = '\0';
        if (len > 0) {
            PEN_ERROR("on compile error: %.*s", len, buf);
        } else {
            PEN_ERROR("on compile failed.");
        }
    }
    close(eb->fd_);
    close(jc->out);
    _destroy_submission_env(jc->submission_id);
    // TODO on compile finished
    // TODO read stdout
}

static bool
_run_once(JudgeClient *jc, int in_fd, int out_fd)
{
    struct rusage resource_usage;
    int status;
    pid_t pid = fork();
    if (pid == -1) return false;
    if (pid == 0) {
        dup2(in_fd, STDIN_FILENO); close(in_fd);
        dup2(out_fd, STDOUT_FILENO); close(out_fd);
        if (!resource_limit(jc))
            return false;
        if (!c_cpp_file_io_seccomp_rules()) {
            PEN_ERROR("set seccomp failed.");
            return false;
        }
        execl("./main", "./main", NULL);
        exit(-1);
    }
    wait4(pid, &status, 0, &resource_usage);
    if (WEXITSTATUS(status) != 0) return false;
    // TODO check resource
    return true;
}

static bool
_run(JudgeClient *jc)
{
    extern int root_case_fd;
    char input[5] = "1.in";
    char output[6] = "1.out";
    int in_fd, out_fd, result_fd;
    int case_dir_fd;
    int i;

    case_dir_fd = openat(root_case_fd, jc->case_id, O_RDONLY | O_DIRECTORY);
    if (case_dir_fd == -1)
        return false;

    for (i = 1; i < 10; i++) {
        in_fd = openat(case_dir_fd, input, O_RDONLY);
        if (in_fd == -1) break;
        out_fd = open(output, O_RDWR | O_CREAT, 0644);
        if (out_fd == -1) return false;
        if (!_run_once(jc, in_fd, out_fd)) return false;

        result_fd = openat(case_dir_fd, output, O_RDONLY);
        if (result_fd == -1 || !check_result(result_fd, out_fd)) {
            fprintf(stderr, "check result failed at %d case.", i);
            return false;
        }

        close(in_fd);
        close(out_fd);
        close(result_fd);
        input[0] ++;
        output[0] ++;
    }
    return i > 1;
}

static int
_do_compile_c(JudgeClient *jc)
{
    pid_t pid;
    int status;

    if (!_init_submission_env(jc))
        return -1;

    if (chdir(judge_dir) != 0)
        return -1;
    if (chdir(jc->submission_id) != 0)
        return -1;

    // Compile
    pid = fork();
    if (pid == -1)
        return -1;
    if (pid == 0) {
        execl("/usr/bin/gcc", "gcc", "main.c", "-O3", "-o", "main", NULL);
        exit(1);
    }
    waitpid(pid, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status))
        return -1;

    // Run
    return _run(jc) ? 0 : 1;
}

bool
compile(JudgeClient *jc)
{
    extern int null_fd;
    extern pen_event_t ev;
    extern const char *judge_dir;
    int pipefd[2];
    pid_t pid;
    int pid_fd;

    if (pipe2(pipefd, O_NONBLOCK)) {
        PEN_ERROR("compile pip open failed.");
        return false;
    }
    pid = fork();
    if (pid < 0) {
        PEN_ERROR("compile fork failed.");
        close(pipefd[1]);
        close(pipefd[0]);
        return false;
    }
    if (pid == 0) {
        close(pipefd[0]);
        dup2(null_fd, STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        exit(_do_compile_c(jc));
    }
    close(pipefd[1]);
    jc->pid = pid;
    jc->out = pipefd[0];
    pid_fd = pidfd_open(pid);
    if (pid_fd == -1) {
        PEN_ERROR("syscall SYS_pidfd_open failed.");
        goto error;
    }

    jc->pid_eb.fd_ = pid_fd;
    jc->pid_eb.on_event_ = _on_compile_child_finish;
    if (!pen_event_add_r(ev, &jc->pid_eb)) {
        close(pid_fd);
        goto error;
    }
    return true;
error:
    kill(pid, SIGKILL);
    close(pipefd[0]);
    return false;
}
