#include "drumplayer.h"
#include <cmath>

DrumPlayer::DrumPlayer(int numSounds, int initialBpm)
    : playing(numSounds, false),
      isPlaying(false),
      currentStep(0),
      bpm(initialBpm),
      sampleRate_(44100) // Initialise le sample rate (tu peux le passer en paramètre aussi)
{
    setBpm(initialBpm);
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

