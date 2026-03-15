#ifndef DISPLAY_H
#define DISPLAY_H

#include "tuner_engine.h"
#include "settings.h"
#include <ncurses.h>
#include <string>
#include <vector>

struct AudioDevice {
    int index;
    std::string name;
};

class Display {
public:
    Display();
    ~Display();

    bool initialize();

    // Device picker (startup or from settings). Returns chosen PortAudio device index.
    int selectDevice(const std::vector<AudioDevice>& devices, int currentIndex = -1);

    // Settings screen. Modifies settings in-place. Returns true if device was changed.
    bool showSettings(Settings& settings, const std::vector<AudioDevice>& devices);

    // Non-blocking key poll — returns getch() result (ERR if nothing pressed)
    int pollKey();

    // Render the tuner. result holds last known detection; hasSignal dims it when false.
    void render(const TuneResult& result, float rmsLevel, bool hasSignal);

private:
    bool initialized_;

    void drawTitle(int centerX);
    void drawNote(int y, int centerX, const TuneResult& result, bool hasSignal);
    void drawNeedle(int y, int centerX, int halfWidth, float centsOff, bool inTune, bool hasSignal);
    void drawSignalBar(int y, int centerX, int width, float rmsLevel);
    void drawDots(int y, int x, int filled, int total);
};

#endif // DISPLAY_H
