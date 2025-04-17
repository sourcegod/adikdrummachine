#ifndef DRUMPLAYER_H
#define DRUMPLAYER_H

#include <vector>
#include <memory> // Pour std::shared_ptr
#include "audiosound.h"

class DrumPlayer {
public:
    DrumPlayer(int numSounds, int initialBpm, const std::vector<std::shared_ptr<AudioSound>>& sounds, int numSteps);
    ~DrumPlayer();

    void playSound(int soundIndex);
    void stopAllSounds();
    void playMetronome();
    void playPattern();
    double softClip(double x);
    void setBpm(int newBpm);
    bool isSoundPlaying() const;

    bool isPlaying;
    bool isClicking;
    int currentStep;
    int bpm;
    double secondsPerStep;
    std::vector<std::shared_ptr<AudioSound>> drumSounds_; // Utilisation de shared_ptr
    int clickStep;
    std::vector<std::vector<bool>> pattern_;
    int numSteps_;
    int sampleRate_;

private:
    int beatCounter_;
};

#endif // DRUMPLAYER_H
