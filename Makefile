# Makefile for adikdrum Machine
# Date: Sat, 12/04/2025
# Author: CoolbrothUer
CC = g++
CFLAGS = -std=c++2a -Wall -Wextra -pedantic
ADIKCUI_LIBS = -lportaudio -lsndfile
ADIKTUI_LIBS = -lncurses

SRCS_DIR = src
BUILD_DIR = build

# Définir les sources et les objets pour adikcui
# ADIKCUI_SRCS = $(wildcard $(SRCS_DIR)/*.cpp)
ADIKCUI_SRCS = $(filter-out $(SRCS_DIR)/adiktui.cpp, $(wildcard $(SRCS_DIR)/*.cpp))
# ADIKCUI_HDRS = $(wildcard $(SRCS_DIR)/*.h)
ADIKCUI_HDRS = $(filter-out $(SRCS_DIR)/adiktui.h, $(wildcard $(SRCS_DIR)/*.h))
ADIKCUI_OBJS = $(patsubst $(SRCS_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(ADIKCUI_SRCS))
ADIKCUI_EXEC = $(BUILD_DIR)/adikcui

# Définir les sources et les objets pour adiktui
ADIKTUI_SRCS = (wildcard $(SRCS_DIR)/adiktui.cpp)
ADIKTUI_HDRS = $(wildcard $(SRCS_DIR)/adiktui.h $(SRCS_DIR)/uiapp.h)
ADIKTUI_OBJS = $(BUILD_DIR)/adiktui.o
ADIKTUI_EXEC = $(BUILD_DIR)/adiktui



# Définir la cible principale
all: $(ADIKCUI_EXEC) $(ADIKTUI_EXEC)

# Règle pour compiler adikcui
$(ADIKCUI_EXEC): $(ADIKCUI_OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(ADIKCUI_OBJS) $(ADIKCUI_LIBS) -o $(ADIKCUI_EXEC)

$(BUILD_DIR)/%.o: $(SRCS_DIR)/%.cpp $(ADIKCUI_HDRS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Règle pour compiler adiktui
$(ADIKTUI_EXEC): $(ADIKTUI_OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(ADIKTUI_OBJS) $(ADIKTUI_LIBS) -o $(ADIKTUI_EXEC)

$(BUILD_DIR)/adiktui.o: $(SRCS_DIR)/adiktui.cpp $(ADIKTUI_HDRS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Règle de nettoyage
clean:
	rm -rf $(BUILD_DIR)

.PHONY: clean all
