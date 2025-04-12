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
#include "audiodriver.h" // Inclure le header de AudioDriver
#include "soundfactory.h" // Inclure le header de SoundFactory
#include "drumplayer.h"

const double PI = 3.14159265358979323846;
const int NUM_SOUNDS = 16; // Notre constante globale pour le nombre de sons

// Mapping des touches et des sons

std::map<char, int> keyToSoundMap = {
    {'q', 0}, {'s', 1}, {'d', 2}, {'f', 3},
    {'g', 4}, {'h', 5}, {'j', 6}, {'k', 7},
    {'a', 8}, {'z', 9}, {'e', 10}, {'r', 11},
    {'t', 12}, {'y', 13}, {'u', 14}, {'i', 15}
};

struct DrumMachineData {
    std::vector<std::vector<double>> sounds;
    std::vector<double>::iterator currentSound[NUM_SOUNDS]; // Utilisation de la constante
    DrumPlayer player;
    int sampleRate;

    // Ajoute ce constructeur !
    DrumMachineData() : player(NUM_SOUNDS), sounds(NUM_SOUNDS) {} // Utilisation de la constante ici aussi
};



static int drumMachineCallback(const void* inputBuffer, void* outputBuffer,
                             unsigned long framesPerBuffer,
                             const PaStreamCallbackTimeInfo* timeInfo,
                             PaStreamCallbackFlags statusFlags,
                             void* userData) {
    DrumMachineData* data = static_cast<DrumMachineData*>(userData);
    float* out = static_cast<float*>(outputBuffer);

    for (unsigned long i = 0; i < framesPerBuffer; ++i) {
        double mixedSample = 0.0;
        for (int j = 0; j < data->sounds.size(); ++j) {
            if (data->player.playing[j] && data->currentSound[j] != data->sounds[j].end()) {
                mixedSample += *data->currentSound[j];
                data->currentSound[j]++;
            } else {
                data->player.playing[j] = false;
            }
        }
        *out++ = static_cast<float>(data->player.softClip(mixedSample * 0.2));
    }
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

int main() {
    AudioDriver audioDriver;
    PaError err = audioDriver.initialize();
    if (err != paNoError) {
        return 1;
    }

    try {
        int sampleRate = 44100;
        double defaultDuration = 0.5;
        SoundFactory soundFactory(sampleRate, defaultDuration);
        std::vector<std::vector<double>> drumSounds(NUM_SOUNDS);
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


        DrumMachineData drumData;
        drumData.sounds = drumSounds;
        for (int i = 0; i < NUM_SOUNDS; ++i) {
            drumData.currentSound[i] = drumData.sounds[i].begin();
        }
        drumData.player = DrumPlayer(NUM_SOUNDS); // Initialiser DrumPlayer avec le nombre de sons
        drumData.sampleRate = sampleRate;

        err = audioDriver.openStream(sampleRate, drumMachineCallback, &drumData);
        if (err != paNoError) {
            throw std::runtime_error("Erreur lors de l'ouverture du flux audio.");
        }


        err = audioDriver.startStream();
        if (err != paNoError) {
            throw std::runtime_error("Erreur lors du démarrage du flux audio.");
        }

        std::cout << "Test des sons refactorisés avec AudioDriver..." << std::endl;
        for (int i = 0; i < NUM_SOUNDS; ++i) {
            drumData.player.triggerSound(drumData.sounds, drumData.currentSound, i);
            Pa_Sleep(static_cast<int>(drumData.sounds[i].size() * 1000.0 / sampleRate * 0.8));
        }

        std::cout << "\nAdik Drum Machine interactive! Appuyez sur les touches q, s, d, f, g, h, j, k pour jouer les sons." << std::endl;
        std::cout << "Appuyez sur Entrée pour arrêter." << std::endl;

        termios oldt = initTermios(0);
        char key;
        while (read(STDIN_FILENO, &key, 1) == 1) {
            if (key == '\n') break;
            if (keyToSoundMap.count(key)) {
                int soundIndex = keyToSoundMap[key];
                drumData.player.triggerSound(drumData.sounds, drumData.currentSound, soundIndex);
            }
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

