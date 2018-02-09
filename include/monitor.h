/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   monitor.h
 * Author: root
 *
 * Created on February 9, 2018, 12:18 PM
 */

#ifndef MONITOR_H
#define MONITOR_H
#include <poll.h>
#include <asm/barrier.h>
#include <linux/compiler.h>
#include "cpu.h"
#include "counters.h"
#include "consts.h"
#include <unistd.h>
#define PERF_SAMPLE_MAX_SIZE (1 << 16)

void checkEvents(CpuIndicator *cpuInd);

#endif /* MONITOR_H */

