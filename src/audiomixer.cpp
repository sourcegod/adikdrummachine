#include "audiomixer.h"
#include "audiosound.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <memory> // Pour std::shared_ptr

AudioMixer::AudioMixer(int numChannels) 
  : channels_(), 
    globalVolume_(0.8f), // Initialiser le volume global à 0.8
    numChannels_(numChannels) {
    if (numChannels > channels_.size()) {
        std::cerr << "Attention : Le nombre de canaux demandé dépasse la taille du mixer." << std::endl;
    }
    for (auto& channel : channels_) {
        channel.active = false;
        channel.volume = 1.0f;
        channel.sound = nullptr; // Initialiser le shared_ptr à nullptr
        channel.reserved = false; // Initialiser reserved à false

    }
    //
    // Réserver le canal du métronome
    if (metronomeChannel_ >= 0 && metronomeChannel_ < channels_.size()) {
        channels_[metronomeChannel_].reserved = true;
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
    if (channel >= 0 && channel < channels_.size()) {
        // Autoriser la lecture sur le canal du métronome même s'il est réservé
        if (channel == metronomeChannel_ || !channels_[channel].reserved) {
            channels_[channel].sound = sound;
            channels_[channel].active = true;
            channels_[channel].startPos = 0;
            channels_[channel].curPos = 0;
            channels_[channel].endPos = sound ? sound->getLength() : 0; // Gérer le cas où sound est nul
            if (sound) {
                sound->resetPlayhead();
                sound->setActive(true);
            }
        } else {
            std::cerr << "Erreur: Canal " << channel << " est réservé et ne peut pas être utilisé pour la lecture." << std::endl;
        }
    } else {
        std::cerr << "Erreur: Canal " << channel << " invalide pour la lecture." << std::endl;
    }
}

bool AudioMixer::isChannelPlaying(int channel) const {
    if (channel >= 0 && channel < channels_.size()) {
        return channels_[channel].isPlaying();
    }
    return false;
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

float AudioMixer::getVolume(int channel) const {
    if (channel >= 0 && channel < channels_.size()) {
        return channels_[channel].volume;
    } else {
        std::cerr << "Canal invalide : " << channel << std::endl;
        return 0.0f;
    }
}

void AudioMixer::setVolume(int channel, float volume) {
    if (channel >= 0 && channel < channels_.size()) {
        channels_[channel].volume = std::clamp(volume, 0.0f, 1.0f);
    } else {
        std::cerr << "Canal invalide : " << channel << std::endl;
    }
}

float AudioMixer::getGlobalVolume() const {
    return globalVolume_;
}


void AudioMixer::setGlobalVolume(float volume) {
    globalVolume_ = std::clamp(volume, 0.0f, 1.0f);
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

void AudioMixer::reserveChannel(int channel, bool reserved) {
    if (channel >= 0 && channel < channels_.size()) {
        channels_[channel].reserved = reserved;
    } else {
        std::cerr << "Erreur: Canal " << channel << " invalide pour la réservation." << std::endl;
    }
}

bool AudioMixer::isChannelReserved(int channel) const {
    if (channel >= 0 && channel < channels_.size()) {
        return channels_[channel].reserved;
    }
    return false;
}

size_t AudioMixer::getChannelCurPos(int channel) const {
    if (channel >= 0 && channel < channels_.size()) {
        return channels_[channel].curPos;
    }
    return 0;
}

void AudioMixer::setChannelCurPos(int channel, size_t pos) {
    if (channel >= 0 && channel < channels_.size()) {
        channels_[channel].curPos = pos;
    }
}

size_t AudioMixer::getChannelEndPos(int channel) const {
    if (channel >= 0 && channel < channels_.size()) {
        return channels_[channel].endPos;
    }
    return 0;
}


