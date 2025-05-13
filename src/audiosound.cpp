#include "audiosound.h"
#include <vector>
#include <cstring> // for std::memcpy
#include <cmath>
#include <iostream>

namespace adikdrum {

AudioSound::AudioSound(std::vector<float> data, size_t numChannels, size_t sampleRate, size_t bitDepth)
    : rawData_(std::move(data)), numChannels_(numChannels),
      sampleRate_(sampleRate), 
      bitDepth_(bitDepth),
      length_(rawData_.size()),
      active_(false) {
    startPos = 0;
    curPos = 0;
    endPos = length_;
}
//----------------------------------------

AudioSound::~AudioSound() {
    // Rien de spécifique à faire ici pour l'instant
}
//----------------------------------------

void AudioSound::setActive(bool active) {
    active_ = active;
}
//----------------------------------------

float AudioSound::getNextSample() {
    if (curPos < endPos) {
        return rawData_[curPos++];
    }
    return 0.0f;
}
//----------------------------------------

// with linear interpolation  for pitch
size_t AudioSound::readData(std::vector<float>& bufData, size_t numFrames) {
    if (!active_) return 0;

    size_t framesRead = 0;
    size_t bufferIndex = 0;
    float currentSamplePos = static_cast<float>(curPos);
    while (framesRead < numFrames && currentSamplePos < endPos / numChannels_) {
        if (speed_ == 1.0f) {
            // Si la vitesse est de 1.0, on lit directement l'échantillon sans interpolation
            size_t sourceIndex = static_cast<size_t>(currentSamplePos);
            for (size_t channel = 0; channel < numChannels_; ++channel) {
                bufData[bufferIndex++] = rawData_[sourceIndex * numChannels_ + channel];
            }
            currentSamplePos += 1.0f;
        } else {
            // Sinon, on effectue l'interpolation linéaire comme avant
            size_t sourceIndex = static_cast<size_t>(std::floor(currentSamplePos));
            float sampleWeight = currentSamplePos - sourceIndex;

            for (size_t channel = 0; channel < numChannels_; ++channel) {
                float sample1 = rawData_[sourceIndex * numChannels_ + channel];
                float sample2 = (sourceIndex + 1 < endPos / numChannels_)
                                  ? rawData_[(sourceIndex + 1) * numChannels_ + channel]
                                  : sample1;

                float interpolatedSample =
                    sample1 * (1 - sampleWeight) + sample2 * sampleWeight;
                bufData[bufferIndex++] = interpolatedSample;
            }
            currentSamplePos += speed_;
        }
        curPos = static_cast<size_t>(std::floor(currentSamplePos));
        framesRead++;
    }
    return framesRead;
}
//----------------------------------------

/*
// Without speed
size_t AudioSound::readData(std::vector<float>& bufData, size_t numFrames) {
    size_t samplesToRead = numFrames * numChannels_;
    size_t samplesRemaining = endPos - curPos;
    size_t actualSamplesRead = std::min(samplesToRead, samplesRemaining);
    // std::cout << "In ReadData, numChannels: " << numChannels_ << "\n";
    // std::cout << "In ReadData, samplesToRead: " << samplesToRead << ", samplesRemaiNing: " << samplesRemaining << ", actualSamplesRead: " << actualSamplesRead << "\n";

    if (actualSamplesRead > 0) {
        const float* sourceBegin = rawData_.data() + curPos;
        float* destBegin = bufData.data();
        std::memcpy(destBegin, sourceBegin, actualSamplesRead * sizeof(float));
        curPos += actualSamplesRead;
    }

    return actualSamplesRead / numChannels_;
}
//----------------------------------------
*/

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
//----------------------------------------

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
//----------------------------------------
//==== End of class AudioSound ====

} // namespace adikdrum
