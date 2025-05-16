#include "simpledelay.h"
#include <cmath>
#include <iostream>
#include <algorithm>

SimpleDelay::SimpleDelay(size_t bufferSize, float sampleRate)
    : bufferSize_(bufferSize), sampleRate_(sampleRate), writeIndex_(0),
      delayTimeMs_(0.0f), feedback_(0.0f), gain_(1.0f), isEnabled_(false) {
    delayBuffer_.assign(bufferSize_, 0.0f);
}

SimpleDelay::~SimpleDelay() {}

void SimpleDelay::setDelayTime(float delayTimeMs) {
    delayTimeMs_ = std::max(0.0f, delayTimeMs); // Assure une valeur positive
}

void SimpleDelay::setFeedback(float feedback) {
    feedback_ = std::max(0.0f, std::min(1.0f, feedback)); // Limite entre 0 et 1
}

void SimpleDelay::setGain(float gain) {
    gain_ = gain;
}

void SimpleDelay::setIsEnabled(bool isEnabled) {
    isEnabled_ = isEnabled;
}

// Fonction process modifiée pour traiter un buffer:
void SimpleDelay::process(std::vector<float>& buffer, size_t numFrames, int numChannels) {
    if (!isEnabled_) {
        return; // Ne fait rien si le délai est désactivé
    }

    float delayInSamples = delayTimeMs_ * sampleRate_ / 1000.0f;
    if (delayInSamples < 1)
        return;

    for (size_t i = 0; i < numFrames; ++i) {
        for (int channel = 0; channel < numChannels; ++channel) {
            size_t sampleIndex = i * numChannels + channel;
            size_t readIndex = (writeIndex_ - static_cast<size_t>(delayInSamples) + bufferSize_) % bufferSize_;

            // Applique le délai à l'échantillon
            float delayedSample = delayBuffer_[readIndex];
            delayBuffer_[writeIndex_] = buffer[sampleIndex] + delayedSample * feedback_;
            buffer[sampleIndex] = buffer[sampleIndex] + delayedSample * gain_; // Ajoute le délai à l'original

            writeIndex_ = (writeIndex_ + 1) % bufferSize_;
        }
    }
}

