#include "drumplayer.h"
#include "audiosound.h"
#include "audiomixer.h"
#include "adikpattern.h"

#include <cmath>
#include <vector>
#include <iostream>
#include <memory>
#include <algorithm> // pour std::clamp
namespace adikdrum {

DrumPlayer::DrumPlayer(int numSounds, int numSteps)
    : currentStep(0),
      clickStep(0),
      // pattern_(numSounds, std::vector<bool>(numSteps, false)),
      numSteps_(numSteps),
      sampleRate_(44100),
      playing_(false),
      clicking_(false),
      bpm_(100),
      beatCounter_(0),
      mixer_(nullptr), // Initialiser à nullptr
      isMuted_(numSounds, false), // Initialiser tous les sons comme non mutés
      lastSoundIndex_(0),
      numSounds_(numSounds)

{
    setBpm(bpm_);
    std::cout << "DrumPlayer::Constructor - numSteps_: " << numSteps_ << std::endl;
    // Création d'un objet AdikPattern avec 2 barres
    curPattern_ = std::make_shared<AdikPattern>(2);
    patData_ = curPattern_->getPatData();

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
    clicking_ = false;
    currentStep = 0;
    clickStep = 0;
    beatCounter_ = 0;
}
//----------------------------------------


void DrumPlayer::startClick() {
    clicking_ = true;
    if (playing_) {
      clickStep = currentStep;
      beatCounter_ = clickStep % numSteps_;
    } else {
      clickStep =0;
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
        beatCounter_ = currentStep / 4; // Note: Le résultat est une division entière puisque les deux nombres sont des entiers.
    }
    
    if (mixer_) {
        if (beatCounter_ % 4 == 0 && soundClick1_) {
            mixer_->play(0, soundClick1_);
        } else if (soundClick2_) {
            mixer_->play(0, soundClick2_);
        }
        beatCounter_ = (beatCounter_ + 1) % 4;
    }
}
//----------------------------------------

void DrumPlayer::playPattern() {
    if (mixer_ && playing_) {
        // Vérifie si curPattern_ est valide
        if (curPattern_) {
            // Récupère l'index de la barre courante depuis l'objet AdikPattern
            currentBar_ = curPattern_->getCurrentBar();
            // Récupère le nombre total de barres
            numTotalBars_ = curPattern_->getBar();
            // Récupère le nombre de pas dans la barre actuelle
            numSteps_ = curPattern_->getBarLength(currentBar_);

            // Affiche la barre courante (pour le débogage, peut être supprimé)
            // Pour chaque son dans la barre actuelle, vérifie si la note est active à l'étape courante
            for (size_t i = 0; i < curPattern_->getPatData()[currentBar_].size(); ++i) {
                // Si la note est active à l'étape actuelle (currentStep est un membre de DrumPlayer)
                if (currentStep < curPattern_->getPatData()[currentBar_][i].size() && // Vérification de la limite de currentStep
                    curPattern_->getPatData()[currentBar_][i][currentStep]) {
                    if (drumSounds_[i]) {
                        // Jouer le son sur le canal correspondant (i + 1)
                        mixer_->play(i + 1, drumSounds_[i]);
                    }
                }
            }

            // Incrémente le pas courant
            currentStep++;

            // Si le pas courant dépasse la longueur de la barre actuelle
            if (currentStep >= numSteps_) {
                currentStep =0;
                size_t nextBarIndex = currentBar_ + 1; // Passe à la barre suivante

                // Si la barre suivante dépasse le nombre total de barres, revient à la première barre
                if (nextBarIndex >= numTotalBars_) {
                    nextBarIndex = 0; // Boucle vers la première barre
                }
                // Met à jour la barre courante dans l'objet AdikPattern
                curPattern_->setCurrentBar(nextBarIndex);
            }
        } else {
            // Gérer le cas où curPattern_ n'est pas initialisé (par exemple, afficher un message d'erreur)
            std::cerr << "Erreur: curPattern_ n'est pas initialisé dans DrumPlayer::playPattern." << std::endl;
        }
    }
}
//----------------------------------------


/*
void DrumPlayer::playPattern0() {
    if (mixer_ && playing_) {
        for (size_t i = 0; i < pattern_.size(); ++i) { //
            if (pattern_[i][currentStep]) {
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

void DrumPlayer::toggleRecord() {
    recording_ = !recording_;
}
//----------------------------------------

bool DrumPlayer::isRecording() const {
    return recording_;
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
        // size_t currentStep = currentStep_; 

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



//==== End of class DrumPlayer ====

} // namespace adikdrum
