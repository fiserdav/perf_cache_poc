/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   alarm.h
 * Author: root
 *
 * Created on February 9, 2018, 12:01 PM
 */

#ifndef ALARM_H
#define ALARM_H

#include "counters.h"
#include <stdint.h>


const float_t ALARM_TRESHOLD = 0.99f;

void eventAlarm(float_t value, struct event_ref * event);
#endif /* ALARM_H */

