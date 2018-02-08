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
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <iostream>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <asm/barrier.h>
#include <linux/compiler.h>
#include <asm/unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sched.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>


using namespace std;
typedef float float_t;

//#define DEBUG
// if LLC-loads and LLC-load-misses available
//#define HW_LLC 1

#ifdef HW_LLC
#define CACHE_TYPE PERF_TYPE_HW_CACHE
// LLC-loads
#define CACHE_REFS_CONFIG PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16)
// LLC-load-misses
#define CACHE_MISS_CONFIG PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16)
#else
#define CACHE_TYPE PERF_TYPE_HARDWARE
#define CACHE_REFS_CONFIG PERF_COUNT_HW_CACHE_REFERENCES
#define CACHE_MISS_CONFIG  PERF_COUNT_HW_CACHE_MISSES
#endif

#define PERF_SAMPLE_MAX_SIZE (1 << 16)

static int perf_event_open(struct perf_event_attr *e, pid_t pid, int cpu, int group_fd,unsigned long flags) {
    return (int)syscall(__NR_perf_event_open,e,pid,cpu,group_fd,flags);;
}

//
struct perf_event_attr getRefsAttr() {
    struct perf_event_attr ea;
    
    memset(&ea, 0, sizeof(struct perf_event_attr));
    ea.type = CACHE_TYPE;
    ea.size = sizeof(struct perf_event_attr);
    ea.disabled = true;
    ea.sample_type = PERF_SAMPLE_CPU | PERF_SAMPLE_STREAM_ID | PERF_SAMPLE_TID | PERF_SAMPLE_READ | PERF_SAMPLE_TIME | PERF_SAMPLE_IDENTIFIER;
    ea.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;
    ea.pinned = true;
    ea.sample_period = 10000;
    ea.wakeup_events = 1;
    ea.config = CACHE_REFS_CONFIG;
    
    return ea;
}

//
struct perf_event_attr getMissAttr() {
    struct perf_event_attr ea;
    
    memset(&ea, 0, sizeof(struct perf_event_attr));
    ea.type = CACHE_TYPE;
    ea.size = sizeof(struct perf_event_attr);
    ea.sample_type  = PERF_SAMPLE_CPU | PERF_SAMPLE_STREAM_ID | PERF_SAMPLE_TID | PERF_SAMPLE_READ | PERF_SAMPLE_TIME | PERF_SAMPLE_IDENTIFIER;
    ea.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;
    //ea.disabled = true;
    ea.config = CACHE_MISS_CONFIG;
    
    return ea;
}
/*
 * 
 */
const float_t INFO_TRESHOLD = 0.97f;
const float_t WARN_TRESHOLD = 0.98f;
const float_t ERR_TRESHOLD = 0.99f;


struct CpuIndicator {    
    uint64_t refCount;
    uint64_t missCount;
    
    uint64_t lastRefCount;
    uint64_t lastMissCount;
    
    int ref_fd;
    int miss_fd;
    
    uint64_t ref_id;
    uint64_t miss_id;
    
    //struct perf_event_attr ref_ea;
    //struct perf_event_attr miss_ea;
    
    void * ringbuffer = MAP_FAILED;
    
    void updateCount(uint64_t newRefs, uint64_t newMiss);
    
    uint64_t getMissDelta();
    uint64_t getRefDelta();
    
    float_t getMissRatio();
};

uint64_t CpuIndicator::getMissDelta(){
    return missCount - lastMissCount;
}
uint64_t CpuIndicator::getRefDelta(){
    return refCount - lastRefCount;
}

float_t CpuIndicator::getMissRatio(){
    if(getRefDelta() > 0.0f) {
        return (float_t)getMissDelta() / (float_t)getRefDelta();
    }
    return 0.0f;
}

void CpuIndicator::updateCount(uint64_t newRefs, uint64_t newMiss){
    lastMissCount = missCount;
    missCount = newMiss;
    
    lastRefCount = refCount;
    refCount = newRefs;
}

struct perf_event_attr ea;
struct perf_event_attr eb;

int PAGE_SIZE = getpagesize();
int PAGE_COUNT = 0x8;

bool initCpuCounters(CpuIndicator * cpuIndArray, size_t const cpuCount) {
    ea = getRefsAttr();
    eb = getMissAttr();
    
    for(int i = 0; i < cpuCount;i++) {
        cpuIndArray[i].ref_fd = perf_event_open(&ea,-1,i,-1,PERF_FLAG_FD_CLOEXEC);
        if(cpuIndArray[i].ref_fd != -1) {
            cout << "CPU " << i << " LLC_REFERENCES init OK" << endl;
            ioctl(cpuIndArray[i].ref_fd, PERF_EVENT_IOC_ID, &cpuIndArray[i].ref_id);
            
        }else {
            cout << "CPU " << i << " LLC_REFERENCES event init failed; reason: " << errno << endl;
            return false;
        }
        cpuIndArray[i].miss_fd = perf_event_open(&eb,-1,i,cpuIndArray->ref_fd,PERF_FLAG_FD_CLOEXEC);
        
        if(cpuIndArray[i].miss_fd != -1) {
            cout << "CPU " << i << " LLC_MISSES init OK" << endl;
            ioctl(cpuIndArray[i].miss_fd, PERF_EVENT_IOC_ID, &cpuIndArray->miss_id);
        }else {
            cout << "CPU " << i << " LLC_MISSES event init failed; reason: " << errno << endl;
            return false;
        }
      
        
        cpuIndArray[i].ringbuffer = mmap(NULL,
                                    (PAGE_COUNT+1) * PAGE_SIZE,
                                    PROT_READ | PROT_WRITE, 
                                    MAP_SHARED,
                                    cpuIndArray[i].ref_fd, 0);
        
        if(cpuIndArray[i].ringbuffer == MAP_FAILED) {
            cout << "CPU " << i << " MMAP FAILED; reason = " << errno << endl; 
            return false;
        }else {
           cout << "CPU " << i << " MMAP OK; address = " << cpuIndArray[i].ringbuffer << endl;            
        }
    }
    return true;
}

void closeHandles(CpuIndicator * cpuIndArray, size_t const cpuCount) {
    for(int i = 0; i < cpuCount;i++) {
        if(cpuIndArray[i].ref_fd != -1) {
            close(cpuIndArray[i].ref_fd);
        
        }
        if(cpuIndArray[i].miss_fd != -1) {
            close(cpuIndArray[i].miss_fd);
        }
        if(cpuIndArray[i].ringbuffer != MAP_FAILED) {
            munmap(cpuIndArray,
                    (PAGE_COUNT+1) * PAGE_SIZE);
            
        }
    }
    
    
}
struct counter_value{
     uint64_t id;
     uint64_t value;
};

struct counter_group {
    uint64_t time_enabled;
    uint64_t time_running;
    struct counter_value couter_values[2];
};


 struct event_ref{
    struct perf_event_header header;
    u64    sample_id;   /* if PERF_SAMPLE_IDENTIFIER */
    u32    pid, tid;    /* if PERF_SAMPLE_TID */
    u64    time;        /* if PERF_SAMPLE_TIME */
    u64    stream_id;   /* if PERF_SAMPLE_STREAM_ID */
    u32    cpu, res;    /* if PERF_SAMPLE_CPU */
    struct counter_group counters; // guess
};
bool monitor;

void sigIntHandler(int d){
    monitor = false;
    cout << "terminating app" << endl;
}

void alarm(float_t value, struct event_ref * event){
    if( value > ERR_TRESHOLD) {
        cout << "ERROR: HIGH PROBABILITY, CACHE-SIDE-CHANNEL-ATTACK" << endl;
        
    }else if(value > WARN_TRESHOLD) {
        cout << "WARNING possible cache side channel attack" <<endl;
    }else if(value > INFO_TRESHOLD){
        cout << "INFO: bigger cache miss ratio" <<endl;
    }else {
        return;
    }
    cout << "pid: " << event->pid << "; tid: " << event->tid << "; cpu: " << event->cpu  << endl;
    
}

void handle_event(struct perf_event_header * msg, CpuIndicator * cpu) {
    if(msg->type == PERF_RECORD_SAMPLE) {
        struct event_ref * sample = (struct event_ref*)msg;
        
        uint64_t *pMiss,*pRef;
        if(sample->counters.couter_values[0].id == cpu->ref_id){
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
        alarm(cpu->getMissRatio(), sample);
    }
}


int main(int argc, char** argv) {
    //int cpuCount = sysconf(_SC_NPROCESSORS_ONLN);
    int cpuCount = 1;
    monitor = true;
    signal(SIGINT,sigIntHandler);
    signal(SIGTERM,sigIntHandler);
    
    cout << "Found " << cpuCount << " cpu(s)" << endl;
    
    CpuIndicator * cpuIndArray = new CpuIndicator[cpuCount];

    bool initOk = initCpuCounters(cpuIndArray,cpuCount);
 
    //          
 
    if(initOk) {
        for(int i = 0; i < cpuCount;i++) { // start counters
            ioctl(cpuIndArray[i].ref_fd, PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
            ioctl(cpuIndArray[i].ref_fd, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
        }     
        
        while(monitor){
            
      
       for(int i = 0; i < cpuCount;i++) {
             struct pollfd pfd;
             pfd.fd = cpuIndArray[i].ref_fd;
             pfd.events = POLLIN;
             
             poll(&pfd,1,-1);
             if(pfd.revents & POLLIN != 0) {
                struct perf_event_mmap_page * header = ((perf_event_mmap_page *)cpuIndArray[i].ringbuffer);
                
                uint64_t data_tail = header->data_tail;
                
                uint64_t data_head = READ_ONCE(header->data_head);
                rmb();
                
                
                uint8_t * dataPointer = (uint8_t*)(((uint8_t*)header) + PAGE_SIZE);
                uint8_t * begin = dataPointer + data_tail % (PAGE_COUNT * PAGE_SIZE);
                uint8_t * end = dataPointer + data_head % (PAGE_COUNT * PAGE_SIZE);

                int diff = end - begin;
                struct perf_event_header * event_header = (perf_event_header *)dataPointer;
                if(diff >= sizeof(struct perf_event_header)){
                    size_t size = event_header->size;
                    if( size < sizeof(struct perf_event_header) || diff < (int)size) {
                        // broken event
                        goto broken;
                    }
                    uint8_t localBuffer[PERF_SAMPLE_MAX_SIZE];
                    memcpy(localBuffer,begin,size);
                    struct perf_event_header * cpyHeader = (perf_event_header*)localBuffer;
                    handle_event(cpyHeader, &cpuIndArray[i]);
                }
		broken:
                // notify kernel
                  data_tail = data_head;
                  mb();
                  header->data_tail = data_tail;
                  data_head = header->data_head;       
             }
        }
        
       
    }
    }
    closeHandles(cpuIndArray, cpuCount);
    
    delete [] cpuIndArray;
    
    cout << "main  app exit" << endl;
    
    return 0;
}

