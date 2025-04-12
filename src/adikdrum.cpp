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
#include "soundgenerator.h" // Inclure le header de SoundGenerator

const double PI = 3.14159265358979323846;

class SoundFactory {
private:
    SoundGenerator generator;
    int sampleRate;
    double defaultDuration;

public:
    SoundFactory(int sr, double duration) : sampleRate(sr), defaultDuration(duration), generator() {}

    std::vector<double> generateKick() {
        double frequency = 60.0;
        return applyEnvelope(generator.generateSineWave(frequency, sampleRate, defaultDuration), 10.0);
    }

    std::vector<double> generateKick2() {
        double frequency = 45.0;
        return applyEnvelope(generator.generateSineWave(frequency, sampleRate, defaultDuration, 0.8), 12.0);
    }

    std::vector<double> generateSnare() {
        return applyNoiseEnvelope(generator.generateWhiteNoise(sampleRate, defaultDuration, 0.5), 20.0);
    }

    std::vector<double> generateSnare2() {
        double highFreq = 440.0 * 2;
        std::vector<double> noise = generator.generateWhiteNoise(sampleRate, defaultDuration, 0.3);
        std::vector<double> tone = generator.generateSineWave(highFreq, sampleRate, defaultDuration, 0.2);
        std::vector<double> result(noise.size());
        std::vector<double> envelope = createEnvelope(sampleRate, defaultDuration, 15.0);
        for (size_t i = 0; i < result.size(); ++i) {
            result[i] = envelope[i] * (noise[i] + tone[i]);
        }
        return result;
    }

    std::vector<double> generateHiHat(double durationScale = 0.25) {
        double duration = defaultDuration * durationScale;
        return applySquareEnvelope(generator.generateSquareWave(440.0 * 4, sampleRate, duration, 0.2), sampleRate, duration, 100.0);
    }

    std::vector<double> generateCymbal(double durationScale = 3.0) {
        double duration = defaultDuration * durationScale;
        return applyNoiseEnvelope(generator.generateWhiteNoise(sampleRate, duration, 0.8), 0.7); // Enveloppe plus lente
    }

    std::vector<double> generateTestTone(double frequency) {
        return generator.generateSineWave(frequency, sampleRate, defaultDuration, 0.4);
    }

private:
    std::vector<double> applyEnvelope(std::vector<double> wave, double decayRate) {
        for (size_t i = 0; i < wave.size(); ++i) {
            double time = static_cast<double>(i) / sampleRate;
            wave[i] *= exp(-time * decayRate);
        }
        return wave;
    }

    std::vector<double> applyNoiseEnvelope(std::vector<double> wave, double decayRate) {
        return applyEnvelope(wave, decayRate); // Réutilise applyEnvelope pour le bruit aussi
    }

    std::vector<double> applySquareEnvelope(std::vector<double> wave, int sr, double dur, double decayRate) {
        for (size_t i = 0; i < wave.size(); ++i) {
            double time = static_cast<double>(i) / sr;
            wave[i] *= exp(-time * decayRate);
        }
        return wave;
    }

    std::vector<double> createEnvelope(int sr, double dur, double decayRate) {
        int numSamples = static_cast<int>(dur * sr);
        std::vector<double> envelope(numSamples);
        for (int i = 0; i < numSamples; ++i) {
            double time = static_cast<double>(i) / sr;
            envelope[i] = exp(-time * decayRate);
        }
        return envelope;
    }
};

class DrumPlayer {
public:
    DrumPlayer(int numSounds) : playing(numSounds, false) {}

    void triggerSound(std::vector<std::vector<double>>& sounds, std::vector<double>::iterator currentSound[], int soundIndex) {
        if (soundIndex >= 0 && soundIndex < sounds.size()) {
            currentSound[soundIndex] = sounds[soundIndex].begin();
            playing[soundIndex] = true;
        }
    }

    double softClip(double x) {
        return tanh(x);
    }

    std::vector<bool> playing;
};

struct DrumMachineData {
    std::vector<std::vector<double>> sounds;
    std::vector<double>::iterator currentSound[8];
    DrumPlayer player;
    int sampleRate;
    //
    // Ajoute ce constructeur !
    DrumMachineData() : player(8) {}
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
        std::vector<std::vector<double>> drumSounds(8);
        // ... (génération des sons) ...
        drumSounds[0] = soundFactory.generateKick();
        drumSounds[1] = soundFactory.generateSnare();
        drumSounds[2] = soundFactory.generateHiHat(0.25);
        drumSounds[3] = soundFactory.generateKick2();
        drumSounds[4] = soundFactory.generateSnare2();
        drumSounds[5] = soundFactory.generateCymbal(3.0);
        drumSounds[6] = soundFactory.generateTestTone(440.0);
        drumSounds[7] = soundFactory.generateTestTone(550.0);

        DrumMachineData drumData;
        drumData.sounds = drumSounds;
        for (int i = 0; i < 8; ++i) {
            drumData.currentSound[i] = drumData.sounds[i].begin();
        }
        drumData.player = DrumPlayer(8); // Initialiser DrumPlayer avec le nombre de sons
        drumData.sampleRate = sampleRate;
        //
        // Plus besoin de créer outputParameters ici !

        err = audioDriver.openStream(sampleRate, drumMachineCallback, &drumData);
        if (err != paNoError) {
            throw std::runtime_error("Erreur lors de l'ouverture du flux audio.");
        }


        err = audioDriver.startStream();
        if (err != paNoError) {
            throw std::runtime_error("Erreur lors du démarrage du flux audio.");
        }

        std::cout << "Test des sons refactorisés avec AudioDriver..." << std::endl;
        for (int i = 0; i < 8; ++i) {
            drumData.player.triggerSound(drumData.sounds, drumData.currentSound, i);
            Pa_Sleep(static_cast<int>(drumData.sounds[i].size() * 1000.0 / sampleRate * 0.8));
        }

        std::cout << "\nAdik Drum Machine interactive! Appuyez sur les touches q, s, d, f, g, h, j, k pour jouer les sons." << std::endl;
        std::cout << "Appuyez sur Entrée pour arrêter." << std::endl;

        std::map<char, int> keyToSoundMap = {
            {'q', 0}, {'s', 1}, {'d', 2}, {'f', 3},
            {'g', 4}, {'h', 5}, {'j', 6}, {'k', 7}
        };


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

