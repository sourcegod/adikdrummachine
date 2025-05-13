#ifndef SOUNDGENERATOR_H
#define SOUNDGENERATOR_H

#include <vector>
#include <cmath>
#include <random>

namespace adikdrum {

class SoundGenerator {
public:
    std::vector<float> generateSilence(int length);
    std::vector<float> generateSineWave(float frequency, int sampleRate, float durationSec, float amplitude = 1.0f, float attackTime = 0.01f, float releaseTime = 0.1f);
    std::vector<float> generateSquareWave(float frequency, int sampleRate, float durationSec, float amplitude = 1.0f);
    std::vector<float> generateSawtoothWave(float frequency, int sampleRate, int durationMs);
    std::vector<float> generateTriangleWave(float frequency, int sampleRate, int durationMs);
    std::vector<float> generateCosineWave(float frequency, int sampleRate, int durationMs);
    std::vector<float> generateWhiteNoise(int sampleRate, float durationSec, float amplitude = 1.0f, float attackTime = 0.01f, float releaseTime = 0.1f);
    //
    // Tu peux ajouter d'autres déclarations de fonctions de génération d'ondes ici
};

} // namespace adikdrum

#endif // SOUNDGENERATOR_H


