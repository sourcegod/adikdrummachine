#include "audiosound.h"
#include <algorithm> // Pour std::min
#include <vector>
#include <iostream>
AudioSound::AudioSound(std::vector<double> data, int numChannels) 
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

size_t AudioSound::readData(size_t numFrames) {
  // std::cout << "voici length: " << length_ << std::endl;  
  size_t samplesToRead = std::min(numFrames * numChannels_, static_cast<size_t>(endPos - curPos) * numChannels_);

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


