#ifndef DRUMPLAYER_H
#define DRUMPLAYER_H

#include <vector>
#include <memory> // Pour std::shared_ptr
#include "audiosound.h"
#include "audiomixer.h" // Assurez-vous que l'inclusion est là
#include <cmath>
#include <algorithm> // pour std::clamp
using namespace adikdrum;
class DrumPlayer {
public:
    DrumPlayer(int numSounds, int numSteps);
    // DrumPlayer(int numSounds, int initialBpm, const std::vector<std::shared_ptr<AudioSound>>& sounds, int numSteps);
    ~DrumPlayer();

    void playSound(size_t soundIndex);
    void stopAllSounds();
    void playMetronome();
    void playPattern();

    double softClip(double x) { return tanh(x); }
    float hardClip(double x) { return std::clamp(x, -1.0, 1.0); }

// Nouvelle fonction de hard clipping

    double getBpm() const { return bpm_; }
    void setBpm(double newBpm);
    bool isSoundPlaying() const;
    void setMixer(AudioMixer& mixer); // Nouvelle fonction pour assigner le mixer
    void startClick();
    void stopClick();
    int getNumSteps() const { return numSteps_; }
    int getNumSounds() const { return drumSounds_.size(); } // On peut déduire le nombre de sons de la taille du vecteur
    bool isSoundMuted(size_t soundIndex) const;
    void setSoundMuted(size_t soundIndex, bool muted);
    void resetMute();

    bool isPlaying() const { return playing_; }
    bool isClicking() const { return clicking_; }
    void togglePlay() { playing_ = ! playing_; }
    void toggleClick() { clicking_ = ! clicking_; }
    void playLastSound();

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
    bool playing_;
    bool clicking_;
    double bpm_;
    int beatCounter_;
    AudioMixer* mixer_; // Pointeur vers l'AudioMixer
    std::vector<bool> isMuted_; // true si le son est muté
    size_t lastSoundIndex_; // Nouveau membre privé

    std::shared_ptr<AudioSound> getSound(size_t soundIndex); 

};
//==== End of class DrumPlayer ====

#endif // DRUMPLAYER_H
