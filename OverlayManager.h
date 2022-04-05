#ifndef OVERLAY_MANAGER_H
#define OVERLAY_MANAGER_H

#include <vector>
#include <ArduinoJson.h>

#include <WiFiUdp.h>
#include <NTPClient.h>

class SegmentTransition {
    protected:
        size_t _numSegments;
        size_t _ledsPerSegment;

        unsigned long _durationMillis;

    public:
        SegmentTransition(size_t numSegments, size_t ledsPerSegment);
        virtual void transition(double *brightness, uint8_t *targetSegments, unsigned long deltaMillis);
};

class NoTransition : public SegmentTransition {
    public:
        NoTransition(size_t numSegments, size_t ledsPerSegment);
        void transition(double *brightness, uint8_t *targetSegments, unsigned long deltaMillis);
};

class FadeTransition : public SegmentTransition {
    public:
        FadeTransition(size_t numSegments, size_t ledsPerSegment);
        void transition(double *brightness, uint8_t *targetSegments, unsigned long deltaMillis);
};

class FlowTransition : public SegmentTransition {
    public:
        FlowTransition(size_t numSegments, size_t ledsPerSegment);
        void transition(double *brightness, uint8_t *targetSegments, unsigned long deltaMillis);

        void flowDim(double *brightness, int seg, int startIndex, int endIndex, double fac);
        int getDistToNextOn(int seg, uint8_t *targetSegments, uint8_t left, uint8_t on);
};

class OverlayElement {
    protected:
        size_t _startIndex;

    public:
        OverlayElement(size_t startIndex);
        virtual void update(uint8_t *colors) = 0;
};

class SevenSegmentElement : public OverlayElement {
    public:
        static const size_t NUM_SEGMENTS = 7;
        static const size_t LEDS_PER_SEGMENT = 8;
        static const size_t LENGTH = NUM_SEGMENTS * LEDS_PER_SEGMENT;

    private:
        int _number;
        double _brightness[LENGTH];
        unsigned long _lastUpdateMillis;

        SegmentTransition *_transition;

        const uint8_t SEGMENTS[10][7] = {
            {1,1,1,0,1,1,1},
            {1,0,0,0,1,0,0},
            {1,1,0,1,0,1,1},
            {1,1,0,1,1,1,0},
            {1,0,1,1,1,0,0},
            {0,1,1,1,1,1,0},
            {0,1,1,1,1,1,1},
            {1,1,0,0,1,0,0},
            {1,1,1,1,1,1,1},
            {1,1,1,1,1,1,0}
        };

    public:
        SevenSegmentElement(size_t startIndex);
        void update(uint8_t *colors);
        void setNumber(int number);
};


class DotElement : public OverlayElement {
    private:
        size_t _counter;

    public:
        static const size_t LENGTH = 8;

        DotElement(size_t startIndex);
        void update(uint8_t *colors);
};


class DataSource {
    public:
        DataSource();
        virtual void update() = 0;
        virtual int getDigit(size_t index);
};

class TimeDataSource : public DataSource {
    private:
        WiFiUDP _ntpUdp;
        // Define NTP Client to get time
        NTPClient _timeClient;

    public:
        TimeDataSource();
        void update();
        int getDigit(size_t index);
};

class CountdownDataSource : public DataSource {
    private:
        unsigned long _startMillis;
        unsigned long _valueMillis;
        unsigned long _elapsedMillis;

    public:
        CountdownDataSource();
        void update();
        int getDigit(size_t index);
};

class CountupDataSource : public DataSource {
    private:
        unsigned long _startMillis;
        unsigned long _elapsedMillis;

    public:
        CountupDataSource();
        void update();
        int getDigit(size_t index);
};


class ESPOverlayManager {
    private:
        std::vector<SevenSegmentElement> _digits;
        DotElement _dot;

        DataSource *_dataSource;

    public:
        ESPOverlayManager();
        void begin();
        void update(uint8_t *colors, size_t numLeds);

        void fromJson(JsonObject &root);
        void toJson(JsonObject &root);
};

extern ESPOverlayManager OverlayManager;

#endif
