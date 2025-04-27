#include "audiosound.h"
#include <algorithm> // Pour std::min
#include <vector>

AudioSound::AudioSound(std::vector<double> data, int numChannels) 
    : rawData_(std::move(data)), numChannels_(numChannels),
    active_(false),
    length_(data.size()), playhead_(0) {}

AudioSound::~AudioSound() {
    // Pas de ressources spécifiques à libérer pour l'instant
}

void AudioSound::setActive(bool active) {
    active_ = active;
    if (!active_) {
        playhead_ = 0; // Réinitialiser la lecture quand le son s'arrête
    }
}

double AudioSound::getNextSample() {
    if (active_ && playhead_ < length_) {
        return rawData_[playhead_++];
    } else {
        active_ = false; // Le son est terminé
        playhead_ = 0;
        return 0.0; // Retourner 0.0 quand le son est fini
    }
}

void AudioSound::resetPlayhead() {
    playhead_ = 0;
}


std::vector<float> AudioSound::readData(size_t numFrames) {
    size_t samplesToRead = std::min(numFrames * numChannels_, static_cast<size_t>(length_ - playhead_) * numChannels_);

    if (samplesToRead > 0) {
        std::vector<float> buffer(samplesToRead, 0.0f); // Créer le buffer à la taille exacte des données à lire
        const double* sourceBegin = rawData_.data() + (playhead_ * numChannels_);
        float* destBegin = buffer.data();
        for (size_t i = 0; i < samplesToRead; ++i) {
            destBegin[i] = static_cast<float>(sourceBegin[i]);
        }
        playhead_ += samplesToRead / numChannels_; // Avancer le playhead en frames
        return buffer;
    } else {
        // Retourner un vecteur vide si aucune donnée n'a été lue
        return {};
    }
}

/*
// Autre version
std::vector<float> AudioSound::readData(size_t numFrames) {
    std::vector<float> buffer(numFrames * numChannels_);
    size_t framesRead = 0;

    while (framesRead < numFrames && playhead_ < length_) {
        for (int channel = 0; channel < numChannels_; ++channel) {
            buffer[(framesRead * numChannels_) + channel] = static_cast<float>(rawData_[playhead_ * numChannels_ + channel]);
        }
        playhead_++;
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


