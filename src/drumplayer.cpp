#include "drumplayer.h"
#include "audiosound.h"
#include "audiomixer.h"

#include <cmath>
#include <vector>
#include <iostream>
#include <memory>

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
        // Trouver un canal libre et jouer le son
        for (int i = 0; i < 17; ++i) {
            if (!mixer_->isChannelActive(i)) {
                mixer_->play(i, drumSounds_[soundIndex]);
                break; // Jouer le son sur le premier canal libre trouvé
            }
        }
    }
}

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

void DrumPlayer::playMetronome() {
    if (clickStep % 4 == 0 && drumSounds_.size() > 16 && drumSounds_[16]) {
        drumSounds_[16]->setActive(true);
        drumSounds_[16]->resetPlayhead();
    } else if (drumSounds_.size() > 17 && drumSounds_[17]) {
        drumSounds_[17]->setActive(true);
        drumSounds_[17]->resetPlayhead();
    }
    clickStep = (clickStep + 1) % numSteps_;
}

void DrumPlayer::playPattern() {
    for (int i = 0; i < drumSounds_.size() - 2; ++i) {
        if (pattern_[i][currentStep] && drumSounds_[i]) {
            drumSounds_[i]->setActive(true);
            drumSounds_[i]->resetPlayhead();
        }
    }
}

double DrumPlayer::softClip(double x) {
    return tanh(x);
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
