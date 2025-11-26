CXX = g++
CXXFLAGS = -Wall -std=c++17
SRC_DIR = src
TEST_DIR = test
BIN_DIR = bin
TARGET = $(BIN_DIR)/main
MIC_TARGET = $(BIN_DIR)/mic

PORTAUDIO_CFLAGS = -I$(shell brew --prefix portaudio)/include
PORTAUDIO_LDFLAGS = -L$(shell brew --prefix portaudio)/lib -lportaudio

all: $(TARGET)

$(TARGET): $(SRC_DIR)/main.cpp
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@
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