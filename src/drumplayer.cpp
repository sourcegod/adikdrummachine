#include "drumplayer.h"
#include <cmath>

DrumPlayer::DrumPlayer(int numSounds, int initialBpm, const std::vector<std::vector<double>>& sounds)
    : playing(numSounds, false),
      isPlaying(false),
      currentStep(0),
      bpm(initialBpm),
      sampleRate_(44100),
      drumSounds_(sounds)
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


void DrumPlayer::triggerSound(std::vector<std::vector<double>>& sounds, std::vector<double>::iterator currentSound[], int soundIndex) {
    if (soundIndex >= 0 && soundIndex < sounds.size()) {
        currentSound[soundIndex] = sounds[soundIndex].begin();
        playing[soundIndex] = true;
    }
}

double DrumPlayer::softClip(double x) {
    return tanh(x);
}

void DrumPlayer::setBpm(int newBpm) {
    bpm = newBpm;
    secondsPerStep = (60.0 / bpm) / 4.0;
}

