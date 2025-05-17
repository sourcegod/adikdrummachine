#include "simpledelay.h"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <iostream>

SimpleDelay::SimpleDelay(size_t bufferSize, float sampleRate)
    : bufferSize_(bufferSize), sampleRate_(sampleRate), 
    writeIndex_(0), delayTimeSec_(0.0f), 
    feedback_(0.0f), gain_(1.0f), 
    active_(false) {
    delayBuffer_.assign(bufferSize_, 0.0f);
    // std::cout << "voici bufferSize: " << bufferSize_ << "\n";
}
//----------------------------------------

SimpleDelay::~SimpleDelay() {}
//----------------------------------------

void SimpleDelay::setDelayTime(float delayTimeSec) {
    delayTimeSec_ = std::max(0.0f, delayTimeSec); // Assure une valeur positive
}
//----------------------------------------

void SimpleDelay::setFeedback(float feedback) {
    feedback_ = std::max(0.0f, std::min(1.0f, feedback)); // Limite entre 0 et 1
}
//----------------------------------------

void SimpleDelay::setGain(float gain) {
    gain_ = gain;
}
//----------------------------------------

void SimpleDelay::setActive(bool active) {
    active_ = active;
}
//----------------------------------------

// Fonction process modifiée pour traiter un buffer:
void SimpleDelay::processData(std::vector<float>& bufData, size_t numFrames, int numChannels) {
    if (!active_) return; // Ne fait rien si le délai est désactivé

    float delayInSamples = delayTimeSec_ * sampleRate_;
    if (delayInSamples < 1) return;
    // std::cout << "voici bufferSize: " << bufferSize_ << "\n";
    // std::cout << "delayInSamples: " << delayInSamples << "\n";
    for (size_t i = 0; i < numFrames; ++i) {
        for (int channel = 0; channel < numChannels; ++channel) {
            size_t sampleIndex = i * numChannels + channel;
            size_t readIndex = (writeIndex_ - static_cast<size_t>(delayInSamples) + bufferSize_) % bufferSize_;

            // Applique le délai à l'échantillon
            float delayedSample = delayBuffer_[readIndex];
            delayBuffer_[writeIndex_] = bufData[sampleIndex] + delayedSample * feedback_;
            
            // Ajoute le délai à l'original
            bufData[sampleIndex] += delayedSample * gain_; 
            // Mise à jour de l'index d'écriture
            writeIndex_ = (writeIndex_ + 1) % bufferSize_;
        }

    }
}
//----------------------------------------

//==== End of class SimpleDelay ====

