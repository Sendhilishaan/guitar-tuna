#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <chrono>

#include <portaudio.h>

#include "audio_capture.h"
#include "frequency_detector.h"
#include "tuner_engine.h"
#include "display.h"
#include "settings.h"

static std::vector<AudioDevice> enumerateInputDevices() {
    std::vector<AudioDevice> devices;
    if (Pa_Initialize() != paNoError) return devices;
    int n = Pa_GetDeviceCount();
    for (int i = 0; i < n; i++) {
        const PaDeviceInfo* info = Pa_GetDeviceInfo(i);
        if (info && info->maxInputChannels > 0)
            devices.push_back({i, info->name});
    }
    Pa_Terminate();
    return devices;
}

int main() {
    auto devices = enumerateInputDevices();
    if (devices.empty()) {
        std::cerr << "No input devices found" << std::endl;
        return 1;
    }

    Display display;
    if (!display.initialize()) {
        std::cerr << "Failed to initialize display" << std::endl;
        return 1;
    }

    Settings settings;
    settings.deviceIndex = display.selectDevice(devices);

    FrequencyDetector detector(44100);
    TunerEngine engine;

    TuneResult latestResult{};
    float      latestRMS   = 0.0f;
    bool       hasSignal   = false;
    std::mutex resultMutex;

    // Encapsulates creating, wiring, and starting an AudioCapture
    std::unique_ptr<AudioCapture> audio;

    auto startAudio = [&]() -> bool {
        audio = std::make_unique<AudioCapture>(44100, 4096);
        if (!audio->initialize(settings.deviceIndex)) return false;

        audio->setCallback([&](const float* samples, int numSamples) {
            float freq = detector.detect(samples, numSamples);
            std::lock_guard<std::mutex> lock(resultMutex);
            latestRMS = detector.getLastRMS();
            if (freq > 0.0f) {
                latestResult = engine.analyze(freq);
                hasSignal    = true;
            } else {
                hasSignal = false;
                // Keep latestResult so the display doesn't blank out
            }
        });

        return audio->start();
    };

    if (!startAudio()) {
        std::cerr << "Failed to start audio" << std::endl;
        return 1;
    }

    while (true) {
        int key = display.pollKey();

        if (key == 'q' || key == 'Q') break;

        if (key == 's' || key == 'S') {
            audio->stop();

            bool deviceChanged = display.showSettings(settings, devices);

            // Apply threshold settings to detector immediately
            detector.setRMSThreshold(settings.getRMSThreshold());
            detector.setYINThreshold(settings.getYINThreshold());

            if (deviceChanged) {
                // Destroy old AudioCapture (closes stream + terminates PA),
                // then re-init with the new device
                audio.reset();
                if (!startAudio()) {
                    std::cerr << "Failed to restart audio after device change" << std::endl;
                    return 1;
                }
            } else {
                audio->start();
            }
            continue;
        }

        TuneResult result;
        float rms;
        bool  signal;
        {
            std::lock_guard<std::mutex> lock(resultMutex);
            result = latestResult;
            rms    = latestRMS;
            signal = hasSignal;
        }

        display.render(result, rms, signal);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    audio->stop();
    return 0;
}
