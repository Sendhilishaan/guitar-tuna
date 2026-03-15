#ifndef TUNER_ENGINE_H
#define TUNER_ENGINE_H

#include <string>

struct TuneResult {
    bool valid;
    float frequency;
    std::string note;      // e.g. "E2", "A2"
    int stringNumber;      // 1 (low E) to 6 (high E)
    float centsOff;        // negative = flat, positive = sharp
    bool inTune;           // within ±5 cents
};

class TunerEngine {
public:
    TunerEngine();
    TuneResult analyze(float frequency);

private:
    static const float STRING_FREQS[6];
    static const char* STRING_NAMES[6];
};

#endif // TUNER_ENGINE_H
