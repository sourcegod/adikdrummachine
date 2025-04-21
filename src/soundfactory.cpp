#include "soundfactory.h"
#include "audiosound.h"
#include <cmath>
#include <vector>
#include <memory>
#include "soundgenerator.h"

SoundFactory::SoundFactory(int sampleRate, double defaultDuration) :
    sampleRate_(sampleRate),
    defaultDuration_(defaultDuration),
    generator_() {}

SoundFactory::~SoundFactory() {}

std::shared_ptr<AudioSound> SoundFactory::applyEnvelopeToAudioSound(std::shared_ptr<AudioSound> audioSound, double decayRate) {
    std::vector<double>& wave = audioSound->getRawData();
    for (size_t i = 0; i < wave.size(); ++i) {
        double time = static_cast<double>(i) / sampleRate_;
        wave[i] *= exp(-time * decayRate);
    }
    return audioSound;
}

std::shared_ptr<AudioSound> SoundFactory::applyNoiseEnvelopeToAudioSound(std::shared_ptr<AudioSound> audioSound, double decayRate) {
    return applyEnvelopeToAudioSound(audioSound, decayRate);
}

std::shared_ptr<AudioSound> SoundFactory::applySquareEnvelopeToAudioSound(std::shared_ptr<AudioSound> audioSound, double duration, double decayRate) {
    std::vector<double>& wave = audioSound->getRawData();
    for (size_t i = 0; i < wave.size(); ++i) {
        double time = static_cast<double>(i) / sampleRate_;
        wave[i] *= exp(-time * decayRate);
    }
    return audioSound;
}

std::shared_ptr<AudioSound> SoundFactory::createEnvelopeForAudioSound(double duration, double decayRate) {
    int numSamples = static_cast<int>(duration * sampleRate_);
    std::vector<double> envelope(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        double time = static_cast<double>(i) / sampleRate_;
        envelope[i] = exp(-time * decayRate);
    }
    return std::make_shared<AudioSound>(envelope); // Retourne un AudioSound contenant l'enveloppe (ce n'est probablement pas ce que tu veux faire ici)
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


std::vector<double> SoundFactory::applyEnvelope(const std::vector<double>& wave, double decayRate) {
    std::vector<double> envelopedWave = wave;
    for (size_t i = 0; i < envelopedWave.size(); ++i) {
        double time = static_cast<double>(i) / sampleRate_;
        envelopedWave[i] *= exp(-time * decayRate);
    }
    return envelopedWave;
}


std::shared_ptr<AudioSound> SoundFactory::generateKick() {
    double frequency = 60.0;
    std::vector<double> wave = generator_.generateSineWave(frequency, sampleRate_, defaultDuration_);
    return applyEnvelopeToAudioSound(std::make_shared<AudioSound>(wave), 10.0);
}

std::shared_ptr<AudioSound> SoundFactory::generateKick2() {
    double frequency = 45.0;
    std::vector<double> wave = generator_.generateSineWave(frequency, sampleRate_, defaultDuration_, 0.8);
    return applyEnvelopeToAudioSound(std::make_shared<AudioSound>(wave), 12.0);
}

std::shared_ptr<AudioSound> SoundFactory::generateSnare() {
    std::vector<double> wave = generator_.generateWhiteNoise(sampleRate_, defaultDuration_, 0.5);
    return applyNoiseEnvelopeToAudioSound(std::make_shared<AudioSound>(wave), 20.0);
}

std::shared_ptr<AudioSound> SoundFactory::generateSnare2() {
    double highFreq = 440.0 * 2;
    std::vector<double> noise = generator_.generateWhiteNoise(sampleRate_, defaultDuration_, 0.3);
    std::vector<double> tone = generator_.generateSineWave(highFreq, sampleRate_, defaultDuration_, 0.2);
    std::vector<double> result(noise.size());
    std::vector<double> envelope = createEnvelope(sampleRate_, defaultDuration_, 15.0);
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = envelope[i] * (noise[i] + tone[i]);
    }
    return std::make_shared<AudioSound>(result);
}

std::shared_ptr<AudioSound> SoundFactory::generateHiHat(double durationScale) {
    double duration = defaultDuration_ * durationScale;
    std::vector<double> wave = generator_.generateSquareWave(440.0 * 4, sampleRate_, duration, 0.2);
    return applySquareEnvelopeToAudioSound(std::make_shared<AudioSound>(wave), duration, 100.0);
}

std::shared_ptr<AudioSound> SoundFactory::generateCymbal(double durationScale) {
    double duration = defaultDuration_ * durationScale;
    std::vector<double> wave = generator_.generateWhiteNoise(sampleRate_, duration, 0.8);
    return applyNoiseEnvelopeToAudioSound(std::make_shared<AudioSound>(wave), 0.7);
}

std::shared_ptr<AudioSound> SoundFactory::generateTestTone(double frequency=440.0, double duration=0.5) {
    std::vector<double> wave = generator_.generateSineWave(frequency, sampleRate_, duration, 0.4);
    return std::make_shared<AudioSound>(wave);
}

std::shared_ptr<AudioSound> SoundFactory::generateBuzzer(double frequency, double duration) {
    double durationSec = duration / 1000.0;
    std::vector<double> wave = generator_.generateSineWave(frequency, sampleRate_, durationSec, 0.2, 0.005, 0.005);
    return std::make_shared<AudioSound>(wave);
}

