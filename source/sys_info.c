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
#include <limits.h>
#include <fcntl.h>
#include <sys/sysinfo.h>
#include <pen_utils/pen_types.h>
#include "sys_info.h"

typedef struct {
    unsigned long user;
    unsigned long nice;
    unsigned long system;
    unsigned long idle;
    unsigned long iowait;
    unsigned long irq;
    unsigned long softirq;
    unsigned long steal;
} CPUData;

static char hostname[HOST_NAME_MAX] = "\0";
static long ncpu = 0;
static struct {
    int fd;
    CPUData last_cpu;
} cpu_info;

static bool
read_cpu_data(CPUData *data) {
    char buffer[256];
    ssize_t n = pread(cpu_info.fd, buffer, sizeof(buffer) - 1, 0);

    if (n <= 0) {
        PEN_ERROR("pread failed.");
        return false;
    }
    buffer[n] = '\0';

    sscanf(buffer, "cpu %lu %lu %lu %lu %lu %lu %lu %lu",
           &data->user, &data->nice, &data->system, &data->idle,
           &data->iowait, &data->irq, &data->softirq, &data->steal);
    return true;
}

static inline float
calculate_cpu_usage(const CPUData *prev, const CPUData *curr)
{
    unsigned long prev_total = prev->user + prev->nice + prev->system + prev->idle +
                              prev->iowait + prev->irq + prev->softirq + prev->steal;

    unsigned long curr_total = curr->user + curr->nice + curr->system + curr->idle +
                              curr->iowait + curr->irq + curr->softirq + curr->steal;

    unsigned long total_diff = curr_total - prev_total;

    if (total_diff == 0) return 0.0;

    unsigned long idle_diff = (curr->idle + curr->iowait) - (prev->idle + prev->iowait);

    return 1.0 - (float)idle_diff / total_diff;
}

float
get_cpu_percent() {
    CPUData data;
    float usage;

    if (!read_cpu_data(&data))
        return 0.;

    usage = calculate_cpu_usage(&cpu_info.last_cpu, &data);
    cpu_info.last_cpu = data;

    return usage;
}

float
get_mem_percent()
{
    unsigned long used_bytes;
    struct sysinfo info;

    if (sysinfo(&info)) {
        PEN_ERROR("sysinfo failed.");
        return 0.;
    }
    used_bytes = info.totalram - info.freeram - info.bufferram;
    return 1.0 * used_bytes / info.totalram;
}

const char *
get_hostname()
{
    return hostname;
}

int
get_ncpu()
{
    return (int)ncpu;
}

bool
init_sys_info()
{
    int fd;
    cpu_info.fd = -1;
    if (gethostname(hostname, sizeof(hostname) - 1) != 0) {
        PEN_ERROR("gethostname failed.");
        return false;
    }
    ncpu = sysconf(_SC_NPROCESSORS_ONLN);
    if (ncpu == -1) {
        PEN_ERROR("sysconf failed.");
        return false;
    }
    fd = open("/proc/stat", O_RDONLY);
    if (fd == -1) {
        PEN_ERROR("open /proc/stat failed.");
        return false;
    }
    cpu_info.fd = fd;
    read_cpu_data(&cpu_info.last_cpu);
    return true;
}

void
destroy_sys_info()
{
    if (cpu_info.fd != -1) {
        close(cpu_info.fd);
        cpu_info.fd = -1;
    }
}

