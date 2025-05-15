#include "soundgenerator.h"
#include <cmath>
#include <random>
namespace adikdrum {
const float PI = 3.14159265358979323846f;

// Fonction pour générer du silence
std::vector<float> SoundGenerator::generateSilence(int length) {
    std::vector<float> wave(length, 0.0f);
    return wave;
}

// Fonction pour générer une onde sinusoïdale
std::vector<float> SoundGenerator::generateSine(float frequency, int sampleRate, float durationSec, float amplitude, float attackTime, float releaseTime) {
    int numSamples = static_cast<int>(durationSec * sampleRate);
    std::vector<float> wave(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        float time = static_cast<float>(i) / sampleRate;
        float envelope = 1.0f;
        if (time < attackTime) {
            envelope = time / attackTime;
        } else if (time > durationSec - releaseTime) {
            envelope = (durationSec - time) / releaseTime;
        }
        wave[i] = envelope * amplitude * sinf(2.0f * PI * frequency * time);
    }
    return wave;
}

std::vector<float> SoundGenerator::generateSquare(float frequency, int sampleRate, float durationSec, float amplitude) {
    int numSamples = static_cast<int>(durationSec * sampleRate);
    std::vector<float> wave(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        float time = static_cast<float>(i) / sampleRate;
        wave[i] = amplitude * (sinf(2.0f * PI * frequency * time) >= 0 ? 1.0f : -1.0f);
    }
    return wave;
}

// Fonction pour générer une onde en dents de scie
std::vector<float> SoundGenerator::generateSawtooth(float frequency, int sampleRate, int durationMs) {
    int numSamples = static_cast<int>((durationMs / 1000.0f) * sampleRate);
    std::vector<float> wave(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        float time = static_cast<float>(i) / sampleRate;
        wave[i] = 2.0f * (fmodf(frequency * time, 1.0f) - 0.5f);
    }
    return wave;
}


// Fonction pour générer une onde triangulaire
std::vector<float> SoundGenerator::generateTriangle(float frequency, int sampleRate, int durationMs) {
    int numSamples = static_cast<int>((durationMs / 1000.0f) * sampleRate);
    std::vector<float> wave(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        float time = static_cast<float>(i) / sampleRate;
        float phase = fmodf(frequency * time, 1.0f);
        if (phase < 0.5f) {
            wave[i] = 2.0f * phase - 1.0f;
        } else {
            wave[i] = 1.0f - 2.0f * (phase - 0.5f);
        }
    }
    return wave;
}

// Fonction pour générer une onde cosinus
std::vector<float> SoundGenerator::generateCosine(float frequency, int sampleRate, int durationMs) {
    int numSamples = static_cast<int>((durationMs / 1000.0f) * sampleRate);
    std::vector<float> wave(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        float time = static_cast<float>(i) / sampleRate;
        wave[i] = cosf(2.0f * PI * frequency * time);
    }
    return wave;
}

// Fonction pour générer du bruit blanc
std::vector<float> SoundGenerator::generateWhiteNoise(int sampleRate, float durationSec, float amplitude, float attackTime, float releaseTime) {
    int numSamples = static_cast<int>(durationSec * sampleRate);
    std::vector<float> wave(numSamples);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distrib(-1.0f, 1.0f);
    for (int i = 0; i < numSamples; ++i) {
        float time = static_cast<float>(i) / sampleRate;
        float envelope = 1.0f;
        if (time < attackTime) {
            envelope = time / attackTime;
        } else if (time > durationSec - releaseTime) {
            envelope = (durationSec - time) / releaseTime;
        }
        wave[i] = envelope * amplitude * distrib(gen);
    }
    return wave;
}

} // namespace adikdrum
