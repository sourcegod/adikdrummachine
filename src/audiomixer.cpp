#include "audiomixer.h"
#include "audiosound.h"
#include "audiosample.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <memory> // Pour std::shared_ptr

namespace adikdrum {

AudioMixer::AudioMixer(size_t numChannels) 
  : channelList_(numChannels), // initialiser la taille du vecteur  
    globalVolume_(0.8f), // Initialiser le volume global à 0.8
    numChannels_(numChannels), 
    soundFactory_(44100, 0.3) {

    soundBuffer = {};
    if (numChannels > channelList_.size()) {
        std::cerr << "Attention : Le nombre de canaux demandé dépasse la taille du mixer." << std::endl;
    }
    for (auto& channel : channelList_) {
        channel.active_ = false;
        channel.volume = 1.0f;
        channel.sound = nullptr; // Initialiser le shared_ptr à nullptr
        channel.reserved = false; // Initialiser reserved à false
        channel.speed = 1.0f;

    }
    //
    // Réserver le canal du métronome
    if (metronomeChannel_ >= 0 && metronomeChannel_ < channelList_.size()) {
        channelList_[metronomeChannel_].reserved = true;
    }
    // auto soundFactory_ = soundFactory_(44100, 0.3);

}
//----------------------------------------

AudioMixer::~AudioMixer() {
    close();
}
//----------------------------------------

bool AudioMixer::init(int sampleRate, int channels, int bits) {
    // Ajoute ici toute initialisation spécifique à AudioMixer si nécessaire
    (void)sampleRate;
    (void)channels;
    (void)bits;
    std::cout << "AudioMixer initialized." << std::endl;
    return true;
}
//----------------------------------------

void AudioMixer::close() {
    // Ajoute ici toute fermeture spécifique à AudioMixer si nécessaire
    std::cout << "AudioMixer closed." << std::endl;
}
//----------------------------------------

void AudioMixer::play(size_t channel, SoundPtr sound) {
    // Note: channel est de type size_t, donc forcément >=0, donc, on n'a pas besoin de le tester.
    if (channel < channelList_.size()) {
        // Autoriser la lecture sur le canal du métronome même s'il est réservé
        if (channel == metronomeChannel_ || !channelList_[channel].reserved) {
            channelList_[channel].sound = sound;
            channelList_[channel].active_ = true;
            channelList_[channel].startPos = 0;
            channelList_[channel].curPos = 0;
            channelList_[channel].endPos = sound ? sound->getSize() : 0; // Gérer le cas où sound est nul
            channelList_[channel].speed = sound ? sound->getSpeed() : 1.0f; // IMPORTANT : Initialiser la vitesse du canal
            if (sound) {
                sound->resetCurPos();
                sound->setActive(true);
            }
        } else {
            std::cerr << "Erreur: Canal " << channel << " est réservé et ne peut pas être utilisé pour la lecture." << std::endl;
        }
    } else {
        std::cerr << "Erreur: Canal " << channel << " invalide pour la lecture." << std::endl;
    }
}
//----------------------------------------

bool AudioMixer::isChannelPlaying(size_t channel) const {
    if (channel < channelList_.size()) {
        return channelList_[channel].isPlaying();
    }
    return false;
}
//----------------------------------------

void AudioMixer::pause(size_t channel) {
    if (channel < channelList_.size()) {
        channelList_[channel].active_ = false;
        if (channelList_[channel].sound) {
            channelList_[channel].sound->setActive(false);
        }
    } else {
        std::cerr << "Canal invalide : " << channel << std::endl;
    }
}
//----------------------------------------

void AudioMixer::stop(size_t channel) {
    if (channel < channelList_.size()) {
        channelList_[channel].active_ = false;
        channelList_[channel].sound.reset(); // Décrémente le compteur de références
    } else {
        std::cerr << "Canal invalide : " << channel << std::endl;
    }
}
//----------------------------------------

float AudioMixer::getVolume(size_t channel) const {
    if (channel < channelList_.size()) {
        return channelList_[channel].volume;
    } else {
        std::cerr << "Canal invalide : " << channel << std::endl;
        return 0.0f;
    }
}
//----------------------------------------

void AudioMixer::setVolume(size_t channel, float volume) {
    if (channel < channelList_.size()) {
        channelList_[channel].volume = std::clamp(volume, 0.0f, 1.0f);
    } else {
        std::cerr << "Canal invalide : " << channel << std::endl;
    }
}
//----------------------------------------

float AudioMixer::getGlobalVolume() const {
    return globalVolume_;
}
//----------------------------------------

void AudioMixer::setGlobalVolume(float volume) {
    globalVolume_ = std::clamp(volume, 0.0f, 1.0f);
}
//----------------------------------------

bool AudioMixer::isChannelActive(size_t channel) const {
    if (channel < channelList_.size()) {
        return channelList_[channel].active_;
    } else {
        std::cerr << "Canal invalide : " << channel << std::endl;
        return false;
    }
}
//----------------------------------------

void AudioMixer::setChannelActive(size_t channel, bool active) {
    if (channel < channelList_.size()) {
        channelList_[channel].active_ = active;
    } else {
        std::cerr << "Canal invalide : " << channel << std::endl;
    }
}
//----------------------------------------

SoundPtr AudioMixer::getSound(size_t channel) const {
    if (channel < channelList_.size()) {
        return channelList_[channel].sound; // Retourne le shared_ptr
    } else {
        std::cerr << "Canal invalide : " << channel << std::endl;
        return nullptr;
    }
}
//----------------------------------------

void AudioMixer::reserveChannel(size_t channel, bool reserved) {
    if (channel < channelList_.size()) {
        channelList_[channel].reserved = reserved;
    } else {
        std::cerr << "Erreur: Canal " << channel << " invalide pour la réservation." << std::endl;
    }
}
//----------------------------------------

bool AudioMixer::isChannelReserved(size_t channel) const {
    // channel is size_t, so, no nedd to test >=0
    if (channel < channelList_.size()) {
        return channelList_[channel].reserved;
    }
    return false;
}
//----------------------------------------

size_t AudioMixer::getChannelCurPos(size_t channel) const {
    if (channel < channelList_.size()) {
        return channelList_[channel].curPos;
    }
    return 0;
}
//----------------------------------------

void AudioMixer::setChannelCurPos(size_t channel, size_t pos) {
    if (channel < channelList_.size()) {
        channelList_[channel].curPos = pos;
    }
}
//----------------------------------------

size_t AudioMixer::getChannelEndPos(size_t channel) const {
    if (channel < channelList_.size()) {
        return channelList_[channel].endPos;
    }
    return 0;
}
//----------------------------------------

std::vector<ChannelInfo>& AudioMixer::getChannelList() {
    return channelList_;
}
//----------------------------------------

void AudioMixer::setChannelMuted(size_t channelIndex, bool muted) {
    if (channelIndex < channelList_.size()) {
        channelList_[channelIndex].muted = muted;
        std::cout << "Canal " << channelIndex << " est maintenant " << (muted ? "muté" : "démuté") << "." << std::endl;
    } else {
        std::cerr << "Index de canal invalide: " << channelIndex + 1 << std::endl;
    }
}
//----------------------------------------

bool AudioMixer::isChannelMuted(size_t channelIndex) const {
    if (channelIndex < channelList_.size()) {
        return channelList_[channelIndex].muted;
    }
    return false; // Par défaut, non muté si l'index est invalide
}
//----------------------------------------

void AudioMixer::resetMute() {
    for (auto& channel : channelList_) {
        channel.muted = false;
    }
    std::cout << "Tous les canaux ont été démutés." << std::endl;
}
//----------------------------------------

void AudioMixer::setChannelPan(size_t channelIndex, float panValue) {
    if (channelIndex < channelList_.size()) {
        channelList_[channelIndex].pan = std::clamp(panValue, -1.0f, 1.0f);
        std::cout << "Pan du canal " << channelIndex << " réglé à " << channelList_[channelIndex].pan << std::endl;
    } else {
        std::cerr << "Index de canal invalide: " << channelIndex + 1 << std::endl;
    }
}
//----------------------------------------

float AudioMixer::getChannelPan(size_t channelIndex) const {
    if (channelIndex < channelList_.size()) {
        return channelList_[channelIndex].pan;
    }
    return 0.0f; // Par défaut au centre si l'index est invalide
}
//----------------------------------------

void AudioMixer::fadeInLinear(size_t channelIndex, std::vector<float>& bufData, unsigned long durationFrames, int outputNumChannels) {
    if (channelIndex < channelList_.size() && durationFrames > 0) {
        ChannelInfo& channel = channelList_[channelIndex];
        float startVolume = channel.volume;
        for (unsigned long frame = 0; frame < durationFrames; ++frame) {
            float fraction = static_cast<float>(frame) / durationFrames;
            float currentVolume = startVolume + (1.0f - startVolume) * fraction;
            for (int i = 0; i < outputNumChannels; ++i) {
                bufData[frame * outputNumChannels + i] *= currentVolume; // Appliquer le gain au buffer
            }
        }
        channel.volume = 1.0f; // S'assurer que le volume du canal est à 1 après le fondu
    }
}
//----------------------------------------

void AudioMixer::fadeOutLinear(size_t channelIndex, std::vector<float>& bufData, unsigned long durationFrames, int outputNumChannels) {
    if (channelIndex < channelList_.size() && durationFrames > 0) {
        ChannelInfo& channel = channelList_[channelIndex];
        float startVolume = channel.volume;
        for (unsigned long frame = 0; frame < durationFrames; ++frame) {
            float fraction = static_cast<float>(frame) / durationFrames;
            float currentVolume = startVolume * (1.0f - fraction);
            for (int i = 0; i < outputNumChannels; ++i) {
                bufData[frame * outputNumChannels + i] *= currentVolume; // Appliquer le gain au buffer
            }
        }
        // channel.volume = 0.0f; // S'assurer que le volume du canal est à 0 après le fondu
    }
}
//----------------------------------------

    void AudioMixer::mixSoundData(std::vector<float>& outputBuffer, size_t numFrames, size_t outputNumChannels) {
        for (size_t i = 0; i < channelList_.size(); ++i) {
            auto& chan = channelList_[i];
            if (chan.isActive() && !chan.muted && chan.sound) {
                auto numSoundChannels = chan.sound->getNumChannels();
                soundBuffer.assign(numFrames * numSoundChannels, 0.0f);

                size_t framesRead = chan.sound->readData(soundBuffer, numFrames); // Passer la vitesse à readData
                if (framesRead > 0) {
                    float volume = chan.volume;
                    float pan = chan.pan;

                    for (size_t j = 0; j < numFrames; ++j) {
                        float leftSample = 0.0f;
                        float rightSample = 0.0f;

                        if (numSoundChannels == 1) {
                            leftSample = soundBuffer[j] * volume * std::max(0.0f, 1.0f - pan);
                            rightSample = soundBuffer[j] * volume * std::max(0.0f, 1.0f + pan);
                        } else if (numSoundChannels == 2) {
                            leftSample = soundBuffer[j * numSoundChannels] * volume * std::max(0.0f, 1.0f - pan);
                            rightSample = soundBuffer[j * numSoundChannels + 1] * volume * std::max(0.0f, 1.0f + pan);
                        }
                        outputBuffer[j * outputNumChannels] += leftSample;
                        outputBuffer[j * outputNumChannels + 1] += rightSample;
                    }
                }
            }
        }
    }

//----------------------------------------

/*
void AudioMixer::mixSoundData(std::vector<float>& outputBuffer, size_t numFrames, size_t outputNumChannels) {
    for (size_t i = 0; i < channelList_.size(); ++i) {
        auto& chan = channelList_[i];
        if (chan.isActive() && !chan.muted && chan.sound) {
            // soudBuffer must be initialized before passing to readData function
            //
            // Note: soundBuffer size can be half smaller of outputBuffer size, for mono sound, 
            // but same size of outputBuffer for stereo sound, to avoid overflow, segmentation fault.
            // 

            auto numSoundChannels = chan.sound->getNumChannels();
            soundBuffer.assign(numFrames * numSoundChannels, 0.0f);
            // std::cout << "In mixSoundData, mixChannel: " << i << ", numFrames: " << numFrames << std::endl;
            // std::cout << "Sound numChannels: " << chan.sound->getNumChannels() << ", length: " << chan.sound->getLength() << "\n";


            if (chan.sound->readData(soundBuffer, numFrames) > 0) {
                float volume = chan.volume;
                float pan = chan.pan;

                for (size_t j=0; j < numFrames; ++j) {
                    float leftSample = 0.0f;
                    float rightSample = 0.0f;
                    if (numSoundChannels == 1) {
                        leftSample = soundBuffer[j * numSoundChannels] * volume * std::max(0.0f, 1.0f - pan);
                        rightSample = soundBuffer[j * numSoundChannels] * volume * std::max(0.0f, 1.0f + pan);
                    } else if (numSoundChannels == 2) {
                        leftSample = soundBuffer[j * numSoundChannels] * volume * std::max(0.0f, 1.0f - pan);
                        rightSample = soundBuffer[j * numSoundChannels + 1] * volume * std::max(0.0f, 1.0f + pan);
                    }

                    outputBuffer[j * outputNumChannels] += leftSample;     // Accumulation pour le canal gauche
                    outputBuffer[j * outputNumChannels + 1] += rightSample;    // Accumulation pour le canal droit
                }
            }
        }
    }
}
//----------------------------------------
*/


void AudioMixer::setSpeed(size_t channel, float speed) {
    if (channel < channelList_.size()) {
        channelList_[channel].speed = speed;
        if (channelList_[channel].sound) {
            channelList_[channel].sound->setSpeed(speed);
        }
        std::cout << "Vitesse du canal " << channel << " réglée à " << speed << std::endl;
    } else {
        std::cerr << "Erreur : Canal " << channel << " invalide pour régler la vitesse." << std::endl;
    }
}
//----------------------------------------

SoundPtr AudioMixer::loadSound(const std::string& filePath) {
    return std::make_shared<AudioSample>(filePath); // Charger le fichier
}
//----------------------------------------

SoundPtr AudioMixer::genTone(const std::string& type, float freq, float length) {
    return soundFactory_.tone(type, freq, length);
}
//----------------------------------------


//==== End of class AudioMixer ====

} // namespace adikdrum
