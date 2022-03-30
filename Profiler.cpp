#include "Profiler.h"

#include <Arduino.h>

#define DEBUGGING false
#define DEBUG if(DEBUGGING)Serial

void ESPProfiler::addTimestamp() {
    _timestamps.push_back({millis(), ""});
}

void ESPProfiler::addTimestamp(String message) {
    _timestamps.push_back({millis(), message});
}

void ESPProfiler::print() {
    DEBUG.println("====================");

    for (std::vector<log_t>::iterator it = _timestamps.begin(); it != _timestamps.end(); it++) {
        log_t log = *it;
        unsigned long timestamp = log.timestamp;
        DEBUG.print(timestamp);
        DEBUG.print("ms ");
    }
    DEBUG.println();
    DEBUG.println();


    size_t index = 1;
    for (std::vector<log_t>::iterator it2 = _timestamps.begin(), it1 = it2++; it2 != _timestamps.end(); it1 = it2++, index++) {
        log_t log1 = *it1;
        log_t log2 = *it2;

        unsigned long t1 = log1.timestamp;
        unsigned long t2 = log2.timestamp;
        unsigned long diff = t2 - t1;

        String message = log2.message;

        DEBUG.print(index);
        DEBUG.print(". ");
        DEBUG.print(message);
        DEBUG.print(": ");
        DEBUG.print(diff);
        DEBUG.println("ms");
    }
    DEBUG.println();


    DEBUG.println("====================");

    _timestamps.clear();
}

ESPProfiler Profiler;
