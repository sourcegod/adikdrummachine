#include "audiosound.h"
#include <vector>
#include <cstring> // for std::memcpy
#include <cmath>

namespace adikdrum {

AudioSound::AudioSound(std::vector<float> data, size_t numChannels, size_t sampleRate, size_t bitDepth)
    : rawData_(std::move(data)), numChannels_(numChannels),
      sampleRate_(sampleRate), bitDepth_(bitDepth),
      active_(false),
      length_(rawData_.size()) {
    startPos = 0;
    curPos = 0;
    endPos = length_;
}

AudioSound::~AudioSound() {
    // Rien de spécifique à faire ici pour l'instant
}

void AudioSound::setActive(bool active) {
    active_ = active;
}

float AudioSound::getNextSample() {
    if (curPos < endPos) {
        return rawData_[curPos++];
    }
    return 0.0f;
}

size_t AudioSound::readData(std::vector<float>& bufData, size_t numFrames) {
    size_t samplesToRead = numFrames * numChannels_;
    size_t samplesRemaining = endPos - curPos;
    size_t actualSamplesRead = std::min(samplesToRead, samplesRemaining);

    if (actualSamplesRead > 0) {
        const float* sourceBegin = rawData_.data() + curPos;
        float* destBegin = bufData.data();
        std::memcpy(destBegin, sourceBegin, actualSamplesRead * sizeof(float));
        curPos += actualSamplesRead;
    }

    return actualSamplesRead / numChannels_;
}

void AudioSound::applyStaticFadeOutLinear(float fadeOutStartPercent) {
    if (fadeOutStartPercent < 0.0f || fadeOutStartPercent > 1.0f) {
        return;
    }
    size_t fadeOutStart = static_cast<size_t>(length_ * fadeOutStartPercent);
    for (size_t i = fadeOutStart; i < length_; ++i) {
        float amplitude = 1.0f - static_cast<float>(i - fadeOutStart) / (length_ - fadeOutStart);
        rawData_[i] *= amplitude;
    }
}

void AudioSound::applyStaticFadeOutExp(float fadeOutStartPercent, float powerFactor) {
    if (fadeOutStartPercent < 0.0f || fadeOutStartPercent > 1.0f || powerFactor <= 0.0f) {
        return;
    }
    size_t fadeOutStart = static_cast<size_t>(length_ * fadeOutStartPercent);
    for (size_t i = fadeOutStart; i < length_; ++i) {
        float t = static_cast<float>(i - fadeOutStart) / (length_ - fadeOutStart);
        float amplitude = std::pow(1.0f - t, powerFactor);
        rawData_[i] *= amplitude;
    }
}

} // namespace adikdrum
