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
      lastSoundIndex_(0),
      numSounds_(numSounds),
      lastUpdateTime_(std::chrono::high_resolution_clock::now()), // Initialisation de lastUpdateTime_
      lastKeyPressTime_(std::chrono::high_resolution_clock::now()), // *** AJOUTEZ CETTE LIGNE ***
      stepsPerBeat_(4.0) // Initialisation de stepsPerBeat_ (4 pour des 16èmes)

{
    setBpm(bpm_);
    std::cout << "DrumPlayer::Constructor - numSteps_: " << numSteps_ << std::endl;
    // Création d'un objet AdikPattern avec 2 barres
    curPattern_ = std::make_shared<AdikPattern>(2);
    patData_ = curPattern_->getPatData();
    // lastUpdateTime_ = std::chrono::high_resolution_clock::now(); // Initialisation

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

            currentBar_ = curPattern_->getCurrentBar();
            numTotalBars_ = curPattern_->getBar();
            numSteps_ = curPattern_->getBarLength(currentBar_);

            // Jouer les sons du pas actuel (cette partie reste inchangée)
            auto& currentBarData = curPattern_->getPatData()[currentBar_];
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
            }

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
            numTotalBars_ = curPattern_->getBar();
            // Récupère le nombre de pas dans la barre actuelle
            numSteps_ = curPattern_->getBarLength(currentBar_); // Votre variable numSteps_ est mise à jour ici

            // Pour chaque son dans la barre actuelle, vérifie si la note est active à l'étape courante
            // Utilisation de la référence locale pour la lisibilité
            auto& currentBarData = curPattern_->getPatData()[currentBar_];
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
            numTotalBars_ = curPattern_->getBar();
            // Récupère le nombre de pas dans la barre actuelle
            numSteps_ = curPattern_->getBarLength(currentBar_); // Votre variable numSteps_ est mise à jour ici

            // Pour chaque son dans la barre actuelle, vérifie si la note est active à l'étape courante
            for (size_t i = 0; i < curPattern_->getPatData()[currentBar_].size(); ++i) {
                // Si la note est active à l'étape actuelle (currentStep est un membre de DrumPlayer)
                if (currentStep_ < curPattern_->getPatData()[currentBar_][i].size() && // Vérification de la limite de currentStep
                    curPattern_->getPatData()[currentBar_][i][currentStep_]) {
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
        currentBar < curPattern_->getBar()) { // Ajout de la vérification de la barre

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
    size_t totalBars = curPattern_->getBar(); // Assurez-vous que getBar() retourne le nombre total de barres
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

void DrumPlayer::addPendingRecording(int soundIndex, size_t barIndex, size_t stepIndex) {
    // Il est important de s'assurer que curPattern_ est valide ici
    // et que les indices sont dans les limites.
    if (!curPattern_ || soundIndex < 0 || static_cast<size_t>(soundIndex) >= numSounds_ ||
        barIndex >= curPattern_->getBar() || stepIndex >= curPattern_->getNumSteps()) {
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

        if (barIndex < curPattern_->getBar() && stepIndex < curPattern_->getNumSteps() &&
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
size_t DrumPlayer::quantizeStep(size_t currentStep, std::chrono::high_resolution_clock::time_point keyPressTime) {
    // Calcul de la durée d'un pas en secondes
    // bpm_ (battements par minute)
    // stepsPerBeat_ (subdivisions par battement, ex: 4 pour des 16èmes)
    // numSteps_ (pas par mesure) n'est pas directement utilisé ici pour la durée d'UN pas,
    // mais bpm / (stepsPerBeat * 4) donnerait les 16èmes de notes.
    // Il faut que "stepsPerBeat_" reflète la granularité de ton 'step'.
    // Si 16 steps/measure, et 4 beats/measure, alors 4 steps/beat.
    // Ta variable stepsPerBeat_ est déjà 4.0, c'est bon.
    double secondsPerStep = (60.0 / bpm_) / stepsPerBeat_; // Durée d'une 16ème note en secondes

    // Calcul du décalage de la frappe par rapport au DÉBUT du currentStep_
    std::chrono::duration<double> latency = keyPressTime - lastUpdateTime_; // 'double' pour avoir des secondes

    // Convertir en millisecondes pour un affichage plus lisible
    double latencyMs = latency.count() * 1000.0;
    double stepDurationMs = secondsPerStep * 1000.0;

    std::cout << "DEBUG: Key press at latency: " << latencyMs << " ms relative to step " << currentStep << " start." << std::endl;
    std::cout << "DEBUG: Each step lasts " << stepDurationMs << " ms." << std::endl;

    size_t quantizedStep = currentStep;

    // Logique de quantification au pas le plus proche
    // Si la frappe est au-delà de la moitié de la durée du pas, on quantifie au pas suivant.
    if (latency.count() > (secondsPerStep / 2.0)) {
        quantizedStep = (currentStep + 1);
        // Gérer le wrap-around à la fin de la mesure
        if (quantizedStep >= numSteps_) { // numSteps_ est le nombre de pas PAR MESURE (ex: 16)
            quantizedStep = 0; // Revient au début de la mesure suivante
        }
        std::cout << "DEBUG: Quantizing to next step: " << quantizedStep << std::endl;
    } else {
        // La frappe est avant ou jusqu'à la moitié du pas actuel, on garde le pas actuel.
        std::cout << "DEBUG: Quantizing to current step: " << quantizedStep << std::endl;
    }

    // Gérer les cas où la frappe est significativement en avance (latence négative).
    // Si la frappe est plus proche du pas précédent, on peut quantizer au pas précédent.
    // Cela dépend de la "fenêtre de quantification" que tu souhaites.
    // Pour l'instant, on se base sur le pas actuel et le suivant.
    // Si latency est négative, elle signifie que la frappe a eu lieu AVANT le début du `currentStep_`.
    // Pour des raisons de simplicité et si la latence négative n'est pas trop grande,
    // on peut la laisser sur le `currentStep` si elle n'atteint pas le seuil de "demi-pas avant".
    // Si `latency.count() < -(secondsPerStep / 2.0)`, cela voudrait dire "plus proche du pas précédent".
    // On peut l'ajouter si tu vois encore des décalages bizarres.
    // Pour l'instant, la logique actuelle considère `currentStep` si `latency` est négative ou juste un peu positive.


    return quantizedStep;
}



/*
size_t DrumPlayer::quantizeStep(size_t currentStep, std::chrono::high_resolution_clock::time_point keyPressTime) {
    // Mesurer le décalage (pour le diagnostic)
    // lastUpdateTime_ contient le moment où le 'currentStep_' actuel a été initié ou mis à jour.
    // std::cout << "voici, keyPressTime: " << keyPressTime << ", lastUpdateTime: " << lastUpdateTime_ << std::endl;
    std::chrono::duration<double> latency = keyPressTime - lastUpdateTime_; // Décalage de la frappe par rapport au début du pas
    double secondsPerStep = 60.0 / bpm_ / stepsPerBeat_; // Durée d'un pas en secondes

    // Pour le diagnostic, affichons le décalage
    std::cout << "DEBUG: Key press at latency: " << latency.count() * 1000 << " ms relative to current step: " << currentStep << " start." << std::endl;
    std::cout << "DEBUG: Each step lasts " << secondsPerStep * 1000 << " ms." << std::endl;

    // Logique de quantification (première ébauche : très simple)
    // Si la frappe est plus proche du pas actuel, on garde le pas actuel.
    // Si la frappe est plus proche du pas suivant, on se décale sur le pas suivant.
    // La "moitié de pas" est le seuil.
    if (latency.count() > (secondsPerStep / 2.0)) {
        // La frappe est plus proche du pas suivant
        size_t nextStep = (currentStep + 1) % numSteps_; // numSteps_ doit être la taille totale des pas dans une mesure
        std::cout << "DEBUG: Quantizing to next step: " << nextStep << std::endl;
        return nextStep;
    } else {
        // La frappe est plus proche du pas actuel (ou exactement sur/avant)
        std::cout << "DEBUG: Quantizing to current step: " << currentStep << std::endl;
        return currentStep;
    }
}
//----------------------------------------
*/



/*
// Petite modification pour la lecture du pattern, pour potentiellement intégrer la fusion
// quand on passe au pas suivant ou à la barre suivante
void DrumPlayer::updatePlayback() {
    if (!playing_ || !curPattern_) {
        return;
    }

    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = now - lastUpdateTime_;

    double beatsElapsed = elapsed.count() * (bpm_ / 60.0);
    double stepsElapsed = beatsElapsed * stepsPerBeat_; // ou stepsPerBeat (1 beat = 4 steps pour 16th notes)

    if (stepsElapsed >= 1.0) {
        int stepsToAdvance = static_cast<int>(stepsElapsed);
        for (int i = 0; i < stepsToAdvance; ++i) {
            // *** C'EST ICI QU'ON POURRAIT DÉCLENCHER LA FUSION ***
            // Idée: Fusionner à chaque nouveau pas, ou à chaque nouvelle barre
            // Si on fusionne à chaque nouveau pas, le délai sera minime.
            // Si on fusionne à chaque barre, ça sera moins fréquent mais plus de latence potentielle.

            // Option 1: Fusionner à chaque pas (plus réactif pour l'enregistrement)
            mergePendingRecordings(); // Fusionne ce qui a été enregistré dans le pas précédent ou actuel.

            currentStep_ = (currentStep_ + 1) % numSteps_; // Avance au pas suivant
            if (currentStep_ == 0) {
                // Avance à la barre suivante quand un cycle de pas est terminé
                currentBar_ = (currentBar_ + 1) % curPattern_->getBar();
                // Option 2: Fusionner ici (fin de barre)
                // mergePendingRecordings();
            }

            // Play sounds for the current step
            if (curPattern_) {
                const auto& currentBarPattern = curPattern_->getPatternBar(currentBar_);
                for (size_t soundIdx = 0; soundIdx < numSounds_; ++soundIdx) {
                    if (currentBarPattern[soundIdx][currentStep_]) {
                        // Play the sound
                        playSound(soundIdx);
                    }
                }
            }
        }
        lastUpdateTime_ = now;
    }
}
*/

//==== End of class DrumPlayer ====

} // namespace adikdrum
