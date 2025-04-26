#include "audiosound.h"

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
