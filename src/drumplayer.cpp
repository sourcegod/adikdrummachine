#include "drumplayer.h"
#include "audiosound.h"
#include "audiomixer.h"

#include <cmath>
#include <vector>
#include <iostream>
#include <memory>
#include <algorithm> // pour std::clamp

DrumPlayer::DrumPlayer(int numSounds, int initialBpm, const std::vector<std::shared_ptr<AudioSound>>& sounds, int numSteps)
    : isPlaying(false),
      isClicking(false),
      currentStep(0),
      bpm(initialBpm),
      drumSounds_(sounds),
      clickStep(0),
      pattern_(numSounds, std::vector<bool>(numSteps, false)),
      numSteps_(numSteps),
      sampleRate_(44100),
      beatCounter_(0),
      mixer_(nullptr) // Initialiser à nullptr

{
    setBpm(initialBpm);
}

DrumPlayer::~DrumPlayer() {
    // Les shared_ptr se chargeront de la gestion de la durée de vie des AudioSound
}

void DrumPlayer::setMixer(AudioMixer& mixer) {
    mixer_ = &mixer;
}
void DrumPlayer::playSound(int soundIndex) {
    if (soundIndex >= 0 && soundIndex < drumSounds_.size() && drumSounds_[soundIndex]) {
        drumSounds_[soundIndex]->setActive(true);
        drumSounds_[soundIndex]->resetPlayhead();
        std::cout << "playSound called for sound index: " << soundIndex << std::endl;
        // Trouver un canal libre et non réservé et jouer le son
        for (int i = 1; i < 17; ++i) { // Commencer à partir du canal 1
            std::cout << "Checking channel: " << i << ", Active: " << mixer_->isChannelActive(i)
                      << ", Reserved: " << mixer_->isChannelReserved(i) << std::endl;
            if (!mixer_->isChannelActive(i)
                && !mixer_->isChannelReserved(i)) {
                std::cout << "Playing sound " << soundIndex << " on channel " << i << std::endl;
                mixer_->play(i, drumSounds_[soundIndex]);
                return; // Jouer le son sur le premier canal non réservé trouvé
            }
        }
        // Le message d'erreur s'affiche uniquement si aucun canal n'a été trouvé
        std::cerr << "Erreur: Aucun canal non réservé et inactif disponible pour jouer le son (index: "
                  << soundIndex << ")" << std::endl;
    }
}

/*
void DrumPlayer::playSound(int soundIndex) {
    if (soundIndex >= 0 && soundIndex < drumSounds_.size() && drumSounds_[soundIndex]) {
        drumSounds_[soundIndex]->setActive(true);
        drumSounds_[soundIndex]->resetPlayhead();
        // Trouver un canal libre et non réservé et jouer le son
        for (int i = 1; i < 17; ++i) { // Commencer à partir du canal 1 (en supposant que 0 est réservé)
            if (!mixer_->isChannelActive(i)
                && !mixer_->isChannelReserved(i)) {
                mixer_->play(i, drumSounds_[soundIndex]);
                return; // Jouer le son sur le premier canal non réservé trouvé
            }
        }
        // Le message d'erreur s'affiche uniquement si aucun canal n'a été trouvé
        std::cerr << "Erreur: Aucun canal non réservé et inactif disponible pour jouer le son." << std::endl;
    }
}
*/

void DrumPlayer::stopAllSounds() {
    for (const auto& sound : drumSounds_) {
        if (sound) {
            sound->setActive(false);
        }
    }
    isPlaying = false;
    isClicking = false;
    currentStep = 0;
    clickStep = 0;
    beatCounter_ = 0;
}

void DrumPlayer::startClick() {
    isClicking = true;
    if (isPlaying) {
      clickStep = currentStep;
      beatCounter_ = clickStep % numSteps_;
    } else {
      clickStep =0;
      beatCounter_ =0;
    }

    if (mixer_) {
        mixer_->setChannelActive(0, true); 
    }
    // Activer les sons du métronome (s'assurer qu'ils sont chargés)
    if (soundClick1_) {
        soundClick1_->setActive(false);
    }
    if (soundClick2_) {
        soundClick2_->setActive(false);
    }

}

void DrumPlayer::stopClick() {
    isClicking = false;
    if (mixer_) {
        if (mixer_->isChannelActive(0)) {
            mixer_->stop(0);
        }
    }
    if (soundClick1_) {
        soundClick1_->setActive(false);
    }
    if (soundClick2_) {
        soundClick2_->setActive(false);
    }

}

void DrumPlayer::playMetronome() {
    if (isPlaying && currentStep == 0) {
      beatCounter_ =0;
    }
    if (mixer_) {
        if (beatCounter_ % 4 == 0 && soundClick1_) {
            soundClick1_->setActive(true);
            soundClick1_->resetPlayhead();
            mixer_->play(0, soundClick1_);
        } else if (soundClick2_) {
            soundClick2_->setActive(true);
            soundClick2_->resetPlayhead();
            mixer_->play(0, soundClick2_);
        }
        beatCounter_= (beatCounter_ + 1) % 4;
    }
}


void DrumPlayer::playPattern() {
    if (mixer_ && isPlaying) {
        for (int i = 0; i < drumSounds_.size() - 2; ++i) { // Exclude metronome sounds
            if (pattern_[i][currentStep]) {
                if (drumSounds_[i]) {
                    drumSounds_[i]->setActive(true);
                    drumSounds_[i]->resetPlayhead();
                    if (!mixer_->isChannelActive(i + 1)) { // Utiliser les canaux 1 à NUM_SOUNDS
                        mixer_->play(i + 1, drumSounds_[i]);
                    }
                }
            }
        }
    }
}
double DrumPlayer::softClip(double x) {
    return tanh(x);
}

double DrumPlayer::hardClip(double x) {
    return std::clamp(x, -1.0, 1.0);
}

void DrumPlayer::setBpm(int newBpm) {
    bpm = newBpm;
    secondsPerStep = (60.0 / bpm) / 4.0;
}

bool DrumPlayer::isSoundPlaying() const {
    for (const auto& sound : drumSounds_) {
        if (sound && sound->isActive()) {
            return true;
        }
    }
    return false;
}
