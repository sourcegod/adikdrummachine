#ifndef DRUMPLAYER_H
#define DRUMPLAYER_H

#include <vector>

class DrumPlayer {
public:

    std::vector<bool> playing;
    bool isPlaying;
    int currentStep;
    int bpm;
    double secondsPerStep;


    DrumPlayer(int numSounds, int initialBpm);

    void triggerSound(std::vector<std::vector<double>>& sounds, std::vector<double>::iterator currentSound[], int soundIndex);
    double softClip(double x);
    void setBpm(int newBpm);
private:
    int sampleRate_; // Ajoute une variable pour stocker le sample rate (nécessaire pour setBpm)

};

#endif // DRUMPLAYER_H
