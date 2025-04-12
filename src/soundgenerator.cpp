#include "soundgenerator.h"
#include <cmath>
#include <random>

const double PI = 3.14159265358979323846;

std::vector<double> SoundGenerator::generateSineWave(double frequency, int sampleRate, double durationSec, double amplitude, double attackTime, double releaseTime) {
    int numSamples = static_cast<int>(durationSec * sampleRate);
    std::vector<double> wave(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        double time = static_cast<double>(i) / sampleRate;
        double envelope = 1.0;
        if (time < attackTime) {
            envelope = time / attackTime;
        } else if (time > durationSec - releaseTime) {
            envelope = (durationSec - time) / releaseTime;
        }
        wave[i] = envelope * amplitude * sin(2.0 * PI * frequency * time);
    }
    return wave;
}

std::vector<double> SoundGenerator::generateSquareWave(double frequency, int sampleRate, double durationSec, double amplitude) {
    int numSamples = static_cast<int>(durationSec * sampleRate);
    std::vector<double> wave(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        double time = static_cast<double>(i) / sampleRate;
        wave[i] = amplitude * (sin(2.0 * PI * frequency * time) >= 0 ? 1.0 : -1.0);
    }
    return wave;
}

std::vector<double> SoundGenerator::generateWhiteNoise(int sampleRate, double durationSec, double amplitude, double attackTime, double releaseTime) {
    int numSamples = static_cast<int>(durationSec * sampleRate);
    std::vector<double> wave(numSamples);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distrib(-1.0, 1.0);
    for (int i = 0; i < numSamples; ++i) {
        double time = static_cast<double>(i) / sampleRate;
        double envelope = 1.0;
        if (time < attackTime) {
            envelope = time / attackTime;
        } else if (time > durationSec - releaseTime) {
            envelope = (durationSec - time) / releaseTime;
        }
        wave[i] = envelope * amplitude * distrib(gen);
    }
    return wave;
}

// Tu peux ajouter d'autres implémentations de fonctions de génération d'ondes ici
