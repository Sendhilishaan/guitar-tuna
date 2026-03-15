#include "display.h"
#include <cmath>
#include <cstring>
#include <cstdio>
#include <algorithm>

// Color pair IDs
static const int CP_RED       = 1;
static const int CP_GREEN     = 2;
static const int CP_CYAN      = 3;
static const int CP_WHITE     = 4;
static const int CP_YELLOW    = 5;
static const int CP_HIGHLIGHT = 6;  // black on cyan

Display::Display() : initialized_(false) {}

Display::~Display() {
    if (initialized_) endwin();
}

bool Display::initialize() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);

    if (has_colors()) {
        start_color();
        init_pair(CP_RED,       COLOR_RED,    COLOR_BLACK);
        init_pair(CP_GREEN,     COLOR_GREEN,  COLOR_BLACK);
        init_pair(CP_CYAN,      COLOR_CYAN,   COLOR_BLACK);
        init_pair(CP_WHITE,     COLOR_WHITE,  COLOR_BLACK);
        init_pair(CP_YELLOW,    COLOR_YELLOW, COLOR_BLACK);
        init_pair(CP_HIGHLIGHT, COLOR_BLACK,  COLOR_CYAN);
    }

    initialized_ = true;
    return true;
}

int Display::pollKey() {
    return getch();
}

// ─── shared header ────────────────────────────────────────────────────────────

void Display::drawTitle(int centerX) {
    const char* fish  = "><(((°>";
    const char* title = " GUITAR TUNA ";
    const char* fish2 = "<°)))><";
    int len = static_cast<int>(strlen(fish) + strlen(title) + strlen(fish2));

    attron(COLOR_PAIR(CP_CYAN) | A_BOLD);
    mvprintw(1, centerX - len / 2, "%s%s%s", fish, title, fish2);
    attroff(COLOR_PAIR(CP_CYAN) | A_BOLD);

    attron(COLOR_PAIR(CP_WHITE));
    for (int x = 2; x < COLS - 2; x++) mvaddch(2, x, ACS_HLINE);
    attroff(COLOR_PAIR(CP_WHITE));
}

// ─── device picker ────────────────────────────────────────────────────────────

int Display::selectDevice(const std::vector<AudioDevice>& devices, int currentIndex) {
    nodelay(stdscr, FALSE);

    // Pre-select the device that's currently active
    int selected = 0;
    for (int i = 0; i < static_cast<int>(devices.size()); i++) {
        if (devices[i].index == currentIndex) { selected = i; break; }
    }

    const int maxVisible = std::max(1, LINES - 10);

    while (true) {
        erase();
        int cx = COLS / 2;
        drawTitle(cx);

        attron(COLOR_PAIR(CP_WHITE) | A_BOLD);
        const char* prompt = "Select Input Device";
        mvprintw(4, cx - static_cast<int>(strlen(prompt)) / 2, "%s", prompt);
        attroff(COLOR_PAIR(CP_WHITE) | A_BOLD);

        int scroll = 0;
        if (selected >= maxVisible) scroll = selected - maxVisible + 1;

        for (int i = 0; i < maxVisible; i++) {
            int idx = i + scroll;
            if (idx >= static_cast<int>(devices.size())) break;

            char line[128];
            snprintf(line, sizeof(line), "  %s  ", devices[idx].name.c_str());

            if (idx == selected) {
                attron(COLOR_PAIR(CP_HIGHLIGHT) | A_BOLD);
            } else {
                attron(COLOR_PAIR(CP_WHITE));
            }
            mvprintw(6 + i, cx - static_cast<int>(strlen(line)) / 2, "%s", line);
            if (idx == selected) attroff(COLOR_PAIR(CP_HIGHLIGHT) | A_BOLD);
            else                 attroff(COLOR_PAIR(CP_WHITE));
        }

        attron(COLOR_PAIR(CP_WHITE));
        mvprintw(LINES - 2, cx - 20, "j/k or UP/DOWN to navigate   Enter to select");
        attroff(COLOR_PAIR(CP_WHITE));
        refresh();

        int ch = getch();
        int n = static_cast<int>(devices.size());
        if ((ch == 'k' || ch == KEY_UP)   && selected > 0)     selected--;
        else if ((ch == 'j' || ch == KEY_DOWN) && selected < n - 1) selected++;
        else if (ch == '\n' || ch == KEY_ENTER)                      break;
        else if (ch == 27 /* ESC */)                                 break;
    }

    nodelay(stdscr, TRUE);
    return devices[selected].index;
}

// ─── settings screen ──────────────────────────────────────────────────────────

void Display::drawDots(int y, int x, int filled, int total) {
    for (int i = 0; i < total; i++) {
        if (i < filled) {
            attron(COLOR_PAIR(CP_CYAN) | A_BOLD);
            mvprintw(y, x + i * 2, "●");
            attroff(COLOR_PAIR(CP_CYAN) | A_BOLD);
        } else {
            attron(COLOR_PAIR(CP_WHITE));
            mvprintw(y, x + i * 2, "○");
            attroff(COLOR_PAIR(CP_WHITE));
        }
    }
}

bool Display::showSettings(Settings& settings, const std::vector<AudioDevice>& devices) {
    nodelay(stdscr, FALSE);

    // Find current device name for display
    auto deviceName = [&](int idx) -> std::string {
        for (auto& d : devices) if (d.index == idx) return d.name;
        return "Unknown";
    };

    const int NUM_ROWS = 3;  // Device, Sensitivity, Confidence
    int row = 0;
    bool deviceChanged = false;
    Settings original = settings;

    while (true) {
        erase();
        int cx = COLS / 2;
        drawTitle(cx);

        attron(COLOR_PAIR(CP_WHITE) | A_BOLD);
        const char* heading = "S E T T I N G S";
        mvprintw(4, cx - static_cast<int>(strlen(heading)) / 2, "%s", heading);
        attroff(COLOR_PAIR(CP_WHITE) | A_BOLD);

        // Row layouts: label col and value col
        const int labelCol = cx - 22;
        const int valueCol = cx - 2;

        struct Row { const char* label; } rows[NUM_ROWS] = {
            {"Input Device"},
            {"Sensitivity "},
            {"Confidence  "},
        };

        for (int r = 0; r < NUM_ROWS; r++) {
            int y = 6 + r * 2;
            bool sel = (r == row);

            // Cursor
            if (sel) {
                attron(COLOR_PAIR(CP_CYAN) | A_BOLD);
                mvprintw(y, labelCol - 2, ">");
                attroff(COLOR_PAIR(CP_CYAN) | A_BOLD);
            }

            attron(sel ? (COLOR_PAIR(CP_WHITE) | A_BOLD) : COLOR_PAIR(CP_WHITE));
            mvprintw(y, labelCol, "%s", rows[r].label);
            attroff(sel ? (COLOR_PAIR(CP_WHITE) | A_BOLD) : COLOR_PAIR(CP_WHITE));

            if (r == 0) {
                // Device name
                std::string name = deviceName(settings.deviceIndex);
                if (static_cast<int>(name.size()) > 28) name = name.substr(0, 25) + "...";
                attron(sel ? (COLOR_PAIR(CP_CYAN) | A_BOLD) : COLOR_PAIR(CP_WHITE));
                mvprintw(y, valueCol, "[%s]", name.c_str());
                attroff(sel ? (COLOR_PAIR(CP_CYAN) | A_BOLD) : COLOR_PAIR(CP_WHITE));
            } else if (r == 1) {
                drawDots(y, valueCol, settings.sensitivityLevel + 1, Settings::NUM_SENSITIVITY);
                attron(COLOR_PAIR(CP_WHITE));
                mvprintw(y, valueCol + Settings::NUM_SENSITIVITY * 2 + 1, "%s",
                         Settings::sensitivityName(settings.sensitivityLevel));
                attroff(COLOR_PAIR(CP_WHITE));
            } else if (r == 2) {
                drawDots(y, valueCol, settings.confidenceLevel + 1, Settings::NUM_CONFIDENCE);
                attron(COLOR_PAIR(CP_WHITE));
                mvprintw(y, valueCol + Settings::NUM_CONFIDENCE * 2 + 1, "%s",
                         Settings::confidenceName(settings.confidenceLevel));
                attroff(COLOR_PAIR(CP_WHITE));
            }
        }

        attron(COLOR_PAIR(CP_WHITE));
        mvprintw(LINES - 3, cx - 30, "j/k  navigate");
        mvprintw(LINES - 2, cx - 30, "h/l or </> change value   Enter  pick device   q  back");
        attroff(COLOR_PAIR(CP_WHITE));
        refresh();

        int ch = getch();

        if (ch == 'q' || ch == 'Q' || ch == 27) break;

        if (ch == 'k' || ch == KEY_UP)   { row = (row - 1 + NUM_ROWS) % NUM_ROWS; continue; }
        if (ch == 'j' || ch == KEY_DOWN) { row = (row + 1) % NUM_ROWS;            continue; }

        if (row == 0) {
            // Device row — Enter or h/l opens picker
            if (ch == '\n' || ch == KEY_ENTER || ch == 'l' || ch == KEY_RIGHT) {
                int chosen = selectDevice(devices, settings.deviceIndex);
                if (chosen != settings.deviceIndex) {
                    settings.deviceIndex = chosen;
                    deviceChanged = true;
                }
            }
        } else if (row == 1) {
            // Sensitivity
            if ((ch == 'h' || ch == KEY_LEFT || ch == '<') && settings.sensitivityLevel > 0)
                settings.sensitivityLevel--;
            if ((ch == 'l' || ch == KEY_RIGHT || ch == '>') && settings.sensitivityLevel < Settings::NUM_SENSITIVITY - 1)
                settings.sensitivityLevel++;
        } else if (row == 2) {
            // Confidence
            if ((ch == 'h' || ch == KEY_LEFT || ch == '<') && settings.confidenceLevel > 0)
                settings.confidenceLevel--;
            if ((ch == 'l' || ch == KEY_RIGHT || ch == '>') && settings.confidenceLevel < Settings::NUM_CONFIDENCE - 1)
                settings.confidenceLevel++;
        }
    }

    nodelay(stdscr, TRUE);
    return deviceChanged || (settings.deviceIndex != original.deviceIndex);
}

// ─── tuner screen ─────────────────────────────────────────────────────────────

void Display::drawSignalBar(int y, int centerX, int width, float rmsLevel) {
    float logLevel = 0.0f;
    if (rmsLevel > 0.0001f) {
        logLevel = std::log10(rmsLevel / 0.0001f) / std::log10(0.5f / 0.0001f);
        logLevel = std::max(0.0f, std::min(1.0f, logLevel));
    }
    int filled = static_cast<int>(logLevel * width);

    attron(COLOR_PAIR(CP_WHITE));
    mvprintw(y, centerX - width / 2 - 4, "MIC ");
    mvaddch(y, centerX - width / 2 - 1, '[');
    attroff(COLOR_PAIR(CP_WHITE));

    for (int i = 0; i < width; i++) {
        int x = centerX - width / 2 + i;
        if (i < filled) {
            attron(COLOR_PAIR(i < static_cast<int>(width * 0.6f) ? CP_GREEN : CP_YELLOW));
            mvaddch(y, x, '|');
            attroff(COLOR_PAIR(i < static_cast<int>(width * 0.6f) ? CP_GREEN : CP_YELLOW));
        } else {
            attron(COLOR_PAIR(CP_WHITE));
            mvaddch(y, x, ' ');
            attroff(COLOR_PAIR(CP_WHITE));
        }
    }

    attron(COLOR_PAIR(CP_WHITE));
    mvaddch(y, centerX + width / 2, ']');
    attroff(COLOR_PAIR(CP_WHITE));
}

void Display::drawNote(int y, int centerX, const TuneResult& result, bool hasSignal) {
    const int boxW = 13;
    const int boxH = 3;
    int boxX = centerX - boxW / 2;

    int colorPair;
    if (!hasSignal) {
        colorPair = CP_WHITE;
    } else if (result.inTune) {
        colorPair = CP_GREEN;
    } else if (std::abs(result.centsOff) < 15.0f) {
        colorPair = CP_YELLOW;
    } else {
        colorPair = CP_RED;
    }

    int attrs = hasSignal ? (COLOR_PAIR(colorPair) | A_BOLD) : COLOR_PAIR(colorPair);
    attron(attrs);

    mvaddch(y, boxX, ACS_ULCORNER);
    for (int x = 1; x < boxW - 1; x++) mvaddch(y, boxX + x, ACS_HLINE);
    mvaddch(y, boxX + boxW - 1, ACS_URCORNER);

    mvaddch(y + 1, boxX, ACS_VLINE);
    mvprintw(y + 1, boxX + 1, "  %5s    ", result.note.c_str());
    mvaddch(y + 1, boxX + boxW - 1, ACS_VLINE);

    mvaddch(y + boxH - 1, boxX, ACS_LLCORNER);
    for (int x = 1; x < boxW - 1; x++) mvaddch(y + boxH - 1, boxX + x, ACS_HLINE);
    mvaddch(y + boxH - 1, boxX + boxW - 1, ACS_LRCORNER);

    attroff(attrs);

    char strLabel[32];
    snprintf(strLabel, sizeof(strLabel), "String %d", result.stringNumber);
    attron(COLOR_PAIR(CP_WHITE));
    mvprintw(y + boxH + 1, centerX - static_cast<int>(strlen(strLabel)) / 2, "%s", strLabel);
    attroff(COLOR_PAIR(CP_WHITE));

    char freqStr[32];
    snprintf(freqStr, sizeof(freqStr), "%.2f Hz", result.frequency);
    attron(COLOR_PAIR(CP_WHITE));
    mvprintw(y + boxH + 2, centerX - static_cast<int>(strlen(freqStr)) / 2, "%s", freqStr);
    attroff(COLOR_PAIR(CP_WHITE));
}

void Display::drawNeedle(int y, int centerX, int halfWidth, float centsOff, bool inTune, bool hasSignal) {
    int lineStart = centerX - halfWidth;
    int lineEnd   = centerX + halfWidth;

    int colorPair;
    if (!hasSignal) {
        colorPair = CP_WHITE;
    } else if (inTune) {
        colorPair = CP_GREEN;
    } else if (std::abs(centsOff) < 15.0f) {
        colorPair = CP_YELLOW;
    } else {
        colorPair = CP_RED;
    }

    attron(COLOR_PAIR(CP_WHITE));
    mvprintw(y, lineStart - 2, "b");
    mvprintw(y, lineEnd + 1,   "#");
    for (int x = lineStart; x <= lineEnd; x++)
        mvaddch(y, x, x == centerX ? ACS_PLUS : ACS_HLINE);
    attroff(COLOR_PAIR(CP_WHITE));

    float clamped = std::max(-50.0f, std::min(50.0f, centsOff));
    int needleX = centerX + static_cast<int>(clamped / 50.0f * halfWidth);

    attron(COLOR_PAIR(colorPair) | A_BOLD);
    mvaddch(y + 1, needleX, '^');
    attroff(COLOR_PAIR(colorPair) | A_BOLD);

    if (hasSignal) {
        char statusStr[32];
        if (inTune)
            snprintf(statusStr, sizeof(statusStr), "IN TUNE");
        else
            snprintf(statusStr, sizeof(statusStr), "%.1f cents %s",
                     std::abs(centsOff), centsOff < 0 ? "flat" : "sharp");
        int statusLen = static_cast<int>(strlen(statusStr));
        attron(COLOR_PAIR(colorPair) | A_BOLD);
        mvprintw(y + 2, centerX - statusLen / 2, "%s", statusStr);
        attroff(COLOR_PAIR(colorPair) | A_BOLD);
    }
}

void Display::render(const TuneResult& result, float rmsLevel, bool hasSignal) {
    erase();
    int cx = COLS / 2;

    drawTitle(cx);

    int barWidth = std::min(COLS / 2, 24);
    drawSignalBar(3, cx, barWidth, rmsLevel);

    if (result.valid) {
        // Always draw note and needle — dim when no active signal
        drawNote(5, cx, result, hasSignal);
        int halfWidth = std::min(COLS / 2 - 6, 25);
        drawNeedle(12, cx, halfWidth, result.centsOff, result.inTune, hasSignal);
    } else {
        // Before any pitch has ever been detected
        attron(COLOR_PAIR(CP_WHITE));
        const char* msg = "Play a note to begin...";
        mvprintw(LINES / 2, cx - static_cast<int>(strlen(msg)) / 2, "%s", msg);
        attroff(COLOR_PAIR(CP_WHITE));
    }

    attron(COLOR_PAIR(CP_WHITE));
    mvprintw(LINES - 1, cx - 22, "q  quit    s  settings");
    attroff(COLOR_PAIR(CP_WHITE));

    refresh();
}
