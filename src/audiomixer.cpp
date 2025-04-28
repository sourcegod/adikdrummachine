#include "audiomixer.h"
#include "audiosound.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <memory> // Pour std::shared_ptr

AudioMixer::AudioMixer(int numChannels) 
  : channelList_(numChannels), // initialiser la taille du vecteur  
    globalVolume_(0.8f), // Initialiser le volume global à 0.8
    numChannels_(numChannels) {
    if (numChannels > channelList_.size()) {
        std::cerr << "Attention : Le nombre de canaux demandé dépasse la taille du mixer." << std::endl;
    }
    for (auto& channel : channelList_) {
        channel.active_ = false;
        channel.volume = 1.0f;
        channel.sound = nullptr; // Initialiser le shared_ptr à nullptr
        channel.reserved = false; // Initialiser reserved à false

    }
    //
    // Réserver le canal du métronome
    if (metronomeChannel_ >= 0 && metronomeChannel_ < channelList_.size()) {
        channelList_[metronomeChannel_].reserved = true;
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
    if (channel >= 0 && channel < channelList_.size()) {
        // Autoriser la lecture sur le canal du métronome même s'il est réservé
        if (channel == metronomeChannel_ || !channelList_[channel].reserved) {
            channelList_[channel].sound = sound;
            channelList_[channel].active_ = true;
            channelList_[channel].startPos = 0;
            channelList_[channel].curPos = 0;
            channelList_[channel].endPos = sound ? sound->getSize() : 0; // Gérer le cas où sound est nul
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
    if (channel >= 0 && channel < channelList_.size()) {
        return channelList_[channel].isPlaying();
    }
    return false;
}

void AudioMixer::pause(int channel) {
    if (channel >= 0 && channel < channelList_.size()) {
        channelList_[channel].active_ = false;
        if (channelList_[channel].sound) {
            channelList_[channel].sound->setActive(false);
        }
    } else {
        std::cerr << "Canal invalide : " << channel << std::endl;
    }
}

void AudioMixer::stop(int channel) {
    if (channel >= 0 && channel < channelList_.size()) {
        channelList_[channel].active_ = false;
        channelList_[channel].sound.reset(); // Décrémente le compteur de références
    } else {
        std::cerr << "Canal invalide : " << channel << std::endl;
    }
}

float AudioMixer::getVolume(int channel) const {
    if (channel >= 0 && channel < channelList_.size()) {
        return channelList_[channel].volume;
    } else {
        std::cerr << "Canal invalide : " << channel << std::endl;
        return 0.0f;
    }
}

void AudioMixer::setVolume(int channel, float volume) {
    if (channel >= 0 && channel < channelList_.size()) {
        channelList_[channel].volume = std::clamp(volume, 0.0f, 1.0f);
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
    if (channel >= 0 && channel < channelList_.size()) {
        return channelList_[channel].active_;
    } else {
        std::cerr << "Canal invalide : " << channel << std::endl;
        return false;
    }
}

void AudioMixer::setChannelActive(int channel, bool active) {
    if (channel >= 0 && channel < channelList_.size()) {
        channelList_[channel].active_ = active;
    } else {
        std::cerr << "Canal invalide : " << channel << std::endl;
    }
}


std::shared_ptr<AudioSound> AudioMixer::getSound(int channel) const {
    if (channel >= 0 && channel < channelList_.size()) {
        return channelList_[channel].sound; // Retourne le shared_ptr
    } else {
        std::cerr << "Canal invalide : " << channel << std::endl;
        return nullptr;
    }
}

void AudioMixer::reserveChannel(int channel, bool reserved) {
    if (channel >= 0 && channel < channelList_.size()) {
        channelList_[channel].reserved = reserved;
    } else {
        std::cerr << "Erreur: Canal " << channel << " invalide pour la réservation." << std::endl;
    }
}

bool AudioMixer::isChannelReserved(int channel) const {
    if (channel >= 0 && channel < channelList_.size()) {
        return channelList_[channel].reserved;
    }
    return false;
}

size_t AudioMixer::getChannelCurPos(int channel) const {
    if (channel >= 0 && channel < channelList_.size()) {
        return channelList_[channel].curPos;
    }
    return 0;
}

void AudioMixer::setChannelCurPos(int channel, size_t pos) {
    if (channel >= 0 && channel < channelList_.size()) {
        channelList_[channel].curPos = pos;
    }
}

size_t AudioMixer::getChannelEndPos(int channel) const {
    if (channel >= 0 && channel < channelList_.size()) {
        return channelList_[channel].endPos;
    }
    return 0;
}

std::vector<ChannelInfo>& AudioMixer::getChannelList() {
    return channelList_;
}

void AudioMixer::setChannelMuted(int channelIndex, bool muted) {
    if (channelIndex >= 0 && channelIndex < channelList_.size()) {
        channelList_[channelIndex].muted = muted;
        std::cout << "Canal " << channelIndex << " est maintenant " << (muted ? "muté" : "démuté") << "." << std::endl;
    } else {
        std::cerr << "Index de canal invalide: " << channelIndex + 1 << std::endl;
    }
}

bool AudioMixer::isChannelMuted(int channelIndex) const {
    if (channelIndex >= 0 && channelIndex < channelList_.size()) {
        return channelList_[channelIndex].muted;
    }
    return false; // Par défaut, non muté si l'index est invalide
}

void AudioMixer::resetMute() {
    for (auto& channel : channelList_) {
        channel.muted = false;
    }
    std::cout << "Tous les canaux ont été démutés." << std::endl;
}
void AudioMixer::setChannelPan(int channelIndex, float panValue) {
    if (channelIndex >= 0 && channelIndex < channelList_.size()) {
        channelList_[channelIndex].pan = std::clamp(panValue, -1.0f, 1.0f);
        std::cout << "Pan du canal " << channelIndex << " réglé à " << channelList_[channelIndex].pan << std::endl;
    } else {
        std::cerr << "Index de canal invalide: " << channelIndex + 1 << std::endl;
    }
}

float AudioMixer::getChannelPan(int channelIndex) const {
    if (channelIndex >= 0 && channelIndex < channelList_.size()) {
        return channelList_[channelIndex].pan;
    }
    return 0.0f; // Par défaut au centre si l'index est invalide
}

void AudioMixer::fadeInLinear(int channelIndex, unsigned long durationFrames) {
    if (channelIndex >= 0 && channelIndex < channelList_.size()) {
        ChannelInfo& channel = channelList_[channelIndex];
        if (durationFrames > 0) {
            // Implémentation d'un fondu en entrée linéaire (exemple)
            float startVolume = channel.volume;
            for (unsigned long i = 0; i < durationFrames; ++i) {
                float fraction = static_cast<float>(i) / durationFrames;
                channel.volume = startVolume + (1.0f - startVolume) * fraction;
                // Peut-être ajouter un délai ici si l'appel n'est pas fait dans le callback
            }
            channel.volume = 1.0f; // S'assurer d'atteindre le volume maximal
        } else {
            channel.volume = 1.0f;
        }
    }
}

void AudioMixer::fadeOutLinear(int channelIndex, unsigned long durationFrames) {
    if (channelIndex >= 0 && channelIndex < channelList_.size()) {
        ChannelInfo& channel = channelList_[channelIndex];
        if (durationFrames > 0) {
            // Implémentation d'un fondu en sortie linéaire (exemple)
            float startVolume = channel.volume;
            for (unsigned long i = 0; i < durationFrames; ++i) {
                float fraction = static_cast<float>(i) / durationFrames;
                channel.volume = startVolume * (1.0f - fraction);
                // Peut-être ajouter un délai ici si l'appel n'est pas fait dans le callback
            }
            channel.volume = 0.0f; // S'assurer d'atteindre le silence
        } else {
            channel.volume = 0.0f;
        }
        channel.active_ = false; // Désactiver le canal après le fondu
    }
}

