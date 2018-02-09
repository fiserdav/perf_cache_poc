/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   clean.h
 * Author: root
 *
 * Created on February 9, 2018, 12:13 PM
 */

#ifndef CLEAN_H
#define CLEAN_H
#include "cpu.h"
#include "consts.h"
#include <sys/mman.h>
#include <unistd.h>
#include "consts.h"

void doClean(CpuIndicator *cpuInd);

#endif /* CLEAN_H */

