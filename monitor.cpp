

#include "monitor.h"


void checkEvents(CpuIndicator *cpuInd) {
    struct pollfd pfd;
    pfd.fd = cpuInd->ref_fd;
    pfd.events = POLLIN;
    
    int PAGE_SIZE = getpagesize();
    poll(&pfd, 1, -1);
    
    if (pfd.revents & POLLIN != 0) {
        struct perf_event_mmap_page * header = ((perf_event_mmap_page *) cpuInd->ringbuffer);

        uint64_t data_tail = header->data_tail;
        uint64_t data_head = READ_ONCE(header->data_head);
        rmb();

        uint8_t * dataPointer = (uint8_t*) (((uint8_t*) header) + PAGE_SIZE);
        uint8_t * begin = dataPointer + data_tail % (PAGE_COUNT * PAGE_SIZE);
        uint8_t * end = dataPointer + data_head % (PAGE_COUNT * PAGE_SIZE);

        int diff = end - begin;
        struct perf_event_header * event_header = (perf_event_header *) dataPointer;
        if (diff >= sizeof (struct perf_event_header)) {
            size_t size = event_header->size;
            if (size < sizeof (struct perf_event_header) || diff < (int) size) {
                // broken event
                goto broken;
            }
            uint8_t localBuffer[PERF_SAMPLE_MAX_SIZE];
            memcpy(localBuffer, begin, size);
            struct perf_event_header * cpyHeader = (perf_event_header*) localBuffer;
            handle_event(cpyHeader, cpuInd);
        }
broken:
        // notify kernel
        data_tail = data_head;
        mb();
        header->data_tail = data_tail;
        data_head = header->data_head;
    }
}