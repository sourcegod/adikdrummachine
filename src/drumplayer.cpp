#include "drumplayer.h"
#include <cmath>
//
//

DrumPlayer::DrumPlayer(int numSounds, int initialBpm, const std::vector<std::vector<double>>& sounds)
    : playing(numSounds, false),
      isPlaying(false),
      isClicking(false), // Initialise isClicking à false (désactivé par défaut)
      currentStep(0),
      bpm(initialBpm),
      sampleRate_(44100),
      drumSounds_(sounds),
      isFirstBeat_(true),
      beatCounter_(0), // Initialise beatCounter à 0
      clickStep(0) // Initialise metronomeStep à 0

{
    currentSound_ = new std::vector<double>::iterator[numSounds];
    for (int i = 0; i < numSounds; ++i) {
        currentSound_[i] = drumSounds_[i].begin();
    }
    setBpm(initialBpm);
}

DrumPlayer::~DrumPlayer() {
    delete[] currentSound_;
}

void DrumPlayer::playSound(int soundIndex) {
    if (soundIndex >= 0 && soundIndex < drumSounds_.size()) {
        currentSound_[soundIndex] = drumSounds_[soundIndex].begin();
        playing[soundIndex] = true;
    }
}

void DrumPlayer::stopAllSounds() {
    isPlaying = false;
    isClicking = false; // Réinitialise aussi l'état du métronome
    currentStep = 0; 
    isFirstBeat_ = true; // Réinitialiser pour le prochain démarrage
    beatCounter_ = 0; // Réinitialise aussi le compteur de temps
    clickStep = 0; // Réinitialise aussi le pas du métronome

    for (int i = 0; i < playing.size(); ++i) {
        playing[i] = false;
    }
}

void DrumPlayer::playMetronome() {
    if (isPlaying && currentStep  == 0) {
       beatCounter_ =0;
    }
    if (beatCounter_ % 4 == 0) { // Jouer au début de chaque temps (tous les 4 pas)
        currentSound_[16] = drumSounds_[16].begin();
        playing[16] = true;
    } else {
        currentSound_[17] = drumSounds_[17].begin();
        playing[17] = true;
    }
    beatCounter_ = (beatCounter_ + 1) % 4; // Incrémente et boucle de 0 à 3

}




double DrumPlayer::softClip(double x) {
    return tanh(x);
}

void DrumPlayer::setBpm(int newBpm) {
    bpm = newBpm;
    secondsPerStep = (60.0 / bpm) / 4.0;
}

