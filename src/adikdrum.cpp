/*
 *  File: adikdrum.cpp
 *  Drum Machine Engine in C++
 *  Compile: g++ adikdrum.cpp -o adikdrum -lportaudio -lpthread
 *  Date: Sat, 12/04/2025
 *  Author: Coolbrother
 *  */

#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <portaudio.h>
#include <algorithm>
#include <map>
#include <termios.h>
#include <unistd.h>
#include <utility> // Pour utiliser std::pair

// for performance checking
#include <chrono>
#include <thread>

#include "audiodriver.h" // Inclure le header de AudioDriver
#include "soundfactory.h" // Inclure le header de SoundFactory
#include "drumplayer.h"

const double PI = 3.14159265358979323846;
const int NUM_SOUNDS = 16; // Notre constante globale pour le nombre de sons
const int NUM_STEPS = 16;
std::pair<int, int> cursor_pos = {0, 0}; // {x, y}
std::vector<std::vector<bool>> pattern(NUM_SOUNDS, std::vector<bool>(NUM_STEPS, false));
volatile int callbackCounter =0;

// Mapping des touches et des sons
std::map<char, int> keyToSoundMap = {
    {'q', 0}, {'s', 1}, {'d', 2}, {'f', 3},
    {'g', 4}, {'h', 5}, {'j', 6}, {'k', 7},
    {'a', 8}, {'z', 9}, {'e', 10}, {'r', 11},
    {'t', 12}, {'y', 13}, {'u', 14}, {'i', 15}
};

struct DrumMachineData {
    DrumPlayer player;
    int sampleRate;

    // Ajoute ce constructeur !
    // DrumMachineData() : player(NUM_SOUNDS, 100), sounds(NUM_SOUNDS), sampleRate(44100) {}
    DrumMachineData(const std::vector<std::vector<double>>& sounds) 
      : player(NUM_SOUNDS +2, 100, sounds), 
      sampleRate(44100) {}

};

void beep() {
    std::cout << '\a' << std::flush;
}

static int drumMachineCallback(const void* inputBuffer, void* outputBuffer,
                             unsigned long framesPerBuffer,
                             const PaStreamCallbackTimeInfo* timeInfo,
                             PaStreamCallbackFlags statusFlags,
                             void* userData) {
    DrumMachineData* data = static_cast<DrumMachineData*>(userData);
    float* out = static_cast<float*>(outputBuffer);
    static unsigned long frameCounter = 0;

    if (data->player.isPlaying) {
        unsigned long samplesPerStep = static_cast<unsigned long>(data->sampleRate * data->player.secondsPerStep);

        if (frameCounter >= samplesPerStep) {
            data->player.currentStep = (data->player.currentStep + 1) % NUM_STEPS;
            frameCounter = 0;
            // beep();
            // Jouer le métronome seulement tous les 4 pas
            if (data->player.currentStep % 4 == 0) {
                data->player.playMetronome();
            }

            for (int i = 0; i < NUM_SOUNDS; ++i) {
                if (pattern[i][data->player.currentStep]) {
                    data->player.playSound(i); // Utilise l'index directement

                }
            }
        }
    }

    for (unsigned long i = 0; i < framesPerBuffer; ++i) {
        double mixedSample = 0.0;
        for (int j = 0; j < NUM_SOUNDS +2; ++j) { // Utilise NUM_SOUNDS ici
          if (data->player.playing[j] && data->player.currentSound_[j] != data->player.drumSounds_[j].end()) {
              mixedSample += *data->player.currentSound_[j];
              data->player.currentSound_[j]++;
   
            } else {
                data->player.playing[j] = false;
            }
        }

        *out++ = static_cast<float>(data->player.softClip(mixedSample * 0.2));
    }

    if (data->player.isPlaying) {
        frameCounter += framesPerBuffer;
    }

    // beep();
    // callbackCounter++;
    return paContinue;
}

// Fonction pour initialiser le terminal
termios initTermios(int echo) {
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~(ICANON);
    if (echo == 0) {
        newt.c_lflag &= ~(ECHO);
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    return oldt;
}

// Fonction pour restaurer les paramètres du terminal
void resetTermios(termios oldt) {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

void displayGrid(const std::vector<std::vector<bool>>& grid, std::pair<int, int> cursor) {
    // std::cout << "\033[2J\033[H";
    std::cout << "  ";
    for (int i = 0; i < NUM_STEPS; ++i) {
        std::cout << (i + 1) % 10 << " ";
    }
    std::cout << std::endl;
    for (int i = 0; i < NUM_SOUNDS; ++i) {
        std::cout << (i + 1) % 10 << " ";
        for (int j = 0; j < NUM_STEPS; ++j) {
            if (cursor.first == j && cursor.second == i) {
                std::cout << "x ";
            } else if (grid[i][j]) {
                std::cout << "# "; // Affiche '#' si le pas est activé
            } else {
                std::cout << "- "; // Affiche '-' si le pas est désactivé
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    AudioDriver audioDriver;
    // DrumMachineData drumData;

    PaError err = audioDriver.initialize();
    if (err != paNoError) {
        return 1;
    }

    try {
        int sampleRate = 44100;
        double defaultDuration = 0.5;
        SoundFactory soundFactory(sampleRate, defaultDuration);
        std::vector<std::vector<double>> drumSounds(NUM_SOUNDS + 2); // +2 pour les sons du métronome

        // ... (génération des sons) ...
        drumSounds[0] = soundFactory.generateKick();
        drumSounds[1] = soundFactory.generateSnare();
        drumSounds[2] = soundFactory.generateHiHat(0.25);
        drumSounds[3] = soundFactory.generateKick2();
        drumSounds[4] = soundFactory.generateSnare2();
        drumSounds[5] = soundFactory.generateCymbal(3.0);
        drumSounds[6] = soundFactory.generateTestTone(440.0);
        drumSounds[7] = soundFactory.generateTestTone(550.0);
        drumSounds[8] = soundFactory.generateTestTone(220.0); // Exemple pour RimShot
        drumSounds[9] = soundFactory.generateTestTone(330.0); // Exemple pour HandClap
        drumSounds[10] = soundFactory.generateHiHat(0.5); // Exemple pour HiHatOpen
        drumSounds[11] = soundFactory.generateTestTone(110.0); // Exemple pour LowTom
        drumSounds[12] = soundFactory.generateTestTone(165.0); // Exemple pour MidTom
        drumSounds[13] = soundFactory.generateTestTone(275.0); // Exemple pour HiTom
        drumSounds[14] = soundFactory.generateTestTone(660.0); // Exemple pour CowBell
        drumSounds[15] = soundFactory.generateTestTone(880.0); // Exemple pour Tambourine
        
        // Générer les sons du métronome
        drumSounds[16] = soundFactory.generateBuzzer(880.0, 50); // Son aigu
        drumSounds[17] = soundFactory.generateBuzzer(440.0, 50); // Son grave


        DrumMachineData drumData(drumSounds); // Passe la liste des sons au constructeur
        drumData.sampleRate = sampleRate;

        err = audioDriver.openStream(sampleRate, drumMachineCallback, &drumData);
        if (err != paNoError) {
            throw std::runtime_error("Erreur lors de l'ouverture du flux audio.");
        }


        err = audioDriver.startStream();
        if (err != paNoError) {
            throw std::runtime_error("Erreur lors du démarrage du flux audio.");
        }
        
        /*
        std::cout << "Vérification de la fréquence du callback..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Attend une seconde
        std::cout << "Nombre d'appels du callback en 1 seconde : " << callbackCounter << std::endl;

        // audioDriver.stopStream();
        // Réinitialise le compteur pour une prochaine vérification si tu le souhaites
        // callbackCounter = 0;
        */


        std::cout << "Test des sons refactorisés avec AudioDriver..." << std::endl;
        for (int i = 0; i < NUM_SOUNDS +2; ++i) {
            drumData.player.playSound(i);
            Pa_Sleep(static_cast<int>(drumData.player.drumSounds_[i].size() * 1000.0 / sampleRate * 0.8));
        }


        std::cout << "\nAdik Drum Machine interactive! Appuyez sur les touches q, s, d, f, g, h, j, k pour jouer les sons." << std::endl;
        std::cout << "Appuyez sur Entrée pour arrêter." << std::endl;

        termios oldt = initTermios(0);

        // displayGrid(pattern, cursor_pos); // Affiche la grille après chaque action
        char key;
        while (read(STDIN_FILENO, &key, 1) == 1) {
            if (key == 'X') break;

            if (key == '\n') { // Touche Enter
                pattern[cursor_pos.second][cursor_pos.first] = true; // Active toujours le pas
                std::cout << "Step " << cursor_pos.first + 1 << " on sound " << cursor_pos.second + 1 << " activated and playing." << std::endl;
                drumData.player.playSound(cursor_pos.second); // Utilise l'index directement

            } else if (key == 127) { // Touche Backspace (code ASCII 127)
                pattern[cursor_pos.second][cursor_pos.first] = false;
                std::cout << "Step " << cursor_pos.first + 1 << " on sound " << cursor_pos.second + 1 << " deactivated." << std::endl;
            } else if (key == ' ') { // Touche Espace
                drumData.player.isPlaying = !drumData.player.isPlaying;
                std::cout << "Play: " << (drumData.player.isPlaying ? "ON" : "OFF") << std::endl;
            } else if (key == 'v') {
                drumData.player.stopAllSounds();
                std::cout << "All sounds stopped." << std::endl;

            } else if (key == '(') {
                if (drumData.player.bpm > 5) {
                    drumData.player.setBpm(drumData.player.bpm - 5);
                    std::cout << "BPM decreased to " << drumData.player.bpm << std::endl;
                } else {
                    beep();
                    std::cout << "Minimum BPM reached." << std::endl;
                }
            } else if (key == ')') {
                if (drumData.player.bpm < 800) {
                    drumData.player.setBpm(drumData.player.bpm + 5);
                    std::cout << "BPM increased to " << drumData.player.bpm << std::endl;
                } else {
                    beep();
                    std::cout << "Maximum BPM reached." << std::endl;
                }

            } else if (keyToSoundMap.count(key)) {
                int soundIndex = keyToSoundMap[key];
                drumData.player.playSound(soundIndex); // Utilise l'index directement

            } else if (key == '\033') { // Code d'échappement pour les séquences de touches spéciales (comme les flèches)
                read(STDIN_FILENO, &key, 1); // Lit le caractère '['
                read(STDIN_FILENO, &key, 1); // Lit le code de la flèche
                if (key == 'A') { // Flèche haut
                    if (cursor_pos.second > 0) {
                        cursor_pos.second--;
                        displayGrid(pattern, cursor_pos); // Affiche la grille après chaque action

                    } else {
                        beep();
                    }
                    drumData.player.playSound(cursor_pos.second); // Utilise l'index directement
                    std::cout << "Cursor up, playing sound " << cursor_pos.second << std::endl;
                } else if (key == 'B') { // Flèche bas
                    if (cursor_pos.second < NUM_SOUNDS - 1) {
                        cursor_pos.second++;
                        displayGrid(pattern, cursor_pos); // Affiche la grille après chaque action
                    } else {
                        beep();
                    }
                    drumData.player.playSound(cursor_pos.second); // Utilise l'index directement
                    std::cout << "Cursor down, playing sound " << cursor_pos.second << std::endl;

                } else if (key == 'C') { // Flèche droite
                    if (cursor_pos.first < NUM_STEPS - 1) {
                        cursor_pos.first++;
                        displayGrid(pattern, cursor_pos); // Affiche la grille après chaque action
                        std::cout << "Cursor right, step " << cursor_pos.first << std::endl;
                    } else {
                        beep();
                        std::cout << "Reached the end (right)." << std::endl;
                    }

                } else if (key == 'D') { // Flèche gauche
                    if (cursor_pos.first > 0) {
                        cursor_pos.first--;
                        displayGrid(pattern, cursor_pos); // Affiche la grille après chaque action
                        std::cout << "Cursor left, step " << cursor_pos.first << std::endl;
                    } else {
                        beep();
                        std::cout << "Reached the beginning (left)." << std::endl;
                    }

                }
            }
        
          // beep();
        
        }
        resetTermios(oldt);

        audioDriver.stopStream();
        audioDriver.closeStream();

    } catch (const std::runtime_error& e) {
        std::cerr << "Erreur: " << e.what() << std::endl;
    }

    audioDriver.terminate();
    return 0;
}

