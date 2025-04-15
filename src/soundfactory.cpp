#include "soundfactory.h"
#include "soundgenerator.h" // Assuming SoundGenerator is in its own files
#include <cmath>
#include <random>

const double PI = 3.14159265358979323846;

SoundFactory::SoundFactory(int sr, double duration) : sampleRate(sr), defaultDuration(duration), generator() {}

std::vector<double> SoundFactory::generateKick() {
    double frequency = 60.0;
    return applyEnvelope(generator.generateSineWave(frequency, sampleRate, defaultDuration), 10.0);
}

std::vector<double> SoundFactory::generateKick2() {
    double frequency = 45.0;
    return applyEnvelope(generator.generateSineWave(frequency, sampleRate, defaultDuration, 0.8), 12.0);
}

std::vector<double> SoundFactory::generateSnare() {
    return applyNoiseEnvelope(generator.generateWhiteNoise(sampleRate, defaultDuration, 0.5), 20.0);
}

std::vector<double> SoundFactory::generateSnare2() {
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

std::vector<double> SoundFactory::generateHiHat(double durationScale) {
    double duration = defaultDuration * durationScale;
    return applySquareEnvelope(generator.generateSquareWave(440.0 * 4, sampleRate, duration, 0.2), sampleRate, duration, 100.0);
}

std::vector<double> SoundFactory::generateCymbal(double durationScale) {
    double duration = defaultDuration * durationScale;
    return applyNoiseEnvelope(generator.generateWhiteNoise(sampleRate, duration, 0.8), 0.7);
}

std::vector<double> SoundFactory::generateTestTone(double frequency) {
    return generator.generateSineWave(frequency, sampleRate, defaultDuration, 0.4);
}

std::vector<double> SoundFactory::generateBuzzer(double frequency, int durationMs) {
    std::vector<double> sound(static_cast<int>(sampleRate * durationMs / 1000.0));
    double amplitude = 0.2;
    for (size_t i = 0; i < sound.size(); ++i) {
        double time = static_cast<double>(i) / sampleRate;
        sound[i] = amplitude * sin(2.0 * M_PI * frequency * time);
    }
    return sound;
}

 
std::vector<double> SoundFactory::applyEnvelope(std::vector<double> wave, double decayRate) {
    for (size_t i = 0; i < wave.size(); ++i) {
        double time = static_cast<double>(i) / sampleRate;
        wave[i] *= exp(-time * decayRate);
    }
    return wave;
}

std::vector<double> SoundFactory::applyNoiseEnvelope(std::vector<double> wave, double decayRate) {
    return applyEnvelope(wave, decayRate);
}

std::vector<double> SoundFactory::applySquareEnvelope(std::vector<double> wave, int sr, double dur, double decayRate) {
    for (size_t i = 0; i < wave.size(); ++i) {
        double time = static_cast<double>(i) / sr;
        wave[i] *= exp(-time * decayRate);
    }
    return wave;
}

std::vector<double> SoundFactory::createEnvelope(int sr, double dur, double decayRate) {
    int numSamples = static_cast<int>(dur * sr);
    std::vector<double> envelope(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        double time = static_cast<double>(i) / sr;
        envelope[i] = exp(-time * decayRate);
    }
    return envelope;
}
