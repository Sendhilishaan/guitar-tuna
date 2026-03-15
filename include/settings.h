#ifndef SETTINGS_H
#define SETTINGS_H

struct Settings {
    int deviceIndex      = -1;
    int sensitivityLevel = 2;   // 0=very low .. 4=max
    int confidenceLevel  = 1;   // 0=strict   .. 3=very relaxed

    static constexpr int NUM_SENSITIVITY = 5;
    static constexpr int NUM_CONFIDENCE  = 4;

    static const char* sensitivityName(int level) {
        static const char* n[] = {"Very Low", "Low", "Medium", "High", "Max"};
        return n[level];
    }
    static const char* confidenceName(int level) {
        static const char* n[] = {"Strict", "Normal", "Relaxed", "Very Relaxed"};
        return n[level];
    }

    // Maps to actual threshold values used by FrequencyDetector
    float getRMSThreshold() const {
        static const float t[] = {0.010f, 0.005f, 0.002f, 0.001f, 0.0005f};
        return t[sensitivityLevel];
    }
    float getYINThreshold() const {
        static const float t[] = {0.10f, 0.15f, 0.20f, 0.25f};
        return t[confidenceLevel];
    }
};

#endif // SETTINGS_H
