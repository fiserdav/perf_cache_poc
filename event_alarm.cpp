#include "event_alarm.h"
#include <iostream>
#include <ctime>
using namespace std;

void eventAlarm(float_t value, struct event_ref * event) {
    time_t tm = time(NULL);
    if (value > ALARM_TRESHOLD) {
        cout << tm << "; " << "ERR: " << value << "\t" << event->cpu << "\t" << event->pid << "\t" << event->tid << "\t" 
                << event->counters.couter_values[0].value << "\t" << event->counters.couter_values[1].value << endl;

    } 
    //cout << "pid: " << event->pid << "; tid: " << event->tid << "; cpu: " << event->cpu << endl;

}
