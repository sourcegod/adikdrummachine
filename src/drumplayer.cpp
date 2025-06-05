#include "drumplayer.h"
#include "audiosound.h"
#include "audiomixer.h"
#include "adikpattern.h"

#include <cmath>
#include <vector>
#include <iostream>
#include <memory>
#include <algorithm> // pour std::clamp
#include <chrono>


namespace adikdrum {

DrumPlayer::DrumPlayer(int numSounds, int numSteps)
    : currentStep_(0),
      clickStep_(0),
      // pattern_(numSounds, std::vector<bool>(numSteps, false)),
      numSteps_(numSteps),
      sampleRate_(44100),
      playing_(false),
      recording_(false),
      clicking_(false),
      bpm_(100),
      beatCounter_(0),
      mixer_(nullptr), // Initialiser à nullptr
      isMuted_(numSounds, false), // Initialiser tous les sons comme non mutés
      numSounds_(numSounds),
      lastSoundIndex_(0),
      lastUpdateTime_(std::chrono::high_resolution_clock::now()), // Initialisation de lastUpdateTime_
      lastKeyPressTime_(std::chrono::high_resolution_clock::now()), // *** AJOUTEZ CETTE LIGNE ***
      stepsPerBeat_(4.0), // Initialisation de stepsPerBeat_ (4 pour des 16èmes)
      quantRecReso_(16),
      quantPlayReso_(0)

{
    setBpm(bpm_);
    std::cout << "DrumPlayer::Constructor - numSteps_: " << numSteps_ << std::endl;
    // Création d'un objet AdikPattern avec 2 barres
    curPattern_ = std::make_shared<AdikPattern>(2);
    patternData_ = curPattern_->getPatternData();
    curPattern_->setPosition(0, 0);

    quantResolutionMap[1] = numSteps_;       // Ronde = Mesure complète (16 steps)
    quantResolutionMap[2] = numSteps_ / 2;   // Blanche = Demi-mesure (8 steps)
    quantResolutionMap[4] = numSteps_ / 4;   // Noire = Quart de mesure (4 steps)
    quantResolutionMap[8] = numSteps_ / 8;   // Croche = Huitième de mesure (2 steps)
    quantResolutionMap[16] = numSteps_ / 16; // Double-Croche = Seizième de mesure (1 step)

}
//----------------------------------------

DrumPlayer::~DrumPlayer() {
    // Les shared_ptr se chargeront de la gestion de la durée de vie des AudioSound
}
//----------------------------------------

void DrumPlayer::setMixer(AudioMixer& mixer) {
    mixer_ = &mixer;
}
//----------------------------------------

bool DrumPlayer::isValidForSoundOperation(const std::string& functionName) const {
    if (!curPattern_) {
        std::cerr << "ERREUR: Aucun pattern chargé dans DrumPlayer::" << functionName << "." << std::endl;
        return false;
    }

    // getLastPlayedSoundIndex() n'existe pas en tant que méthode séparée, mais lastSoundIndex_
    // est un membre direct. Nous pouvons l'utiliser directement.
    if (lastSoundIndex_ >= curPattern_->getNumSoundsPerBar()) {
        std::cerr << "Erreur: Aucun son valide n'a été joué récemment dans DrumPlayer::" << functionName << "." << std::endl;
        return false;
    }
    
    return true;
}
//----------------------------------------




/*
void DrumPlayer::playSound0(int soundIndex) {
  // Note: this function is reserved for latter
    if (soundIndex >= 0 && soundIndex < drumSounds_.size() && drumSounds_[soundIndex]) {
        // Trouver un canal libre et non réservé et jouer le son
        for (auto i = 1; i < drumSounds_.size(); ++i) { // Commencer à partir du canal 1 (en supposant que 0 est réservé)
            if (!mixer_->isChannelActive(i)
                && !mixer_->isChannelReserved(i)) {
               mixer_->play(i, drumSounds_[soundIndex]);
               return; // Jouer le son sur le premier canal non réservé trouvé
            }
        }
        // Le message d'erreur s'affiche uniquement si aucun canal n'a été trouvé
        std::cerr << "Erreur: Aucun canal non réservé et inactif disponible pour jouer le son (index: "
                  << soundIndex << ")" << std::endl;
    }
}
*/
//----------------------------------------

SoundPtr DrumPlayer::getSound(size_t soundIndex) {
    if (soundIndex < drumSounds_.size()) {
        return drumSounds_[soundIndex];
    } else {
        std::cerr << "Erreur: Index de son hors limites: " << soundIndex << std::endl;
        return nullptr;
    }
}
//----------------------------------------

void DrumPlayer::playSound(size_t soundIndex) {
    auto sound = getSound(soundIndex);
    if (sound) {
        mixer_->play(soundIndex+1, sound);
        lastSoundIndex_ = soundIndex;
    } else {
        std::cerr << "Erreur: Aucun son trouvé avec cet (index: "
                  << soundIndex << ")" << std::endl;
    }
}
//----------------------------------------

void DrumPlayer::playLastSound() {
    SoundPtr lastSound = getSound(lastSoundIndex_);
    if (lastSound) {
        const int channelIndex = 31;
        // Créer une *copie* du AudioSound en utilisant le constructeur de copie
        SoundPtr soundToPlay = std::make_shared<AudioSound>(*lastSound); // Appel explicite au constructeur de copie
        mixer_->play(channelIndex, soundToPlay); // Jouer la copie

    }
}
//----------------------------------------

void DrumPlayer::startPlay() {
    playing_ = true;
}
//----------------------------------------

void DrumPlayer::stopPlay() {
    playing_ = false;
    stopRecord(); // Important : Arrête l'enregistrement quand la lecture est arrêtée
}
//----------------------------------------

void DrumPlayer::stopAllSounds() {
    auto& chanList = mixer_->getChannelList();  
    for (auto& chan : chanList) {
        chan.setActive(0);
        chan.curPos =0;
    }

    for (const auto& sound : drumSounds_) {
        if (sound) {
            sound->setActive(false);
        }
    }

    playing_ = false;
    recording_ = false;
    clicking_ = false;
    currentStep_ = 0;
    clickStep_ = 0;
    beatCounter_ = 0;
}
//----------------------------------------


void DrumPlayer::startClick() {
    clicking_ = true;
    if (playing_) {
      clickStep_ = currentStep_;
      beatCounter_ = clickStep_ % numSteps_;
    } else {
      clickStep_ =0;
      beatCounter_ =0;
    }

    if (mixer_) {
        mixer_->setChannelActive(0, true); 
    }
    // Activer les sons du métronome (s'assurer qu'ils sont chargés)
    if (soundClick1_) {
        soundClick1_->setActive(true);
    } if (soundClick2_) {
        soundClick2_->setActive(true);
    }

}
//----------------------------------------

void DrumPlayer::stopClick() {
    clicking_ = false;
    if (mixer_) {
        if (mixer_->isChannelActive(0)) {
            mixer_->stop(0);
        }
    }
    if (soundClick1_) {
        soundClick1_->setActive(false);
    } if (soundClick2_) {
        soundClick2_->setActive(false);
    }

}
//----------------------------------------

void DrumPlayer::playMetronome() {
    if (playing_) {
        // valable aussi pour si currentStep =0 et beatCounter =0
        beatCounter_ = currentStep_ / 4; // Note: Le résultat est une division entière puisque les deux nombres sont des entiers.
    }
    
    if (mixer_) {
        if (beatCounter_ % 4 == 0 && soundClick1_) {
            mixer_->play(0, soundClick1_);
        } else if (soundClick2_) {
            mixer_->play(0, soundClick2_);
        }
        beatCounter_ = (beatCounter_ + 1) % 4;
    }
    // clickStep_ = (clickStep_ + 1) % 16;

}
//----------------------------------------

void DrumPlayer::playPattern(size_t mergeIntervalSteps) {
    if (mixer_ && playing_) {
        if (curPattern_) {
            // *** Mettre à jour lastUpdateTime_ ICI, au début de chaque "pas" logique ***
            lastUpdateTime_ = std::chrono::high_resolution_clock::now();
            // auto endTime = std::chrono::high_resolution_clock::now();
            // std::chrono::duration<double> lastTime = lastUpdateTime_ - endTime;
            // std::cout << "DEBUG, in playPattern: lastUpdateTime: " << lastTime.count() << "\n";

            currentBar_ = curPattern_->getCurrentBar();
            numTotalBars_ = curPattern_->getNumBars();
            numSteps_ = curPattern_->getBarLength(currentBar_);

            // Jouer les sons du pas actuel (cette partie reste inchangée)
            auto& currentBarData = curPattern_->getPatternData()[currentBar_];
            for (size_t i = 0; i < currentBarData.size(); ++i) {
                if (currentStep_ < currentBarData[i].size() &&
                    currentBarData[i][currentStep_]) {
                    if (drumSounds_[i]) {
                        mixer_->play(static_cast<int>(i + 1), drumSounds_[i]);
                    }
                }
            }

            // Incrémente le pas courant
            currentStep_++;

            // Si le pas courant dépasse la longueur de la barre actuelle
            if (currentStep_ >= numSteps_) {
                currentStep_ = 0;
                size_t nextBarIndex = currentBar_ + 1;

                if (nextBarIndex >= numTotalBars_) {
                    nextBarIndex = 0;
                }
                curPattern_->setCurrentBar(nextBarIndex);
                currentBar_ = nextBarIndex;
            }
            curPattern_->setCurrentStep(currentStep_);

            // *** NOUVELLE LOGIQUE POUR LA FUSION : DÉCLENCHEMENT CONDITIONNEL ***
            // La fusion est déclenchée si le pas courant (après incrémentation)
            // est un multiple de 'mergeIntervalSteps'.
            // Si 'mergeIntervalSteps' est 16 (une mesure), ça se déclenchera au début de chaque mesure.
            // Si 'mergeIntervalSteps' est 4 (un beat), ça se déclenchera au début de chaque beat.
            // Assurez-vous que numSteps_ est le nombre de pas d'une mesure (ex: 16)
            // Et currentStep_ est le pas **après** l'incrémentation ci-dessus, donc c'est le pas pour le PROCHAIN cycle.

            // On va déclencher la fusion juste *avant* que le pas courant ne soit joué si c'est un intervalle de fusion
            // Ou plus simplement, au moment où on franchit un seuil de fusion.

            // Option 1: Déclencher si on vient de franchir un pas qui est un multiple de mergeIntervalSteps
            // (Cela signifie que les enregistrements du pas précédent seront fusionnés)
            if (currentStep_ % mergeIntervalSteps == 0) {
                 mergePendingRecordings();
            }

            // Si le mergeIntervalSteps est égal à numSteps_ (16), cela équivaut à la fusion par mesure.
            // Si le mergeIntervalSteps est égal à stepsPerBeat_ (4), cela équivaut à la fusion par beat.


        } else {
            std::cerr << "Erreur: curPattern_ n'est pas initialisé dans DrumPlayer::playPattern." << std::endl;
        }
    }

}
//----------------------------------------


/*
// Marche bien avec fusion des enregistrements par mesure
void DrumPlayer::playPattern3() {
    if (mixer_ && playing_) { // Condition d'origine maintenue
        if (curPattern_) {

            // Récupère l'index de la barre courante depuis l'objet AdikPattern
            currentBar_ = curPattern_->getCurrentBar();
            // Récupère le nombre total de barres
            numTotalBars_ = curPattern_->getNumBars();
            // Récupère le nombre de pas dans la barre actuelle
            numSteps_ = curPattern_->getBarLength(currentBar_); // Votre variable numSteps_ est mise à jour ici

            // Pour chaque son dans la barre actuelle, vérifie si la note est active à l'étape courante
            // Utilisation de la référence locale pour la lisibilité
            auto& currentBarData = curPattern_->getPatternData()[currentBar_];
            for (size_t i = 0; i < currentBarData.size(); ++i) { // Utilise currentBarData.size()
                if (currentStep_ < currentBarData[i].size() &&
                    currentBarData[i][currentStep_]) {
                    if (drumSounds_[i]) {
                        mixer_->play(static_cast<int>(i + 1), drumSounds_[i]);
                    }
                }
            }

            // Incrémente le pas courant (logique conservée)
            currentStep_++;

            // Si le pas courant dépasse la longueur de la barre actuelle (logique conservée)
            if (currentStep_ >= numSteps_) {
                currentStep_ = 0;
                size_t nextBarIndex = currentBar_ + 1;

                // Si la barre suivante dépasse le nombre total de barres, revient à la première barre
                if (nextBarIndex >= numTotalBars_) {
                    nextBarIndex = 0;
                }
                // Met à jour la barre courante dans l'objet AdikPattern
                curPattern_->setCurrentBar(nextBarIndex);

                // *** NOUVELLE LOGIQUE POUR LA FUSION : DÉCLENCHEMENT À CHAQUE NOUVELLE MESURE ***
                // Ceci est appelé uniquement lorsque currentStep_ repasse à 0 (nouvelle mesure)
                mergePendingRecordings();
            }
        } else {
            std::cerr << "Erreur: curPattern_ n'est pas initialisé dans DrumPlayer::playPattern." << std::endl;
        }
    }
}
*/


/*
// Marche mal, car fusion des enregistrements à chaque pas, ce qui déclenche un double son
void DrumPlayer::playPattern2() {
    if (mixer_ && playing_) { // Condition d'origine maintenue
        // Vérifie si curPattern_ est valide
        if (curPattern_) {

            // --- NOUVELLE LOGIQUE : GESTION DU TEMPS ET FUSION DES ENREGISTREMENTS ---
            // Le but est de s'assurer que la fusion se fait au bon moment, même si le callback n'est pas parfaitement synchrone.
            // On calcule combien de "pas" logiques se sont écoulés depuis la dernière exécution.
            auto now = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = now - lastUpdateTime_;

            double beatsPerSecond = bpm_ / 60.0;
            double stepsPerSecond = beatsPerSecond * stepsPerBeat_;
            double stepsPassed = elapsed.count() * stepsPerSecond;

            // Si au moins un pas logique s'est écoulé, nous fusionnons.
            // Cela permet de ne pas surcharger la fusion si le callback est appelé très fréquemment,
            // et de capturer les événements si le callback est appelé moins fréquemment.
            if (stepsPassed >= 1.0) {
                mergePendingRecordings(); // Fusionne les enregistrements en attente
                // Mettre à jour lastUpdateTime_ pour le prochain calcul
                lastUpdateTime_ = now;
            }
            // --- FIN DE LA NOUVELLE LOGIQUE ---


            // Récupère l'index de la barre courante depuis l'objet AdikPattern
            // Cette ligne était déjà là et est conservée.
            currentBar_ = curPattern_->getCurrentBar();
            // Récupère le nombre total de barres
            numTotalBars_ = curPattern_->getNumBars();
            // Récupère le nombre de pas dans la barre actuelle
            numSteps_ = curPattern_->getBarLength(currentBar_); // Votre variable numSteps_ est mise à jour ici

            // Pour chaque son dans la barre actuelle, vérifie si la note est active à l'étape courante
            for (size_t i = 0; i < curPattern_->getPatternData()[currentBar_].size(); ++i) {
                // Si la note est active à l'étape actuelle (currentStep est un membre de DrumPlayer)
                if (currentStep_ < curPattern_->getPatternData()[currentBar_][i].size() && // Vérification de la limite de currentStep
                    curPattern_->getPatternData()[currentBar_][i][currentStep_]) {
                    if (drumSounds_[i]) {
                        // Jouer le son sur le canal correspondant (i + 1)
                        mixer_->play(static_cast<int>(i + 1), drumSounds_[i]);
                    }
                }
            }

            // Incrémente le pas courant (cette logique est conservée)
            currentStep_++;

            // Si le pas courant dépasse la longueur de la barre actuelle (cette logique est conservée)
            if (currentStep_ >= numSteps_) {
                currentStep_ = 0;
                size_t nextBarIndex = currentBar_ + 1; // Passe à la barre suivante

                // Si la barre suivante dépasse le nombre total de barres, revient à la première barre
                if (nextBarIndex >= numTotalBars_) {
                    nextBarIndex = 0; // Boucle vers la première barre
                }
                // Met à jour la barre courante dans l'objet AdikPattern
                curPattern_->setCurrentBar(nextBarIndex);
                // currentBar_ est mis à jour par curPattern_->getCurrentBar() au début du prochain appel.
            }
        } else {
            // Gérer le cas où curPattern_ n'est pas initialisé (par exemple, afficher un message d'erreur)
            std::cerr << "Erreur: curPattern_ n'est pas initialisé dans DrumPlayer::playPattern." << std::endl;
        }
    }
}
//----------------------------------------
*/



/*
void DrumPlayer::playPattern0() {
    if (mixer_ && playing_) {
        for (size_t i = 0; i < pattern_.size(); ++i) { //
            if (pattern_[i][currentStep_]) {
                if (drumSounds_[i]) {
                    
                     
                    // On ne cherche plus un canal libre  pour jouer, chaque canal à son index de son
                    // if (!mixer_->isChannelActive(i + 1)) { // Utiliser les canaux 1 à NUM_SOUNDS
                    //    mixer_->play(i + 1, drumSounds_[i]);
                    // }
                    // 
                    
                    mixer_->play(i + 1, drumSounds_[i]);
                }
            }
        }
    }
}
//----------------------------------------
*/

void DrumPlayer::setBpm(double newBpm) {
    if (newBpm >=5 &&  newBpm <= 800) {
        bpm_ = newBpm;
        secondsPerStep = (60.0 / bpm_) / 4.0;
    }

}
//----------------------------------------

bool DrumPlayer::isSoundPlaying() const {
    for (const auto& sound : drumSounds_) {
        if (sound && sound->isActive()) {
            return true;
        }
    }
    return false;
}
//----------------------------------------

bool DrumPlayer::isSoundMuted(size_t soundIndex) const {
    if (soundIndex < isMuted_.size()) {
        return isMuted_[soundIndex];
    }
    return false; // Par défaut, non muté si l'index est invalide
}
//----------------------------------------

void DrumPlayer::setSoundMuted(size_t soundIndex, bool muted) {
    if (mixer_ && soundIndex < drumSounds_.size()) {
        int channelToMute = soundIndex+1;
        mixer_->setChannelMuted(channelToMute, muted); // Utiliser la fonction de AudioMixer
        isMuted_[soundIndex] = muted; // Garder une trace locale si nécessaire
        std::cout << "Son " << soundIndex<< " (canal " << channelToMute << ") est maintenant " << (muted ? "muté" : "démuté") << "." << std::endl;
    }
}
//----------------------------------------

void DrumPlayer::resetMute() {
    if (mixer_) {
        mixer_->resetMute(); // S'assurer que le mixer est aussi réinitialisé
    }

    // Note: best way to modify directly an vector
    std::fill(isMuted_.begin(), isMuted_.end(), false);
    std::cout << "Tous les sons ont été démutés." << std::endl;
}
//----------------------------------------

void DrumPlayer::startRecord() {
    recording_ = true;
    startClick(); // Démarre le métronome
    playing_ = true; // Démarre la lecture du pattern
    // Vous pouvez également ajouter ici une logique pour réinitialiser le pattern
    // si vous voulez que l'enregistrement commence sur un pattern vide.
    // curPattern_->clearData(); // Exemple: si vous voulez effacer le pattern
}
//----------------------------------------

void DrumPlayer::stopRecord() {
    recording_ = false;
    stopClick(); // Arrête le métronome
    // La lecture (playing_) continue comme demandé
}
//----------------------------------------

void DrumPlayer::recordStep(size_t soundIndex) {
    if (!recording_) {
        return; // Ne fait rien si l'enregistrement n'est pas actif
    }

    if (curPattern_) {
        size_t currentBar = curPattern_->getCurrentBar();
        // Utilise currentStep_ qui est le pas de lecture actuel du DrumPlayer
        // Au lieu de currentStep directement, utilisez la variable membre currentStep_
        // de DrumPlayer qui est mise à jour par la boucle de lecture.
        size_t currentStep = currentStep_; 

        // S'assurer que l'index du son et le pas sont valides
        if (soundIndex < numSounds_ && currentStep < curPattern_->getNumSteps()) {
            curPattern_->getPatternBar(currentBar)[soundIndex][currentStep] = true;
            playSound(soundIndex); // Joue le son immédiatement lors de l'enregistrement
        }
    } else {
        // En mode console, on ne peut pas afficher de message directement depuis ici.
        // C'est AdikDrum qui gère l'affichage des messages.
        std::cerr << "Erreur: Aucun pattern chargé pour enregistrer un pas dans DrumPlayer." << std::endl;
    }
}
//----------------------------------------

bool DrumPlayer::deleteStepAtPos(int soundIndex, size_t currentStep, size_t currentBar) {
    if (!curPattern_) {
        std::cerr << "Erreur interne: Aucun pattern chargé dans DrumPlayer::deleteStepAtPos." << std::endl;
        return false;
    }

    // Vérifications d'indices plus robustes ici, car cette fonction peut être appelée de n'importe où
    if (soundIndex >= 0 && static_cast<size_t>(soundIndex) < numSounds_ &&
        currentStep < curPattern_->getNumSteps() &&
        currentBar < curPattern_->getNumBars()) { // Ajout de la vérification de la barre

        curPattern_->getPatternBar(currentBar)[soundIndex][currentStep] = false;
        return true; // Suppression réussie
    } else {
        std::cerr << "Erreur interne: Indices de suppression invalides dans DrumPlayer::deleteStepAtPos." << std::endl;
        return false; // Suppression échouée (indices invalides)
    }
}
//----------------------------------------


bool DrumPlayer::clearSoundFromPattern(int soundIndex) {
    if (!curPattern_) {
        std::cerr << "Erreur interne: Aucun pattern chargé dans DrumPlayer::clearSoundFromPattern." << std::endl;
        return false;
    }

    // Vérifier si l'index du son est valide
    if (soundIndex < 0 || static_cast<size_t>(soundIndex) >= numSounds_) {
        std::cerr << "Erreur interne: Index de son invalide dans DrumPlayer::clearSoundFromPattern." << std::endl;
        return false;
    }

    // Parcourir toutes les barres et tous les pas pour désactiver le son
    size_t totalBars = curPattern_->getNumBars(); // Assurez-vous que getNumBars() retourne le nombre total de barres
    size_t totalSteps = curPattern_->getNumSteps();

    bool changed = false;
    for (size_t barIdx = 0; barIdx < totalBars; ++barIdx) {
        for (size_t stepIdx = 0; stepIdx < totalSteps; ++stepIdx) {
            if (curPattern_->getPatternBar(barIdx)[soundIndex][stepIdx]) {
                curPattern_->getPatternBar(barIdx)[soundIndex][stepIdx] = false;
                changed = true;
            }
        }
    }
    return changed; // Indique si des modifications ont été apportées
}
//----------------------------------------

bool DrumPlayer::clearPattern() {
    if (!curPattern_) {
        std::cerr << "Erreur interne: Aucun pattern chargé dans DrumPlayer::clearPattern." << std::endl;
        return false;
    }

    // Récupérer le nombre total de mesures, de sons et de pas par mesure
    size_t totalBars = curPattern_->getNumBars();
    size_t totalSounds = numSounds_; // curPattern_->getNumSounds();
    size_t totalSteps = curPattern_->getNumSteps();

    bool changed = false;

    // Parcourir toutes les mesures
    for (size_t barIdx = 0; barIdx < totalBars; ++barIdx) {
        // Obtenir la référence à la barre de pattern actuelle pour éviter de la copier
        // Assurez-vous que getPatternBar() renvoie une référence ou un moyen modifiable.
        // Si getPatternBar() renvoie const, vous aurez besoin d'une version non-const ou d'une autre approche.
        // Supposons que getPatternBar(barIdx) renvoie un std::vector<std::vector<bool>>&
        auto& currentBar = curPattern_->getPatternBar(barIdx);

        // Parcourir tous les sons dans cette mesure
        for (size_t soundIdx = 0; soundIdx < totalSounds; ++soundIdx) {
            // Parcourir tous les pas pour ce son
            for (size_t stepIdx = 0; stepIdx < totalSteps; ++stepIdx) {
                // Si le step est actif, le désactiver et marquer qu'un changement a eu lieu
                if (currentBar[soundIdx][stepIdx]) {
                    currentBar[soundIdx][stepIdx] = false;
                    changed = true;
                }
            }
        }
    }

    // Le pattern a été modifié, donc l'interface utilisateur devra peut-être être rafraîchie.
    // Vous pouvez déclencher un événement ou mettre à jour un flag si nécessaire.
    return changed; // Indique si le pattern a été effacé (si quelque chose était actif)
}
//----------------------------------------

void DrumPlayer::addPendingRecording(int soundIndex, size_t barIndex, size_t stepIndex) {
    // Il est important de s'assurer que curPattern_ est valide ici
    // et que les indices sont dans les limites.
    if (!curPattern_ || soundIndex < 0 || static_cast<size_t>(soundIndex) >= numSounds_ ||
        barIndex >= curPattern_->getNumBars() || stepIndex >= curPattern_->getNumSteps()) {
        std::cerr << "Erreur: Tentative d'ajouter un enregistrement en attente invalide." << std::endl;
        return;
    }
    pendingRecordings_.emplace_back(soundIndex, barIndex, stepIndex);
}
//----------------------------------------

bool DrumPlayer::mergePendingRecordings() {
    if (pendingRecordings_.empty() || !curPattern_) {
        return false;
    }

    bool changed = false;
    // On itère sur une copie pour vider immédiatement pendingRecordings_
    std::vector<std::tuple<int, size_t, size_t>> recordingsToMerge = pendingRecordings_;
    pendingRecordings_.clear();

    for (const auto& rec : recordingsToMerge) {
        int soundIndex = std::get<0>(rec);
        size_t barIndex = std::get<1>(rec);
        size_t stepIndex = std::get<2>(rec);

        if (barIndex < curPattern_->getNumBars() && stepIndex < curPattern_->getNumSteps() &&
            soundIndex >= 0 && static_cast<size_t>(soundIndex) < numSounds_)
        {
            // Utilisation de la référence locale pour la lisibilité
            auto& targetBar = curPattern_->getPatternBar(barIndex);
            if (!targetBar[soundIndex][stepIndex]) {
                targetBar[soundIndex][stepIndex] = true;
                changed = true;
            }
        }
    }
    return changed;
}
//----------------------------------------

// /*
size_t DrumPlayer::quantizeRecordedSteps(size_t currentStep, std::chrono::high_resolution_clock::time_point keyPressTime) {
    // --- PARTIE 1 : COMPENSATION DE LATENCE (INCHANGÉE) ---
    const double resetThresholdMs = 50.0;
    double secondsPerStep = (60.0 / bpm_) / stepsPerBeat_; // Durée d'un pas de base (seizième de note)
    std::chrono::duration<double> rawLatency = keyPressTime - lastUpdateTime_;
    double rawLatencyMs = rawLatency.count() * 1000.0;
    double stepDurationMs = secondsPerStep * 1000.0;

    if (!recentLatencies_.empty()) {
        double currentAverage = calculateAverageLatency();
        if (std::abs(rawLatencyMs - currentAverage) > resetThresholdMs) {
            std::cout << "\nDEBUG: Changement significatif détecté (" << rawLatencyMs << "ms vs Moyenne " << currentAverage << "ms). Réinitialisation de l'historique de latence." << std::endl;
            recentLatencies_.clear();
        }
    }
    recentLatencies_.push_back(rawLatencyMs);
    while (recentLatencies_.size() > maxRecentLatencies_) {
        recentLatencies_.erase(recentLatencies_.begin());
    }
    double averageLatencyMs = calculateAverageLatency();
    double compensatedLatencyMs = rawLatencyMs - averageLatencyMs;
    // --- FIN DE LA PARTIE 1 ---

    std::cout << "\nMesure: " << currentBar_ << ", Pas: " << currentStep_ << std::endl;
    std::cout << "DEBUG: Latence brute frappe : " << rawLatencyMs << " ms (Moy: " << averageLatencyMs << " ms, Taille Hist: " << recentLatencies_.size() << ")" << std::endl;
    std::cout << "DEBUG: Latence compensée : " << compensatedLatencyMs 
        << " ms par rapport au début de la mesure: " << currentBar_ 
        << ", Pas: " << currentStep << "." << std::endl;
    std::cout << "DEBUG: Chaque pas de *base* dure " << stepDurationMs << " ms." << std::endl;

    size_t quantizedStep = currentStep; // Valeur par défaut si rien n'est quantifié

    // --- PARTIE 2 : CALCUL DU PAS CIBLE APRÈS COMPENSATION ---
    // On convertit la latence compensée en un décalage en nombre de *pas de base*.
    // C'est la position "réelle" de la frappe sur la grille des seizièmes de note.
    double stepOffset = compensatedLatencyMs / stepDurationMs; // Ex: 0.5 signifie un demi-pas en avant
    double targetStepDouble = static_cast<double>(currentStep) + stepOffset;

    // Arrondit au pas de base (seizième) le plus proche
    // Utilise round() pour arrondir au plus proche, car stepOffset peut être négatif ou positif
    size_t initialQuantizedBaseStep = static_cast<size_t>(std::round(targetStepDouble));

    // Gérer le "wrap-around" si initialQuantizedBaseStep dépasse ou est négatif
    if (initialQuantizedBaseStep >= numSteps_) {
        initialQuantizedBaseStep = initialQuantizedBaseStep % numSteps_; // S'assure que c'est dans la mesure
    }
    // Gérer les cas où targetStepDouble serait négatif et round() le laisserait comme tel,
    // ou si on a un wrap-around sur la mesure précédente (ex: -1 -> 15)
    // std::round(targetStepDouble) avec targetStepDouble = -0.5 donnera 0, -0.6 donnera -1.
    // Il faut que notre pas soit toujours positif et dans la mesure.
    if (targetStepDouble < 0) {
        // Si la frappe est très en avance, elle pourrait se retrouver sur le pas précédent de la mesure précédente.
        // On la cale sur la mesure actuelle pour simplifier, sauf si on veut gérer ça.
        // Pour l'instant, on se contente de la ramener dans la mesure.
        // Si targetStepDouble est -0.1, round donne 0. Si -0.6, round donne -1.
        // Mais nous voulons qu'un -0.6 soit le pas 15 de la mesure précédente.
        // Pour simplifier, si c'est négatif, on le cale sur 0, sauf si on a activé la quantification au pas précédent.
        // Comme nous avons commenté la logique de quantification au pas précédent,
        // nous allons simplement nous assurer que c'est >= 0.
        initialQuantizedBaseStep = initialQuantizedBaseStep % numSteps_;
        // Note: initialBaseStep est un size_t, donc ne peut être négatif
        // if (initialQuantizedBaseStep < 0) initialQuantizedBaseStep += numSteps_; // Au cas où modulo retournerait un négatif
    }
    // Assurer que initialQuantizedBaseStep est bien dans les limites 0 à numSteps_-1
    initialQuantizedBaseStep = initialQuantizedBaseStep % numSteps_;


    std::cout << "DEBUG: Pas de base cible initial (après compensation): " << initialQuantizedBaseStep << std::endl;

    // --- PARTIE 3 : APPLICATION DE LA RÉSOLUTION DE QUANTIFICATION ---
    if (quantRecReso_ == 0) {
        // Pas de quantification : on utilise le pas de base le plus proche après compensation
        quantizedStep = initialQuantizedBaseStep;
        std::cout << "DEBUG: Pas de quantification : Utilise le pas de base : " << quantizedStep << " et resolution est: " << quantRecReso_ << std::endl;
    } else {
        // Quantification active : calage sur la grille choisie
        auto it = quantResolutionMap.find(quantRecReso_);
        if (it != quantResolutionMap.end()) {
            size_t quantUnitSteps = it->second; // Nombre de pas pour cette résolution (ex: 4 pour noire)

            // Calcule le début de l'unité de quantification où se trouve initialQuantizedBaseStep
            size_t quantGridStartStep = (initialQuantizedBaseStep / quantUnitSteps) * quantUnitSteps;

            // Calcule la moitié de l'unité de quantification pour l'arrondi
            size_t halfQuantUnitSteps = quantUnitSteps / 2;

            // Détermine la position de initialQuantizedBaseStep dans son unité de quantification
            size_t positionInQuantUnit = initialQuantizedBaseStep - quantGridStartStep;

            if (positionInQuantUnit >= halfQuantUnitSteps) {
                // Si la position est dans la seconde moitié de l'unité, on arrondit à l'unité suivante
                quantizedStep = quantGridStartStep; // + quantUnitSteps;
                if (quantizedStep >= numSteps_) {
                    quantizedStep = 0; // Gérer le wrap-around de la mesure
                }
                std::cout << "DEBUG: Quantification: ne pas Arrondi à l'unité de résolution suivante (" << quantUnitSteps << " pas) : " << quantizedStep << std::endl;
            } else {
                // Sinon, on arrondit au début de l'unité actuelle
                quantizedStep = quantGridStartStep;
                std::cout << "DEBUG: Quantification: Arrondi à l'unité de résolution actuelle (" << quantUnitSteps << " pas) : " << quantizedStep << std::endl;
            }
        } else {
            // Si quantRecReso_ n'est pas valide (pas dans la map), fallback à la double-croche (1 step)
            std::cerr << "AVERTISSEMENT: Résolution de quantification inconnue : " << quantRecReso_ << ". Utilisation de la double-croche (1 step)." << std::endl;
            quantizedStep = initialQuantizedBaseStep; // Pas de calage grossier, juste le pas de base
        }
    }
    std::cout << "\n----------------------------------------\n";

    return quantizedStep;
}
//----------------------------------------
// */


/*
// --- MODIFICATION DE quantizeStep ---
size_t DrumPlayer::quantizeStep(size_t currentStep, std::chrono::high_resolution_clock::time_point keyPressTime) {
    double secondsPerStep = (60.0 / bpm_) / stepsPerBeat_;
    std::chrono::duration<double> rawLatency = keyPressTime - lastUpdateTime_;
    double rawLatencyMs = rawLatency.count() * 1000.0;
    double stepDurationMs = secondsPerStep * 1000.0;

    // --- LOGIQUE DE RÉINITIALISATION DE LA MOYENNE ---
    // Si la pile est vide ou si la nouvelle latence est très différente de la moyenne actuelle,
    // on vide la pile pour "recommencer" la calibration.
    // Le seuil de 50 ms est un exemple, ajuste-le selon ce qui te semble bien.
    const double resetThresholdMs = 50.0; // Seuil pour réinitialiser la moyenne

    if (!recentLatencies_.empty()) {
        double currentAverage = calculateAverageLatency();
        if (std::abs(rawLatencyMs - currentAverage) > resetThresholdMs) {
            std::cout << "DEBUG: Detected significant change (" << rawLatencyMs << "ms vs Avg " << currentAverage << "ms). Resetting latency history." << std::endl;
            recentLatencies_.clear(); // Vide l'historique
        }
    }

    // *** 1. Stocker la latence brute (rawLatencyMs) ***
    recentLatencies_.push_back(rawLatencyMs);
    // Assure-toi que la pile ne dépasse pas maxRecentLatencies_ après ajout,
    // même si on a réinitialisé juste avant.
    while (recentLatencies_.size() > maxRecentLatencies_) {
        recentLatencies_.erase(recentLatencies_.begin()); // Supprime la plus ancienne si trop grand
    }

    // *** 2. Calculer la latence moyenne ***
    double averageLatencyMs = calculateAverageLatency();

    // *** 3. Appliquer la compensation ***
    double compensatedLatencyMs = rawLatencyMs - averageLatencyMs;
    std::chrono::duration<double> compensatedLatency = std::chrono::duration<double>(compensatedLatencyMs / 1000.0);

    std::cout << "DEBUG: Raw Key press latency: " << rawLatencyMs << " ms (Avg: " << averageLatencyMs << " ms, History Size: " << recentLatencies_.size() << ")" << std::endl;
    std::cout << "DEBUG: Compensated Latency: " << compensatedLatencyMs << " ms relative to step " << currentStep << " start." << std::endl;
    std::cout << "DEBUG: Each step lasts " << stepDurationMs << " ms." << std::endl;

    size_t quantizedStep = currentStep;
    double halfStepDuration = secondsPerStep / 2.0;

    // Logique de quantification au pas le plus proche, utilisant la latence COMPENSÉE
    if (compensatedLatency.count() > halfStepDuration) {
        // La frappe est au-delà de la moitié du pas actuel -> quantifie sur le pas suivant
        quantizedStep = (currentStep + 1);
        if (quantizedStep >= numSteps_) {
            quantizedStep = 0; // Gère le retour au début de la mesure
        }
        std::cout << "DEBUG: Quantizing to next step: " << quantizedStep << std::endl;
    }
    // else if (compensatedLatency.count() < -halfStepDuration) {
    //     // La frappe est avant la moitié du pas actuel ET significativement en avance sur le pas précédent
    //     // Si tu commentes cette partie, les latences négatives (même importantes)
    //     // seront quantifiées sur le currentStep.
    //     if (currentStep == 0) {
    //         quantizedStep = numSteps_ - 1;
    //     } else {
    //         quantizedStep = currentStep - 1;
    //     }
    //     std::cout << "DEBUG: Quantizing to previous step: " << quantizedStep << std::endl;
    // }
    else {
        // La frappe est à l'intérieur de la première moitié du pas actuel
        // (y compris les latences négatives faibles ou toute latence négative si la partie ci-dessus est commentée)
        std::cout << "DEBUG: Quantizing to current step: " << quantizedStep << std::endl;
    }


    return quantizedStep;
}
//----------------------------------------
*/

double DrumPlayer::calculateAverageLatency() const {
    if (recentLatencies_.empty()) {
        return 0.0;
    }
    double sum = std::accumulate(recentLatencies_.begin(), recentLatencies_.end(), 0.0);
    return sum / recentLatencies_.size();
}
//----------------------------------------

void DrumPlayer::setRecQuantizeResolution(size_t resolution) {
    if (resolution == 0 || quantResolutionMap.count(resolution)) {
        quantRecReso_ = resolution;
        std::cout << "Résolution d'enregistrement réglée sur : " << quantRecReso_ << std::endl;
    } else {
        std::cerr << "AVERTISSEMENT: Résolution d'enregistrement invalide : " << resolution << ". Garde la résolution précédente." << std::endl;
    }
}
//----------------------------------------

void DrumPlayer::setPlayQuantizeResolution(size_t resolution) {
    // Valider la résolution (doit être une clé valide dans quantResolutionMap ou 0)
    if (resolution == 0 || quantResolutionMap.count(resolution)) {
        quantPlayReso_ = resolution;
        std::cout << "Résolution de lecture/quantification réglée sur : " << quantPlayReso_ << std::endl;
    } else {
        std::cerr << "AVERTISSEMENT: Résolution de lecture/quantification invalide : " << resolution << ". Garde la résolution précédente." << std::endl;
    }
}
//----------------------------------------

void DrumPlayer::quantizePlayedSteps() {
    std::cout << "\nDEBUG: Début de la quantification du pattern en lecture avec résolution : " << quantPlayReso_ << std::endl;

    if (!curPattern_) {
        std::cerr << "ERREUR: curPattern_ n'est pas initialisé (nullptr) dans quantizePlay." << std::endl;
        return;
    }

    if (quantPlayReso_ == 0) {
        std::cout << "DEBUG: Quantification en lecture désactivée (quantPlayReso_ = 0). Aucune modification du pattern." << std::endl;
        return;
    }

    size_t quantUnitSteps = 0;
    auto it = quantResolutionMap.find(quantPlayReso_);

    if (it != quantResolutionMap.end()) {
        quantUnitSteps = it->second;
    } else {
        std::cerr << "AVERTISSEMENT: Résolution de quantification en lecture inconnue : " << quantPlayReso_ << ". Impossible de quantifier." << std::endl;
        return;
    }

    // Create a new AdikPattern object to store the quantized notes.
    // It's crucial to correctly size this new pattern.
    // AdikPattern's constructor takes numBars. Its internal numSoundsPerBar_ is fixed at 16.
    // Its internal numSteps_ (per bar) is fixed at 16.
    // So, we need to ensure this new pattern matches the dimensions we intend to iterate over.
    size_t numBarsInCurPattern = curPattern_->getNumBars();
    size_t numSoundsInCurPattern = curPattern_->getNumSoundsPerBar(); // This is 16 based on your AdikPattern
    size_t numStepsInCurPattern = curPattern_->getNumSteps(); // This is 16 based on your AdikPattern

    // Create a new pattern with the same number of bars as the current one.
    // AdikPattern constructor implicitly sets numSoundsPerBar_ and numSteps_.
    std::shared_ptr<AdikPattern> newPattern = std::make_shared<AdikPattern>(numBarsInCurPattern);


    // Iterate over all bars, sounds, and steps of the current pattern.
    for (size_t barIdx = 0; barIdx < numBarsInCurPattern; ++barIdx) {
        for (size_t soundIdx = 0; soundIdx < numSoundsInCurPattern; ++soundIdx) {
            for (size_t step = 0; step < numStepsInCurPattern; ++step) {
                // If a note exists at this step for this sound in the ORIGINAL pattern
                if (curPattern_->getNote(barIdx, soundIdx, step)) { // --- USAGE: curPattern_->getNote(bar, sound, step) ---
                    // It's the "source" step of the note before quantization
                    size_t initialSourceStep = step;

                    // --- Quantization logic ---
                    size_t quantGridStartStep = (initialSourceStep / quantUnitSteps) * quantUnitSteps;
                    size_t halfQuantUnitSteps = quantUnitSteps / 2;
                    size_t positionInQuantUnit = initialSourceStep - quantGridStartStep;

                    size_t quantizedTargetStep;

                    if (positionInQuantUnit >= halfQuantUnitSteps) {
                        // Pour l'instant on n'arrondi pas la quantize au Pas supérieur
                        quantizedTargetStep = quantGridStartStep; // + quantUnitSteps;
                        if (quantizedTargetStep >= numStepsInCurPattern) { // Use numStepsInCurPattern for wrap-around
                            quantizedTargetStep = 0;
                        }
                    } else {
                        quantizedTargetStep = quantGridStartStep;
                    }

                    // Add the quantized note to the *new* pattern
                    // --- USAGE: newPattern->setNote(bar, sound, step, value) ---
                    newPattern->setNote(barIdx, soundIdx, quantizedTargetStep, true);
                    std::cout << "DEBUG: Son " << soundIdx << " au pas " << initialSourceStep << " (bar " << barIdx << ") quantifié au pas " << quantizedTargetStep << std::endl;
                }
            }
        }
    }

    // Replace the main pattern with the newly quantized pattern
    curPattern_ = newPattern; // --- UPDATE THE SHARED POINTER ---
    std::cout << "DEBUG: Quantification du pattern terminée. Pattern mis à jour." << std::endl;
    std::cout << "\n----------------------------------------" << std::endl;
}
//----------------------------------------

bool DrumPlayer::genStepsFromSound() {
    if (!isValidForSoundOperation("genStepsFromSound")) {
        return false;
    }

    int soundIndex = lastSoundIndex_;

    // Récupérer la mesure courante du pattern
    size_t currentBarIdx = curPattern_->getCurrentBar();
    if (currentBarIdx >= curPattern_->getNumBars()) {
         std::cerr << "Erreur interne: Mesure courante invalide dans DrumPlayer::genStepsFromSound." << std::endl;
         return false;
    }

    // Obtenir la référence à la barre de pattern actuelle pour la modifier
    // Assurez-vous que getPatternBar() renvoie une référence modifiable (non-const)
    auto& currentBar = curPattern_->getPatternBar(currentBarIdx);

    size_t totalSteps = curPattern_->getNumSteps(); // Nombre de pas par mesure (ex: 16)

    // --- Logique de détermination de quantUnitSteps basée sur quantizePlayedSteps ---
    size_t quantUnitSteps = 0;
    auto it = quantResolutionMap.find(quantPlayReso_); // Utilisation de la map

    if (it != quantResolutionMap.end()) {
        quantUnitSteps = it->second;
    } else {
        std::cerr << "AVERTISSEMENT: Résolution de quantification en lecture inconnue : " << quantPlayReso_ << ". Utilisation de 1/16ème (tous les pas)." << std::endl;
        quantUnitSteps = 1; // Si résolution inconnue, par défaut 1/16ème (tous les pas)
    }

    if (quantUnitSteps == 0) { // Si la résolution est 0 (ex: quantPlayReso_ == 0)
        // Ce cas est pour désactiver la quantification, mais pour genStepsFromSound,
        // nous devons générer quelque chose. Si quantUnitSteps est 0, c'est un problème.
        // On pourrait décider d'activer tous les pas ou de ne rien faire.
        // Pour être sécuritaire, si 0 est une valeur possible issue de la map,
        // on peut la traiter comme "tous les pas" (1/16e).
        std::cerr << "AVERTISSEMENT: quantUnitSteps est 0, cela indique potentiellement une quantification désactivée ou une erreur. Activation de tous les pas." << std::endl;
        quantUnitSteps = 1;
        return 0;
    }
    // --- Fin de la logique de détermination de quantUnitSteps ---

    bool changed = false;

    // 1. Désactiver TOUS les pas pour ce son dans la mesure courante
    for (size_t stepIdx = 0; stepIdx < totalSteps; ++stepIdx) {
        if (currentBar[soundIndex][stepIdx]) { // Si le step est actif
            currentBar[soundIndex][stepIdx] = false; // Désactiver
            changed = true;
        }
    }

    // 2. Activer les pas pertinents selon la résolution quantifiée
    for (size_t stepIdx = 0; stepIdx < totalSteps; stepIdx += quantUnitSteps) {
        // Le `quantUnitSteps` ici représente l'intervalle entre les pas actifs.
        // Exemple: si quantUnitSteps est 4 (pour 1/4), on active 0, 4, 8, 12.
        if (!currentBar[soundIndex][stepIdx]) { // Si le step n'est PAS actif
            currentBar[soundIndex][stepIdx] = true; // L'activer
            changed = true;
        }
    }

    return changed;
}
//----------------------------------------

bool DrumPlayer::quantizeStepsFromSound() {
    std::cout << "\nDEBUG: Début de la quantification du dernier son joué avec résolution : " << quantPlayReso_ << std::endl;
    if (!isValidForSoundOperation("quantizeStepsFromSound")) { // Ou "quantizeStepsFromSound"
        return false;
    }

    int soundIndex = lastSoundIndex_;

    // Récupérer la mesure courante du pattern
    size_t currentBarIdx = curPattern_->getCurrentBar();
    if (currentBarIdx >= curPattern_->getNumBars()) {
         std::cerr << "Erreur interne: Mesure courante invalide dans DrumPlayer::quantizeStepsFromSound." << std::endl;
         return false;
    }

    // --- Logique de détermination de quantUnitSteps basée sur quantizePlayedSteps ---
    size_t quantUnitSteps = 0;
    auto it = quantResolutionMap.find(quantPlayReso_);

    if (it != quantResolutionMap.end()) {
        quantUnitSteps = it->second;
    } else {
        std::cerr << "AVERTISSEMENT: Résolution de quantification inconnue pour le dernier son : " << quantPlayReso_ << ". Aucune quantification effectuée." << std::endl;
        return false; // Pas de quantification si la résolution est inconnue
    }

    if (quantUnitSteps == 0) {
        std::cout << "DEBUG: Quantification désactivée pour le dernier son (quantUnitSteps = 0). Aucune modification." << std::endl;
        return false; // Si la quantification est "désactivée" ou mal configurée
    }
    // --- Fin de la logique de détermination de quantUnitSteps ---

    // Obtenir la référence à la barre de pattern actuelle pour la modifier
    auto& currentBar = curPattern_->getPatternBar(currentBarIdx);
    size_t numStepsInCurPattern = curPattern_->getNumSteps(); // Nombre de pas par mesure (ex: 16)

    bool changed = false;
    
    // Pour éviter des problèmes lors de la modification de la liste pendant l'itération,
    // on va collecter les pas actifs, puis les désactiver, puis activer les pas quantifiés.
    std::vector<size_t> stepsToQuantize;

    // Collecter tous les pas actifs pour le 'soundIndex' dans la mesure courante
    for (size_t step = 0; step < numStepsInCurPattern; ++step) {
        if (currentBar[soundIndex][step]) {
            stepsToQuantize.push_back(step);
            // On ne désactive pas encore, on le fera après la collecte.
        }
    }

    // Si aucun pas n'est actif pour ce son, rien à quantifier.
    if (stepsToQuantize.empty()) {
        std::cout << "DEBUG: Aucun pas actif pour le dernier son joué. Aucune quantification nécessaire." << std::endl;
        return false;
    }

    // Désactiver TOUS les pas existants pour ce son dans la mesure courante
    // avant de réactiver les pas quantifiés.
    for (size_t stepIdx = 0; stepIdx < numStepsInCurPattern; ++stepIdx) {
        if (currentBar[soundIndex][stepIdx]) {
            currentBar[soundIndex][stepIdx] = false;
            changed = true; // Marque comme changé si un pas a été désactivé
        }
    }

    // Activer les nouveaux pas quantifiés
    for (size_t initialSourceStep : stepsToQuantize) {
        // --- Logique de quantification, identique à quantizePlayedSteps ---
        size_t quantGridStartStep = (initialSourceStep / quantUnitSteps) * quantUnitSteps;
        size_t halfQuantUnitSteps = quantUnitSteps / 2;
        size_t positionInQuantUnit = initialSourceStep - quantGridStartStep;

        size_t quantizedTargetStep;

        if (positionInQuantUnit >= halfQuantUnitSteps) {
            // Arrondi au début de l'unité de quantification suivante, si plus proche
            quantizedTargetStep = quantGridStartStep + quantUnitSteps;
            if (quantizedTargetStep >= numStepsInCurPattern) {
                // Gérer le cas où le pas quantifié dépasse la fin de la mesure
                quantizedTargetStep = 0; // Ou le premier pas de la mesure si c'est ce que vous voulez
            }
        } else {
            // Arrondi au début de l'unité de quantification actuelle
            quantizedTargetStep = quantGridStartStep;
        }
        // --- Fin de la logique de quantification ---

        // Si le pas quantifié n'est pas déjà actif, l'activer
        if (!currentBar[soundIndex][quantizedTargetStep]) {
            currentBar[soundIndex][quantizedTargetStep] = true;
            changed = true; // Marque comme changé si un nouveau pas a été activé
            std::cout << "DEBUG: Son " << soundIndex << " au pas " << initialSourceStep << " quantifié au pas " << quantizedTargetStep << std::endl;
        }
    }

    std::cout << "DEBUG: Quantification des pas du dernier son terminée. Pattern mis à jour." << std::endl;
    return changed;
}
//----------------------------------------

//==== End of class DrumPlayer ====

} // namespace adikdrum
