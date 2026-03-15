CXX = g++
CXXFLAGS = -Wall -std=c++17 -Iinclude
SRC_DIR = src
TEST_DIR = test
BIN_DIR = bin
TARGET = $(BIN_DIR)/guitar-tuna
MIC_TARGET = $(BIN_DIR)/mic

PORTAUDIO_CFLAGS = -I$(shell brew --prefix portaudio)/include
PORTAUDIO_LDFLAGS = -L$(shell brew --prefix portaudio)/lib -lportaudio
NCURSES_LDFLAGS = -lncurses

SRCS = $(SRC_DIR)/main.cpp \
       $(SRC_DIR)/audio_capture.cpp \
       $(SRC_DIR)/frequency_detector.cpp \
       $(SRC_DIR)/tuner_engine.cpp \
       $(SRC_DIR)/display.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(PORTAUDIO_CFLAGS) $^ $(PORTAUDIO_LDFLAGS) $(NCURSES_LDFLAGS) -o $@
	@echo "Compiled successfully! Executable: $(TARGET)"

mic: $(MIC_TARGET)

$(MIC_TARGET): $(TEST_DIR)/mic.cpp
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(PORTAUDIO_CFLAGS) $< $(PORTAUDIO_LDFLAGS) -o $@
	@echo "Compiled mic test! Executable: $(MIC_TARGET)"

clean:
	rm -rf $(BIN_DIR)

run: $(TARGET)
	./$(TARGET)

run-mic: $(MIC_TARGET)
	./$(MIC_TARGET)

.PHONY: all clean run mic run-mic
