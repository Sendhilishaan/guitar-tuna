#include "frequency_detector.h"
#include <vector>
#include <cmath>
#include <algorithm>

FrequencyDetector::FrequencyDetector(int sampleRate) : sampleRate_(sampleRate) {}

float FrequencyDetector::detect(const float* samples, int numSamples) {
    // Compute RMS so callers can check signal level
    float sumSq = 0.0f;
    for (int i = 0; i < numSamples; i++) sumSq += samples[i] * samples[i];
    lastRms_ = std::sqrt(sumSq / numSamples);

    if (lastRms_ < rmsThreshold_) return -1.0f;

    return yin(samples, numSamples);
}

// YIN pitch detection algorithm (de Cheveigne & Kawahara, 2002)
float FrequencyDetector::yin(const float* samples, int numSamples) {
    const float threshold = yinThreshold_;
    int halfSize = numSamples / 2;

    // Guitar range: ~70 Hz (below E2=82.41) to ~400 Hz (above E4=329.63)
    int minLag = static_cast<int>(sampleRate_ / 400.0f);
    int maxLag = static_cast<int>(sampleRate_ / 70.0f);
    maxLag = std::min(maxLag, halfSize - 1);

    // Step 1: Difference function
    // d(tau) = sum_j (x[j] - x[j+tau])^2
    std::vector<float> diff(maxLag + 1, 0.0f);
    for (int tau = 1; tau <= maxLag; tau++) {
        for (int j = 0; j < halfSize; j++) {
            float delta = samples[j] - samples[j + tau];
            diff[tau] += delta * delta;
        }
    }

    // Step 2: Cumulative mean normalized difference
    std::vector<float> cmnd(maxLag + 1);
    cmnd[0] = 1.0f;
    float runningSum = 0.0f;
    for (int tau = 1; tau <= maxLag; tau++) {
        runningSum += diff[tau];
        cmnd[tau] = (runningSum > 0.0f) ? diff[tau] * tau / runningSum : 1.0f;
    }

    // Step 3: Find first local minimum below threshold
    int bestTau = -1;
    for (int tau = minLag; tau <= maxLag; tau++) {
        if (cmnd[tau] < threshold) {
            while (tau + 1 <= maxLag && cmnd[tau + 1] < cmnd[tau]) tau++;
            bestTau = tau;
            break;
        }
    }

    // Fallback: use the global minimum in range if it's reasonably clear
    if (bestTau == -1) {
        float minVal = 1.0f;
        for (int tau = minLag; tau <= maxLag; tau++) {
            if (cmnd[tau] < minVal) {
                minVal = cmnd[tau];
                bestTau = tau;
            }
        }
        if (minVal > 0.35f) return -1.0f;  // too noisy, not a real pitch
    }

    // Step 4: Parabolic interpolation for sub-sample accuracy
    float refinedTau = static_cast<float>(bestTau);
    if (bestTau > minLag && bestTau < maxLag) {
        float s0 = cmnd[bestTau - 1];
        float s1 = cmnd[bestTau];
        float s2 = cmnd[bestTau + 1];
        float denom = 2.0f * (2.0f * s1 - s2 - s0);
        if (denom != 0.0f) {
            refinedTau = bestTau + (s2 - s0) / denom;
        }
    }

    return sampleRate_ / refinedTau;
}
