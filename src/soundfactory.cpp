#include "soundfactory.h"
#include "audiosound.h"
#include <cmath>
#include <vector>
#include <memory>
#include "soundgenerator.h"

namespace adikdrum {

SoundFactory::SoundFactory(int sampleRate, float defaultDuration) :
    sampleRate_(sampleRate),
    defaultDuration_(defaultDuration),
    generator_() {}

SoundFactory::~SoundFactory() {}

SoundPtr SoundFactory::applyEnvelopeToAudioSound(SoundPtr audioSound, float decayRate) {
    std::vector<float>& wave = audioSound->getRawData();
    for (size_t i = 0; i < wave.size(); ++i) {
        float time = static_cast<float>(i) / sampleRate_;
        wave[i] *= expf(-time * decayRate);
    }
    return audioSound;
}

SoundPtr SoundFactory::applyNoiseEnvelopeToAudioSound(SoundPtr audioSound, float decayRate) {
    return applyEnvelopeToAudioSound(audioSound, decayRate);
}

SoundPtr SoundFactory::applySquareEnvelopeToAudioSound(SoundPtr audioSound, float duration, float decayRate) {
    (void)duration;
    std::vector<float>& wave = audioSound->getRawData();
    for (size_t i = 0; i < wave.size(); ++i) {
        float time = static_cast<float>(i) / sampleRate_;
        wave[i] *= expf(-time * decayRate);
    }
    return audioSound;
}

SoundPtr SoundFactory::createEnvelopeForAudioSound(float duration, float decayRate) {
    (void)duration;
    int numSamples = static_cast<int>(duration * sampleRate_);
    std::vector<float> envelope(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        float time = static_cast<float>(i) / sampleRate_;
        envelope[i] = expf(-time * decayRate);
    }
    return std::make_shared<AudioSound>(envelope, 1, sampleRate_, 16); // Utilisation du constructeur de AudioSound et SoundPtr
}

std::vector<float> SoundFactory::createEnvelope(int sr, float dur, float decayRate) {
    int numSamples = static_cast<int>(dur * sr);
    std::vector<float> envelope(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        float time = static_cast<float>(i) / sr;
        envelope[i] = expf(-time * decayRate);
    }
    return envelope;
}

std::vector<float> SoundFactory::applyEnvelope(const std::vector<float>& wave, float decayRate) {
    std::vector<float> envelopedWave = wave;
    for (size_t i = 0; i < envelopedWave.size(); ++i) {
        float time = static_cast<float>(i) / sampleRate_;
        envelopedWave[i] *= expf(-time * decayRate);
    }
    return envelopedWave;
}

SoundPtr SoundFactory::generateKick() {
    float frequency = 60.0f;
    std::vector<float> wave = generator_.generateSine(frequency, sampleRate_, defaultDuration_);
    return applyEnvelopeToAudioSound(std::make_shared<AudioSound>(wave, 1, sampleRate_, 16), 10.0f); // Utilisation du constructeur de AudioSound et SoundPtr
}

SoundPtr SoundFactory::generateKick2() {
    float frequency = 45.0f;
    std::vector<float> wave = generator_.generateSine(frequency, sampleRate_, defaultDuration_, 0.8f);
    return applyEnvelopeToAudioSound(std::make_shared<AudioSound>(wave, 1, sampleRate_, 16), 12.0f); // Utilisation du constructeur de AudioSound et SoundPtr
}

SoundPtr SoundFactory::generateSnare() {
    std::vector<float> wave = generator_.generateWhiteNoise(sampleRate_, defaultDuration_, 0.5f);
    return applyNoiseEnvelopeToAudioSound(std::make_shared<AudioSound>(wave, 1, sampleRate_, 16), 20.0f); // Utilisation du constructeur de AudioSound et SoundPtr
}

SoundPtr SoundFactory::generateSnare2() {
    float highFreq = 440.0f * 2;
    std::vector<float> noise = generator_.generateWhiteNoise(sampleRate_, defaultDuration_, 0.3f);
    std::vector<float> tone = generator_.generateSine(highFreq, sampleRate_, defaultDuration_, 0.2f);
    std::vector<float> result(noise.size());
    std::vector<float> envelope = createEnvelope(sampleRate_, defaultDuration_, 15.0f);
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = envelope[i] * (noise[i] + tone[i]);
    }
    return std::make_shared<AudioSound>(result, 1, sampleRate_, 16); // Utilisation du constructeur de AudioSound et SoundPtr
}

SoundPtr SoundFactory::generateHiHat(float durationScale) {
    float duration = defaultDuration_ * durationScale;
    std::vector<float> wave = generator_.generateSquare(440.0f * 4, sampleRate_, duration, 0.2f);
    return applySquareEnvelopeToAudioSound(std::make_shared<AudioSound>(wave, 1, sampleRate_, 16), duration, 100.0f); // Utilisation du constructeur de AudioSound et SoundPtr
}

SoundPtr SoundFactory::generateCymbal(float durationScale) {
    float duration = defaultDuration_ * durationScale;
    std::vector<float> wave = generator_.generateWhiteNoise(sampleRate_, duration, 0.8f);
    return applyNoiseEnvelopeToAudioSound(std::make_shared<AudioSound>(wave, 1, sampleRate_, 16), 0.7f); // Utilisation du constructeur de AudioSound et SoundPtr
}

SoundPtr SoundFactory::generateTestTone(float frequency, float duration) {
    std::vector<float> wave = generator_.generateSine(frequency, sampleRate_, duration, 0.4f);
    return std::make_shared<AudioSound>(wave, 1, sampleRate_, 16); // Utilisation du constructeur de AudioSound et SoundPtr
}

SoundPtr SoundFactory::generateBuzzer(float frequency, float duration) {
    float durationSec = duration / 1000.0f;
    std::vector<float> wave = generator_.generateSine(frequency, sampleRate_, durationSec, 0.2f, 0.005f, 0.005f);
    return std::make_shared<AudioSound>(wave, 1, sampleRate_, 16); // Utilisation du constructeur de AudioSound et SoundPtr
}

SoundPtr SoundFactory::tone(const std::string& type, float frequency, float length) {
    if (type == "sine") {
        return std::make_shared<AudioSound>(generator_.generateSine(frequency, sampleRate_, length));
    } else if (type == "cosine") {
        return std::make_shared<AudioSound>(generator_.generateCosine(frequency, sampleRate_, length));
    } else if (type == "square") {
        return std::make_shared<AudioSound>(generator_.generateSquare(frequency, sampleRate_, length));
    } else if (type == "sawtooth") {
        return std::make_shared<AudioSound>(generator_.generateSawtooth(frequency, sampleRate_, length));
    } else if (type == "triangle") {
        return std::make_shared<AudioSound>(generator_.generateTriangle(frequency, sampleRate_, length));
    } else if (type == "silence") {
        return std::make_shared<AudioSound>(generator_.generateSilence(length));
    } else {
        throw std::invalid_argument("Type de son non valide : " + type);
    }
}

} // namespace adikdrum
