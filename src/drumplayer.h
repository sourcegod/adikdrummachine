#ifndef DRUMPLAYER_H
#define DRUMPLAYER_H

#include <vector>
#include <memory> // Pour std::shared_ptr
#include "audiosound.h"
#include "audiomixer.h" // Assurez-vous que l'inclusion est là
#include "adikpattern.h"

#include <cmath>
#include <algorithm> // pour std::clamp

namespace adikdrum {
class DrumPlayer {
public:
    DrumPlayer(int numSounds, int numSteps);
    // DrumPlayer(int numSounds, int initialBpm, const std::vector<SoundPtr>& sounds, int numSteps);
    ~DrumPlayer();

    size_t currentStep_;
    double secondsPerStep;
    std::vector<SoundPtr> drumSounds_; // Utilisation de shared_ptr
    SoundPtr soundClick1_; // Nouveau membre pour le son aigu du métronome
    SoundPtr soundClick2_; // Nouveau membre pour le son grave du métronome

    size_t clickStep_;
    std::vector<std::vector<bool>> pattern_;
    size_t numSteps_;
    int sampleRate_;
    std::shared_ptr<AdikPattern> curPattern_; // CHANGEMENT ICI : c'est maintenant un pointeur intelligent
    std::vector<std::vector<std::vector<bool>>> patData_;
    size_t currentBar_;  // Supposons que ces membres existent et sont gérés
    size_t numTotalBars_ =0;


    void playSound(size_t soundIndex);
    void stopAllSounds();
    void playMetronome();
    void playPattern();

    double softClip(double x) { return tanh(x); }
    float hardClip(double x) { return std::clamp(x, -1.0, 1.0); }


    double getBpm() const { return bpm_; }
    void setBpm(double newBpm);
    bool isSoundPlaying() const;
    void setMixer(AudioMixer& mixer); // Nouvelle fonction pour assigner le mixer
    void startClick();
    void stopClick();
    size_t getNumSteps() const { return numSteps_; }
    size_t getCurrentStep() const { return currentStep_; }
    size_t getNumSounds() const { return drumSounds_.size(); } // On peut déduire le nombre de sons de la taille du vecteur
    bool isSoundMuted(size_t soundIndex) const;
    void setSoundMuted(size_t soundIndex, bool muted);
    void resetMute();

    bool isPlaying() const { return playing_; }
    bool isClicking() const { return clicking_; }
    void togglePlay() { playing_ = ! playing_; }
    void toggleClick() { clicking_ = ! clicking_; }
    void playLastSound();
    void startPlay();
    void stopPlay();

    size_t getLastSoundIndex() const { return lastSoundIndex_; }

    // Partie Enregistrement
    void toggleRecord() { recording_ = !recording_; }
    bool isRecording() const { return recording_; }
    void startRecord();
    void stopRecord();
    void recordStep(size_t soundIndex);
    void deleteStepAtCurrentPosition(); // Nouvelle fonction pour supprimer un pas au pas courant


private:
    bool playing_;
    bool clicking_;
    double bpm_;
    int beatCounter_;
    AudioMixer* mixer_; // Pointeur vers l'AudioMixer
    std::vector<bool> isMuted_; // true si le son est muté
    size_t lastSoundIndex_; // Nouveau membre privé
    size_t numSounds_;
    bool recording_ = false; // Nouvelle variable pour l'état d'enregistrement

    SoundPtr getSound(size_t soundIndex); 

};
//==== End of class DrumPlayer ====

} // namespace adikdrum

#endif // DRUMPLAYER_H
