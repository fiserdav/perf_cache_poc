/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   consts.h
 * Author: root
 *
 * Created on February 9, 2018, 12:35 PM
 */

#ifndef CONSTS_H
#define CONSTS_H
#include <stdint.h>
//#define DEBUG

const int PAGE_COUNT = 0x8;
const uint64_t SAMPLING_PERIOD = 10000;

// if LLC-loads and LLC-load-misses available
//#define HW_LLC 1
#ifdef HW_LLC
#define CACHE_TYPE PERF_TYPE_HW_CACHE
// LLC-loads
#define CACHE_REFS_CONFIG PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16)
// LLC-load-misses
#define CACHE_MISS_CONFIG PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16)
#define MODE "LLC-loads / LLC-load-misses mode"
#else
#define CACHE_TYPE PERF_TYPE_HARDWARE
#define CACHE_REFS_CONFIG PERF_COUNT_HW_CACHE_REFERENCES
#define CACHE_MISS_CONFIG  PERF_COUNT_HW_CACHE_MISSES

#define MODE "LLC-REFERENCE / LLC-MISSES mode"
#endif

#endif /* CONSTS_H */

