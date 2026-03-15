#include "audio_capture.h"
#include <iostream>

AudioCapture::AudioCapture(int sampleRate, int framesPerBuffer)
    : sampleRate_(sampleRate),
      framesPerBuffer_(framesPerBuffer),
      stream_(nullptr),
      initialized_(false) {}

AudioCapture::~AudioCapture() {
    if (stream_) {
        Pa_CloseStream(stream_);
    }
    if (initialized_) {
        Pa_Terminate();
    }
}

bool AudioCapture::initialize(int deviceIndex) {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio init error: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }
    initialized_ = true;

    PaStreamParameters inputParams;
    if (deviceIndex < 0) {
        inputParams.device = Pa_GetDefaultInputDevice();
    } else {
        inputParams.device = deviceIndex;
    }
    if (inputParams.device == paNoDevice) {
        std::cerr << "No input device found" << std::endl;
        return false;
    }
    inputParams.channelCount = 1;
    inputParams.sampleFormat = paFloat32;
    inputParams.suggestedLatency = Pa_GetDeviceInfo(inputParams.device)->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;

    err = Pa_OpenStream(&stream_, &inputParams, nullptr,
                        sampleRate_, framesPerBuffer_, paClipOff,
                        paCallback, this);
    if (err != paNoError) {
        std::cerr << "Stream open error: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    return true;
}

bool AudioCapture::start() {
    PaError err = Pa_StartStream(stream_);
    if (err != paNoError) {
        std::cerr << "Stream start error: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }
    return true;
}

bool AudioCapture::stop() {
    PaError err = Pa_StopStream(stream_);
    return err == paNoError;
}

bool AudioCapture::isActive() const {
    return stream_ && Pa_IsStreamActive(stream_) == 1;
}

void AudioCapture::setCallback(AudioCallback callback) {
    callback_ = callback;
}

int AudioCapture::paCallback(const void* inputBuffer, void* /*outputBuffer*/,
                              unsigned long framesPerBuffer,
                              const PaStreamCallbackTimeInfo* /*timeInfo*/,
                              PaStreamCallbackFlags /*statusFlags*/,
                              void* userData) {
    AudioCapture* self = static_cast<AudioCapture*>(userData);
    const float* input = static_cast<const float*>(inputBuffer);

    if (self->callback_ && input) {
        self->callback_(input, static_cast<int>(framesPerBuffer));
    }

    return paContinue;
}
