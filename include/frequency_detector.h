#ifndef FREQUENCY_DETECTOR_H
#define FREQUENCY_DETECTOR_H

class FrequencyDetector {
public:
    FrequencyDetector(int sampleRate = 44100);

    // Returns detected frequency in Hz, or -1.0 if no pitch found
    float detect(const float* samples, int numSamples);

    float getLastRMS() const { return lastRms_; }

    void setRMSThreshold(float t) { rmsThreshold_ = t; }
    void setYINThreshold(float t) { yinThreshold_ = t; }

private:
    int   sampleRate_;
    float lastRms_      = 0.0f;
    float rmsThreshold_ = 0.002f;
    float yinThreshold_ = 0.15f;

    float yin(const float* samples, int numSamples);
};

#endif // FREQUENCY_DETECTOR_H
