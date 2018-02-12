/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */



#include "counters.h"
#include "event_alarm.h"
#include <iostream>

using namespace std;

static int perf_event_open(struct perf_event_attr *e, pid_t pid, int cpu, int group_fd, unsigned long flags) {
    return (int) syscall(__NR_perf_event_open, e, pid, cpu, group_fd, flags);
}


// returns perf_event_attr for References/loads
struct perf_event_attr getRefsAttr() {
    struct perf_event_attr ea;

    memset(&ea, 0, sizeof (struct perf_event_attr));
    ea.type = CACHE_TYPE;
    ea.size = sizeof (struct perf_event_attr);
    ea.disabled = true;
    ea.sample_type = PERF_SAMPLE_CPU | PERF_SAMPLE_STREAM_ID | PERF_SAMPLE_TID | PERF_SAMPLE_READ | PERF_SAMPLE_TIME | PERF_SAMPLE_IDENTIFIER;
    ea.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;
    ea.pinned = true;
    ea.sample_period = SAMPLING_PERIOD;
    ea.wakeup_events = 1;
    ea.config = CACHE_REFS_CONFIG;
    ea.exclude_kernel = 1;
    ea.exclude_idle = 1;
    
    
    return ea;
}

// returns perf_event_attr for misses
struct perf_event_attr getMissAttr() {
    struct perf_event_attr ea;

    memset(&ea, 0, sizeof (struct perf_event_attr));
    ea.type = CACHE_TYPE;
    ea.size = sizeof (struct perf_event_attr);
    ea.sample_type = PERF_SAMPLE_CPU | PERF_SAMPLE_STREAM_ID | PERF_SAMPLE_TID | PERF_SAMPLE_READ | PERF_SAMPLE_TIME | PERF_SAMPLE_IDENTIFIER;
    ea.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;
    //ea.disabled = true;
    ea.config = CACHE_MISS_CONFIG;
    ea.exclude_kernel = 1;
    ea.exclude_idle = 1;
    
    return ea;
}


bool initCpuCounters(CpuIndicator * cpuInd,int cpu) {
    cpuInd->ringbuffer = MAP_FAILED;
    struct perf_event_attr ea = getRefsAttr();
    struct perf_event_attr eb = getMissAttr();


    cpuInd->ref_fd = perf_event_open(&ea, -1, cpu, -1, PERF_FLAG_FD_CLOEXEC);
    if (cpuInd->ref_fd != -1) {
#ifdef DEBUG
        cout << "CPU " << cpu << " LLC_REFERENCES init OK" << endl;
#endif
        ioctl(cpuInd->ref_fd, PERF_EVENT_IOC_ID, &cpuInd->ref_id);

    } else {
        cout << "CPU " << cpu << " LLC_REFERENCES event init failed; reason: " << errno << endl;
        return false;
    }
    cpuInd->miss_fd = perf_event_open(&eb, -1, cpu, cpuInd->ref_fd, PERF_FLAG_FD_CLOEXEC);

    if (cpuInd->miss_fd != -1) {
        #ifdef DEBUG
        cout << "CPU " << cpu << " LLC_MISSES init OK" << endl;
#endif
        ioctl(cpuInd->miss_fd, PERF_EVENT_IOC_ID, &cpuInd->miss_id);
    } else {
        cout << "CPU " << cpu << " LLC_MISSES event init failed; reason: " << errno << endl;
        return false;
    }


    cpuInd->ringbuffer = mmap(NULL,
            (PAGE_COUNT + 1) * getpagesize(),
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            cpuInd->ref_fd, 0);

    if (cpuInd->ringbuffer == MAP_FAILED) {
        cout << "CPU " << cpu << " MMAP FAILED; reason = " << errno << endl;
        return false;
    } else {
#ifdef DEBUG
        cout << "CPU " << cpu << " MMAP OK; address = " << cpuInd->ringbuffer << endl;
#endif
    }

    return true;
}


void handle_event(struct perf_event_header * msg, CpuIndicator * cpu) {
    if (msg->type == PERF_RECORD_SAMPLE) {
        struct event_ref * sample = (struct event_ref*) msg;
#ifdef DEBUG
        if(getpid() == sample->pid) {
            cout << "ignoring pid: " << sample->pid << " cpu: " << sample->cpu << endl;
            return;
        }
#endif
        uint64_t *pMiss, *pRef;
        if (sample->counters.couter_values[0].id == cpu->ref_id) {
            pMiss = &sample->counters.couter_values[1].value;
            pRef = &sample->counters.couter_values[0].value;
        } else {
            pMiss = &sample->counters.couter_values[0].value;
            pRef = &sample->counters.couter_values[1].value;
        }
        cpu->updateCount(*pRef, *pMiss);
#ifdef DEBUG
        cout << "sample cpu: " << sample->cpu << endl;
        cout << "sample pid: " << sample->pid << endl;
        cout << "sample tid: " << sample->tid << endl;
        cout << "sample stream_id: " << sample->stream_id << endl;
        cout << "sample time: " << sample->time << endl;

        cout << "COUNTER VALUES:" << endl;
        cout << "\t" << "REFERENCES/LOADS: " << *pRef << endl;
        cout << "\t" << "MISSES: " << *pMiss << endl;
        cout << "\t" << "MISS RATIO: " << cpu->getMissRatio() << endl;

#endif
        if(cpu->lastRefCount > 0){
            eventAlarm(cpu->getMissRatio(), sample);
        }
    }
}