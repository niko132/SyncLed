#ifndef PROFILER_H
#define PROFILER_H

#include <Arduino.h>
#include <vector>

struct log_t {
    unsigned long timestamp;
    String message;
};

class ESPProfiler {
    private:
        std::vector<log_t> _timestamps;

    public:
        void addTimestamp();
        void addTimestamp(String message);
        void print();
};

extern ESPProfiler Profiler;

#endif
