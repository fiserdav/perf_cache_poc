/*
 * PERFORMANCE COUNTERS
 * 
 * HW CACHE-MISS RATION DETECTION POC
 */

/* 
 * File:   main.cpp
 * Author: dev
 *
 * Created on February 6, 2018, 1:45 PM
 */
#include <cstdlib>
#include <iostream>
#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include "monitor.h"
#include <signal.h>
#include "fork.h"
#include "cleaner.h"
#include "consts.h"

using namespace std;

bool monitor;
// exit from monitor loop
void sigIntHandler(int d) {
    monitor = false;
    cout << "terminating app" << endl;
}

int main(int argc, char** argv) {  
    int cpu = doFork();
    
    monitor = true;
    signal(SIGINT, sigIntHandler);
    signal(SIGTERM, sigIntHandler);

    CpuIndicator cpuInd;

    bool initOk = initCpuCounters(&cpuInd, cpu);

    if (initOk) {
        ioctl(cpuInd.ref_fd, PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
        ioctl(cpuInd.ref_fd, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);

        while (monitor) {
            checkEvents(&cpuInd);
        }
    }
    doClean(&cpuInd);
#ifdef DEBUG
    cout << "main  app exit" << endl;
#endif
    return 0;
}

