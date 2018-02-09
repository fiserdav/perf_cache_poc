#include "event_alarm.h"
#include <iostream>

using namespace std;

void eventAlarm(float_t value, struct event_ref * event) {
    if (value > ERR_TRESHOLD) {
        cout << "ERROR: HIGH PROBABILITY, CACHE-SIDE-CHANNEL-ATTACK" << endl;

    } else if (value > WARN_TRESHOLD) {
        cout << "WARNING possible cache side channel attack" << endl;
    } else if (value > INFO_TRESHOLD) {
        cout << "INFO: bigger cache miss ratio" << endl;
    } else {
        return;
    }
    cout << "pid: " << event->pid << "; tid: " << event->tid << "; cpu: " << event->cpu << endl;

}