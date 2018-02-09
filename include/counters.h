/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   counters.h
 * Author: root
 *
 * Created on February 9, 2018, 12:05 PM
 */

#ifndef COUNTERS_H
#define COUNTERS_H

#include "cpu.h"
#include <errno.h>
#include <sys/mman.h>
#include <iostream>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <string.h>
#include <unistd.h>
#include <asm/unistd.h>
#include "consts.h"
#include <sys/ioctl.h>


struct counter_value {
    uint64_t value;
    uint64_t id;
};

struct counter_group {
    uint64_t nr;
    //uint64_t time_enabled;
    //uint64_t time_running;
    struct counter_value couter_values[2];
};

struct event_ref {
    struct perf_event_header header;
    u64 sample_id; /* if PERF_SAMPLE_IDENTIFIER */
    u32 pid, tid; /* if PERF_SAMPLE_TID */
    u64 time; /* if PERF_SAMPLE_TIME */
    u64 stream_id; /* if PERF_SAMPLE_STREAM_ID */
    u32 cpu, res; /* if PERF_SAMPLE_CPU */
    struct counter_group counters; // guess
};

static int perf_event_open(struct perf_event_attr *e, pid_t pid, int cpu, int group_fd, unsigned long flags);
void handle_event(struct perf_event_header * msg, CpuIndicator * cpu);
bool initCpuCounters(CpuIndicator * cpuInd, int cpu);

#endif /* COUNTERS_H */

