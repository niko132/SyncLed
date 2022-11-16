#ifndef OVERLAY_MANAGER_H
#define OVERLAY_MANAGER_H

#include <vector>
#include <ArduinoJson.h>

#define OVERLAY_NONE 1
#define OVERLAY_CLOCK 2
#define OVERLAY_COUNTDOWN 3
#define OVERLAY_COUNTUP 4

#define TRANSITION_NONE 1
#define TRANSITION_FADE 2
#define TRANSITION_FLOW 3

class SegmentTransition {
    private:
        unsigned long _id;

    protected:
        size_t _numSegments;
        size_t _ledsPerSegment;

        unsigned long _durationMillis;

    public:
        SegmentTransition(unsigned long id);
        unsigned long getId();
        virtual void transition(double *brightness, uint8_t *targetSegments, unsigned long deltaMillis);

        virtual void fromJson(JsonObject &root);
        virtual void toJson(JsonObject &root);
};

class NoTransition : public SegmentTransition {
    public:
        NoTransition();
        void transition(double *brightness, uint8_t *targetSegments, unsigned long deltaMillis);
};

class FadeTransition : public SegmentTransition {
    public:
        FadeTransition();
        void transition(double *brightness, uint8_t *targetSegments, unsigned long deltaMillis);
};

class FlowTransition : public SegmentTransition {
    public:
        FlowTransition();
        void transition(double *brightness, uint8_t *targetSegments, unsigned long deltaMillis);

        void flowDim(double *brightness, int seg, int startIndex, int endIndex, double fac);
        int getDistToNextOn(int seg, uint8_t *targetSegments, uint8_t left, uint8_t on);
};

class OverlayElement {
    protected:
        size_t _startIndex;

    public:
        OverlayElement(size_t startIndex);
        // virtual void update(uint8_t *colors) = 0;
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
        void update(uint8_t *colors, SegmentTransition *transition);
        void setNumber(int number);
};


class DotElement : public OverlayElement {
    public:
        static const size_t LENGTH = 8;

        DotElement(size_t startIndex);
        void update(uint8_t *colors);
};


class DataSource {
    private:
        unsigned long _id;

    public:
        DataSource(unsigned long id);
        unsigned long getId();
        virtual void update() = 0;
        virtual int getDigit(size_t index);
        virtual bool animateDots();

        virtual void fromJson(JsonObject &root) {};
        virtual void toJson(JsonObject &root) {};
};

class NoneDataSource : public DataSource {
    public:
        NoneDataSource();
        void update();
        int getDigit(size_t index);
        bool animateDots();
};

class TimeDataSource : public DataSource {
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

        void fromJson(JsonObject &root);
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
        SegmentTransition *_segmentTransition;

    public:
        ESPOverlayManager();
        void begin();
        void update(uint8_t *colors, size_t numLeds);

        void fromJson(JsonObject &root);
        void toJson(JsonObject &root);
};

extern ESPOverlayManager OverlayManager;

#endif
