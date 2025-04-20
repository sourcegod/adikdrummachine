#ifndef DRUMPLAYER_H
#define DRUMPLAYER_H

#include <vector>
#include <memory> // Pour std::shared_ptr
#include "audiosound.h"
#include "audiomixer.h" // Assurez-vous que l'inclusion est là

class DrumPlayer {
public:
    DrumPlayer(int numSounds, int initialBpm, const std::vector<std::shared_ptr<AudioSound>>& sounds, int numSteps);
    ~DrumPlayer();

    void playSound(int soundIndex);
    void stopAllSounds();
    void playMetronome();
    void playPattern();

    double softClip(double x);
    double hardClip(double x); // Nouvelle fonction de hard clipping

    void setBpm(int newBpm);
    bool isSoundPlaying() const;
    void setMixer(AudioMixer& mixer); // Nouvelle fonction pour assigner le mixer
    void startClick();
    void stopClick();

    bool isPlaying;
    bool isClicking;
    int currentStep;
    int bpm;
    double secondsPerStep;
    std::vector<std::shared_ptr<AudioSound>> drumSounds_; // Utilisation de shared_ptr
    std::shared_ptr<AudioSound> soundClick1_; // Nouveau membre pour le son aigu du métronome
    std::shared_ptr<AudioSound> soundClick2_; // Nouveau membre pour le son grave du métronome

    int clickStep;
    std::vector<std::vector<bool>> pattern_;
    int numSteps_;
    int sampleRate_;

private:
    int beatCounter_;
    AudioMixer* mixer_; // Pointeur vers l'AudioMixer

};

#endif // DRUMPLAYER_H
