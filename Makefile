# Makefile for adikdrum Machine
# Date: Sat, 12/04/2025
# Author: CoolbrothUer
CC = g++
CFLAGS = -std=c++2a -Wall -Wextra -pedantic #
LIBS = -lportaudio

SRCS_DIR = src
BUILD_DIR = build
SRCS = $(wildcard $(SRCS_DIR)/*.cpp)
HDRS = $(wildcard $(SRCS_DIR)/*.h)
OBJS = $(patsubst $(SRCS_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
EXEC = $(BUILD_DIR)/adikdrum

all: $(EXEC)

$(EXEC): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $(EXEC)

$(BUILD_DIR)/%.o: $(SRCS_DIR)/%.cpp $(HDRS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
