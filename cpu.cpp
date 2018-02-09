/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "cpu.h"




uint64_t CpuIndicator::getMissDelta() {
    return this->missCount - this->lastMissCount;
}

uint64_t CpuIndicator::getRefDelta() {
    return this->refCount - this->lastRefCount;
}

float_t CpuIndicator::getMissRatio() {
    if (this->getRefDelta() > 0.0f) {
        return (float_t) this->getMissDelta() / (float_t) this->getRefDelta();
    }
    return 0.0f;
}


void CpuIndicator::updateCount(uint64_t newRefs, uint64_t newMiss) {
    this->lastMissCount = this->missCount;
    this->missCount = newMiss;

    this->lastRefCount = this->refCount;
    this->refCount = newRefs;
}