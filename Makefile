# MakeFile for adikdrummachine
# Date: Sat, 12/04/2025
# Author: Coolbrother

CXX=g++
CXXFLAGS := -std=c++11 -Wall -Wextra -pedantic -g -D__LINUX_ALSA__
CPPFLAGS=-lasound -lpthread

EXEC := adikdrum
BUILD_DIR := ./build
SRC_DIR := ./src
MKDIR_P=mkdir -p

# $(wildcard *.cpp /xxx/xxx/*.cpp): get all .cpp files from the current
# directory and dir "/xxx/xxx/"
SRCS := $(wildcard $(SRC_DIR)/*.cpp)

# $(patsubst %.cpp,%.o,$(SRCS)): substitute all ".cpp" file name strings to
# ".o" file name strings
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
TARGET := $(BUILD_DIR)/$(EXEC)
$(info Create build directory if not exists)
$(shell mkdir -vp $(BUILD_DIR))

# for debugging
# $(info srcs:  $(SRCS))
# $(info objs:  $(OBJS))
# $(info target:  $(TARGET))

# Compile C++ source
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(SRC_DIR)/%.h
	$(info Compiling C++ sources:  $@, $<)
	$(CXX) $(CXXFLAGS) -c -o $@ $<


# building targets
# Note: important to place the CPPFLAGS variable at the end of command, to
# building with the necessary audio libraries required.
$(TARGET): $(OBJS)
	$(info Building target:  $@, $^)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(CPPFLAGS)
	$(info Terminated successfully.)


all: $(TARGET)

clean:
	$(info Cleaning...)
	rm -f $(BUILD_DIR)/*.o $(TARGET)

run:
	$(TARGET)
