/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cpu.h
 * Author: root
 *
 * Created on February 9, 2018, 12:03 PM
 */

#ifndef CPU_H
#define CPU_H
#include <stdint.h>
#include <math.h>

struct CpuIndicator {
    uint64_t refCount;
    uint64_t missCount;

    uint64_t lastRefCount;
    uint64_t lastMissCount;

    int ref_fd;
    int miss_fd;

    uint64_t ref_id;
    uint64_t miss_id;

    void * ringbuffer;

    void updateCount(uint64_t newRefs, uint64_t newMiss);

    uint64_t getMissDelta();
    uint64_t getRefDelta();

    float_t getMissRatio();
};

#endif /* CPU_H */

