#include "audiosound.h"
#include <cmath> // for std::pow
#include <algorithm> // Pour std::min
#include <vector>
#include <iostream>

AudioSound::AudioSound(std::vector<double> data, size_t numChannels) 
    : rawData_(std::move(data)), numChannels_(numChannels),
    active_(false),
    length_(data.size()) {
      
    length_ = getSize();
    startPos =0;
    curPos =0;
    endPos = getSize();
    soundBuffer_ = {};
}

AudioSound::~AudioSound() {
    // Pas de ressources spécifiques à libérer pour l'instant
}

void AudioSound::setActive(bool active) {
    active_ = active;
    if (!active_) {
        curPos =0; // Réinitialiser la lecture quand le son s'arrête
    }
}

double AudioSound::getNextSample() {
    if (active_ && curPos < endPos) {
        return rawData_[curPos++];
    } else {
        active_ = false; // Le son est terminé
        curPos =0;
        return 0.0; // Retourner 0.0 quand le son est fini
    }
}

size_t AudioSound::readData(std::vector<float>& bufData, size_t numFrames) {
  // std::cout << "voici length: " << length_ << std::endl;  
  size_t samplesToRead = std::min(numFrames, static_cast<size_t>(endPos - curPos) * numChannels_);

    if (samplesToRead > 0) {
        // réinitialiser le vecteur existant
        bufData.assign(numFrames, 0.0f);
        const double* sourceBegin = rawData_.data() + (curPos * numChannels_);
        float* destBegin = bufData.data();
        for (size_t i = 0; i < samplesToRead; ++i) {
            destBegin[i] = static_cast<float>(sourceBegin[i]);
        }
        curPos += samplesToRead / numChannels_; // Avancer le curPos en frames
    }

    return samplesToRead;
}


/*
size_t AudioSound::readData(size_t numFrames) {
  // std::cout << "voici length: " << length_ << std::endl;  
  size_t samplesToRead = std::min(numFrames, static_cast<size_t>(endPos - curPos) * numChannels_);

    if (samplesToRead > 0) {
      // réinitialiser le vecteur existant
        soundBuffer_.assign(samplesToRead, 0.0f); // Créer le buffer à la taille exacte des données à lire
        const double* sourceBegin = rawData_.data() + (curPos * numChannels_);
        float* destBegin = soundBuffer_.data();
        for (size_t i = 0; i < samplesToRead; ++i) {
            destBegin[i] = static_cast<float>(sourceBegin[i]);
        }
        curPos += samplesToRead / numChannels_; // Avancer le curPos en frames
    }

    return samplesToRead;
}
*/

/*
// Autre version
std::vector<float> AudioSound::readData(size_t numFrames) {
    std::vector<float> buffer(numFrames * numChannels_);
    size_t framesRead = 0;

    while (framesRead < numFrames && curPos < endPos) {
        for (int channel = 0; channel < numChannels_; ++channel) {
            buffer[(framesRead * numChannels_) + channel] = static_cast<float>(rawData_[curPos * numChannels_ + channel]);
        }
        curPos++;
        framesRead++;
    }

    // Si on a lu moins de frames que demandé, on remplit le reste avec des zéros (silence)
    while (framesRead < numFrames) {
        for (int channel = 0; channel < numChannels_; ++channel) {
            buffer[(framesRead * numChannels_) + channel] = 0.0f;
        }
        framesRead++;
    }

    return buffer;
}
*/


void AudioSound::applyStaticFadeOutLinear(float fadeOutStartPercent) {
    if (fadeOutStartPercent >= 0.0f && fadeOutStartPercent <= 1.0f) {
        unsigned long fadeOutStartFrame = static_cast<unsigned long>(length_ * fadeOutStartPercent);
        unsigned long fadeOutDurationFrames = length_ - fadeOutStartFrame;

        if (fadeOutDurationFrames > 0) {
            for (unsigned long i = 0; i < fadeOutDurationFrames; ++i) {
                float gain = 1.0f - static_cast<float>(i) / fadeOutDurationFrames;
                size_t index = (fadeOutStartFrame + i) * numChannels_;
                if (index < rawData_.size()) {
                    for (size_t channel = 0; channel < numChannels_; ++channel) {
                        rawData_[index + channel] *= gain;
                    }
                }
            }
        }
    }
}

void AudioSound::applyStaticFadeOutExp(float fadeOutStartPercent, float powerFactor) {
    if (fadeOutStartPercent >= 0.0f && fadeOutStartPercent <= 1.0f && powerFactor > 0.0f) {
        unsigned long fadeOutStartFrame = static_cast<unsigned long>(length_ * fadeOutStartPercent);
        unsigned long fadeOutDurationFrames = length_ - fadeOutStartFrame;

        if (fadeOutDurationFrames > 0) {
            for (unsigned long i = 0; i < fadeOutDurationFrames; ++i) {
                float timeRatio = static_cast<float>(i) / fadeOutDurationFrames;
                float gain = 1.0f - std::pow(timeRatio, powerFactor);
                size_t index = (fadeOutStartFrame + i) * numChannels_;
                if (index < rawData_.size()) {
                    for (size_t channel = 0; channel < numChannels_; ++channel) {
                        rawData_[index + channel] *= gain;
                    }
                }
            }
        }
    }
}

