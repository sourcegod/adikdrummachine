# Makefile pour adikdrum Machine
# Date: Sat, 12/04/2025
# Author: CoolbrothUer
CC = g++
CFLAGS = -std=c++2a -Wall -Wextra -pedantic

# Bibliothèques externes
PORTAUDIO_LIB = -lportaudio
SNDFILE_LIB = -lsndfile
NCURSES_LIB = -lncurses

ADIKCUI_LIBS = $(PORTAUDIO_LIB) $(SNDFILE_LIB)
ADIKTUI_LIBS = $(PORTAUDIO_LIB) $(SNDFILE_LIB) $(NCURSES_LIB)

# Répertoires
SRCS_DIR = src
BUILD_DIR = build

# Fichiers sources communs
COMMON_SRCS = $(filter-out $(SRCS_DIR)/adiktui.cpp $(SRCS_DIR)/adikcuiapp.cpp, $(shell find $(SRCS_DIR) -name '*.cpp'))
COMMON_OBJS = $(patsubst $(SRCS_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(COMMON_SRCS))

# --- Configuration pour adikcui ---
ADIKCUI_SRCS = $(COMMON_SRCS) $(SRCS_DIR)/adikcuiapp.cpp
ADIKCUI_OBJS = $(COMMON_OBJS) $(patsubst $(SRCS_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS_DIR)/adikcuiapp.cpp)
ADIKCUI_EXEC_NAME = adikcui
ADIKCUI_EXEC = $(BUILD_DIR)/$(ADIKCUI_EXEC_NAME)

# --- Configuration pour adiktui ---
ADIKTUI_SRCS = $(COMMON_SRCS) $(SRCS_DIR)/adiktui.cpp
ADIKTUI_OBJS = $(COMMON_OBJS) $(patsubst $(SRCS_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS_DIR)/adiktui.cpp)
ADIKTUI_EXEC_NAME = adiktui
ADIKTUI_EXEC = $(BUILD_DIR)/$(ADIKTUI_EXEC_NAME)

# Définir la cible principale
all: $(ADIKCUI_EXEC) $(ADIKTUI_EXEC)

# Règle pour créer le répertoire de build
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Règle générique pour compiler les .cpp en .o
$(BUILD_DIR)/%.o: $(SRCS_DIR)/%.cpp | $(BUILD_DIR)
	@echo "Compiling $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Règle pour linker adikcui
$(ADIKCUI_EXEC): $(ADIKCUI_OBJS) | $(BUILD_DIR)
	@echo "Linking $(ADIKCUI_EXEC_NAME)"
	$(CC) $(CFLAGS) $(ADIKCUI_OBJS) $(ADIKCUI_LIBS) -o $(ADIKCUI_EXEC)

# Règle pour linker adiktui
$(ADIKTUI_EXEC): $(ADIKTUI_OBJS) | $(BUILD_DIR)
	@echo "Linking $(ADIKTUI_EXEC_NAME)"
	$(CC) $(CFLAGS) $(ADIKTUI_OBJS) $(ADIKTUI_LIBS) -o $(ADIKTUI_EXEC)

# Règle de nettoyage
clean:
	rm -rf $(BUILD_DIR)
	@echo "Cleaning build directory"

.PHONY: clean all


