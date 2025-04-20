#include "audiomixer.h"
#include "audiosound.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <memory> // Pour std::shared_ptr

AudioMixer::AudioMixer(int numChannels) 
  : channels_(), 
    globalVolume_(1.0f) { // Initialiser le volume global à 1.0
    if (numChannels > channels_.size()) {
        std::cerr << "Attention : Le nombre de canaux demandé dépasse la taille du mixer." << std::endl;
    }
    for (auto& channel : channels_) {
        channel.active = false;
        channel.volume = 1.0f;
        channel.sound = nullptr; // Initialiser le shared_ptr à nullptr
    }
}

AudioMixer::~AudioMixer() {
    close();
}

bool AudioMixer::init(int sampleRate, int channels, int bits) {
    // Ajoute ici toute initialisation spécifique à AudioMixer si nécessaire
    std::cout << "AudioMixer initialized." << std::endl;
}

void AudioMixer::close() {
    // Ajoute ici toute fermeture spécifique à AudioMixer si nécessaire
    std::cout << "AudioMixer closed." << std::endl;
}

void AudioMixer::play(int channel, std::shared_ptr<AudioSound> sound) {
    if (channel >= 0 && channel < channels_.size() && sound != nullptr) {
        channels_[channel].active = true;
        channels_[channel].sound = sound; // Simplement assigner le shared_ptr
        channels_[channel].sound->setActive(true);
        channels_[channel].sound->resetPlayhead();
    } else {
        std::cerr << "Erreur : Canal invalide ou pointeur de son nul (canal=" << channel << ")" << std::endl;
    }
}

void AudioMixer::pause(int channel) {
    if (channel >= 0 && channel < channels_.size()) {
        channels_[channel].active = false;
        if (channels_[channel].sound) {
            channels_[channel].sound->setActive(false);
        }
    } else {
        std::cerr << "Canal invalide : " << channel << std::endl;
    }
}

void AudioMixer::stop(int channel) {
    if (channel >= 0 && channel < channels_.size()) {
        channels_[channel].active = false;
        channels_[channel].sound.reset(); // Décrémente le compteur de références
    } else {
        std::cerr << "Canal invalide : " << channel << std::endl;
    }
}

void AudioMixer::setVolume(int channel, float volume) {
    if (channel >= 0 && channel < channels_.size()) {
        channels_[channel].volume = std::clamp(volume, 0.0f, 1.0f);
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

void AudioMixer::setChannelActive(int channel, bool active) {
    if (channel >= 0 && channel < channels_.size()) {
        channels_[channel].active = active;
    } else {
        std::cerr << "Canal invalide : " << channel << std::endl;
    }
}


std::shared_ptr<AudioSound> AudioMixer::getSound(int channel) const {
    if (channel >= 0 && channel < channels_.size()) {
        return channels_[channel].sound; // Retourne le shared_ptr
    } else {
        std::cerr << "Canal invalide : " << channel << std::endl;
        return nullptr;
    }
}
void AudioMixer::setGlobalVolume(float volume) {
    globalVolume_ = std::clamp(volume, 0.0f, 1.0f);
}

float AudioMixer::getGlobalVolume() const {
    return globalVolume_;
}

