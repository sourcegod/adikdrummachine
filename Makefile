# Makefile for adikdrum Machine
# Date: Sat, 12/04/2025
# Author: CoolbrothUer
CC = g++
CFLAGS = -std=c++2a -Wall -Wextra -pedantic

# Bibliothèques externes
ADIKCUI_LIBS = -lportaudio -lsndfile
ADIKTUI_LIBS = -lportaudio -lsndfile -lncurses

# Répertoires
SRCS_DIR = src
BUILD_DIR = build

# Tous les fichiers sources
ALL_SRCS = $(wildcard $(SRCS_DIR)/*.cpp)

# --- Configuration pour adikcui ---
ADIKCUI_SRCS = $(filter-out $(SRCS_DIR)/adiktui.cpp, $(ALL_SRCS)) # Exclut adiktui.cpp
ADIKCUI_OBJS = $(patsubst $(SRCS_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(ADIKCUI_SRCS))
ADIKCUI_EXEC = $(BUILD_DIR)/adikcui

# --- Configuration pour adiktui ---
# ADIKTUI_SRCS = $(SRCS_DIR)/adiktui.cpp # SEULEMENT adiktui.cpp, qui contient main() pour la version ncurses.
ADIKTUI_SRCS = $(filter-out $(SRCS_DIR)/adikcuiapp.cpp, $(ALL_SRCS)) # Exclutadikcuiapp.cpp
ADIKTUI_OBJS = $(patsubst $(SRCS_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(ADIKTUI_SRCS))
ADIKTUI_EXEC = $(BUILD_DIR)/adiktui

# Définir la cible principale
all: $(ADIKCUI_EXEC) $(ADIKTUI_EXEC)
# all: $(ADIKTUI_EXEC)

# Règle générique pour compiler les .cpp en .o
$(BUILD_DIR)/%.o: $(SRCS_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Règle pour linker adikcui
$(ADIKCUI_EXEC): $(ADIKCUI_OBJS)
	@mkdir -p $(BUILD_DIR)
	@echo "Linking $(ADIKCUI_EXEC)"
	$(CC) $(CFLAGS) $(ADIKCUI_OBJS) $(ADIKCUI_LIBS) -o $(ADIKCUI_EXEC)

# Règle pour linker adiktui
$(ADIKTUI_EXEC): $(ADIKTUI_OBJS)
	@mkdir -p $(BUILD_DIR)
	@echo "Linking $(ADIKTUI_EXEC)"
	$(CC) $(CFLAGS) $(ADIKTUI_OBJS) $(ADIKTUI_LIBS) -o $(ADIKTUI_EXEC)

# Règle de nettoyage
clean:
	rm -rf $(BUILD_DIR)
	@echo "Cleaning build directory"

.PHONY: clean all
