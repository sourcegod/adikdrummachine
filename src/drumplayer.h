#ifndef DRUMPLAYER_H
#define DRUMPLAYER_H

#include <vector>
#include <memory> // Pour std::shared_ptr
#include "audiosound.h"
#include "audiomixer.h" // Assurez-vous que l'inclusion est là
#include <cmath>
#include <algorithm> // pour std::clamp

class DrumPlayer {
public:
    DrumPlayer(int numSounds, int numSteps);
    // DrumPlayer(int numSounds, int initialBpm, const std::vector<std::shared_ptr<AudioSound>>& sounds, int numSteps);
    ~DrumPlayer();

    void playSound(int soundIndex);
    void stopAllSounds();
    void playMetronome();
    void playPattern();

    double softClip(double x) { return tanh(x); }
    float hardClip(double x) { return std::clamp(x, -1.0, 1.0); }

// Nouvelle fonction de hard clipping

    const double getBpm() const;
    void setBpm(double newBpm);
    bool isSoundPlaying() const;
    void setMixer(AudioMixer& mixer); // Nouvelle fonction pour assigner le mixer
    void startClick();
    void stopClick();
    int getNumSteps() const { return numSteps_; }
    int getNumSounds() const { return drumSounds_.size(); } // On peut déduire le nombre de sons de la taille du vecteur
    bool isSoundMuted(int soundIndex) const;
    void setSoundMuted(int soundIndex, bool muted);
    void resetMute();

    bool isPlaying;
    bool isClicking;
    int currentStep;
    double secondsPerStep;
    std::vector<std::shared_ptr<AudioSound>> drumSounds_; // Utilisation de shared_ptr
    std::shared_ptr<AudioSound> soundClick1_; // Nouveau membre pour le son aigu du métronome
    std::shared_ptr<AudioSound> soundClick2_; // Nouveau membre pour le son grave du métronome

    int clickStep;
    std::vector<std::vector<bool>> pattern_;
    int numSteps_;
    int sampleRate_;

private:
    double bpm_;
    int beatCounter_;
    AudioMixer* mixer_; // Pointeur vers l'AudioMixer
    std::vector<bool> isMuted_; // true si le son est muté

};

#endif // DRUMPLAYER_H
