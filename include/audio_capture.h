#ifndef AUDIO_CAPTURE_H
#define AUDIO_CAPTURE_H

#include <portaudio.h>
#include <vector>
#include <functional>

/**
 * audio capture - handles microphone input using PortAudio
 * 
 * initialize audio device
 * capture audio samples in real-time
 * provide callback mechanism for processing audio data
 */
class AudioCapture {
public:
    // Audio callback function type
    using AudioCallback = std::function<void(const float*, int)>;
    
    AudioCapture(int sampleRate = 44100, int framesPerBuffer = 2048);
    ~AudioCapture();
    
    // initialize portaudio and open audio stream
    bool initialize();
    
    // start / stop capturing
    bool start();
    
    bool stop();
    
    bool isActive() const;
    
    void setCallback(AudioCallback callback);
    
    int getSampleRate() const { return sampleRate_; }
    
    int getFramesPerBuffer() const { return framesPerBuffer_; }
    
private:
    int sampleRate_;
    int framesPerBuffer_;
    PaStream* stream_;
    AudioCallback callback_;
    bool initialized_;
    
    // Static callback for PortAudio (bridges to instance method)
    static int paCallback(const void* inputBuffer,
                         void* outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void* userData);
};

#endif // AUDIO_CAPTURE_H