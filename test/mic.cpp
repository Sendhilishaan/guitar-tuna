#include <portaudio.h>
#include <iostream>
// import works text editor is stupid; note for future me

int main() {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "Error: " << Pa_GetErrorText(err) << std::endl;
        return 1;
    }
    
    std::cout << "PortAudio initialized successfully!" << std::endl;
    std::cout << "Available audio devices:" << std::endl;
    
    int numDevices = Pa_GetDeviceCount();
    for (int i = 0; i < numDevices; i++) {
        const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);
        std::cout << i << ": " << deviceInfo->name 
                  << " (inputs: " << deviceInfo->maxInputChannels << ")" 
                  << std::endl;
    }
    
    PaError err2 = Pa_Terminate();
    if ( err2 != paNoError) {
        std::cout << Pa_GetErrorText(err2) << std::endl;
    }
    return 0;
}