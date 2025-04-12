#ifndef SOUNDGENERATOR_H
#define SOUNDGENERATOR_H

#include <vector>
#include <cmath>
#include <random>

class SoundGenerator {
public:
    std::vector<double> generateSilence(int length);
    std::vector<double> generateSineWave(double frequency, int sampleRate, double durationSec, double amplitude = 1.0, double attackTime = 0.01, double releaseTime = 0.1);
    std::vector<double> generateSquareWave(double frequency, int sampleRate, double durationSec, double amplitude = 1.0);
    std::vector<double> generateSawtoothWave(double frequency, int sampleRate, int durationMs);
    std::vector<double> generateTriangleWave(double frequency, int sampleRate, int durationMs);
    std::vector<double> generateCosineWave(double frequency, int sampleRate, int durationMs);
    std::vector<double> generateWhiteNoise(int sampleRate, double durationSec, double amplitude = 1.0, double attackTime = 0.01, double releaseTime = 0.1);
    //
    // Tu peux ajouter d'autres déclarations de fonctions de génération d'ondes ici
};

#endif // SOUNDGENERATOR_H
