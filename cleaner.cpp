/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "cleaner.h"


void doClean(CpuIndicator * cpuInd){
    if (cpuInd->ref_fd != -1) {
        close(cpuInd->ref_fd);

    }
    if (cpuInd->miss_fd != -1) {
        close(cpuInd->miss_fd);
    }
    if (cpuInd->ringbuffer != MAP_FAILED) {
        munmap(cpuInd->ringbuffer,
                (PAGE_COUNT + 1) * getpagesize());

    }
}