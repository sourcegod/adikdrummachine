#ifndef DRUMPLAYER_H
#define DRUMPLAYER_H

#include <vector>
#include <memory> // Pour std::shared_ptr
#include "audiosound.h"
#include "audiomixer.h" // Assurez-vous que l'inclusion est là
#include "adikpattern.h"
#include "quantizer.h"

#include <cmath>
#include <algorithm> // pour std::clamp
#include <chrono>
#include <map>

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
    std::vector<std::vector<std::vector<bool>>> patternData_;
    size_t currentBar_;  // Supposons que ces membres existent et sont gérés
    size_t numTotalBars_ =0;

    // Nouvelle structure pour stocker les enregistrements en attente
    // Tuple: soundIndex, barIndex, stepIndex
    std::vector<std::tuple<int, size_t, size_t>> pendingRecordings_;

    void playSound(size_t soundIndex);
    void stopAllSounds();
    void playMetronome();
    void playPattern(size_t mergeIntervalSteps=16);

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
    bool deleteStepAtPos(int soundIndex, size_t currentStep, size_t currentBar); 
    bool clearSoundFromPattern(int soundIndex);
    bool clearPattern();

    // Fonction pour ajouter un enregistrement en attente
    void addPendingRecording(int soundIndex, size_t barIndex, size_t stepIndex);
    
    // Fonction pour fusionner les enregistrements en attente dans le pattern courant
    bool mergePendingRecordings();

    // Assurez-vous d'avoir des getters pour currentBar_ et currentStep_
    size_t getCurrentBar() const { return currentBar_; }
    // Fonction pour calculer la moyenne de latence
    double calculateAverageLatency() const;
    void setRecQuantizeResolution(size_t resolution);
    size_t quantizeRecordedSteps(size_t currentStep, std::chrono::high_resolution_clock::time_point keyPressTime);
    void setPlayQuantizeResolution(size_t resolution); // --- NOUVEAU: Pour quantPlayReso_ (lecture/édition) ---
    void quantizePlayedSteps(); // --- NOUVEAU: Pour appliquer la quantification au pattern en mémoire ---
    bool genStepsFromSound();
    bool quantizeStepsFromSound();





private:
    bool playing_;
    bool recording_ = false; // Nouvelle variable pour l'état d'enregistrement
    bool clicking_;
    double bpm_;
    int beatCounter_;
    AudioMixer* mixer_; // Pointeur vers l'AudioMixer
    std::vector<bool> isMuted_; // true si le son est muté
    size_t numSounds_;
    size_t lastSoundIndex_; // Nouveau membre privé
    // Variables pour la gestion du temps de lecture
    std::chrono::high_resolution_clock::time_point lastUpdateTime_;
    std::chrono::high_resolution_clock::time_point lastKeyPressTime_; // Temps de la dernière frappe de touche
    double stepsPerBeat_; // Par exemple 4 pour des 16èmes de notes


    // Membre pour stocker les latences récentes
    std::vector<double> recentLatencies_;
    const size_t maxRecentLatencies_ = numSteps_; // Nombre de latences à conserver pour la moyenne
    size_t quantRecReso_; // 0: Désactivé, 1: Mesure, 2: Demi-Mesure, etc.
    size_t quantPlayReso_; // --- NOUVEAU: Résolution de quantification pour la lecture/édition ---
    std::map<size_t, size_t> quantResolutionMap;



    SoundPtr getSound(size_t soundIndex); 
    bool isValidForSoundOperation(const std::string& functionName) const; 
    size_t getQuantUnitSteps() const; 
    // Instance de la classe Quantizer pour gérer toute la logique de quantification
    // Nous utilisons un unique_ptr car DrumPlayer "possède" le quantificateur.
    std::unique_ptr<Quantizer> quantizer_;



};
//==== End of class DrumPlayer ====

} // namespace adikdrum

#endif // DRUMPLAYER_H
