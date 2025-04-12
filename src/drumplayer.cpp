#include "drumplayer.h"
#include <cmath>

DrumPlayer::DrumPlayer(int numSounds) : playing(numSounds, false) {}

void DrumPlayer::triggerSound(std::vector<std::vector<double>>& sounds, std::vector<double>::iterator currentSound[], int soundIndex) {
    if (soundIndex >= 0 && soundIndex < sounds.size()) {
        currentSound[soundIndex] = sounds[soundIndex].begin();
        playing[soundIndex] = true;
    }
}

double DrumPlayer::softClip(double x) {
    return tanh(x);
}
