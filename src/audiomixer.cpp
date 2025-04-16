#include "audiomixer.h"
#include <iostream>
#include <cmath> // Pour std::clamp
#include <algorithm> // Pour std::clamp

AudioMixer::AudioMixer(int numChannels) : channels_() {
    if (numChannels > channels_.size()) {
        std::cerr << "Attention : Le nombre de canaux demandé dépasse la taille du mixer." << std::endl;
    }
    for (auto& channel : channels_) {
        channel.active = false;
        channel.volume = 1.0f; // Volume initial à 100%
        channel.soundIndex = -1; // Aucun son assigné initialement
    }
}

AudioMixer::~AudioMixer() {
    // Pas de ressources spécifiques à libérer pour l'instant
}

void AudioMixer::play(int channel, int soundIndex) {
    if (channel >= 0 && channel < channels_.size()) {
        channels_[channel].active = true;
        channels_[channel].soundIndex = soundIndex;
        // Ici, tu devrais aussi réinitialiser l'itérateur de lecture du son
        // dans ta classe DrumPlayer pour ce soundIndex.
    } else {
        std::cerr << "Canal invalide : " << channel << std::endl;
    }
}

void AudioMixer::pause(int channel) {
    if (channel >= 0 && channel < channels_.size()) {
        channels_[channel].active = false;
    } else {
        std::cerr << "Canal invalide : " << channel << std::endl;
    }
}

void AudioMixer::stop(int channel) {
    if (channel >= 0 && channel < channels_.size()) {
        channels_[channel].active = false;
        channels_[channel].soundIndex = -1;
        // Ici, tu pourrais aussi arrêter la lecture du son dans DrumPlayer.
    } else {
        std::cerr << "Canal invalide : " << channel << std::endl;
    }
}

void AudioMixer::setVolume(int channel, float volume) {
    if (channel >= 0 && channel < channels_.size()) {
        channels_[channel].volume = std::clamp(volume, 0.0f, 1.0f); // S'assurer que le volume est entre 0 et 1
    } else {
        std::cerr << "Canal invalide : " << channel << std::endl;
    }
}

float AudioMixer::getVolume(int channel) const {
    if (channel >= 0 && channel < channels_.size()) {
        return channels_[channel].volume;
    } else {
        std::cerr << "Canal invalide : " << channel << std::endl;
        return 0.0f;
    }
}

bool AudioMixer::isChannelActive(int channel) const {
    if (channel >= 0 && channel < channels_.size()) {
        return channels_[channel].active;
    } else {
        std::cerr << "Canal invalide : " << channel << std::endl;
        return false;
    }
}

