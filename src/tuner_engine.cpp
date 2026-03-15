#include "tuner_engine.h"
#include <cmath>
#include <limits>

// Standard tuning: string 1 (low E) to string 6 (high E)
const float TunerEngine::STRING_FREQS[6] = {
    82.41f,   // E2 - string 6 (low E)
    110.00f,  // A2 - string 5
    146.83f,  // D3 - string 4
    196.00f,  // G3 - string 3
    246.94f,  // B3 - string 2
    329.63f,  // E4 - string 1 (high E)
};

const char* TunerEngine::STRING_NAMES[6] = {
    "E2", "A2", "D3", "G3", "B3", "E4"
};

TunerEngine::TunerEngine() {}

TuneResult TunerEngine::analyze(float frequency) {
    TuneResult result;
    result.valid = false;

    if (frequency <= 0.0f) return result;

    // Find the closest string by cents distance
    int closest = 0;
    float minAbsCents = std::numeric_limits<float>::max();

    for (int i = 0; i < 6; i++) {
        // cents = 1200 * log2(f / f_ref)
        float cents = 1200.0f * std::log2(frequency / STRING_FREQS[i]);
        float absCents = std::abs(cents);
        if (absCents < minAbsCents) {
            minAbsCents = absCents;
            closest = i;
        }
    }

    result.valid = true;
    result.frequency = frequency;
    result.note = STRING_NAMES[closest];
    result.stringNumber = closest + 1;  // 1-indexed
    result.centsOff = 1200.0f * std::log2(frequency / STRING_FREQS[closest]);
    result.inTune = std::abs(result.centsOff) <= 5.0f;

    return result;
}
