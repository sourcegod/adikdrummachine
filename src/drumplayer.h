#ifndef DRUMPLAYER_H
#define DRUMPLAYER_H

#include <vector>

class DrumPlayer {
public:
    DrumPlayer(int numSounds);

    void triggerSound(std::vector<std::vector<double>>& sounds, std::vector<double>::iterator currentSound[], int soundIndex);

    double softClip(double x);

    std::vector<bool> playing;
};

#endif // DRUMPLAYER_H
