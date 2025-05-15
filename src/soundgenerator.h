#ifndef SOUNDGENERATOR_H
#define SOUNDGENERATOR_H

#include <vector>
#include <cmath>
#include <random>

namespace adikdrum {

class SoundGenerator {
public:
    std::vector<float> generateSilence(int length);
    std::vector<float> generateSine(float frequency, int sampleRate, float durationSec, float amplitude = 1.0f, float attackTime = 0.01f, float releaseTime = 0.1f);
    std::vector<float> generateSquare(float frequency, int sampleRate, float durationSec, float amplitude = 1.0f);
    std::vector<float> generateSawtooth(float frequency, int sampleRate, int durationMs);
    std::vector<float> generateTriangle(float frequency, int sampleRate, int durationMs);
    std::vector<float> generateCosine(float frequency, int sampleRate, int durationMs);
    std::vector<float> generateWhiteNoise(int sampleRate, float durationSec, float amplitude = 1.0f, float attackTime = 0.01f, float releaseTime = 0.1f);
    //
    // Tu peux ajouter d'autres déclarations de fonctions de génération d'ondes ici
};

//==== End of class SoundGenerator ====

} // namespace adikdrum

#endif // SOUNDGENERATOR_H


