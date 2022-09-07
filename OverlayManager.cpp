#include "OverlayManager.h"
#include "TimeManager.h"

SegmentTransition::SegmentTransition(unsigned long id) : _id(id) {
    _numSegments = SevenSegmentElement::NUM_SEGMENTS;
    _ledsPerSegment = SevenSegmentElement::LEDS_PER_SEGMENT;

    _durationMillis = 300;
}

unsigned long SegmentTransition::getId() {
    return _id;
}

void SegmentTransition::fromJson(JsonObject &root) {
    _durationMillis = root["dMs"] | _durationMillis;
}

void SegmentTransition::toJson(JsonObject &root) {
    root["dMs"] = _durationMillis;
}

NoTransition::NoTransition() : SegmentTransition(TRANSITION_NONE) {

}

void NoTransition::transition(double *brightness, uint8_t *targetSegments, unsigned long deltaMillis) {
    for (size_t seg = 0; seg < _numSegments; seg++) {
        uint8_t off = !targetSegments[seg];

        for (size_t offset = 0; offset < _ledsPerSegment; offset++) {
            size_t index = seg * _ledsPerSegment + offset;

            if (off) {
                brightness[index] = 0.0;
            } else {
                brightness[index] = 1.0;
            }
        }
    }
}

FadeTransition::FadeTransition() : SegmentTransition(TRANSITION_FADE) {

}

void FadeTransition::transition(double *brightness, uint8_t *targetSegments, unsigned long deltaMillis) {
    const double briPerMilli = 1.0 / _durationMillis;

    for (size_t seg = 0; seg < _numSegments; seg++) {
        uint8_t off = !targetSegments[seg];

        for (size_t offset = 0; offset < _ledsPerSegment; offset++) {
            size_t index = seg * _ledsPerSegment + offset;

            if (off) {
                brightness[index] -= briPerMilli * deltaMillis;
            } else {
                brightness[index] += briPerMilli * deltaMillis;
            }

            if (brightness[index] < 0.0) {
                brightness[index] = 0.0;
            } else if (brightness[index] > 1.0) {
                brightness[index] = 1.0;
            }
        }
    }
}

FlowTransition::FlowTransition() : SegmentTransition(TRANSITION_FLOW) {

}

void FlowTransition::transition(double *brightness, uint8_t *targetSegments, unsigned long deltaMillis) {
    for (size_t seg = 0; seg < _numSegments; seg++) {
        uint8_t off = !targetSegments[seg];

        int leftDist = getDistToNextOn(seg, targetSegments, 1, off);
        int rightDist = getDistToNextOn(seg, targetSegments, 0, off);

        double fac = 1.0;
        if (off) {
            fac = -1.0;
        }

        fac *= deltaMillis;

        // TODO: improve readability
        const int i1 = 0;
        const int i2 = _ledsPerSegment / 2 - 1;
        const int i3 = _ledsPerSegment / 2;
        const int i4 = _ledsPerSegment - 1;

        if (leftDist > 1 && rightDist > 1) {
            flowDim(brightness, seg, i4, i3, fac);
            flowDim(brightness, seg, i1, i2, fac);
        } else if (leftDist > rightDist) {
            flowDim(brightness, seg, i4, i1, fac);
        } else if (leftDist < rightDist) {
            flowDim(brightness, seg, i1, i4, fac);
        } else {
            flowDim(brightness, seg, i2, i1, fac);
            flowDim(brightness, seg, i3, i4, fac);
        }
    }
}

void FlowTransition::flowDim(double *brightness, int seg, int startIndex, int endIndex, double fac) {
    int currentOffset = startIndex;
    int dir = 1;
    if (endIndex < startIndex) {
        dir = -1;
    }

    double amountPerMilli = 1.0 / _durationMillis;
    double amount = ((endIndex - startIndex) * dir + 1) * amountPerMilli * fac;

    while (true) {
        size_t index = seg * _ledsPerSegment + currentOffset;
        brightness[index] += amount;

        if (brightness[index] < 0.0) {
            amount = brightness[index];
            brightness[index] = 0.0;
        } else if (brightness[index] > 1.0) {
            amount = brightness[index] - 1.0;
            brightness[index] = 1.0;
        } else {
            break;
        }

        if (currentOffset == endIndex) {
            break;
        }

        currentOffset += dir;
    }
}

int FlowTransition::getDistToNextOn(int seg, uint8_t *targetSegments, uint8_t left, uint8_t on) {
    if (targetSegments[seg] == on) {
        return 0;
    }

    uint8_t leftAdj[7][7] = {
        {0, 1, 0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0, 0, 0},
        {0, 0, 0, 1, 0, 0, 1},
        {1, 0, 0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0, 1, 0},
        {0, 0, 0, 0, 0, 0, 1},
        {0, 0, 1, 1, 0, 0, 0},
    };

    uint8_t rightAdj[7][7] = {
        {0, 0, 0, 1, 1, 0, 0},
        {1, 0, 0, 0, 0, 0, 0},
        {0, 1, 0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0, 0, 1},
        {1, 0, 0, 1, 0, 0, 0},
        {0, 0, 0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0, 1, 0},
    };


    for (size_t i = 0; i < 7; i++) {
        if ((left && leftAdj[seg][i] == 1 && targetSegments[i]) || (!left && rightAdj[seg][i] == 1 && targetSegments[i])) {
            if (on) {
                return 1;
            } else {
                return 99999;
            }
        }
    }

    if (on) {
        return 99999;
    } else {
        return 1;
    }
}


OverlayElement::OverlayElement(size_t startIndex) {
    _startIndex = startIndex;
}


SevenSegmentElement::SevenSegmentElement(size_t startIndex) : OverlayElement(startIndex) {
    for (int i = 0; i < LENGTH; i++) {
        _brightness[i] = 0;
    }

    _lastUpdateMillis = millis();
}

void SevenSegmentElement::update(uint8_t *colors, SegmentTransition *transition) {
    const uint8_t *segments = SEGMENTS[_number];

    unsigned long now = millis();
    unsigned long deltaMillis = now - _lastUpdateMillis;
    _lastUpdateMillis = now;

    if (transition) {
        transition->transition(_brightness, (uint8_t*)segments, deltaMillis);
    }

    for (size_t i = 0; i < LENGTH; i++) {
        colors[(_startIndex + i) * 3] *= _brightness[i];
        colors[(_startIndex + i) * 3 + 1] *= _brightness[i];
        colors[(_startIndex + i) * 3 + 2] *= _brightness[i];
    }
}

void SevenSegmentElement::setNumber(int number) {
    _number = number % 10; // safety
}



DotElement::DotElement(size_t startIndex) : OverlayElement(startIndex) {

}

void DotElement::update(uint8_t *colors) {
    double frac = (TimeManager.syncedMillis() % 2000) / 1000.0;
    if (frac > 1.0) {
        frac = 2.0 - frac;
    }

    for (size_t offset = 0; offset < LENGTH; offset++) {
        colors[(_startIndex + offset) * 3] *= frac;
        colors[(_startIndex + offset) * 3 + 1] *= frac;
        colors[(_startIndex + offset) * 3 + 2] *= frac;
    }
}


DataSource::DataSource(unsigned long id) : _id(id) {

}

unsigned long DataSource::getId() {
    return _id;
}

int DataSource::getDigit(size_t index) {
    return 0;
}

bool DataSource::animateDots() {
    return true;
}

NoneDataSource::NoneDataSource() : DataSource(OVERLAY_NONE) {

}

void NoneDataSource::update() {
    // do nothing
}

int NoneDataSource::getDigit(size_t index) {
    return 8; // light up every segment
}

bool NoneDataSource::animateDots() {
    return false;
}

TimeDataSource::TimeDataSource() : DataSource(OVERLAY_CLOCK) {
    
}

void TimeDataSource::update() {

}

int TimeDataSource::getDigit(size_t index) {
    switch(index) {
        case 0:
            return TimeManager.getMinutes() % 10;
        case 1:
            return TimeManager.getMinutes() / 10;
        case 2:
            return TimeManager.getHours() % 10;
        case 3:
            return TimeManager.getHours() / 10;
        default:
            return DataSource::getDigit(index);
    }
}


CountdownDataSource::CountdownDataSource() : DataSource(OVERLAY_COUNTDOWN) {
    _startMillis = millis();
    _valueMillis = 1 * 60 * 1000;
    _elapsedMillis = 0;
}

void CountdownDataSource::update() {
    unsigned long now = millis();
    _elapsedMillis = now - _startMillis;
}

int CountdownDataSource::getDigit(size_t index) {
    long value = _valueMillis - _elapsedMillis;
    if (value < 0) {
        value = 0;
    }

    int minutes = value / 1000 / 60;
    int seconds = (value / 1000) % 60;

    switch(index) {
        case 0:
            return seconds % 10;
        case 1:
            return seconds / 10;
        case 2:
            return minutes % 10;
        case 3:
            return minutes / 10;
        default:
            return DataSource::getDigit(index);
    }
}

void CountdownDataSource::fromJson(JsonObject &root) {
    DataSource::fromJson(root);

    _valueMillis = root["vMs"] | _valueMillis;
    _startMillis = millis(); // restart the countdown
    _elapsedMillis = 0;
}

CountupDataSource::CountupDataSource() : DataSource(OVERLAY_COUNTUP) {
    _startMillis = millis();
    _elapsedMillis = 0;
}

void CountupDataSource::update() {
    unsigned long now = millis();
    _elapsedMillis = now - _startMillis;
}

int CountupDataSource::getDigit(size_t index) {
    int minutes = _elapsedMillis / 1000 / 60;
    int seconds = (_elapsedMillis / 1000) % 60;

    switch(index) {
        case 0:
            return seconds % 10;
        case 1:
            return seconds / 10;
        case 2:
            return minutes % 10;
        case 3:
            return minutes / 10;
        default:
            return DataSource::getDigit(index);
    }
}


ESPOverlayManager::ESPOverlayManager() : _dot(112) {
    _digits.push_back(SevenSegmentElement(0));
    _digits.push_back(SevenSegmentElement(56));
    _digits.push_back(SevenSegmentElement(120));
    _digits.push_back(SevenSegmentElement(176));

    _dataSource = new TimeDataSource();
    _segmentTransition = new NoTransition();
}

void ESPOverlayManager::begin() {

}

void ESPOverlayManager::update(uint8_t *colors, size_t numLeds) {
    _dataSource->update();

    for (size_t i = 0; i < 4; i++) {
        _digits[i].setNumber(_dataSource->getDigit(i));
        _digits[i].update(colors, _segmentTransition);
    }

    if (_dataSource->animateDots()) {
        _dot.update(colors);
    }
}

void ESPOverlayManager::fromJson(JsonObject &root) {
    unsigned long overlayId = root["oId"] | 0;
    if (overlayId) {
        DataSource *dataSource = NULL;

        switch (overlayId) {
            case OVERLAY_NONE:
                dataSource = new NoneDataSource();
                break;
            case OVERLAY_CLOCK:
                dataSource = new TimeDataSource();
                break;
            case OVERLAY_COUNTDOWN:
                dataSource = new CountdownDataSource();
                break;
            case OVERLAY_COUNTUP:
                dataSource = new CountupDataSource();
        }

        if (dataSource) {
            if (_dataSource)
                delete _dataSource;

            _dataSource = dataSource;
        }
    }

    JsonObject overlayDataObj = root["oD"];
    if (overlayDataObj && _dataSource) {
        _dataSource->fromJson(overlayDataObj);
    }

    unsigned long transitionId = root["tId"] | 0;
    if (transitionId) {
        SegmentTransition *transition = NULL;

        switch (transitionId) {
            case TRANSITION_NONE:
                transition = new NoTransition();
                break;
            case TRANSITION_FADE:
                transition = new FadeTransition();
                break;
            case TRANSITION_FLOW:
                transition = new FlowTransition();
                break;
        }

        if (transition) {
            if (_segmentTransition)
                delete _segmentTransition;

            _segmentTransition = transition;
        }
    }

    JsonObject transitionDataObj = root["tD"];
    if (transitionDataObj && _segmentTransition) {
        _segmentTransition->fromJson(transitionDataObj);
    }
}

void ESPOverlayManager::toJson(JsonObject &root) {
    root["oId"] = _dataSource->getId();
    root["tId"] = _segmentTransition->getId();
}

ESPOverlayManager OverlayManager;
