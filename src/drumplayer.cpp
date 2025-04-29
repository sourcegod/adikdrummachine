#include "drumplayer.h"
#include "audiosound.h"
#include "audiomixer.h"

#include <cmath>
#include <vector>
#include <iostream>
#include <memory>
#include <algorithm> // pour std::clamp

DrumPlayer::DrumPlayer(int numSounds, int numSteps)
    : isPlaying(false),
      isClicking(false),
      currentStep(0),
      bpm_(100),
      clickStep(0),
      pattern_(numSounds, std::vector<bool>(numSteps, false)),
      numSteps_(numSteps),
      sampleRate_(44100),
      beatCounter_(0),
      mixer_(nullptr), // Initialiser à nullptr
      isMuted_(numSounds, false) // Initialiser tous les sons comme non mutés
{
    setBpm(bpm_);
    std::cout << "DrumPlayer::Constructor - numSteps_: " << numSteps_ << std::endl;

}

DrumPlayer::~DrumPlayer() {
    // Les shared_ptr se chargeront de la gestion de la durée de vie des AudioSound
}

void DrumPlayer::setMixer(AudioMixer& mixer) {
    mixer_ = &mixer;
}

/*
void DrumPlayer::playSound0(int soundIndex) {
  // Note: this function is reserved for latter
    if (soundIndex >= 0 && soundIndex < drumSounds_.size() && drumSounds_[soundIndex]) {
        // Trouver un canal libre et non réservé et jouer le son
        for (auto i = 1; i < drumSounds_.size(); ++i) { // Commencer à partir du canal 1 (en supposant que 0 est réservé)
            if (!mixer_->isChannelActive(i)
                && !mixer_->isChannelReserved(i)) {
               mixer_->play(i, drumSounds_[soundIndex]);
               return; // Jouer le son sur le premier canal non réservé trouvé
            }
        }
        // Le message d'erreur s'affiche uniquement si aucun canal n'a été trouvé
        std::cerr << "Erreur: Aucun canal non réservé et inactif disponible pour jouer le son (index: "
                  << soundIndex << ")" << std::endl;
    }
}
*/

void DrumPlayer::playSound(int soundIndex) {
    if (soundIndex >= 0 && soundIndex < drumSounds_.size() && drumSounds_[soundIndex]) {
        mixer_->play(soundIndex+1, drumSounds_[soundIndex]);
    } else {
        std::cerr << "Erreur: Aucun son trouvé avec cet (index: "
                  << soundIndex << ")" << std::endl;
    }
}


void DrumPlayer::stopAllSounds() {
    auto& chanList = mixer_->getChannelList();  
    for (auto& chan : chanList) {
        chan.setActive(0);
        chan.curPos =0;
    }

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
        soundClick1_->setActive(true);
    } if (soundClick2_) {
        soundClick2_->setActive(true);
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
    } if (soundClick2_) {
        soundClick2_->setActive(false);
    }

}

void DrumPlayer::playMetronome() {
    if (isPlaying) {
        // valable aussi pour si currentStep =0 et beatCounter =0
        beatCounter_ = currentStep / 4; // Note: Le résultat est une division entière puisque les deux nombres sont des entiers.
    }
    
    if (mixer_) {
        if (beatCounter_ % 4 == 0 && soundClick1_) {
            mixer_->play(0, soundClick1_);
        } else if (soundClick2_) {
            mixer_->play(0, soundClick2_);
        }
        beatCounter_ = (beatCounter_ + 1) % 4;
    }
}

// /*
void DrumPlayer::playPattern() {
    if (mixer_ && isPlaying) {
        for (int i = 0; i < drumSounds_.size(); ++i) { //
            if (pattern_[i][currentStep]) {
                if (drumSounds_[i]) {
                    
                    /* 
                    // On ne cherche plus un canal libre  pour jouer, chaque canal à son index de son
                    if (!mixer_->isChannelActive(i + 1)) { // Utiliser les canaux 1 à NUM_SOUNDS
                        mixer_->play(i + 1, drumSounds_[i]);
                    }
                    */
                    
                    mixer_->play(i + 1, drumSounds_[i]);
                }
            }
        }
    }
}
// */

const double DrumPlayer::getBpm() const { return bpm_; }
void DrumPlayer::setBpm(double newBpm) {
    if (newBpm >=5 &&  newBpm <= 800) {
        bpm_ = newBpm;
        secondsPerStep = (60.0 / bpm_) / 4.0;
    }

}

bool DrumPlayer::isSoundPlaying() const {
    for (const auto& sound : drumSounds_) {
        if (sound && sound->isActive()) {
            return true;
        }
    }
    return false;
}

bool DrumPlayer::isSoundMuted(int soundIndex) const {
    if (soundIndex >= 0 && soundIndex < isMuted_.size()) {
        return isMuted_[soundIndex];
    }
    return false; // Par défaut, non muté si l'index est invalide
}

void DrumPlayer::setSoundMuted(int soundIndex, bool muted) {
    if (mixer_ && soundIndex >= 0 && soundIndex < drumSounds_.size()) {
        int channelToMute = soundIndex+1;
        mixer_->setChannelMuted(channelToMute, muted); // Utiliser la fonction de AudioMixer
        isMuted_[soundIndex] = muted; // Garder une trace locale si nécessaire
        std::cout << "Son " << soundIndex<< " (canal " << channelToMute << ") est maintenant " << (muted ? "muté" : "démuté") << "." << std::endl;
    }
}

void DrumPlayer::resetMute() {
    if (mixer_) {
        mixer_->resetMute(); // S'assurer que le mixer est aussi réinitialisé
    }

    // Note: best way to modify directly an vector
    std::fill(isMuted_.begin(), isMuted_.end(), false);
    std::cout << "Tous les sons ont été démutés." << std::endl;
}


